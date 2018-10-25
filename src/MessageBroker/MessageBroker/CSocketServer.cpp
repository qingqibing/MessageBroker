
#include "pch.h"
#include "CSocketCtrl.h"
#include "CSocketServer.h"
#include "SockHelper.h"
#include <Mswsock.h>
#include <stdarg.h>

CSocketServer::CSocketServer(const std::string& addr, const int port, OnNewClient newclientCallback)
	: m_addr(addr), m_port(port), m_newclientCallback(newclientCallback), m_sockListen(INVALID_SOCKET),
	m_sockAccept(INVALID_SOCKET), lpfnAccessEx(NULL)
{
	ZeroMemory(&m_overlap, sizeof(m_overlap));

	m_sockListen = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_sockListen == INVALID_SOCKET) {
		log_e("open listen socket failed");
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
		&m_overlap  //get overlap structure
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

bool CSocketServer::Shutdown() {
	if (m_sockListen == INVALID_SOCKET) {
		return false;
	}

	if (m_sockAccept == INVALID_SOCKET) {
		return false;
	}

	if (SOCKET_ERROR == shutdown(m_sockAccept, SD_SEND)) {
		log_e("shutdown failed with error: %d", WSAGetLastError());
		CLOSESOCK(m_sockAccept);
	}

	//TODO: do some receive then close socket
	/*do {

	} while(result >0)*/

	CLOSESOCK(m_sockAccept);
	return true;
}

void CSocketServer::log_e(const char* format, ...) {
	va_list args;
	va_start(args, format);

	SockHelper::PrintError(format, args);
	va_end(args);
}