#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <iostream>
#include <list>
#include <winsock2.h>
using namespace std;

const int EMPTY = 0;
const int LISTEN = 1;
const int RECEIVE = 2;
const int IDLE = 3;
const int SEND = 4;

typedef struct requestLine
{
	string method;
	char uri[255];
	char lang[3] = { "\0" };
	const char* version = "HTTP/1.1 ";
}RequestLine;

typedef struct request
{
	RequestLine requestLine;
	string reqHeader;
	string reqBody;
	int requestLen; //bytes
}Request;

typedef struct timeReceived
{
	string requestBuffer;
	time_t startTime;
}TimeReceived;

class SocketsState
{
public:
	static const int MAX_SOCKETS = 60;
	struct SocketState
	{
		SOCKET id;			// Socket handle
		int	recv;			// Receiving?
		int	send;			// Sending?
		list<TimeReceived> buffer; //list of received requests and time received
		Request request; //currentRequest (head of the list)
		int len;
	};
private:
	SocketState sockets[MAX_SOCKETS] = { 0 };
	int socketsCount;

public:
	SocketsState() : socketsCount(0) {};
	//SocketsState(const SocketsState& s) = delete;
	bool addSocket(SOCKET id, int what);
	void removeSocket(int index);
	SocketState& operator[](int i) { return sockets[i]; };
	void clearCurrRequest(int index);
	void deleteReqFromBuffer(int index);
	void addNewReqToBuffer(int index, int bytesRecv, char* buffer, time_t startTime);
	void setNextSendState(int index);
};
