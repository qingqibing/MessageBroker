// MessageBroker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

#include "../lib/json.hpp"

#include "CSocketCtrl.h"
#include "CSocketServer.h"
#include "CSocketRWObj.h"

using json = nlohmann::json;

static SOCKET sock = INVALID_SOCKET;  //just for test
static CSocketRWObj* readSock;
static CSocketRWObj* writeSock;
static char* _buf;
static int _len;

void on_write_complete(size_t bytesSent) {
}

void on_read_complete(char* buf, int len) {
	_buf = buf;
	_len = len;
}

void on_new_client(SOCKET s) {
	readSock = new CSocketRWObj(s, true, on_read_complete, nullptr);
	readSock->Read();
	readSock->Wait();

	writeSock = new CSocketRWObj(sock, false, nullptr, nullptr);
	writeSock->Write(_buf, _len);
	writeSock->Wait();
}

void test() {
	CSocketCtrl::Startup();

	CSocketServer server("127.0.0.1", 15555, on_new_client);

	for (int i = 0;i < 1;++i) {
		server.StartListen();
		sock = server.WaitForNewConnection();
		on_new_client(sock);
	}

	delete readSock;
	delete writeSock;

	closesocket(sock);  //not necessary because temp obj's destructor will automatilly 
	//be called by compiler
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
	
	std::cin.get();
}

