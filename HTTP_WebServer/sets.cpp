#include "SocketsState.h"
#include "Declarations.h"

fd_set createRecvSet(SocketsState& sockets)
{
	fd_set set;
	FD_ZERO(&set);
	for (int i = 0; i < SocketsState::MAX_SOCKETS; i++)
	{
		if ((sockets[i].recv == LISTEN) || (sockets[i].recv == RECEIVE))
			FD_SET(sockets[i].id, &set);
	}

	return set;
}

fd_set createSendSet(SocketsState& sockets)
{
	fd_set set;
	FD_ZERO(&set);
	for (int i = 0; i < SocketsState::MAX_SOCKETS; i++)
	{
		if (sockets[i].send == SEND)
			FD_SET(sockets[i].id, &set);
	}
	return set;
}

int checkEvents(fd_set* waitRecv, fd_set* waitSend)
{
	// Wait for interesting event.
	int nfd = select(0, waitRecv, waitSend, NULL, NULL);
	if (nfd == SOCKET_ERROR)
	{
		cout << "Web Server: Error at select(): " << WSAGetLastError() << endl;
		WSACleanup();
		exit(1);
	}

	return nfd;
}
