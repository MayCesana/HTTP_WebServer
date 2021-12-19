#include <iostream>
#include <string>
#include <map>;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string.h>
#include <list>

using namespace std;

const int MAX_LEN = 2048;
const int PORT = 27015;
const int MAX_SOCKETS = 60;
const char* HTTP_VERSION = "HTTP/1.1";
const int EMPTY = 0;
const int LISTEN = 1;
const int RECEIVE = 2;
const int IDLE = 3;
const int SEND = 4;

#define NUM_OPTIONS 7

const char* methods[NUM_OPTIONS] = { "OPTIONS", "GET", "HEAD", "POST", "PUT", "Delete", "TRACE" };

map<int, string> status = { {200, "200 OK\n"}, {201,"201 Created\n"}, {204,"204 NO CONTENT\n" }, { 401,"401 Unauthorized\n" },
	{404,"404 NOT FOUND\n"},{408, "408 REQUEST TIMEOUT\n"},{411,"411 LENGTH REQUIRED\n"},{414, "Request-URI Too Large\n"},
	{500,"500 INTERNAL SERVER_ERROR\n"}, {501,"501 NOT IMPLEMENTED\n"}, {503,"503 SERVICE UNAVAILABLE\n"},
	{505,"505 HTTP VERSION NOT SUPPORTED\n"} };
//map<string, string> responseHeader = { {"Server: ", ""}, {"Content-Type: ", "text / html; charset = UTP - 8"},
	//{"Connection: ", "keep - alive"},{"Date: ", ""},{"Content-length: ", ""} };

typedef struct requestLine
{
	string method;
	char* uri;
	char* lang = nullptr;
	const char* version = HTTP_VERSION;
}RequestLine;

typedef struct request
{
	RequestLine requestLine;
	char* header;
	char* body;
}Request;

typedef struct responseLine
{
	const char* version = HTTP_VERSION;
	string status;
}ResponseLine;

typedef struct response 
{
	ResponseLine statusLine;
	char* header;
	char* body;
}Response;

struct SocketState
{
	SOCKET id;			// Socket handle
	int	recv;			// Receiving?
	int	send;			// Sending?
	char buffer[MAX_LEN];
	Request request;
	int len;
};

struct SocketState sockets[MAX_SOCKETS] = { 0 };
int socketsCount = 0;

bool addSocket(SOCKET id, int what); //add to array
void removeSocket(int index); //removes from array
void acceptConnection(int index);
fd_set createRecvSet();
fd_set createSendSet();
void receiveMessage(int index);
//void sendMessage(int index);
WSAData InitWinsock();
SOCKET CreateSocket(); 
sockaddr_in CreateSocketAdd(SOCKET& m_socket);
bool containsParams(char* request);
void getRequestLine(int socket_index, char* requestLine);
void getRequestFromBuffer(int socket_index);
void post(int socket_index, Response& response);
void delete_func(int socket_index, Response& response);
void options(int socket_index, Response& response);