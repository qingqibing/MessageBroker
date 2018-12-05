#include "pch.h"
#include "EventManager.h"
#include "common_template.h"


EventManager& EventManager::getInstance() {
	static EventManager manager;
	return manager;
}

bool EventManager::AddEvent(HANDLE h) {
	std::lock_guard<std::mutex> lock(m_mutex);
	//m_hEvents.push_back(h);
	return append_unique(m_hEvents, h);
}

bool EventManager::RemoveEvent(HANDLE h) {
	std::lock_guard<std::mutex> lock(m_mutex);
	/*auto it = m_hEvents.cbegin();
	while (it != m_hEvents.cend()) {
		if (*it == h) {
			m_hEvents.erase(it);
			return true;
		}
		++it;
	}
	return false;*/

	return remove_first(m_hEvents, h);
}