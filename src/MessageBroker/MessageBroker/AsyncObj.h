#pragma once

#include <Windows.h>
#include "EventManager.h"

class AsyncObj {
public:
	AsyncObj() {
		

	}
	virtual ~AsyncObj() {

	}

protected:
	OVERLAPPED m_overlap;
};


class EventObj : public AsyncObj {
public:
	EventObj() {
		ZeroMemory(&m_overlap, sizeof(m_overlap));
		m_overlap.hEvent = CreateEvent(NULL, false, false, NULL);
		EventManager::getInstance().AddEvent(m_overlap.hEvent);
	}

	virtual ~EventObj() {
		EventManager::getInstance().RemoveEvent(m_overlap.hEvent);
		CloseHandle(m_overlap.hEvent);
	}
};