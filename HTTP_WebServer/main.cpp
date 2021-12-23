#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Response.h" 
#include "Declarations.h"

int main()
{
	WSAData wsaData = InitWinsock();
	SOCKET listenSocket = CreateSocket();
	sockaddr_in serverService = CreateSocketAdd(listenSocket);
	SocketsState* sockets = new SocketsState();
	checkListen(listenSocket);
	sockets->addSocket(listenSocket, LISTEN);

	while (true)
	{
		fd_set waitRecv = createRecvSet(*sockets);
		fd_set waitSend = createSendSet(*sockets);

		int nfd = checkEvents(&waitRecv, &waitSend);
		handleEvents(*sockets, nfd, &waitRecv, &waitSend);
	}

	// Closing connections and Winsock.
	cout << "Web Server: Closing Connection.\n";
	closesocket(listenSocket);
	WSACleanup();
	delete sockets;

	return 0;
}