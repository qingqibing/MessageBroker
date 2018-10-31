#pragma once

#include <vector>
#include <Windows.h>
#include <mutex>


//singleton
class EventManager {
public:
	static EventManager& getInstance();
	~EventManager() = default;

	EventManager(const EventManager&) = delete;
	EventManager& operator=(const EventManager&) = delete;

	void AddEvent(HANDLE h);
	bool RemoveEvent(HANDLE h);
	size_t handle_size() const { return m_hEvents.size(); }
	const std::vector<HANDLE>& get_handles() { return m_hEvents; }

private:
	EventManager() = default;
	std::mutex m_mutex;
	std::vector<HANDLE> m_hEvents;
};
