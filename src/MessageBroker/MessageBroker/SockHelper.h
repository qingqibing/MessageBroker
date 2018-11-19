#pragma once

#include <iostream>
#include <string>

#define CLOSESOCK(s) if(INVALID_SOCKET != s){closesocket(s); s=INVALID_SOCKET;}
typedef void(*OnError)(const SOCKET s);
typedef void(*OnReadComplete)(const SOCKET s, char* data, int len);
typedef void(*OnWriteComplete)(size_t bytes);  //write complete callback


class SockHelper
{
public:
	static void LogLastError(const std::string& func_name);
	static void PrintError(const char* format, ...);
	static void PrintError(const char* format, va_list args);
	static bool is_sock_connected(SOCKET s);
};
