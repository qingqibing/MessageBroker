#include "pch.h"
#include <WinSock2.h>
#include "CSockObj.h"
#include "SockHelper.h"

/*
CSockObj
*/
CSockObj::CSockObj(SOCKET s, CSockObj::OnError cberror) 
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
	sockObj->Complete();
}

void CSockObj::RaiseErrorCallback() const{
	if (m_cbError != nullptr) {
		m_cbError(m_sock);
	}
}

/*
CSockReadObj
*/

CSockReadObj::CSockReadObj(SOCKET s, CSockObj::OnError cberror, CSockReadObj::OnReadComplete cbRead)
	: CSockObj(s, cberror), m_cbRead(cbRead) {

}

CSockReadObj::~CSockReadObj(){}

void CSockReadObj::Read(/*char* pBuf, int len*/) {

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

void CSockReadObj::Complete() {
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

			RaiseErrorCallback();

			std::cout << "close sock: " << m_sock << std::endl;
			if (SOCKET_ERROR == shutdown(m_sock, SD_RECEIVE)) {
				SockHelper::PrintError("shutdown");
			}
			CLOSESOCK(m_sock);
			return;
		}
	}

	if (bytes > 0) {
		m_buf[bytes] = '\0';  //TODO: if bytes == recvBuf size??
		std::cout << "recved: " << bytes << " bytes.\n" << m_buf << std::endl;
		if (m_cbRead != nullptr) {
			char temp[DEFAULT_BUF_SIZE];
			memcpy_s(temp, DEFAULT_BUF_SIZE, m_buf, bytes + 1);  //copy last '\0'
			m_cbRead(m_sock, temp, bytes);
		}
	}
}



/*
CSockWriteObj
*/

CSockWriteObj::CSockWriteObj(SOCKET s, CSockObj::OnError cbError, CSockWriteObj::OnWriteComplete cbWrite)
	: CSockObj(s, cbError), m_cbWrite(cbWrite) {

}

CSockWriteObj::~CSockWriteObj(){}

void CSockWriteObj::Write(const char* buf, int len) {
	WSABUF dataBuf;
	dataBuf.buf = const_cast<char*>(buf);
	dataBuf.len = len;

	DWORD bytes = 0;
	//DWORD flags = 0;
	std::cout << "ready to send to client: " << m_sock << " , data: " << buf << std::endl;

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


void CSockWriteObj::Complete() {
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

			RaiseErrorCallback();

			std::cout << "close sock: " << m_sock << std::endl;
			if (SOCKET_ERROR == shutdown(m_sock, SD_RECEIVE)) {
				SockHelper::PrintError("shutdown");
			}
			CLOSESOCK(m_sock);
			return;
		}
	}

	std::cout << "sent: " << bytes << " bytes.\n" << std::endl;
	if (bytes > 0) {
		if (m_cbWrite != nullptr) {
			m_cbWrite(bytes);
		}
	}
}