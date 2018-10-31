
#include "pch.h"
#include "CSockConnection.h"
#include <iostream>
#include <assert.h>

CSockConnection::CSockConnection(SOCKET s, ReadCallback cbRead) : m_sock(s),
	m_readSock(s, true, cbRead, nullptr), m_writeSock(s, false, nullptr, nullptr)
{
	m_readSock.Read();
}

CSockConnection::~CSockConnection() {
	std::cout << "CSockConnection dtor!";
	assert(0);
}

void CSockConnection::write(char* buf, int len) {
	m_writeSock.Write(buf, len);
}

void CSockConnection::read() {
	m_readSock.Read();
}

void CSockConnection::addtoHandles(EventManager& manager) {
	addToEventManager(manager, m_writeSock);
	addToEventManager(manager, m_readSock);
}



