#pragma once

#include <WinSock2.h>
#include <vector>

#include "EventManager.h"

#define DEFAULT_BUF_SIZE 1024

//TODO: devide this clas to ReadObj and WriteObj
class CSocketRWObj
{
	friend void addToEventManager(EventManager& manager, CSocketRWObj& sockObj);
public:
	typedef void(*OnReadComplete)(const SOCKET s, char* data, int len);  //read complete callback
	typedef void(*OnWriteComplete)(size_t bytes);  //write complete callback

	CSocketRWObj(SOCKET s, bool isRead, OnReadComplete cbReadComplete, OnWriteComplete cbWriteComplete);
	~CSocketRWObj();

	CSocketRWObj(const CSocketRWObj&) = delete;
	CSocketRWObj& operator=(const CSocketRWObj&) = delete;

	void Read(/*char* pBuf, int len*/);
	void Write(const char* pBuf, int len);

	/*
	TODO: need improvements
	just for test

	*/
	bool Wait();

private:
	SOCKET m_sock;
	OVERLAPPED m_overlap;

	//char* m_recvBuf;
	//char* m_sendBuf;
	char* m_buf;

	const bool m_isRead;  //indicate whether is used for read or write

	OnReadComplete m_cbRead;
	OnWriteComplete m_cbWrite;

	//LPWSAOVERLAPPED_COMPLETION_ROUTINE m_readCompleteRoutine;
	//LPWSAOVERLAPPED_COMPLETION_ROUTINE m_writeCompleteRoutine;

	static  void CALLBACK completeRoutine(
		DWORD dwError,
		DWORD cbTransferred,
		LPWSAOVERLAPPED lpOverlapped,
		DWORD dwFlags);

	void Complete();

};
