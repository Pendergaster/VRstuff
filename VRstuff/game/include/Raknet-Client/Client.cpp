#include "Client.h"

using namespace RakNet;
using namespace std;

#define CONSOLE(x) cout << x << endl;

#if 0
Client::Client(string IP, int Port, const char* username)
{

	this->IP = IP;
	this->SERVER_PORT = Port;
	this->username = username;
	string title = "Raknet-Client";
#if 0
	SetConsoleTitle(title.c_str());
#endif
}
#endif

void Client::init_client(string IP, int Port,const char* username)
{
	this->IP = IP;
	this->SERVER_PORT = Port;
	this->username = username;
	string title = "Raknet-Client";
	this->Connected = false;
	this->LoggedIn = false; 
	Peer = RakNet::RakPeerInterface::GetInstance();
	SD = new RakNet::SocketDescriptor(0,0);
}

Client::~Client()
{
	delete SD;
}

void Client::CloseConnection()
{
	RakPeerInterface::DestroyInstance(Peer);
}

/*Does simple setup for client connection*/
void Client::OpenConnection()
{
	Peer = RakPeerInterface::GetInstance();

	Peer->Startup(8, SD, 1);
	Peer->SetOccasionalPing(true);
	Peer->Connect(IP.c_str(),SERVER_PORT, 0, 0);

	Delta = chrono::system_clock::now();
	TimeInterval = (int)((1.0 / 60) * 1000);
}

void Client::Update()
{
	auto TimeDifference = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now() - Delta);
	/*Loads packet from peer*/
	if ((float)TimeDifference.count() > TimeInterval)
	{
		Delta += chrono::milliseconds((int)TimeInterval);
		for (Packet = Peer->Receive(); Packet != 0; Packet = Peer->Receive())
		{
			/*Switch case that lets us check what kind of packet it was*/
			ClientConnectionUpdate(Packet); 
			Peer->DeallocatePacket(Packet); 
		}
	}
}

void Client::ClientConnectionUpdate(RakNet::Packet* Packet)
{
	switch (Packet->data[0])
	{
		case ID_CONNECTION_REQUEST_ACCEPTED:
			HostAddress = Packet->systemAddress;
			CONSOLE("Connection with server at " << IP << " was succesful");
			Connected = true;
			SendUsernameForServer(this->username);
			break;
		case ID_CONNECTION_LOST:
			CONSOLE("Connection lost to server at " << IP);
			Connected = false;
			LoggedIn = false;
			thread(&Client::RetryConnection, this).detach();
			break;
		case PLAYER_COORD:
			CheckForVar(PLAYER_COORD);
			CONSOLE("PLAYER COORD REQUEST RECEIVED");
			break;
		case PLAYER_COORD_UPDATE:
			CONSOLE("RECEIVED NEW COORDS FOR GUID" << Packet->guid.ToString());
			break;
		case LOGIN_ACCEPTED:
			CONSOLE("Server accepted our username: " << username);
			LoggedIn = true;
			break;
		case LOGIN_FAILED:
			CONSOLE("Server did not accept our username");
			thread(&Client::UsernameChange, this).detach();
			LoggedIn = false;
			break;
		case USERNAME:
			CheckForVar(USERNAME);
			CONSOLE("Server is asking for username");
			break;
	}
}

void Client::RetryConnection()
{
	using namespace chrono_literals;
	while (Connected == false)
	{
		CONSOLE("RETRYING CONNECTION");
		Peer->Connect(IP.c_str(),SERVER_PORT, 0, 0);
		this_thread::sleep_for(10s);
	}
	thread(&Client::UsernameChange, this).detach();
}

void Client::UsernameChange()
{
	using namespace chrono_literals;
	std::this_thread::sleep_for(1s);
	std::string newusername;
	std::cout << "Anna username :";
	cin >> newusername;
	username = newusername.c_str();

	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)USERNAME_FOR_GUID);
	bs.Write(username);
	Peer->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, HostAddress, false, 0);
}

void Client::CheckForVar(CustomMessages messageID)
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)messageID);
	bool wasregisted = false;
	for (Var<int> var : IntVars)
	{
		if (var.MessageID == messageID) 

		{
			wasregisted = true;
			for (int* i : var.Values)
			{
				bs.Write(*i);
			}
		}
	}
	for (Var<std::string> var : StringVars)
	{
		if (var.MessageID == messageID) 
		{
			wasregisted = true;
			for (std::string* i : var.Values)
			{
				bs.Write(*i);
			}
		}
	}
	for (Var<float> var : FloatVars)
	{
		if (var.MessageID == messageID) 
		{
			wasregisted = true;
			for (float* i : var.Values)
			{
				bs.Write(*i);
			}
		}
	}
	if (wasregisted == true)
	{
		Peer->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, HostAddress, false, 0);
	}
}

void Client::SetVar(CustomMessages MessageID, std::vector<int*> Vars)
{
	Var<int> tmp;
	tmp.type = Type::INT_TYPE;
	tmp.MessageID = MessageID;
	tmp.Values = Vars;
	this->IntVars.push_back(tmp);

	MessageType regType(Type::INT_TYPE,MessageID);
	registeredServerValues.push_back(regType);
}
void Client::SetVar(CustomMessages MessageID, std::vector<float*> Vars)
{
	Var<float> tmp;
	tmp.type = Type::FLOAT_TYPE;
	tmp.MessageID = MessageID;
	tmp.Values = Vars;
	this->FloatVars.push_back(tmp);

	MessageType regType(Type::FLOAT_TYPE,MessageID);
	registeredServerValues.push_back(regType);
}
void Client::SetVar(CustomMessages MessageID, std::vector<string*> Vars)
{
	Var<string> tmp;
	tmp.type = Type::STRING_TYPE;
	tmp.Values = Vars;
	tmp.MessageID = MessageID;
	this->StringVars.push_back(tmp);

	MessageType regType(Type::STRING_TYPE,MessageID);
	registeredServerValues.push_back(regType);
}
void Client::SendUsernameForServer(RakNet::RakString username)
{
	RakNet::BitStream BS;
	BS.Write((RakNet::MessageID)USERNAME_FOR_GUID);
	BS.Write(username);
	this->username = username;
	Peer->Send(&BS, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, HostAddress, false,0);
}

void Client::SendBackCoord(RakNet::Packet* P)
{
	RakNet::BitStream bs;
	bs.Write((MessageID)PLAYER_COORD);
	Peer->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, HostAddress, false, 0);
}
