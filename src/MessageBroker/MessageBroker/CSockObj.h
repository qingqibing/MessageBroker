#pragma once

#include <WinSock2.h>
#include "AsyncObj.h"
#include "SockHelper.h"

#define DEFAULT_BUF_SIZE 1024

class CSockObj : public EventObj {
public:
	CSockObj(SOCKET s, OnError cbError);
	~CSockObj();

	CSockObj(const CSockObj&) = delete;
	CSockObj& operator=(const CSockObj&) = delete;
	void SetOwner(void* pwner) { m_owner = pwner; }

protected:
	SOCKET m_sock;
	OnError m_cbError;
	char* m_buf;
	void* m_owner;

	static  void CALLBACK completeRoutine(
		DWORD dwError,
		DWORD cbTransferred,
		LPWSAOVERLAPPED lpOverlapped,
		DWORD dwFlags);

	virtual bool Complete() = 0;
	void RaiseErrorCallback(SOCKET s) const;
};




class CSockReadObj : public CSockObj {
public:
	CSockReadObj(SOCKET s, OnError cbError, OnReadComplete cbRead);
	~CSockReadObj();

	void Read();
private:
	OnReadComplete m_cbRead;

	bool Complete() override;
};




class CSockWriteObj : public CSockObj {
public:
	CSockWriteObj(SOCKET s, OnError cbError, OnWriteComplete cbWrite);
	~CSockWriteObj();

	void Write(const char* buf, int len);

private:
	OnWriteComplete m_cbWrite;

	bool Complete() override;

};
