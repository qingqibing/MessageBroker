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
	const void* GetOwner() const { return m_owner; }
	const SOCKET GetSock() const { return m_sock; }

	bool OnComplete();

protected:
	SOCKET m_sock;
	OnError m_cbError;
	char* m_buf;

	virtual bool Complete() = 0;
	void RaiseErrorCallback(SOCKET s) const;

private:
	void* m_owner;
};




class CSockReadObj : public CSockObj {
public:
	CSockReadObj(SOCKET s, OnError cbError, OnReadComplete cbRead);
	~CSockReadObj();

	BOOL Read();

private:
	OnReadComplete m_cbRead;

	bool Complete() override;
};




class CSockWriteObj : public CSockObj {
public:
	CSockWriteObj(SOCKET s, OnError cbError, OnWriteComplete cbWrite);
	~CSockWriteObj();

	BOOL Write(const char* buf, int len);

private:
	OnWriteComplete m_cbWrite;

	bool Complete() override;

};
