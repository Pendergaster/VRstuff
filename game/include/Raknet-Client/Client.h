#pragma once

#include <iostream>
#include <string>
#if 1
#include <WinSock2.h>
#include <Windows.h>
#endif// WINDOWS
#include <sstream>
#include <thread>
#include <vector>
#include <chrono>


#include <Raknet-Shared/MessageCodes.h>
//#include "MessageCodes.h"
#include <Raknet-Shared/ClientInformation.h>
//#include "ClientInformation.h"
//#include "Utility.h"

/*Including basic raknet headeres*/
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <RakNetTypes.h>
#include <BitStream.h>

#define CONSOLE(x) std::cout << x << std::endl;

using namespace std;

class Client
{
/*PUBLIC FUNCTIONS*/
public:
	//Client(string IP, int Port,const char* username);
	Client() =  default;
	void init_client(string IP, int Port,const char* username);
	~Client();
	void Update();
	void OpenConnection();
	void CloseConnection();
	void RetryConnection();
	void SendUsernameForServer(RakNet::RakString username);
	void SendBackCoord(RakNet::Packet* P);
	RakNet::RakString GetUsername() { return RakNet::RakString(username);}
	void UsernameChange();
	void CheckForVar(CustomMessages messageID);
	void SetVar(CustomMessages MessageID, std::vector<string*> Vars);
	void SetVar(CustomMessages MessageID, std::vector<float*>Vars);
	void SetVar(CustomMessages MessageID, std::vector<int*>Vars);

/*PRIVATE FUNCTIONS*/
private:
	void ClientConnectionUpdate(RakNet::Packet* Packet);

/*PUBLIC VARIABLES*/
public:
	bool Connected = false;
	bool LoggedIn = false;
	string IP;
	int SERVER_PORT;
	const char* username;
	bool State = true;
	vector<Var<int>>IntVars;
	vector<Var<string>> StringVars;
	vector<Var<float>> FloatVars;
	vector<MessageType> registeredServerValues;
	std::thread BackupThread;
/*PRIVATE VARIABLES*/
private:

	RakNet::SystemAddress HostAddress;
	RakNet::RakPeerInterface* Peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet* Packet;
	RakNet::SocketDescriptor* SD = new RakNet::SocketDescriptor(0,0);

	std::chrono::system_clock::time_point Delta;
	float TimeInterval;
};

