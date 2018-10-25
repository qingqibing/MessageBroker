#pragma once

#include "pch.h"
#include "CSocketCtrl.h"
#include <MSWSock.h>

typedef void(*OnNewClient)(void);
//implement the basic socketserver
//without business logic in it
class CSocketServer
{
public:
	CSocketServer(const std::string& address, const int port, OnNewClient newclientCallback);
	~CSocketServer();

	bool StartListen();
	bool Shutdown();

private:
	const std::string& m_addr;
	const int m_port;
	OnNewClient m_newclientCallback;

	SOCKET m_sockListen;
	SOCKET m_sockAccept;
	LPFN_ACCEPTEX lpfnAccessEx;
	OVERLAPPED m_overlap;

	void log_e(const char* format, ...);
};