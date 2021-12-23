#pragma once
#include "Declarations.h"

bool containsParams(char* request)
{
	int len = strlen(request);
	for (int i = 0; i < len; i++)
	{
		if (request[i] == '?')
		{
			return true;
		}
	}

	return false;
}

void getRequestLine(SocketsState& sockets, int socket_index, char* requestLine)
{
	const char del[5] = "?=/ ";
	char buffer[512] = { "\0" };
	char* token;
	int strtok_count = 0; //0=methud, 1=uri
	bool isContainsParam;

	strcpy(buffer, requestLine);
	isContainsParam = containsParams(buffer);

	token = strtok(buffer, del);

	while (token != nullptr)
	{
		if (strtok_count == 0)
		{
			sockets[socket_index].request.requestLine.method = token;
		}
		else if (strtok_count == 1)
		{
			strcpy(sockets[socket_index].request.requestLine.uri, token);
		}
		else if (strtok_count > 2 && isContainsParam)
		{
			strcpy(sockets[socket_index].request.requestLine.lang, token);
			break;
		}
		strtok_count++;
		token = strtok(nullptr, del);
	}
}

void getRequestFromBuffer(SocketsState& sockets, int socket_index)
{
	const char del[9] = "\r\n";
	char buffer[MAX_LEN];
	char* token;
	char* requestLine = nullptr;
	int bodyIndex = 9, strtok_count = 0; //0=requestLine, 2=header, 3=body

	strcpy(buffer, sockets[socket_index].buffer.front().requestBuffer.c_str());
	token = strtok(buffer, del);

	while (token != nullptr)
	{
		if (strtok_count == 0)
		{
			requestLine = token;
		}
		else if (strtok_count == bodyIndex)
		{
			sockets[socket_index].request.reqBody = token;
		}
		strtok_count++;
		token = strtok(nullptr, del);
	}

	getRequestLine(sockets, socket_index, requestLine);
}