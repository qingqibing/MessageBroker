#pragma once

#include <WinBase.h>

//use as a stopwatch


class QPCTimestamp {
public:
	QPCTimestamp();
	~QPCTimestamp();

	void start();
	long long elapsed_us() const;  //return unit: miliseconds

private:
	LARGE_INTEGER m_fre;
	LARGE_INTEGER m_start;
};


QPCTimestamp::QPCTimestamp() {
	memset(&m_start, 0, sizeof(m_start));
	QueryPerformanceFrequency(&m_fre);
}

QPCTimestamp::~QPCTimestamp() = default;

void QPCTimestamp::start() {
	QueryPerformanceCounter(&m_start); //return the counter since the system is up
}

long long QPCTimestamp::elapsed_us() const{
	if (m_start.QuadPart == 0) {
		//not start yet
		return 0;
	}

	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);
	return (end.QuadPart - m_start.QuadPart) * 1000000 / m_fre.QuadPart;
}
