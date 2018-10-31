#pragma once

#include <string>
#include <WinSock2.h>
#include "CSocketRWObj.h"

typedef CSocketRWObj::OnReadComplete ReadCallback;

class CSockConnection {
public:
	CSockConnection(SOCKET s, ReadCallback cbRead);
	~CSockConnection();

	const std::string getMessage() const { return m_msg; }
	const SOCKET getSocket() const { return m_sock; }
	void write(char* buf, int len);
	void read();
	void addtoHandles(EventManager& manager);

private:

	SOCKET m_sock;
	std::string m_msg;
	CSocketRWObj m_readSock;
	CSocketRWObj m_writeSock;
};


