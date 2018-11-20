
#include "pch.h"

#include <iostream>
#include <string>
#include "CSocketCtrl.h"
#include "SockHelper.h"

int CSocketCtrl::error_code = 0;

CSocketCtrl::CSocketCtrl() {
	error_code = 0;

	WSADATA wsaData;
	error_code = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (error_code != 0) {
		SockHelper::LogLastError("WSAStartup");
	}

}

CSocketCtrl::~CSocketCtrl() {
	error_code = WSACleanup();
	if (error_code != 0) {
		SockHelper::LogLastError("WSACleanup");
	}
}

int CSocketCtrl::Startup() {
	static CSocketCtrl instance;
	return error_code;
}

