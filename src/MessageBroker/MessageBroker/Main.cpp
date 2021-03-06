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
#include "CSockObj.h"
#include "CSockConnection.h"
#include "SockHelper.h"
#include "TimerWrapper.h"
#include "ILog.h"
#include "common_template.h"

#define SVR_PORT 15555
#define TIME_OUT 1000  //default time out value: 1s

using json = nlohmann::json;


#ifdef _DEBUG
static const long SUICIDE_DELAY_TIME = 10 * 1000;  //10 seconds

#else
static const long SUICIDE_DELAY_TIME = 3 * 60 * 1000;  //3 minutes

#endif //  _DEBUG


static std::vector<std::unique_ptr<CSockConnection>> g_conns; //all client accept connection
static std::mutex g_mutex;
static std::unique_ptr<TimerWrapper> g_twptr;//(new TimerWrapper(SUICIDE_DELAY_TIME));

void send_to_peers(CSockConnection* conn, char* buf, int len) {
	conn->PostWriteRequest(buf, len);
}


void sock_recv_complete(SOCKET s, char* buf, int len) {
	std::lock_guard<std::mutex> lock(g_mutex);

	auto it = g_conns.begin();
	while (it != g_conns.end()) {
		auto sock = (*it)->GetSocket();
		if (sock == s) {  //self
			(*it)->PostReadRequest();
			++it;
			continue;
		}

		//check if sock valid
		/*if (!SockHelper::is_sock_connected(sock)) {
			std::cout << "socket invalid: " << sock << " , will be erased!" << std::endl;
			it = g_conns.erase(it);
			continue;
		}*/
		send_to_peers((*it).get(), buf, len);
		++it;
	}
}

void sock_send_complete(size_t len) {

}

void sock_error(SOCKET s) {
	auto it = g_conns.cbegin();
	while (it != g_conns.cend()) {
		auto sock = (*it)->GetSocket();
		if (sock == s) {
			it = g_conns.erase(it);
			std::cout << "socket invalid: " << sock << " , will be erased!" << std::endl;
			break;
		}
		++it;
	}
}

void on_new_client(SOCKET s) {
	std::lock_guard<std::mutex> lock(g_mutex);

	std::cout << "new connection:" << s << std::endl;

	//use emplace_back instead of push_back can avoid unnecessary copy/move 
	//and emplace_back will do perfect forwarding arguments
	//in this case, will pass to std::unique_ptr ctor
	g_conns.emplace_back(new CSockConnection(s, /*sock_error*/nullptr, sock_recv_complete, sock_send_complete));
}

void test() {

	CSocketCtrl::Startup();

	//create a waitable timer
	g_twptr = make_unique<TimerWrapper>(SUICIDE_DELAY_TIME);

	if (!g_twptr->StartTimer()) {
		StdLogger::me().log("start timer failed");
		return;
	}

	EventManager::getInstance().AddEvent(g_twptr->getHandle());  //add waitable timer's handle to the first element in EventManager

	CSocketServer server("127.0.0.1", SVR_PORT, on_new_client);
	server.StartListen();

	std::cout << "server is listening on port " << SVR_PORT << std::endl;

	while (1)
	{
		auto cnt = EventManager::getInstance().handle_size();
		DWORD index = WaitForMultipleObjectsEx(cnt, EventManager::getInstance().get_handles().data(), false, /*TIME_OUT*/INFINITE, true);
		if (index >= WAIT_OBJECT_0 && index < WAIT_OBJECT_0 + cnt) {
			if (index == WAIT_OBJECT_0) {
				//suicide timer singnaled
				break;
			}
			else if (index == WAIT_OBJECT_0 + 1) {
				//new connection, the second HANDLE in EventManager is NEW socket connection
				on_new_client(server.GetAcceptSock());
				server.StartListen();
				if (!g_twptr->CancelTimer()) {//once new client is connected, stop the timer
					break;  //error happen
				}
			}
		}
		else if (index == WAIT_IO_COMPLETION) {
			/*
			The wait was ended by one or more I / O completion routines that were executed.
			The event that was being waited on is not signaled yet.The application must call
			the WSAWaitForMultipleEvents function again.This return value can only be returned
			if the fAlertable parameter is TRUE.
			*/

			//check CSockConnection' sock status which been set in CompletionRoutine
			auto it = g_conns.cbegin();
			while (it != g_conns.cend()) {
				if ((*it)->GetSockWrong()) {
					std::cout << "socket invalid: " << (*it)->GetSocket() << " , will be erased!" << std::endl;
					it = g_conns.erase(it);
					continue;
				}
				++it;
			}

			if (g_conns.size() == 0) {
				if (!g_twptr->StartTimer()) {  //when no client, reactivate the suicide timer
					break;  //error happen
				}
			}
		}
		else if (index == WAIT_FAILED) {
			StdLogger::me().log_e("WaitForMultipleObjectsEx");
		}
		else if (index == WAIT_TIMEOUT) {

			/*
			since we use INFINITE on WaitForMultipleObjectsEx, WAIT_TIMEOUT 
			NEVER gonna happen
			*/
		}
	}

	std::cout << "quit......" << std::endl;

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
	//todo: use windows message loop to see if better to pass message
	test();
	std::cin.get();
}

