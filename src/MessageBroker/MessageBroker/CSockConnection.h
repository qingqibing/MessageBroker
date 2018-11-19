#pragma once

#include <string>
#include <WinSock2.h>
#include "CSockObj.h"
#include "SockHelper.h"

class CSockConnection {
public:
	CSockConnection(SOCKET s, OnError cbError, OnReadComplete cbRead, OnWriteComplete cbWrite);
	~CSockConnection();

	const std::string getMessage() const { return m_msg; }
	const SOCKET getSocket() const { return m_sock; }

	//todo: change to PostReadRequst, PostWriteRequst
	void write(char* buf, int len);
	void read();

	void SetSockStatus(bool sockWrong) { m_sockWrong = sockWrong; }
	bool GetSockStatus() const{ return m_sockWrong; }

private:

	SOCKET m_sock;
	std::string m_msg;
	CSockReadObj m_readSock;
	CSockWriteObj m_writeSock;
	bool m_sockWrong;
};


