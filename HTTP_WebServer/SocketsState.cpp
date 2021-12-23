#include "SocketsState.h"

bool SocketsState::addSocket(SOCKET id, int what)
{
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].recv == EMPTY)
		{
			sockets[i].id = id;
			sockets[i].recv = what;
			sockets[i].send = IDLE;
			sockets[i].len = 0;
			socketsCount++;
			// Set the socket to be in non-blocking mode.
			unsigned long flag = 1;
			if (ioctlsocket(id, FIONBIO, &flag) != 0) //make the socket non-blocking
			{
				cout << "Web Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
			}
			return (true);
		}
	}
	return (false);
}

void SocketsState::removeSocket(int index)
{
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;
	socketsCount--;
}

void SocketsState::clearCurrRequest(int index)
{
	sockets[index].request.reqBody.clear();
	sockets[index].request.reqHeader.clear();
	sockets[index].request.requestLine.lang[0] = '\0';
	sockets[index].request.requestLine.uri[0] = '\0';
}

void SocketsState::deleteReqFromBuffer(int index)
{
	if (!(this->sockets[index].buffer.empty()))
	{
		this->sockets[index].buffer.pop_front();
	}
	this->sockets[index].len -= sockets[index].request.requestLen;
}

void SocketsState::addNewReqToBuffer(int index, int bytesRecv, char* buffer, time_t startTime)
{
	TimeReceived newReq;
	newReq.requestBuffer = buffer;
	newReq.startTime = startTime;
	sockets[index].request.requestLen = bytesRecv;
	sockets[index].buffer.push_back(newReq);
	sockets[index].len += bytesRecv;
}

void SocketsState::setNextSendState(int index)
{
	if(sockets[index].len == 0)
	{
		sockets[index].send = IDLE;
	}
	else
	{
	sockets[index].send = SEND;
	}
}