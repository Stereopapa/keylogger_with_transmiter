#pragma once
#ifndef CURL_STATICLIB
#define CURL_STATICLIB
#endif
#include <curl/curl.h>
#include <cpr/cpr.h>

#include <string>
#include <mutex>
#include <thread>
#include <vector>

#include "log_node.h"


class TransmiterTests;

class TransmiterHTTP
{
public:
	TransmiterHTTP(std::vector<std::unique_ptr<LogNode>>& log_nodes,
		LogNode& active_node_observator, std::mutex& mutex);

	bool start();
	void stop();
	void set_conf(std::string url, int retries = 100, int frequency = 10);
	cpr::Response send_post_text(const std::string& data, const std::string& url);
	cpr::Response send_post_json(const std::string& data, const std::string & url);

private:
	friend class TransmiterTests;

	void transmit_();

	//thread seciurity
	bool swap_buffers_(std::vector<std::unique_ptr<LogNode>>& temp_buffer);
	void run_();

	//winapi uitils
	void save_machine_guid_();
	void save_username_();
	//json making, packs data buffer, saved mac address and saved ip address
	std::string pack_data_to_json_(std::vector<std::unique_ptr<LogNode>>& data_buffer);
	std::string escape_json_(std::string s);
private:
	std::string url_ = "";
	std::string machine_guid_;
	std::string username_;
	int transmit_frequency_ = 10;
	int retries_ = 100;

	std::thread transmiter_thread_;
	std::string convert_to_utf8(wchar_t* data);

	//zasoby wspó³dzielone
	std::mutex& mutex_;
	std::vector <std::unique_ptr<LogNode>>& log_nodes_;
	LogNode& active_node;

	//state fields
	bool running_ = false;
};

