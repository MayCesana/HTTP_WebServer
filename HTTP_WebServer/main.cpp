#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "Header.h"

using namespace std;
//
WSAData InitWinsock()
{
	WSAData wsaData;

	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Server: Error at WSAStartup()\n";
		exit(0);
	}

	return wsaData;
}

SOCKET CreateSocket()
{
	SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == listen_socket)
	{
		cout << "Time Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		exit(0);
	}

	return listen_socket;
}

sockaddr_in CreateSocketAdd(SOCKET& m_socket)
{
	sockaddr_in serverService;
	serverService.sin_family = AF_INET;
	serverService.sin_addr.s_addr = INADDR_ANY;
	serverService.sin_port = htons(PORT);

	if (SOCKET_ERROR == bind(m_socket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		cout << "Time Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(m_socket);
		WSACleanup();
		exit(0);
	}

	return serverService;
}

void CheckMessage(int bytes, SOCKET& socket, const char* ErrorType)
{
	if (SOCKET_ERROR == bytes)
	{
		cout << "Time Server: Error at" << ErrorType << "(): " << WSAGetLastError() << endl;
		closesocket(socket);
		WSACleanup();
		return;
	}
}

fd_set createRecvSet()
{
	fd_set set;
	FD_ZERO(&set);
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if ((sockets[i].recv == LISTEN) || (sockets[i].recv == RECEIVE))
			FD_SET(sockets[i].id, &set);
	}

	return set;
}

fd_set createSendSet()
{
	fd_set set;
	FD_ZERO(&set);
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].send == SEND)
			FD_SET(sockets[i].id, &set);
	}
	return set;
}

int checkEvents(fd_set waitRecv, fd_set waitSend)
{
	// Wait for interesting event.
	int nfd = select(0, &waitRecv, &waitSend, NULL, NULL);
	if (nfd == SOCKET_ERROR)
	{
		cout << "Time Server: Error at select(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	return nfd;
}

void handleEvents(int nfd, fd_set waitRecv, fd_set waitSend)
{
	for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
	{
		if (FD_ISSET(sockets[i].id, &waitRecv))
		{
			nfd--;
			switch (sockets[i].recv)
			{
			case LISTEN:
				acceptConnection(i);
				break;

			case RECEIVE:
				receiveMessage(i);
				break;
			}
		}
	}

	for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
	{
		if (FD_ISSET(sockets[i].id, &waitSend))
		{
			nfd--;
			switch (sockets[i].send)
			{
			case SEND:
				//sendMessage(i);
				break;
			}
		}
	}
}

void main()
{
	WSAData wsaData = InitWinsock();
	SOCKET listenSocket = CreateSocket();
	sockaddr_in serverService = CreateSocketAdd(listenSocket);

	if (SOCKET_ERROR == listen(listenSocket, 5))
	{
		cout << "Time Server: Error at listen(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}
	addSocket(listenSocket, LISTEN);

	while (true)
	{
		fd_set waitRecv = createRecvSet();
		fd_set waitSend = createSendSet();
		int nfd = checkEvents(waitRecv, waitSend);
		handleEvents(nfd,waitRecv,waitSend);
	}

	// Closing connections and Winsock.
	cout << "Time Server: Closing Connection.\n";
	closesocket(listenSocket);
	WSACleanup();
}

bool addSocket(SOCKET id, int what)
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
			return (true);
		}
	}
	return (false);
}

void removeSocket(int index)
{
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;
	socketsCount--;
}

void acceptConnection(int index)
{
	SOCKET id = sockets[index].id;
	struct sockaddr_in from;		// Address of sending partner
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket)
	{
		cout << "Time Server: Error at accept(): " << WSAGetLastError() << endl;
		return;
	}
	cout << "Time Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;

	//
	// Set the socket to be in non-blocking mode.
	unsigned long flag = 1;
	if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0) //make the socket non-blocking
	{
		cout << "Time Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
	}

	if (addSocket(msgSocket, RECEIVE) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
	return;
}

void receiveMessage(int index)
{
	SOCKET msgSocket = sockets[index].id;
	int len = sockets[index].len;
	int bytesRecv = recv(msgSocket, &sockets[index].buffer[len], sizeof(sockets[index].buffer) - len, 0);

	CheckMessage(bytesRecv, msgSocket, "recv");
	if (bytesRecv == 0)
	{
		closesocket(msgSocket);
		removeSocket(index);
		return;
	}
	else
	{
		getRequestFromBuffer(index);
	}
}

void getRequestFromBuffer(int socket_index)
{
	const char del[9] = "\r\n";
	char buffer[MAX_LEN] = { "\0" };
	char* token;
	char* requestLine = nullptr;
	int bodyIndex = 9, strtok_count = 0; //0=requestLine, 2=header, 3=body

	strcpy(buffer, sockets[socket_index].buffer);
	token = strtok(buffer, del);

	while (token != nullptr)
	{
		if (strtok_count == 0)
		{
			requestLine =  token; 
		}
		else if (strtok_count == bodyIndex)
		{
			sockets[socket_index].request.body = token;
		}
		strtok_count++;
		token = strtok(nullptr, del);
	}

	getRequestLine(socket_index, requestLine);
}

