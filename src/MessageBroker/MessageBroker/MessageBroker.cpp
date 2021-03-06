
#include "pch.h"
#include <exception>

#include "MessageBroker.h"
#include "../lib/json.hpp"

using json = nlohmann::json;

MessageBroker::MessageBroker(){

}

MessageBroker::~MessageBroker() {

}

void MessageBroker::processMessage(const std::string& msg) {
	try {
		json msgobj = json::parse(msg);

		for (auto& conn : m_connections) {
			//if (s != m_activeSock) {
			routeMessage(conn, msg);
		}
	}
	catch (std::exception& e) {
		std::cout << "json  error: " << e.what() << std::endl;
	}
}

void MessageBroker::routeMessage(CSockConnection& s, const std::string& msg) {

	//TODO: now is implementa as friend, need to improve
}

void MessageBroker::killMyself() {
}