#pragma once

#include <string>
#include <WinSock2.h>
#include "CSockObj.h"
#include "SockHelper.h"

class CSockConnection {
public:
	CSockConnection(SOCKET s, OnError cbError, OnReadComplete cbRead, OnWriteComplete cbWrite);
	~CSockConnection();

	CSockConnection(const CSockConnection&) = delete;
	CSockConnection& operator=(const CSockConnection&) = delete;

	const std::string GetMessage() const { return m_msg; }
	const SOCKET GetSocket() const { return m_sock; }

	//todo: change to PostReadRequst, PostWriteRequst
	void PostWriteRequest(char* buf, int len);
	void PostReadRequest();

	void SetSockStatus(bool sockWrong) { m_sockWrong = sockWrong; }
	bool GetSockStatus() const{ return m_sockWrong; }

private:

	SOCKET m_sock;
	std::string m_msg;
	CSockReadObj m_readSock;
	CSockWriteObj m_writeSock;
	bool m_sockWrong;
};


