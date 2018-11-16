#pragma once

#include <WinSock2.h>
#include "AsyncObj.h"

#define DEFAULT_BUF_SIZE 1024

class CSockObj : public EventObj {
public:
	typedef void(*OnError)(const SOCKET s);

	CSockObj(SOCKET s, OnError cbError);
	~CSockObj();

	CSockObj(const CSockObj&) = delete;
	CSockObj& operator=(const CSockObj&) = delete;

protected:
	SOCKET m_sock;
	OnError m_cbError;
	char* m_buf;

	static  void CALLBACK completeRoutine(
		DWORD dwError,
		DWORD cbTransferred,
		LPWSAOVERLAPPED lpOverlapped,
		DWORD dwFlags);

	virtual void Complete() = 0;
	void RaiseErrorCallback() const;
};




class CSockReadObj : public CSockObj {
public:
	typedef void(*OnReadComplete)(const SOCKET s, char* data, int len);
	CSockReadObj(SOCKET s, OnError cbError, OnReadComplete cbRead);
	~CSockReadObj();

	void Read();
private:
	OnReadComplete m_cbRead;

	void Complete() override;
};




class CSockWriteObj : public CSockObj {
public:
	typedef void(*OnWriteComplete)(size_t bytes);  //write complete callback
	CSockWriteObj(SOCKET s, OnError cbError, OnWriteComplete cbWrite);
	~CSockWriteObj();

	void Write(const char* buf, int len);

private:
	OnWriteComplete m_cbWrite;

	void Complete() override;

};
