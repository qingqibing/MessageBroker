#include "pch.h"
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <stdarg.h>

#include "SockHelper.h"

void SockHelper::LogLastError(const std::string& func_name) {
	std::cout << func_name << " failed with error: " << WSAGetLastError() << std::endl;
}

void SockHelper::PrintError(const char* format, ...) {
	va_list args;
	va_start(args, format);

	PrintError(format, args);
	va_end(args);
}

void SockHelper::PrintError(const char* format, va_list args) {
	char buf[512];
	int len = vsprintf_s(buf, 512, format, args);
	if (len < 0) {
		std::cout << "vsprintf_s failed with error: " << GetLastError() << std::endl;
	}

	std::cout << buf << std::endl;
}

//TODO: use modern c++ way to implement va_args print error

bool SockHelper::is_sock_connected(SOCKET s) {
	int result = 0;
	result = recv(s, NULL, 0, MSG_PEEK);  //BLOCKING!!!
	//result = recv(s, NULL, 0, 0);
	if (result == SOCKET_ERROR) {
		return false;
	}

	return true;
}