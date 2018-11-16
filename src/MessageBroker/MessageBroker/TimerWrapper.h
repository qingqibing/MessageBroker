#pragma once

#include <Windows.h>
#include <string>
#include <iostream>
#include "ILog.h"

class TimerWrapper {
public:
	TimerWrapper(LONGLONG dueTimeMiliseconds) 
		: m_hTimer(NULL), m_started(false) {
		m_duetime.QuadPart =0LL - (dueTimeMiliseconds * 1000 * 1000 / 100);  //convert to 100 nanoseconds inteval, relative time
		m_hTimer = CreateWaitableTimer(NULL, true, NULL);
		if (m_hTimer == NULL) {
			StdLogger::me().log_e("CreateWaitableTimer");
			return;
		}
	}
	~TimerWrapper() {
		if (m_hTimer != NULL) {
			CloseHandle(m_hTimer);
		}
	}

	TimerWrapper(const TimerWrapper&) = delete;
	TimerWrapper& operator=(const TimerWrapper&) = delete;

	//only when waitable timer has not started,
	//then call WIN32 API SetWaitableTimer
	bool StartTimer() {
		if (m_hTimer == NULL) {
			return false;
		}
		if (m_started) {
			return true;
		}

		auto result = SetWaitableTimer(m_hTimer, &m_duetime, 0, NULL, NULL, FALSE);
		if (result) {
			m_started = true;
			StdLogger::me().log("suicide timer started, commit suicide after: %d miliseconds", (0LL - m_duetime.QuadPart*100/1000000));
			return true;
		}
		else {
			m_started = false;
			return false;
		}
	}

	bool CancelTimer() {
		if (m_hTimer == NULL) {
			return false;
		}

		auto result = CancelWaitableTimer(m_hTimer);
		if (result) {
			m_started = false;
			StdLogger::me().log("suicide timer stopped");
			return true;
		}
		else {
			return false;
		}
	}

	const HANDLE getHandle() const { return m_hTimer; }

private:
	HANDLE m_hTimer;
	LARGE_INTEGER m_duetime;
	bool m_started;  //indicate the timer has started and not be cancelled
};
