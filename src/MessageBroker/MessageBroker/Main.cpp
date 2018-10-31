// MessageBroker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <mutex>
#include <memory>

#include "../lib/json.hpp"

#include "CSocketCtrl.h"
#include "CSocketServer.h"
#include "CSocketRWObj.h"
#include "CSockConnection.h"

#define SVR_PORT 15555

using json = nlohmann::json;

//static std::vector<HANDLE> g_handles;  //store all the handlers here, need to be improved
static std::vector<std::shared_ptr<CSockConnection>> g_conns; //all client accept connection
static std::mutex g_mutex;

void send_to_peers(CSockConnection* conn, char* buf, int len) {
	conn->write(buf, len);
}

bool is_sock_invalid(SOCKET s) {
	if (s == INVALID_SOCKET) return true;
	int error;
	int error_size = sizeof(error);
	int result = getsockopt(s, SOL_SOCKET, SO_ERROR, (char*)&error, &error_size);
	if (result == SOCKET_ERROR || error != 0) {
		return true;
	}
	return false;
}

void sock_recv_complete(SOCKET s, char* buf, int len) {
	std::lock_guard<std::mutex> lock(g_mutex);

	auto it = g_conns.begin();
	while (it != g_conns.end()) {
		auto sock = (*it)->getSocket();
		if (sock == s) {
			(*it)->read();
			++it;
			continue;
		}

		if (is_sock_invalid(sock)) {
			std::cout << "socket invalid: " << s << " , will be erased!" << std::endl;
			it = g_conns.erase(it);
			//g_handles.erase((*it)->addtoHandles)
			continue;
		}

		send_to_peers((*it).get(), buf, len);
		++it;
	}
}

void on_new_client(SOCKET s) {
	std::lock_guard<std::mutex> lock(g_mutex);

	std::cout << "new connection:" << s << std::endl;
	std::shared_ptr<CSockConnection> conn = std::make_shared<CSockConnection>(s, sock_recv_complete);
	//CSockConnection* conn = new CSockConnection(s, sock_recv_complete);
	g_conns.push_back(conn);
}


bool IsQuit(HANDLE h) {
	DWORD fdwSaveOldMode, fdwMode;
	INPUT_RECORD irBuf[128];
	DWORD numRead;

	if (!GetConsoleMode(h, &fdwSaveOldMode)) {
		return false;
	}

	fdwMode = ENABLE_WINDOW_INPUT;
	if (!SetConsoleMode(h, fdwMode)) {
		return false;
	}

	if (!ReadConsoleInput(
		h,
		irBuf,
		128,
		&numRead
	)) {
		return false;
	}

	for (int i = 0; i < numRead; ++i) {
		switch (irBuf[i].EventType)
		{
		case KEY_EVENT:
			if (irBuf[i].Event.KeyEvent.uChar.AsciiChar == 'q') {
				return true;
			}
		default:
			break;
		}
	}
	return false;
}

void test() {
	CSocketCtrl::Startup();

	//HANDLE hin = GetStdHandle(STD_INPUT_HANDLE);
	//if (hin == INVALID_HANDLE_VALUE) {
	//	return;
	//}

	//std::thread t(thread_entry);
	CSocketServer server("127.0.0.1", SVR_PORT, on_new_client);
	server.StartListen();

	std::cout << "server is listening on port "<<SVR_PORT << std::endl;
	while (1)
	{
		auto cnt = EventManager::getInstance().handle_size();
		DWORD index = WaitForMultipleObjectsEx(cnt, EventManager::getInstance().get_handles().data(), false, INFINITE, true);
		if (index >= WAIT_OBJECT_0 && index < WAIT_OBJECT_0 + cnt) {
			if (index == WAIT_OBJECT_0) {
				//new connection
				on_new_client(server.GetAcceptSock());
				server.StartListen();
			}

		}
		else if (index == WAIT_IO_COMPLETION) {
			/*
			The wait was ended by one or more I / O completion routines that were executed.
			The event that was being waited on is not signaled yet.The application must call
			the WSAWaitForMultipleEvents function again.This return value can only be returned
			if the fAlertable parameter is TRUE.
			*/
		}
		else if (index == WAIT_FAILED) {
			std::cout << "WSAWaitForMultipleEvents failed with error: " << GetLastError() << std::endl;
		}
		else if (index == WAIT_TIMEOUT) {
			//can not happen here, because we use INFINITE
		}
	}

}

void test_json() {
	json j;
	j["id"] = "test";
	std::cout << j.dump(4) << std::endl;

	const std::string& jsonstr = "{\"id\": \"test\"}";
	auto j2 = json::parse(jsonstr);
	std::string t = j2["id"];
	std::cout << j2["id"] << std::endl;
}



int main()
{
	test();
	std::cin.get();
}

