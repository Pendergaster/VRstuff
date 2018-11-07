#include "Client.h"

#include <vector>
#define CONSOLE(x) std::cout << x;
#define CONSOLEL(x) std::cout << x << std::endl; 

bool Running = true;


struct WASD
{
	bool A = false; bool S = false; bool D = false; bool W =false;
};

void KeyStateThread(WASD* input);

//TODO: test #pragma pack(push,1). #pragma pack(pop) metodia structien l‰hett‰misen kanssa.
int main()
{
	Client* Connection = new Client("127.0.0.1", 60000,"Loyalisti");
	Connection->OpenConnection();

	/*This is currently placeholder for input. Pretty ugly and holds no value for the future*/
	WASD* input = new WASD();

	/*We launch new thread that will keep updating values inside WASD class*/
	std::thread(KeyStateThread,input).detach();

	int x = 1; //Luodaan variablet mitk‰ me halutaan jatkuvasti heitt‰‰ serverille. Aka peliss‰ pelaajan positio esimerkiksi
	int y = 2;

	std::vector<int*> cord; //we create vector containing pointers to values we want to register for variable call from server
	cord.push_back(&x);
	cord.push_back(&y);

  /*we register data inside vector for server call named "PLAYER_COORD".
	Each time server calls with that enum client will automatically check if it has that enum registered.*/
	Connection->SetVar(PLAYER_COORD, cord); 

	while (Running)
	{
		Connection->Update();
		if (input->D == true)
		{
			Connection->SendUsernameForServer("Akit");
			input->D = false;
		}
	}

	Connection->CloseConnection();
	delete Connection;
	delete input;
	Running = 0;
	return 0;
}

void KeyStateThread(WASD* input)
{
	while (Running)
	{
		if (GetAsyncKeyState(0x41) != 0) { input->A = true; }
		else { input->A = false; }

		if (GetAsyncKeyState(0x44) != 0) { input->D = true; }
		else { input->D = false; }

		if (GetAsyncKeyState(0x57) != 0) { input->W = true; }
		else { input->W = false; }

		if (GetAsyncKeyState(0x53) != 0) { input->S = true; }
		else { input->S = false; }
		this_thread::sleep_for(0.1s);
	}
}
