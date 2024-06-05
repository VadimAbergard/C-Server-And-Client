#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream> 
#include <cstdio> 
#include <cstring> 
#include <vector> 
#include <thread>
#include <map>
#include <fstream>
#include <string>
#include <winsock2.h> 
#pragma comment(lib, "WS2_32.lib")
using namespace std;

class sendParams {
private:
	char* buffer;
	int indexClient;
public:
	sendParams(char* buffer, int indexClient) {
		this->buffer = buffer;
		this->indexClient = indexClient;
	}

	char* getBuffer() {
		return buffer;
	}

	int getIndex() {
		return indexClient;
	}

	void clearBuffer() {
		memset(buffer, 0, sizeof(buffer)); // clear buffer
};

class Data {
private:
	SOCKET socket;
	string nameClient;
public:
	Data(SOCKET socket) {
		this->socket = socket;
		nameClient = "none";
	}

	SOCKET getSocket() {
		return socket;
	}

	string getName() {
		return nameClient;
	}

	void setName(string name) {
		nameClient = name;
	}
};

map<int, Data*> clients;
int freeId;

DWORD WINAPI serverSend(LPVOID lpParam) { //send data to client
	sendParams sendData = *(sendParams*)lpParam;

	string nameClient = clients[sendData.getIndex()]->getName();
	string bufferText(sendData.getBuffer());

  // check commands and send client
	for (const auto& client : clients) {
		if (sendData.getIndex() == client.first) {
			string checkOn = nameClient + " on";
			string checkNames = nameClient + " names";
			if (strcmp(bufferText.c_str(), checkOn.c_str()) == 0) { // command get online server
				string textSend = "online|" + to_string(clients.size()) + "|";
				send(client.second->getSocket(), textSend.c_str(), 1024, 0);
				return 1;
			}
			if (strcmp(bufferText.c_str(), checkNames.c_str()) == 0) { // command get all names clients
				string names;
				for (const auto& client : clients) {
					names += client.second->getName() + "|";
				}
				string textSend = "names|" + names;
				send(client.second->getSocket(), textSend.c_str(), 1024, 0);
				return 1;
			}
			break;
		}
	}

  // send data every clients
	for (const auto& _client : clients) {
		if (sendData.getIndex() == _client.first) continue;

		SOCKET client = _client.second->getSocket();
		
		if (send(client, bufferText.c_str(), 128, 0) == SOCKET_ERROR) {
			cout << "send failed with error " << WSAGetLastError() << endl;
			return -1;
		}
		
	}
	return 1;
}

DWORD WINAPI serverReceive(LPVOID lpParam) { //get data from client
	string nullCheck = "";

	bool clientExit = false;

	char buffer[1024] = {0}; //buffer data
	SOCKET client = clients[*(int*)lpParam]->getSocket(); //socket ffor client
	while (true) { //Цикл работы сервера
		if (recv(client, buffer, sizeof(buffer), 0) == SOCKET_ERROR) { // error socket
      // disconnect client
			string textDisconect = clients[*(int*)lpParam]->getName() + " disconect|";

			cout << "recv function failed with error " << WSAGetLastError() << endl;
			
			if (strcmp(buffer, nullCheck.c_str())) continue;

			for (int i = 0;i < 1024;i++) {
				buffer[i] = 0;
			}
			for (int i = 0; i < textDisconect.size(); i++) {
				buffer[i] = textDisconect[i];
			}

			clientExit = true;
		}
		
		string loginClient;
		for (int i = 0; i < 1024;i++) {
			if (buffer[i] == ' ') break;
			loginClient += buffer[i];
		}
		clients[*(int*)lpParam]->setName(loginClient);

		cout << "Client: " << buffer << endl; //message client

		DWORD tid; //id
		HANDLE t2 = CreateThread(NULL, 0, serverSend, new sendParams(buffer, *(int*)lpParam), 0, &tid); //create thread for send data
		if (t2 == NULL) {
			cout << "Thread Creation Error: " << WSAGetLastError() << endl;
		}

		WaitForSingleObject(t2, INFINITE);


		memset(buffer, 0, sizeof(buffer)); // clear buffer

		if (clientExit) {
			closesocket(clients[*(int*)lpParam]->getSocket());
			clients.erase(*(int*)lpParam);
			break;
		}


	}
	return 1;
}



bool clientCreated = false;

void createClient(SOCKET server, SOCKADDR_IN clientAddr) {

	SOCKET socketClient;
	

	char buffer[1024]; //create buffer
	int clientAddrSize = sizeof(clientAddr); //init ip addres client
	if ((socketClient = accept(server, (SOCKADDR*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET) {
		clients[freeId] = new Data(socketClient);
		int indexClient = freeId;
		clientCreated = false;
		freeId += 1;
    
		cout << "Client connected!" << endl;
		cout << "Now you can use our live chat application. " << "Enter \"exit\" to disconnect" << endl;

		DWORD tid; //id
			HANDLE t1 = CreateThread(NULL, 0, serverReceive, &indexClient, 0, &tid); //create thread
			if (t1 == NULL) { //error create
				cout << "Thread Creation Error: " << WSAGetLastError() << endl;
			}

			WaitForSingleObject(t1, INFINITE);

			//close socket
			cout << "socket close" << endl;
	}
}

int main() {
	string ipAdd = "127.0.0.1";

	WSADATA WSAData; //data 
	SOCKET server; //server socket
	SOCKADDR_IN serverAddr, clientAddr;
	WSAStartup(MAKEWORD(2, 0), &WSAData);
	server = socket(AF_INET, SOCK_STREAM, 0); //create server
	if (server == INVALID_SOCKET) {
		cout << "Socket creation failed with error:" << WSAGetLastError() << endl;
		return -1;
	}

	serverAddr.sin_addr.s_addr = inet_addr(ipAdd.c_str());
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5555);
	if (bind(server, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		cout << "Bind function failed with error: " << WSAGetLastError() << endl;
		return -1;
	}

	if (listen(server, 0) == SOCKET_ERROR) {
		cout << "Listen function failed with error:" << WSAGetLastError() << endl;
		return -1;
	}
	cout << "Listening for incoming connections...." << endl;

	while (1) {
		while (clientCreated);
		clientCreated = true;
		thread th(createClient, server, clientAddr);
		th.detach();
	}
}
