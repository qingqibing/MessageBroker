
#include "pch.h"
#include "CSocketCtrl.h"
#include "CSocketServer.h"
#include "SockHelper.h"
#include <Mswsock.h>
#include <stdarg.h>
#include <vector>
#include <cassert>
#include "IOCPManager.h"
#include "ILog.h"

CSocketServer::CSocketServer(const std::string& addr, const int port, OnNewClient newclientCallback)
	: m_addr(addr), m_port(port), m_newclientCallback(newclientCallback), m_sockListen(INVALID_SOCKET),
	m_sockAccept(INVALID_SOCKET), lpfnAccessEx(NULL)
{
	m_sockListen = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_sockListen == INVALID_SOCKET) {
		LOG_E("WSASocket");
		return;
	}

	BOOL r =IocpManager::Self().AssiociateIocp((HANDLE)m_sockListen, CK_SOCK_NEW);
	if (!r) {
		assert(0);
	}

	/*
	Keep-alive packets MUST only be sent when no data or
	acknowledgement packets have been received for the
	connection within an interval.  This interval MUST be
	configurable and MUST default to no less than two hours.

	It is extremely important to remember that ACK segments that
	contain no data are not reliably transmitted by TCP.
	Consequently, if a keep-alive mechanism is implemented it
	MUST NOT interpret failure to respond to any specific probe
	as a dead connection.
	*/
	int opt = 1;
	if (setsockopt(m_sockListen, SOL_SOCKET, SO_KEEPALIVE, (const char*)&opt, sizeof(opt))
		!= 0) {
		LOG_E("setsockopt");
		CLOSESOCK(m_sockListen);
		return;
	}


	/*
	//msdn: https://docs.microsoft.com/en-us/windows/desktop/api/Winsock2/nf-winsock2-wsaioctl

	The function pointer for the AcceptEx function must be obtained at run time
	by making a call to the WSAIoctl function with the SIO_GET_EXTENSION_FUNCTION_POINTER
	opcode specified. The input buffer passed to the WSAIoctl function must contain 
	WSAID_ACCEPTEX, a globally unique identifier (GUID) whose value identifies the 
	AcceptEx extension function. On success, the output returned by the WSAIoctl 
	function contains a pointer to the AcceptEx function. The WSAID_ACCEPTEX GUID 
	is defined in the Mswsock.h header file.
	*/

	GUID acceptex_guid = WSAID_ACCEPTEX; //AcceptEx GUID
	DWORD dwBytes = 0;

	//call WSAIoctl to get a lpfnAccessEx pointer
	if (SOCKET_ERROR == WSAIoctl(m_sockListen,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&acceptex_guid,
		sizeof(acceptex_guid),
		&lpfnAccessEx,
		sizeof(lpfnAccessEx),
		&dwBytes,
		NULL,
		NULL
	)) {
		LOG_E("WSAIoctl");
		CLOSESOCK(m_sockListen);
		return;
	}

	sockaddr_in service;
	service.sin_family = AF_INET;
	//service.sin_addr.s_addr = inet_addr(m_addr.c_str());
	inet_pton(AF_INET, m_addr.c_str(), &(service.sin_addr.s_addr));
	service.sin_port = htons(m_port);

	if (SOCKET_ERROR == bind(m_sockListen,
		(SOCKADDR*)&service,
		sizeof(service))) {
		LOG_E("bind");
		CLOSESOCK(m_sockListen);
		return;
	}

	if (SOCKET_ERROR == listen(m_sockListen, 100)) {
		LOG_E("SOCK listen");
		CLOSESOCK(m_sockListen);
		return;
	}
}

CSocketServer::~CSocketServer() {
	if (m_sockListen != INVALID_SOCKET) {
		CLOSESOCK(m_sockListen);
	}
	if (m_sockAccept != INVALID_SOCKET) {
		CLOSESOCK(m_sockAccept);
	}

}

bool CSocketServer::StartListen() {
	
	if (m_sockListen == INVALID_SOCKET) {
		return false;
	}

	if (m_sockAccept != INVALID_SOCKET) {
		return false;
	}

	//prepare a socket
	m_sockAccept = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_sockAccept == INVALID_SOCKET) {
		LOG_E("WSASocket");
		return false;
	}

	char buf[1024];
	DWORD dwBytes = 0;
	//call AcceptEx API
	//msdn : https://docs.microsoft.com/en-us/windows/desktop/api/mswsock/nf-mswsock-acceptex
	if (!lpfnAccessEx(m_sockListen,
		m_sockAccept,
		buf,  //buf normally contains 3 parts:the first block of data sent on a new connection, the local address of the server, and the remote address of the client.
		0, //read nothing(0 byte) from client, and will imediately return when accept a new connection
		sizeof(SOCKADDR_STORAGE) + 16,
		sizeof(SOCKADDR_STORAGE) + 16,
		&dwBytes,
		&m_overlap  //get overlap structure, OVERLAPPED' hEvent is signaled when new connection
	)) {
		if (WSA_IO_PENDING != WSAGetLastError()) {
			LOG_E("AcceptEx");
			CLOSESOCK(m_sockListen);
			CLOSESOCK(m_sockAccept);
			return false;
		}
	}

	return true;
}


void CSocketServer::completionRoutine(DWORD dwError, DWORD cbTransferred, LPOVERLAPPED lpOverlapped, DWORD dwFlags) {

}

bool CSocketServer::Shutdown() {
	if (m_sockListen == INVALID_SOCKET) {
		return false;
	}

	closesocket(m_sockListen);
	return true;
}


SOCKET CSocketServer::GetAcceptSock(){
	if (m_sockAccept == INVALID_SOCKET)
		assert(0);

	SOCKET temp = m_sockAccept;
	m_sockAccept = INVALID_SOCKET;
	return temp;
}
