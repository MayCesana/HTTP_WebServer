#include "Declarations.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS

void checkMessage(int bytes, SOCKET& socket, const char* ErrorType, SocketsState& sockets, int index)
{
	if (SOCKET_ERROR == bytes)
	{
		cout << "Web Server: Error at" << ErrorType << "(): " << WSAGetLastError() << endl;
		closesocket(socket);
		sockets.removeSocket(index);
		WSACleanup();
		exit(1);
	}
}

void checkListen(SOCKET& listenSocket)
{
	if (SOCKET_ERROR == listen(listenSocket, 5))
	{
		cout << "Web Server: Error at listen(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		exit(1);
	}
}

void checkBytesRecv(int bytesRecv, SOCKET& socket, SocketsState& sockets, int index)
{
	if (bytesRecv == 0)
	{
		closesocket(socket);
		sockets.removeSocket(index);
		exit(1);
	}
}

