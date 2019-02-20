#pragma once

#include <Windows.h>

/*
this class derived from overlapped structure
and it contains a pointer to store other useful context info
*/

class DerivedOverlapped : public OVERLAPPED {
private:
	void* m_data;

public:
	DerivedOverlapped(): m_data(NULL){}
	~DerivedOverlapped() = default;

	void* GetContext() {
		return m_data;
	}

	void SetContext(void* ptr) {
		m_data = ptr;
	}
};

