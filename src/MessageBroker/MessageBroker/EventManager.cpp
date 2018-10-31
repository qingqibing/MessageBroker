#include "pch.h"
#include "EventManager.h"

EventManager& EventManager::getInstance() {
	static EventManager manager;
	return manager;
}

void EventManager::AddEvent(HANDLE h) {
	std::lock_guard<std::mutex> lock(m_mutex);
	m_hEvents.push_back(h);
}

bool EventManager::RemoveEvent(HANDLE h) {
	std::lock_guard<std::mutex> lock(m_mutex);
	auto it = m_hEvents.cbegin();
	while (it != m_hEvents.cend()) {
		if (*it == h) {
			m_hEvents.erase(it);
			return true;
		}
		++it;
	}
	return false;
}