#pragma once

#include <string>
#include <WinSock2.h>
#include "CSockObj.h"

typedef CSockReadObj::OnReadComplete ReadCallback;
typedef CSockObj::OnError ErrorCallback;
typedef CSockWriteObj::OnWriteComplete WriteCallback;

class CSockConnection {
public:
	CSockConnection(SOCKET s, ErrorCallback cbError, ReadCallback cbRead, WriteCallback cbWrite);
	~CSockConnection();

	const std::string getMessage() const { return m_msg; }
	const SOCKET getSocket() const { return m_sock; }

	//todo: change to PostReadRequst, PostWriteRequst
	void write(char* buf, int len);
	void read();

private:

	SOCKET m_sock;
	std::string m_msg;
	CSockReadObj m_readSock;
	CSockWriteObj m_writeSock;
};


