#pragma once

#include <iostream>
#include <vector>
#include <string>

#include <WinSock2.h>

#include "CSocketCtrl.h"
#include "CSocketServer.h"
#include "CSockObj.h"
#include "CSockConnection.h"

class MessageBroker
{
public:
	MessageBroker();
	~MessageBroker();

	void processMessage(const std::string& msg);
	std::string getCurMsg(void) const { return message; }
	void killMyself();


private:
	std::string message;  //current message
	//SOCKET m_activeSock;
	std::vector<CSockConnection> m_connections;
	std::vector<HANDLE> m_hEvents;

	void routeMessage(CSockConnection& s, const std::string& msg);
};
