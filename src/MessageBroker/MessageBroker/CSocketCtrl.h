#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

class CSocketCtrl
{
private:
	CSocketCtrl();
	static int error_code;

public:
	~CSocketCtrl();

	CSocketCtrl(const CSocketCtrl& rhs) = delete;
	CSocketCtrl& operator=(const CSocketCtrl& rhs) = delete;

	static int Startup();
};

