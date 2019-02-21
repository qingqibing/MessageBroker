#include "pch.h"
#include <WinSock2.h>
#include "CSockObj.h"
#include "SockHelper.h"
#include "CSockConnection.h"
#include "ILog.h"


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
	LOG("CSockReadObj dtor!");
}

BOOL CSockReadObj::Read(/*char* pBuf, int len*/) {

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
		NULL  //IO Completion port can not used with APC!! this parameter MUST be NULL
	)) {
		if (WSA_IO_PENDING != WSAGetLastError()) {
			LOG_E("WSARecv");
			LOG("close sock: %d", m_sock);
			CLOSESOCK(m_sock);
			return FALSE;
		}
	}
	return TRUE;
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
			LOG_E("WSAGetOverlappedResult");

			//RaiseErrorCallback(m_sock);
			LOG("close sock: %d", m_sock);
			if (SOCKET_ERROR == shutdown(m_sock, SD_RECEIVE)) {
				LOG_E("shutdown");
			}
			CLOSESOCK(m_sock);
			return false;
		}
	}

	if (bytes > 0) {
		m_buf[bytes] = '\0';
		LOG("recved: %d bytes: %s", bytes, m_buf);
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
	LOG("CSOckWriteObj dtor!");
}

BOOL CSockWriteObj::Write(const char* buf, int len) {
	WSABUF dataBuf;
	dataBuf.buf = const_cast<char*>(buf);
	dataBuf.len = len;

	DWORD bytes = 0;
	//DWORD flags = 0;
	LOG("ready to send to client: %d, data: %s", m_sock, buf);

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
			LOG_E("WSASend");
			LOG("close sock: %d", m_sock);
			CLOSESOCK(m_sock);
			return FALSE;
		}
	}
	return TRUE;
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
			LOG_E("WSAGetOverlappedResult");
			LOG("close sock: %d", m_sock);
			if (SOCKET_ERROR == shutdown(m_sock, SD_RECEIVE)) {
				LOG_E("shutdown");
			}
			CLOSESOCK(m_sock);
			return false;
		}
	}
	if (bytes > 0) {
		LOG("sent: %d bytes", bytes);
		if (m_cbWrite != nullptr) {
			m_cbWrite(bytes);
		}
	}
	return true;
}