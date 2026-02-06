#include <string>


typedef struct WindowInfo {
	std::string title;
	std::string process;
	std::string class_name;
	WindowInfo() = default;

	bool operator==(const WindowInfo& other) const {
		return (title == other.title && class_name == other.class_name
			&& process == other.process);
	}
	WindowInfo(std::string title, std::string process_name, std::string window_class) :
		title(title), process(process_name), class_name(window_class){ }
} WindowInfo;

#pragma once
typedef struct LogNode 
{
	WindowInfo window_info;
	std::string time;
	std::string keystrokes;

	LogNode() = default;

	LogNode(WindowInfo window_info, std::string time)
		: window_info(window_info),  time(time),  keystrokes("") {
		keystrokes.reserve(1024);
	}
	LogNode(WindowInfo window_info, std::string time, std::string keystrokes)
		: window_info(window_info), time(time), keystrokes(keystrokes) {
	}

	bool operator==(const LogNode& other) const{
		return (window_info == other.window_info  && time == other.time);
	}

} LogNode;