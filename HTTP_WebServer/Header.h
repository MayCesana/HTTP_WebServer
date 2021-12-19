#include <iostream>
#include <string>
#include <map>;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string.h>

using namespace std;

const int MAX_LEN = 2048;
const int PORT = 27015;
const int MAX_SOCKETS = 60;
const char* HTTP_VERSION = "HTTP/1.1 ";
const int EMPTY = 0;
const int LISTEN = 1;
const int RECEIVE = 2;
const int IDLE = 3;
const int SEND = 4;
#define NUM_OPTIONS 7

//enum Method { OPTIONS, GET, HEAD, POST, PUT, Delete, TRACE};
string methods[NUM_OPTIONS] = { "OPTIONS", "GET", "HEAD", "POST", "PUT", "Delete", "TRACE" };

map<int, string> status = { {200, "200 OK\n"}, {201,"201 Created\n"}, {204,"204 NO CONTENT\n" }, { 401,"401 Unauthorized\n" },
	{404,"404 NOT FOUND\n"},{408, "408 REQUEST TIMEOUT\n"},{411,"411 LENGTH REQUIRED\n"},{414, "Request-URI Too Large\n"},
	{500,"500 INTERNAL SERVER_ERROR\n"}, {501,"501 NOT IMPLEMENTED\n"}, {503,"503 SERVICE UNAVAILABLE\n"},
	{505,"505 HTTP VERSION NOT SUPPORTED\n"} };

typedef struct requestLine
{
	string method;
	char uri[255];
	char lang[3];
	const char* version = HTTP_VERSION;
}RequestLine;

typedef struct request
{
	RequestLine requestLine;
	char header[MAX_LEN];
	char body[MAX_LEN];
}Request;

typedef struct responseLine
{
	char* version = (char*)HTTP_VERSION;
	string status;
}ResponseLine;

typedef struct response 
{
	ResponseLine statusLine;
	string header;
	string body = "";
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
void sendMessage(int socket_index);
WSAData InitWinsock();
SOCKET CreateSocket(); 
sockaddr_in CreateSocketAdd(SOCKET& m_socket);
bool containsParams(char* request);
void getRequestLine(int socket_index, char* requestLine);
void getRequestFromBuffer(int socket_index);
