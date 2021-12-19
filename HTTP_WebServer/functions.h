#include "Header.h"
#pragma once

bool addSocket(SOCKET id, int what); //add to array
void removeSocket(int index); //removes from array
void acceptConnection(int index);
fd_set createRecvSet();
fd_set createSendSet();
void receiveMessage(int index);
void sendMessage(int socket_index);
WSAData InitWinsock();
SOCKET CreateSocket();
sockaddr_in CreateSocketAdd(SOCKET& m_socket);
bool containsParams(char* request);
void getRequestLine(int socket_index, char* requestLine);
void getRequestFromBuffer(int socket_index);

void CheckMessage(int bytes, SOCKET& socket, const char* ErrorType);
char* createFilePath(int socket_index);
void Get(int socket_index, Response* response);
void Post(int socket_index, Response& response);
void Delete_func(int socket_index, Response& response);
void Put(int socket_index, Response* response);
void Head(int socket_index, Response* response);
void Trace(int socket_index, Response* response);
string Options();

