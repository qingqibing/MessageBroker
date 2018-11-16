#pragma once

#include <Windows.h>
#include <string>
#include <iostream>
#include <varargs.h>

class ILog {
public:
	virtual void log_e(const std::string& func_name) = 0;
	virtual void log(const char* format, ...) = 0;
};


//template<typename... Args>
//void log(const std::string& format, const Args&... args) {
//}

class StdLogger : public ILog {
public:

	static StdLogger& me() {
		static StdLogger logger;
		return logger;
	}

	virtual void log_e(const std::string& func_name) {
		std::cout << func_name << " failed with error: " << GetLastError() << std::endl;
	}

	virtual void log(const char* format, ...) {
		va_list args;
		va_start(args, format);

		char buf[1024];
		int len = vsprintf_s(buf, 1024, format, args);
		if (len < 0) {
			log_e("vsprintf_s");
			return;
		}

		buf[len] = '\0';
		std::cout << buf << std::endl;

		va_end(args);
	}
};