
#include "pch.h"
#include "CSockConnection.h"
#include <iostream>
#include <assert.h>
#include "IOCPManager.h"

//todo: consider use virtual method to expose event handler?
CSockConnection::CSockConnection(SOCKET s, OnError cbError, OnReadComplete cbRead, OnWriteComplete cbWrite)
	: m_sock(s), m_readSock(s, cbError, cbRead), m_writeSock(s, cbError, cbWrite), m_sockWrong(false)
{
	BOOL r =IocpManager::Self().AssiociateIocp((HANDLE)s, CK_SOCK_COM);  //assoricate with IOCP
	if (!r) {
		assert(0);
	}

	m_readSock.SetOwner(this);
	m_writeSock.SetOwner(this);
	m_readSock.Read();
}

CSockConnection::~CSockConnection() {
	std::cout << "CSockConnection dtor!";
	//assert(0);
}

void CSockConnection::PostWriteRequest(char* buf, int len) {
	m_writeSock.Write(buf, len);
}

void CSockConnection::PostReadRequest() {
	m_readSock.Read();
}

