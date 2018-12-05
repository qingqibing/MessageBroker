#include "pch.h"
#include "EventManager.h"
#include "common_template.h"


EventManager& EventManager::getInstance() {
	static EventManager manager;
	return manager;
}

bool EventManager::AddEvent(HANDLE h) {
	std::lock_guard<std::mutex> lock(m_mutex);
	return append_unique(m_hEvents, h);
}

bool EventManager::RemoveEvent(HANDLE h) {
	std::lock_guard<std::mutex> lock(m_mutex);
	return remove_first(m_hEvents, h);
}