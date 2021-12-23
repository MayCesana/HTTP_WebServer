#pragma once
#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "Ws2_32.lib")
#include <string>
#include <map>
#include <iostream>
#include "SocketsState.h"
#pragma once

const int MAX_LEN = 2048;
const int PORT = 8080;
const double TIMEOUT = 120;

#define NUM_OPTIONS 7

static map<int, string> Status = { {200, "200 OK"}, {201,"201 Created"}, {204,"204 NO CONTENT" }, { 401,"401 Unauthorized" },
	{404,"404 NOT FOUND"},{408, "408 REQUEST TIMEOUT"},{411,"411 LENGTH REQUIRED"},{414, "Request-URI Too Large"},
	{500,"500 INTERNAL SERVER_ERROR"}, {501,"501 NOT IMPLEMENTED"}, {503,"503 SERVICE UNAVAILABLE"},
	{505,"505 HTTP VERSION NOT SUPPORTED"} };

//sets.cpp functions
fd_set createSendSet(SocketsState& sockets);
fd_set createRecvSet(SocketsState& sockets);
int checkEvents(fd_set* waitRecv, fd_set* waitSend);

//Socket.cpp functions
WSAData InitWinsock();
SOCKET CreateSocket();
sockaddr_in CreateSocketAdd(SOCKET& m_socket);

//requestParseHandler.cpp functions
bool containsParams(char* request);
void getRequestLine(SocketsState& sockets, int socket_index, char* requestLine);
void getRequestFromBuffer(SocketsState& sockets, int socket_index);

//Errors.cpp functions
void checkMessage(int bytes, SOCKET& socket, const char* ErrorType, SocketsState& sockets, int index);
void checkBytesRecv(int bytesRecv, SOCKET& socket, SocketsState& sockets, int index);
void checkListen(SOCKET& listenSocket);

//server.cpp functions
void acceptConnection(SocketsState& sockets, int index);
void handleEvents(SocketsState& sockets, int nfd, fd_set* waitRecv, fd_set* waitSend);
void receiveMessage(SocketsState& sockets, int index);
void sendMessage(SocketsState& sockets, int socket_index);