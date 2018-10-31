#pragma once

#include <iostream>
#include <string>

#define CLOSESOCK(s) if(INVALID_SOCKET != s){closesocket(s); s=INVALID_SOCKET;}

class SockHelper
{
public:
	static void PrintError(const std::string& func_name);
	static void PrintError(const char* format, ...);
	static void PrintError(const char* format, va_list args);
	static bool is_sock_connected(SOCKET s);
};
