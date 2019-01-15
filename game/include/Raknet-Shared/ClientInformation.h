#pragma once
#include <iostream>
#include <string>
#include <chrono>

#include <RakNetTypes.h>

using namespace std;

struct ClientData
{
	ClientData(chrono::system_clock::time_point ConnectionTimestamp, string ConnectionID,RakNet::SystemAddress address) {
		this->ConnectionTimestamp = ConnectionTimestamp;
		this->ConnectionID = ConnectionID;
		this->Username = "NONE";
		this->Address = address;
	}
	RakNet::SystemAddress Address;
	chrono::system_clock::time_point ConnectionTimestamp;
	string ConnectionID;
	string Username;
};
