#include "pch.h"
#include <WinSock2.h>
#include "CSockObj.h"
#include "SockHelper.h"
#include "CSockConnection.h"


/*
CSockObj
*/
CSockObj::CSockObj(SOCKET s, OnError cberror) 
: m_sock(s), m_cbError(cberror), m_buf(new char[DEFAULT_BUF_SIZE]){

}

CSockObj::~CSockObj() {
	if (m_buf != NULL)
		delete[] m_buf;
}

//completeRoutine is a static method
void CSockObj::completeRoutine(DWORD dwError,
	DWORD cbTransferred,
	LPWSAOVERLAPPED lpOverlapped,
	DWORD dwFlags) {

	//TODO: seems that i have some misunderstanding about passing context info in completionRoutine
	//CSocketRWObj* sockObj = reinterpret_cast<CSocketRWObj*>(lpOverlapped->hEvent);
	CSockObj* sockObj = reinterpret_cast<CSockObj*>((char*)lpOverlapped - (char*)(&((CSockObj*)0)->m_overlap));
	if (!sockObj->Complete()) {
		((CSockConnection*)(sockObj->m_owner))->SetSockWrong(true);
	}
}


void CSockObj::RaiseErrorCallback(SOCKET s) const{
	if (m_cbError != nullptr) {
		m_cbError(s);
	}
}

bool CSockObj::OnComplete() {
	return Complete();
}


/*
CSockReadObj
*/

CSockReadObj::CSockReadObj(SOCKET s, OnError cberror, OnReadComplete cbRead)
	: CSockObj(s, cberror), m_cbRead(cbRead) {

}

CSockReadObj::~CSockReadObj() {
	std::cout << "CSockReadObj dtor!" << std::endl;
}

void CSockReadObj::Read(/*char* pBuf, int len*/) {

	//todo: multiple buf bennifits?
	WSABUF dataBuf;
	dataBuf.buf = m_buf;
	dataBuf.len = DEFAULT_BUF_SIZE - 1;  // sub 1 because we will use last char to store '\0' terminator
	/*dataBuf.buf = pBuf;
	dataBuf.len = len;*/

	DWORD recvBytes = 0;
	DWORD flags = 0;

	m_overlap.SetContext(this);

	if (SOCKET_ERROR == WSARecv(m_sock,
		&dataBuf,
		1,
		&recvBytes,
		&flags,
		&m_overlap,  //m_overlap's hEvent can be used to pass context infomation
		NULL
	)) {
		if (WSA_IO_PENDING != WSAGetLastError()) {
			SockHelper::LogLastError("WSARecv");
			std::cout << "close sock: " << m_sock << std::endl;
			CLOSESOCK(m_sock);
			return;
		}
	}
}

bool CSockReadObj::Complete() {
	DWORD bytes = 0;
	DWORD flags = 0;

	if (!WSAGetOverlappedResult(m_sock,
		&m_overlap,
		&bytes,
		false,
		&flags
	)) {
		if (WSA_IO_PENDING != WSAGetLastError()) {
			SockHelper::LogLastError("WSAGetOverlappedResult");

			//RaiseErrorCallback(m_sock);

			std::cout << "close sock: " << m_sock << std::endl;
			if (SOCKET_ERROR == shutdown(m_sock, SD_RECEIVE)) {
				SockHelper::LogLastError("shutdown");
			}
			CLOSESOCK(m_sock);
			return false;
		}
	}

	if (bytes > 0) {
		m_buf[bytes] = '\0';
		std::cout << "recved: " << bytes << " bytes.\n" << m_buf << std::endl;
		if (m_cbRead != nullptr) {
			char temp[DEFAULT_BUF_SIZE];
			memcpy_s(temp, DEFAULT_BUF_SIZE, m_buf, bytes + 1);  //copy last '\0'
			m_cbRead(m_sock, temp, bytes);
		}
	}
	return true;
}



/*
CSockWriteObj
*/

CSockWriteObj::CSockWriteObj(SOCKET s, OnError cbError, OnWriteComplete cbWrite)
	: CSockObj(s, cbError), m_cbWrite(cbWrite) {

}

CSockWriteObj::~CSockWriteObj() {
	std::cout << "CSOckWriteObj dtor!" << std::endl;
}

void CSockWriteObj::Write(const char* buf, int len) {
	WSABUF dataBuf;
	dataBuf.buf = const_cast<char*>(buf);
	dataBuf.len = len;

	DWORD bytes = 0;
	//DWORD flags = 0;
	std::cout << "ready to send to client: " << m_sock << " , data: " << buf << std::endl;

	m_overlap.SetContext(this);

	if (SOCKET_ERROR == WSASend(m_sock,
		&dataBuf,
		1,
		&bytes,
		0,
		&m_overlap,
		NULL
	)) {
		if (WSA_IO_PENDING != WSAGetLastError()) {
			SockHelper::LogLastError("WSASend");
			std::cout << "close sock: " << m_sock << std::endl;
			CLOSESOCK(m_sock);
			return;
		}
	}
}


bool CSockWriteObj::Complete() {
	DWORD bytes = 0;
	DWORD flags = 0;

	if (!WSAGetOverlappedResult(m_sock,
		&m_overlap,
		&bytes,
		false,
		&flags
	)) {
		if (WSA_IO_PENDING != WSAGetLastError()) {
			SockHelper::LogLastError("WSAGetOverlappedResult");

			std::cout << "close sock: " << m_sock << std::endl;
			if (SOCKET_ERROR == shutdown(m_sock, SD_RECEIVE)) {
				SockHelper::LogLastError("shutdown");
			}
			CLOSESOCK(m_sock);
			return false;
		}
	}

	std::cout << "sent: " << bytes << " bytes.\n" << std::endl;
	if (bytes > 0) {
		if (m_cbWrite != nullptr) {
			m_cbWrite(bytes);
		}
	}
	return true;
}