#pragma once

#include "pch.h"
#include "CSocketCtrl.h"
#include <MSWSock.h>
#include "EventManager.h"
#include "AsyncObj.h"

typedef void(*OnNewClient)(SOCKET s);
//implement the basic socketserver
//without business logic in it
class CSocketServer : public EventObj
{
public:
	CSocketServer(const std::string& address, const int port, OnNewClient newclientCallback);
	~CSocketServer();

	CSocketServer(const CSocketServer& rhs) = delete;
	CSocketServer& operator=(const CSocketServer& rhs) = delete;

	//accept new connection
	bool StartListen();
	bool Shutdown();
	void OnComplete();
	SOCKET GetAcceptSock();

private:
	const std::string& m_addr;
	const int m_port;
	OnNewClient m_newclientCallback;

	SOCKET m_sockListen;
	SOCKET m_sockAccept;
	LPFN_ACCEPTEX lpfnAccessEx;

	void log_e(const char* format, ...);
	void completionRoutine(DWORD dwError, DWORD cbTransferred, LPOVERLAPPED lpOverlapped, DWORD dwFlags);
};