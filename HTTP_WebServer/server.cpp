#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Declarations.h"
#include "Response.h"

void acceptConnection(SocketsState& sockets, int index)
{
	SOCKET id = sockets[index].id;
	struct sockaddr_in from;		// Address of sending partner
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket)
	{
		cout << "Web Server: Error at accept(): " << WSAGetLastError() << endl;
		exit(1);
	}
	cout << "Web Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;

	if (sockets.addSocket(msgSocket, RECEIVE) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
}

void sendMessage(SocketsState& sockets, int socket_index)
{
	Response response;
	char sendBuffer[MAX_LEN];
	int bytesSent = 0;
	SOCKET socket = sockets[socket_index].id;
	time_t currTime;

	if (difftime(time(&currTime), sockets[socket_index].buffer.front().startTime) > TIMEOUT)
	{
		response = response.createTimeOutResponse(sockets, socket_index);
		sockets.removeSocket(socket_index);
	}
	else
	{
		response.selectMethod(sockets, socket_index);
		response.createResponseHeader(sockets, socket_index);
	}

	string sendBuff = response.returnRespondAsBuffer();
	strcpy(sendBuffer, sendBuff.c_str());
	bytesSent = send(socket, sendBuffer, (int)strlen(sendBuffer), 0);
	checkMessage(bytesSent, socket, "send", sockets, socket_index);
	cout << "Web Server: Sent: " << bytesSent << "\\" << strlen(sendBuffer) << " bytes of response to HTTP " 
		<< sockets[socket_index].request.requestLine.method << " request" << endl;

	sockets.deleteReqFromBuffer(socket_index);
	sockets.clearCurrRequest(socket_index);
	sockets.setNextSendState(socket_index);
}

void receiveMessage(SocketsState& sockets, int index)
{
	time_t timer;
	SOCKET msgSocket = sockets[index].id;
	char tempBuff[MAX_LEN] = { "\0" };
	int len = sockets[index].len;

	int bytesRecv = recv(msgSocket, tempBuff, sizeof(tempBuff) - len, 0);
	checkMessage(bytesRecv, msgSocket, "recv", sockets, index);
	checkBytesRecv(bytesRecv, msgSocket, sockets, index);

	time(&timer);
	sockets.addNewReqToBuffer(index, bytesRecv, tempBuff, timer);
	getRequestFromBuffer(sockets, index);
	sockets[index].send = SEND;
}

void handleEvents(SocketsState& sockets, int nfd, fd_set* waitRecv, fd_set* waitSend)
{
	for (int i = 0; i < SocketsState::MAX_SOCKETS && nfd > 0; i++)
	{
		if (FD_ISSET(sockets[i].id, waitRecv))
		{
			nfd--;
			switch (sockets[i].recv)
			{
			case LISTEN:
				acceptConnection(sockets, i);
				break;

			case RECEIVE:
				receiveMessage(sockets, i);
				break;
			}
		}
	}

	for (int i = 0; i < SocketsState::MAX_SOCKETS && nfd > 0; i++)
	{
		if (FD_ISSET(sockets[i].id, waitSend))
		{
			nfd--;
			switch (sockets[i].send)
			{
			case SEND:
				sendMessage(sockets, i);
				break;
			}
		}
	}
}