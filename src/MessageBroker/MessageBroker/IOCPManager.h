#pragma once

#include <Windows.h>
#include <cassert>

enum {
	CK_TIMER,
	CK_SOCK_NEW,
	CK_SOCK_COM,
	CK_SOCK_ERR
};


class IocpManager {
public:

	~IocpManager() {
		if (m_iocp != INVALID_HANDLE_VALUE) {
			CloseHandle(m_iocp);
		}
	}

	static IocpManager& Self() {
		static IocpManager self;
		return self;
	}

	bool AssiociateIocp(HANDLE fileHandle, DWORD completionKey) {
		if (fileHandle != INVALID_HANDLE_VALUE) {
			return (m_iocp == CreateIoCompletionPort(fileHandle, m_iocp, completionKey, 0));
		}
		return false;
	}

	BOOL GetQueuedStatus(LPDWORD lpNumberOfBytes, PULONG_PTR completionKey,
		_Out_ LPOVERLAPPED *lpOverlapped,
		_In_  DWORD        dwMilliseconds) {
		return GetQueuedCompletionStatus(m_iocp, lpNumberOfBytes, completionKey, lpOverlapped, dwMilliseconds);
	}

	BOOL PostQueuedStatus(ULONG_PTR completionKey, LPOVERLAPPED pOverlapped) {
		return PostQueuedCompletionStatus(m_iocp, 0, completionKey, pOverlapped);
	}

private:
	IocpManager() {
		m_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (m_iocp == INVALID_HANDLE_VALUE) {
			assert(0);
		}
	}

	HANDLE m_iocp;
	
};