#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream> 
#include <string> 
#include <cstdio> 
#include <cstring> 
#include <thread> 
#include <winsock2.h> 
#pragma comment(lib, "WS2_32.lib")
using namespace std;

bool clientExit;

SOCKET server;

string sendBuffer;

string login;

string textSend;
string textRead = "";
bool read = false;

void send(string text) {
	string finalText = login + " " + text;
	send(server, finalText.c_str(), 1024, 0);
}

DWORD WINAPI clientReceive(LPVOID lpParam) { //get data from server
	char buffer[1024] = { 0 };
	SOCKET server = *(SOCKET*)lpParam;
	while (true) {
		while (read);

		if (recv(server, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
			cout << "recv function failed with error: " << WSAGetLastError() << endl;
			return -1;
		}
		if (strcmp(buffer, "exit\n") == 0) {
			cout << "Server disconnected." << endl;
			clientExit = true;
			return 1;
		}
		string textBuffer(buffer);
		textRead = textBuffer;
		read = true;
		memset(buffer, 0, sizeof(buffer));
	}
	return 1;
}

DWORD WINAPI clientSend(LPVOID lpParam) { //send data server
	char buffer[1024] = { 0 };
	SOCKET server = *(SOCKET*)lpParam;

	bool firstJoin = true;

	while (true) {
		if (firstJoin) {
			send("p"); // for init client in server
			firstJoin = false;
			continue;
		}
		if (textSend != "") {
			send(textSend);
			textSend = "";
		}
	}
	return 1;
}

void createSocket() {
	DWORD tid;
	HANDLE t1 = CreateThread(NULL, 0, clientReceive, &server, 0, &tid);
	if (t1 == NULL) cout << "Thread creation error: " << GetLastError();
	HANDLE t2 = CreateThread(NULL, 0, clientSend, &server, 0, &tid);
	if (t2 == NULL) cout << "Thread creation error: " << GetLastError();

	if (!clientExit) {

		WaitForSingleObject(t1, INFINITE);
		WaitForSingleObject(t2, INFINITE);
		closesocket(server);
		WSACleanup();
	}
}

class ClientSocket {
private:
	
public:
	ClientSocket(string ipAdd, string nick) {
		WSADATA WSAData;
		SOCKADDR_IN addr;
		WSAStartup(MAKEWORD(2, 0), &WSAData);
		if ((server = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
			cout << "Socket creation failed with error: " << WSAGetLastError() << endl;
		}

		addr.sin_addr.s_addr = inet_addr(ipAdd.c_str()); //connect to server
		addr.sin_family = AF_INET;
		addr.sin_port = htons(5555); //port
		if (connect(server, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
			cout << "Server connection failed with error: " << WSAGetLastError() << endl;
		}

		cout << "Connected to server!" << endl;
		cout << "Now you can use our live chat application. " << " Enter \"exit\" to disconnect" << endl;

		thread client(createSocket);
		client.detach();
	}

	void send(string text) {
		textSend = text;
	}

	string read() {
		return textRead;
	}
};

void workWithPocket(string packet) {
	cout << "packet: "  << packet << endl;
}

void readPacket(ClientSocket* clientSocket) {
		string packet = clientSocket->read();
		if (packet != "") {
			read = false;
			workWithPocket(packet);
			packet = "";
		}
}

ClientSocket* clientSocket;

int main() {
	setlocale(LC_ALL, "Russian");

	string ipAdd;
	cout << "ip server: ";
	cin >> ipAdd;
	cout << "create nick name: ";
	cin >> login;

	clientSocket = new ClientSocket(ipAdd, login);

	while (true) {
		readPacket(clientSocket);
		string textForSend;
    cin >> textForSend;
		i++;
		clientSocket->send(textForSend);
	}
}
