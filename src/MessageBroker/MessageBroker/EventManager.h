#pragma once

#include <vector>
#include <winnt.h>

class EventManager {
public:
	EventManager() /*: m_hEvent(CreateEvent(NULL, false, false, NULL))*/ {
		//m_handles.push_back(m_hEvent);
	}
	~EventManager() = default;

	void SignalDummyEvent() { /*SetEvent(m_hEvent); */}

	std::vector<HANDLE> m_handles;

private:

	/*HANDLE m_hEvent;*/
};