void setMethod(int socket_index)
{
	if (strncmp(sockets[socket_index].buffer, "GET", 3) == 0)
	{
		sockets[socket_index].request.requestLine.method = "GET";
	}
	else if (strncmp(sockets[socket_index].buffer, "POST", 4) == 0)
	{
		sockets[socket_index].request.requestLine.method = "POST";
	}
	else if (strncmp(sockets[socket_index].buffer, "PUT", 3) == 0)
	{
		sockets[socket_index].request.requestLine.method = "PUT";
	}
	else if (strncmp(sockets[socket_index].buffer, "DELETE", 6) == 0)
	{
		sockets[socket_index].request.requestLine.method = "Delete";
	}
	else if (strncmp(sockets[socket_index].buffer, "HEAD", 4) == 0)
	{
		sockets[socket_index].request.requestLine.method = "HEAD";
	}
	else if (strncmp(sockets[socket_index].buffer, "OPTIONS", 7) == 0)
	{
		sockets[socket_index].request.requestLine.method = "OPTIONS";
	}
	else if (strncmp(sockets[socket_index].buffer, "TRACE", 5) == 0)
	{
		sockets[socket_index].request.requestLine.method = "TRACE";
	}
	else
	{
		//exception
	}
}

void getRequestLine(int socket_index, char* requestLine)
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
			setMethod(socket_index);
		}
		else if (strtok_count == 1)
		{
			sockets[socket_index].request.requestLine.uri = token;
		}
		else if (strtok_count > 2 && isContainsParam) 
		{
			sockets[socket_index].request.requestLine.lang = token;
		}
		strtok_count++;
		token = strtok(nullptr, del);
	}
}

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

void get(int socket_index, Response& response)
{
	char filePath[MAX_LEN] = { "C:/temp/" };
	FILE* file = nullptr;
	strcat(filePath, sockets[socket_index].request.requestLine.uri);
	if (sockets[socket_index].request.requestLine.lang != nullptr)
	{
		strcat(filePath, ".");
		strcat(filePath, sockets[socket_index].request.requestLine.lang);
	}

	file = fopen(filePath, "r");
	if (file == nullptr)
	{
		response.statusLine.status = status[404];
	}
	else
	{
		response.statusLine.status = status[200];
	}

	fgets(response.body, MAX_LEN, file);
	fclose(file);
}

string GetTime()
{
	time_t timer;
	string time_str;
	time(&timer);
	time_str = ctime(&timer);
	return (time_str.substr(0, time_str.length() - 1));
}

void createResponseHeader(Response response)
{
	char contentLength[MAX_LEN];
	strcat(response.header, response.statusLine.version);
	strcat(response.header, response.statusLine.status.c_str());
	/*if (sockets[index].request == OPTIONS)
	{
		strcat(sendBuff, "Allow: GET, HEAD, PUT, POST, DELETE, OPTIONS, TRACE\r\n");
	}*/
	strcat(response.header, "Server:Apache\r\n");
	strcat(response.header, "Content-Type: text/html; charset=UTP-8\r\n");
	strcat(response.header, "Connection: keep - alive\r\n");
	string currTime = GetTime();
	strcat(response.header, "Date:");
	strcat(response.header, currTime.c_str());
	strcat(response.header, "\r\n");
	strcat(response.header, "Content-length:");
	_itoa(strlen(response.body), contentLength, 10);
	strcat(response.header, contentLength);
	strcat(response.header, "\r\n\r\n");
}

void sentResponse(int socket_index)
{
	Response response;
	int bytesSent = 0;
	char sendBuff[MAX_LEN];
	SOCKET socket = sockets[socket_index].id;
	string method;

	method = sockets[socket_index].request.requestLine.method;

	if (method == "GET")
	{
		get(socket_index, response);
	}
	else if (method == "POST")
	{
		post(socket_index, response);
	}
	else if (method == "PUT")
	{
		//put
	}
	else if (method == "DELETE")
	{
		delete_func(socket_index, response);
	}
	else if (method == "OPTION")
	{
		options(socket_index, response);
	}
	else if (method == "TREACE")
	{
		//trace
	}

	createResponseHeader(response);
}

//void sendMessage(int index)
//{
//	int bytesSent = 0;
//	char sendBuff[255];
//
//	SOCKET msgSocket = sockets[index].id;
//	if (sockets[index].sendSubType == SEND_TIME)
//	{
//		// Answer client's request by the current time string.
//
//		// Get the current time.
//		time_t timer;
//		time(&timer);
//		// Parse the current time to printable string.
//		strcpy(sendBuff, ctime(&timer));
//		sendBuff[strlen(sendBuff) - 1] = 0; //to remove the new-line from the created string
//	}
//	else if (sockets[index].sendSubType == SEND_SECONDS)
//	{
//		// Answer client's request by the current time in seconds.
//
//		// Get the current time.
//		time_t timer;
//		time(&timer);
//		// Convert the number to string.
//		itoa((int)timer, sendBuff, 10);
//	}
//
//	bytesSent = send(msgSocket, sendBuff, (int)strlen(sendBuff), 0);
//	checkMessage(bytesSent, msgSocket, "send");
//
//	cout << "Time Server: Sent: " << bytesSent << "\\" << strlen(sendBuff) << " bytes of \"" << sendBuff << "\" message.\n";
//
//	sockets[index].send = IDLE;//X --> fix: to check if the buffer contains more messages
//}

void post(int socket_index, Response& response)
{
	cout << sockets[socket_index].request.body << endl;
	response.statusLine.status = status[200];
}

void delete_func(int socket_index, Response& response)
{
	int deletedSucceed;

	char filePath[MAX_LEN] = { "C:/temp/" };
	strcat(filePath, sockets[socket_index].request.requestLine.uri);

	deletedSucceed = remove(filePath);

	if (deletedSucceed == 0)
	{
		response.statusLine.status = status[200];
	}
	else
	{
		response.statusLine.status = status[404];
	}
}

void options(int socket_index, Response& response)
{
	char optionsStr[255];

	for (int i = 0; i < NUM_OPTIONS; i++)
	{
		strcat(optionsStr, methods[i]);
		strcat(optionsStr, ", ");
	}

	strcat(optionsStr, "\n");

	response.body = optionsStr;
	response.statusLine.status = status[200];
}