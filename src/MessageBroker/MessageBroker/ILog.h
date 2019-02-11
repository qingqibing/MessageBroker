#pragma once

#include <Windows.h>
#include <string>
#include <iostream>
#include <varargs.h>
#include "QPCTimestamp.h"

#define MAX_LOG_SIZE 10240

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

		char buf[MAX_LOG_SIZE];
		SYSTEMTIME time;  //system time
		GetLocalTime(&time);
		int len = sprintf_s(buf, "[%04d/%02d/%02d %02d:%02d:%02d %03d]   ", time.wYear, time.wMonth, time.wDay,
			time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

		len += vsprintf_s(buf + len, MAX_LOG_SIZE - len, format, args);
		if (len < 0) {
			log_e("vsprintf_s");
			return;
		}

		buf[len] = '\0';
		std::cout << buf << std::endl;

		va_end(args);
	}
};