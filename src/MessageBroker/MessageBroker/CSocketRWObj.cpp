
#include "pch.h"
#include "CSocketRWObj.h"
#include "SockHelper.h"

CSocketRWObj::CSocketRWObj(SOCKET s, bool isRead, OnReadComplete cbReadComplete, OnWriteComplete cbWriteComplete)
	: m_sock(s), m_buf(nullptr),/*m_recvBuf(nullptr), m_sendBuf(nullptr), */m_isRead(isRead),
	m_cbRead(cbReadComplete), m_cbWrite(cbWriteComplete)
{
	ZeroMemory(&m_overlap, sizeof(m_overlap));
	//if overlapped io use completeroutine, then OVERLAPPED'S hEvent 
	//can be use to pass context infomation
	m_overlap.hEvent = CreateEvent(NULL, false, false, NULL);
	EventManager::getInstance().AddEvent(m_overlap.hEvent);

	//TODO: actualy only need one buffer because of CSocketRWObj instance will
	//only do one job(Read/Write)
	//m_recvBuf = new char[DEFAULT_BUF_SIZE];
	//m_sendBuf = new char[DEFAULT_BUF_SIZE];
	m_buf = new char[DEFAULT_BUF_SIZE];
}

CSocketRWObj::~CSocketRWObj() {
	//CLOSESOCK(m_sock);  //sock can not be closed, because we need this sock to send/write data

	//delete[] m_recvBuf;
	//delete[] m_recvBuf;
	EventManager::getInstance().RemoveEvent(m_overlap.hEvent);
	CloseHandle(m_overlap.hEvent);
	delete[] m_buf;
}

void CSocketRWObj::Read(/*char* pBuf, int len*/) {

	WSABUF dataBuf;
	dataBuf.buf = m_buf;
	dataBuf.len = DEFAULT_BUF_SIZE;
	/*dataBuf.buf = pBuf;
	dataBuf.len = len;*/

	DWORD recvBytes = 0;
	DWORD flags = 0;

	if (SOCKET_ERROR == WSARecv(m_sock,
		&dataBuf,
		1,
		&recvBytes,
		&flags,
		&m_overlap,  //m_overlap's hEvent can be used to pass context infomation
		completeRoutine
	)) {
		if (WSA_IO_PENDING != WSAGetLastError()) {
			SockHelper::PrintError("WSARecv");
			std::cout << "close sock: " << m_sock << std::endl;
			CLOSESOCK(m_sock);
			return;
		}
	}

}

void CSocketRWObj::Write(const char* pBuf, int len) {
	WSABUF dataBuf;
	dataBuf.buf = const_cast<char*>(pBuf);
	dataBuf.len = len;

	DWORD bytes = 0;
	//DWORD flags = 0;
	std::cout << "ready to send to client: " << m_sock << " , data: "<<pBuf << std::endl;

	if (SOCKET_ERROR == WSASend(m_sock,
		&dataBuf,
		1,
		&bytes,
		0,
		&m_overlap,
		completeRoutine)) {
		if (WSA_IO_PENDING != WSAGetLastError()) {
			SockHelper::PrintError("WSASend");
			std::cout << "close sock: " << m_sock << std::endl;
			CLOSESOCK(m_sock);
			return;
		}
	}


}

void CSocketRWObj::Complete() {
	DWORD bytes = 0;
	DWORD flags = 0;

	if (!WSAGetOverlappedResult(m_sock,
		&m_overlap,
		&bytes,
		false,
		&flags
	)) {
		if (WSA_IO_PENDING != WSAGetLastError()) {
			SockHelper::PrintError("WSAGetOverlappedResult");
			std::cout << "close sock: " << m_sock << std::endl;
			if (SOCKET_ERROR == shutdown(m_sock, SD_RECEIVE)) {
				SockHelper::PrintError("shutdown");
			}
			CLOSESOCK(m_sock);
			return;
		}
	}


	if (m_isRead) {
		if (bytes > 0) {
			m_buf[bytes] = '\0';  //TODO: if bytes == recvBuf size??
			std::cout << "recved: " << bytes << " bytes.\n" << m_buf << std::endl;
			if (m_cbRead != nullptr) {
				char temp[DEFAULT_BUF_SIZE];
				memcpy_s(temp, DEFAULT_BUF_SIZE, m_buf, bytes + 1);  //copy last '\0'
				m_cbRead(m_sock, temp, bytes);
			}
		}
		//else {
		//	//bytes == 0
		//	if (SOCKET_ERROR == shutdown(m_sock, SD_RECEIVE)) {
		//		SockHelper::PrintError("shutdown");
		//		CLOSESOCK(m_sock);
		//		return;
		//	}
		//}
	}
	else {
		std::cout << "sent: " << bytes << " bytes.\n" << std::endl;
		if (bytes > 0) {
			if (m_cbWrite != nullptr) {
				m_cbWrite(bytes);
			}
		}
		//else {
		//	//bytes == 0
		//	if (SOCKET_ERROR == shutdown(m_sock, SD_SEND)) {
		//		SockHelper::PrintError("shutdown");
		//		CLOSESOCK(m_sock);
		//		return;
		//	}
		//}
	}

}

//completeRoutine is a static method
void CSocketRWObj::completeRoutine(DWORD dwError,
	DWORD cbTransferred,
	LPWSAOVERLAPPED lpOverlapped,
	DWORD dwFlags) {

	//TODO: seems that i have some misunderstanding about passing context info in completionRoutine
	//CSocketRWObj* sockObj = reinterpret_cast<CSocketRWObj*>(lpOverlapped->hEvent);
	CSocketRWObj* sockObj = reinterpret_cast<CSocketRWObj*>((char*)lpOverlapped - (char*)(&((CSocketRWObj*)0)->m_overlap));
	sockObj->Complete();
}

bool CSocketRWObj::Wait() {
	DWORD cnt = 1;

	//Call WSAWaitForMultipleEvents to put Thread to an alertable state, or the completionRoutine
	//will not be invoked

	//NOTE that: the hEvent is not in signaled state, It's the completionRoutine that end
	//the wait state
	//msdn : https://docs.microsoft.com/en-us/windows/desktop/api/winsock2/nf-winsock2-wsawaitformultipleevents
	DWORD index = WSAWaitForMultipleEvents(cnt, &(m_overlap.hEvent), true, INFINITE, true);
	if (index >= WAIT_OBJECT_0 && index < WAIT_OBJECT_0 +cnt) {
		//m_overlap.hEvent = this;
		return true;
	}
	else if (index == WAIT_IO_COMPLETION) {
		/*
		The wait was ended by one or more I / O completion routines that were executed.
		The event that was being waited on is not signaled yet.The application must call
		the WSAWaitForMultipleEvents function again.This return value can only be returned
		if the fAlertable parameter is TRUE.
		*/
		return true;
	}
	else if (index == WAIT_FAILED) {
		std::cout << "WSAWaitForMultipleEvents failed with error: " << GetLastError() << std::endl;
		return false;
	}
	else if (index == WAIT_TIMEOUT) {
		//can not happen here, because we use INFINITE
	}

	return false;
}
