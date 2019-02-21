// MessageBroker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <mutex>
#include <memory>
#include <process.h>

#include "../lib/json.hpp"

#include "CSocketCtrl.h"
#include "CSocketServer.h"
#include "CSockObj.h"
#include "CSockConnection.h"
#include "SockHelper.h"
#include "TimerWrapper.h"
#include "ILog.h"
#include "common_template.h"
#include "IOCPManager.h"

#define SVR_PORT 15555
#define TIME_OUT 1000  //default time out value: 1s

using json = nlohmann::json;


#ifdef _DEBUG
static const long SUICIDE_DELAY_TIME = 20 * 1000;  //10 seconds

#else
static const long SUICIDE_DELAY_TIME = 3 * 60 * 1000;  //3 minutes

#endif //  _DEBUG


static std::vector<std::unique_ptr<CSockConnection>> g_conns; //all client accept connection
static std::mutex g_mutex;

void sock_recv_complete(SOCKET s, char* buf, int len) {
	std::lock_guard<std::mutex> lock(g_mutex);

	auto it = g_conns.begin();
	while (it != g_conns.end()) {
		auto sock = (*it)->GetSocket();
		if (sock == s) {
			BOOL r = (*it)->PostReadRequest();
			if (!r) {
				it = g_conns.erase(it);
			}
			else {
				++it;
			}
		}
		else {
			//send to peers
			BOOL r = (*it)->PostWriteRequest(buf, len);
			if (!r) {
				it = g_conns.erase(it);
			}
			else {
				++it;
			}
		}
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
			LOG("socket invalid: %d, will be erased!", sock);
			break;
		}
		++it;
	}
}

void on_new_client(SOCKET s) {
	//todo: use Critical section
	std::lock_guard<std::mutex> lock(g_mutex);

	LOG("got new connection: %d", s);

	//use emplace_back instead of push_back can avoid unnecessary copy/move 
	//and emplace_back will do perfect forwarding arguments
	//in this case, will pass to std::unique_ptr ctor
	g_conns.emplace_back(new CSockConnection(s, /*sock_error*/nullptr, sock_recv_complete, sock_send_complete));
}


HANDLE g_timeThread;



unsigned __stdcall timer_thread_func(VOID* param) {

	HANDLE hTimer = (HANDLE)param;
	DWORD ret = WaitForSingleObject(hTimer, INFINITE);  //wait for the timer
	if (ret == WAIT_OBJECT_0) {
		//post io completion notification
		LOG("timer expire, commit suicide!");
		BOOL ok = IocpManager::Self().PostQueuedStatus(CK_TIMER, NULL);  //tell the main thread to quit
		if (!ok) {
			//todo: how to notify the main thread?
			assert(0);
		}
	}
	else if (WAIT_IO_COMPLETION) {

	}
	else if (WAIT_FAILED) {

	}

	return 0;
}

void test() {

	CSocketCtrl::Startup();  //load sock lib

	//create a waitable timer
	std::unique_ptr<TimerWrapper> pTimer = make_unique<TimerWrapper>(SUICIDE_DELAY_TIME);

	if (!pTimer->StartTimer()) {
		//LOG"start timer failed");
		LOG("start timer failed");
		return;
	}

	unsigned int threadid = 0;
	//we are not interested with thread handle, call CloseHandle
	CloseHandle((HANDLE)_beginthreadex(NULL, 0, timer_thread_func, (void*)pTimer->getHandle(), 0, &threadid));  //start timer thread

	CSocketServer server("127.0.0.1", SVR_PORT, on_new_client);
	if (!server.StartListen()) {
		LOG("server listen failed!");
		return;
	}

	LOG("server is listening on port: %d", SVR_PORT);

	while (1)
	{
		DWORD bytesTransfered = 0;
		ULONG_PTR completionKey = 0;
		LPOVERLAPPED pOverlapped = NULL;

		BOOL ret = IocpManager::Self().GetQueuedStatus(&bytesTransfered, &completionKey, &pOverlapped, INFINITE);
		if (ret) {
			if (completionKey == CK_TIMER) {
				break;
			}
			else if (completionKey == CK_SOCK_NEW) {
				pTimer->CancelTimer();  //stop timer
				on_new_client(server.GetAcceptSock());
				server.StartListen();
			}
			else if (completionKey == CK_SOCK_COM) {
				DerivedOverlapped* pov = reinterpret_cast<DerivedOverlapped*>(pOverlapped);
				CSockObj* sockObj = reinterpret_cast<CSockObj*>(pov->GetContext());
				if (!sockObj->OnComplete()) {  //deal with sent/recv result
					//something wrong happen, post CK_SOCK_ERR
					IocpManager::Self().PostQueuedStatus(CK_SOCK_ERR, pov);
				}
			}
			else if(completionKey == CK_SOCK_ERR){
				//erase the connection
				DerivedOverlapped* pov = reinterpret_cast<DerivedOverlapped*>(pOverlapped);
				CSockObj* sockObj = reinterpret_cast<CSockObj*>(pov->GetContext());
				SOCKET s = sockObj->GetSock();

				auto it = g_conns.cbegin();
				while (it != g_conns.cend()) {
					if ((*it)->GetSocket() == s) {
						LOG("socket invalid: %d, will be erased!", s);
						it = g_conns.erase(it);
						continue;
					}
					++it;
				}

				if (g_conns.size() == 0) {
					LOG("no client connected, start suicide timer!");
					if (!pTimer->StartTimer()) {  //when no client, reactivate the suicide timer
						break;  //error happen
					}
				}
			}
		}
		else {
			//GetQueuedCompletionStatus failed
			LOG_E("GetQueuedCompletionStatus");
			DWORD dwError = GetLastError();
			if (pOverlapped != NULL) { //client close connection unexpectly
				IocpManager::Self().PostQueuedStatus(CK_SOCK_ERR, pOverlapped);
			}
			else {
				if (dwError == ERROR_NETNAME_DELETED) {

				}
				else if (dwError == WAIT_TIMEOUT) {
					assert(0);
				}
				else {
					assert(0);
				}
			}
		}

	}

	LOG("quit......");
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

