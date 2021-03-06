
#include "pch.h"
#include "CSocketCtrl.h"
#include "CSocketServer.h"
#include "SockHelper.h"
#include <Mswsock.h>
#include <stdarg.h>
#include <vector>
#include <cassert>

CSocketServer::CSocketServer(const std::string& addr, const int port, OnNewClient newclientCallback)
	: m_addr(addr), m_port(port), m_newclientCallback(newclientCallback), m_sockListen(INVALID_SOCKET),
	m_sockAccept(INVALID_SOCKET), lpfnAccessEx(NULL)
{
	//ZeroMemory(&m_overlap, sizeof(m_overlap));
	//m_overlap.hEvent = CreateEvent(NULL, false, false, NULL);  //create a event for OVERLAPPED obj
	//EventManager::getInstance().AddEvent(m_overlap.hEvent);

	m_sockListen = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_sockListen == INVALID_SOCKET) {
		log_e("open listen socket failed");
		return;
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
		log_e("setsockopt SO_KEEPALIVE failed");
		CLOSESOCK(m_sockListen);
		return;
	}

	GUID acceptex_guid = WSAID_ACCEPTEX;
	DWORD dwBytes = 0;

	//msdn: https://docs.microsoft.com/en-us/windows/desktop/api/Winsock2/nf-winsock2-wsaioctl
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
		log_e("failed to obtain lpfnAccessEx pointer");
		CLOSESOCK(m_sockListen);
		return;
	}

	//m_sockAccept = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, )
	sockaddr_in service;
	service.sin_family = AF_INET;
	//service.sin_addr.s_addr = inet_addr(m_addr.c_str());
	inet_pton(AF_INET, m_addr.c_str(), &(service.sin_addr.s_addr));
	service.sin_port = htons(m_port);

	if (SOCKET_ERROR == bind(m_sockListen,
		(SOCKADDR*)&service,
		sizeof(service))) {
		log_e("bind failed with error: %d", WSAGetLastError());
		CLOSESOCK(m_sockListen);
		return;
	}

	if (SOCKET_ERROR == listen(m_sockListen, 100)) {
		log_e("listen failed with error: %d", WSAGetLastError());
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

	//EventManager::getInstance().RemoveEvent(m_overlap.hEvent);
	//CloseHandle(m_overlap.hEvent);
}

bool CSocketServer::StartListen() {
	
	if (m_sockListen == INVALID_SOCKET) {
		return false;
	}

	if (m_sockAccept != INVALID_SOCKET) {
		return false;
	}

	m_sockAccept = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_sockAccept == INVALID_SOCKET) {
		log_e("failed to create accept socket");
		return false;
	}

	char buf[1024];
	DWORD dwBytes = 0;
	//call AcceptEx API
	//msdn : https://docs.microsoft.com/en-us/windows/desktop/api/mswsock/nf-mswsock-acceptex
	if (!lpfnAccessEx(m_sockListen,
		m_sockAccept,
		buf,
		0, //read nothing(0 byte) when new connection accepted
		sizeof(SOCKADDR_STORAGE) + 16,
		sizeof(SOCKADDR_STORAGE) + 16,
		&dwBytes,
		&m_overlap  //get overlap structure, OVERLAPPED' hEvent is signaled when new connection
	)) {
		if (WSA_IO_PENDING != WSAGetLastError()) {
			log_e("AcceptEx failed with error: %d", WSAGetLastError());
			CLOSESOCK(m_sockListen);
			CLOSESOCK(m_sockAccept);
			return false;
		}
	}

	return true;
}

void CSocketServer::OnComplete() {
	DWORD bytes = 0;
	DWORD flags = 0;

	if (!WSAGetOverlappedResult(m_sockListen,
		&m_overlap,
		&bytes,
		true,  // flag that specifies whether the function should wait for the pending overlapped operation to complete. 
		&flags)) {
		if (WSAGetLastError() != WSA_IO_INCOMPLETE) {
			log_e("WSAGetOverlappedResult failed with error: %d", WSAGetLastError());
			return;
		}
	}

	//check m_sockAccept
	char buf[512];
	WSABUF dataBuf;
	dataBuf.buf = buf;
	dataBuf.len = 512;

	if (SOCKET_ERROR == WSARecv(m_sockAccept, &dataBuf, 1, &bytes, &flags, &m_overlap, NULL)) {
		if (WSA_IO_PENDING == WSAGetLastError()) {
			log_e("WSARecv failed with error: %d", WSAGetLastError());
			return;
		}
	}

	DWORD result = WSAWaitForMultipleEvents(1, &m_overlap.hEvent, true, INFINITE, true);
	if (result == WSA_WAIT_FAILED) {
		log_e("WSAWaitForMultipleEvents failed");
		return;
	}

	if (!WSAGetOverlappedResult(m_sockAccept, 
		&m_overlap, 
		&bytes, 
		true,
		&flags))
	{
		log_e("WSAGetOverlappedResult failed");
		return;
	}

	dataBuf.buf[bytes] = '\0';
	log_e("bytes received: %d\n %s", bytes, dataBuf.buf);
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

void CSocketServer::log_e(const char* format, ...) {
	va_list args;
	va_start(args, format);

	SockHelper::PrintError(format, args);
	va_end(args);
}