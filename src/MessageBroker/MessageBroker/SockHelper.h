#pragma once

#include <iostream>
#include <string>

#define CLOSESOCK(s) if(INVALID_SOCKET != s){closesocket(s); s=INVALID_SOCKET;}

using OnError = void(*)(const SOCKET s);
using OnReadComplete = void(*)(const SOCKET s, char* data, int len);
using OnWriteComplete = void(*)(size_t bytes);


class SockHelper
{
public:
	static void LogLastError(const std::string& func_name);
	static void PrintError(const char* format, ...);
	static void PrintError(const char* format, va_list args);
	static bool is_sock_connected(SOCKET s);
};
