#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "functions.h"

using namespace std;

void CheckMessage(int bytes, SOCKET& socket, const char* ErrorType)
{
	if (SOCKET_ERROR == bytes)
	{
		cout << "Web Server: Error at" << ErrorType << "(): " << WSAGetLastError() << endl;
		closesocket(socket);
		WSACleanup();
		return;
	}
}

WSAData InitWinsock()
{
	WSAData wsaData;

	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Web Server: Error at WSAStartup()\n";
		exit(0);
	}

	return wsaData;
}

SOCKET CreateSocket()
{
	SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == listen_socket)
	{
		cout << "Web Server: Error at socket(): " << WSAGetLastError() << endl;
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
		cout << "Web Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(m_socket);
		WSACleanup();
		exit(0);
	}

	return serverService;
}

char* createFilePath(int socket_index)
{
	char fileName[MAX_LEN] = { "C:/TEMP/" };
	strcat(fileName, sockets[socket_index].request.requestLine.uri);
	if (sockets[socket_index].request.requestLine.lang[0] != '\0')
	{
		strcat(fileName, ".");
		strcat(fileName, sockets[socket_index].request.requestLine.lang);
	}
	strcat(fileName, ".txt");
	return fileName;
}

void Get(int socket_index, Response* response)
{
	char* filePath = createFilePath(socket_index);
	char bodyBuff[MAX_LEN];
	FILE* file = nullptr;

	file = fopen(filePath, "r");
	if (file == nullptr)
	{
		response->statusLine.status = status[404];
	}
	else
	{
		response->statusLine.status = status[200];
		fgets(bodyBuff, MAX_LEN, file);
		response->body.append(bodyBuff);
		fclose(file);
	}
}

void Post(int socket_index, Response& response)
{
	cout << sockets[socket_index].request.body << endl;
	response.statusLine.status = status[200];
}

void Delete_func(int socket_index, Response& response)
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

void Put(int socket_index, Response* response)
{
	char filePath[MAX_LEN] = { "C:/temp/" };
	FILE* file = nullptr;
	strcat(filePath, sockets[socket_index].request.requestLine.uri);
	file = fopen(filePath, "r");
	if (file == nullptr)
	{
		response->statusLine.status = status[201];
	}
	else //file exist
	{
		response->statusLine.status = status[200];
		fclose(file);
	}
	file = fopen(filePath, "w");
	if (file == nullptr)
	{
		response->statusLine.status = status[501];
	}
	else
	{
		fputs((sockets[socket_index].request.body).c_str(), file);
		fclose(file);
	}


}

void Head(int socket_index, Response* response)
{
	char* filePath = createFilePath(socket_index);
	FILE* file = nullptr;

	file = fopen(filePath, "r");
	if (file == nullptr)
	{
		response->statusLine.status = status[404];
	}
	else
	{
		response->statusLine.status = status[200];
		fclose(file);
	}
}

void Trace(int socket_index, Response* response)
{
	char temp[MAX_LEN];
	strncpy(temp, sockets[socket_index].buffer, sockets[socket_index].request.requestLen);
	response->body = temp;
	response->statusLine.status = status[200];
}

string Options()
{
	string optionsStr;

	for (int i = 0; i < NUM_OPTIONS; i++)
	{
		optionsStr += methods[i];
		optionsStr += ", ";
	}

	optionsStr += "\n";

	return optionsStr;
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

void handleEvents(int nfd, fd_set* waitRecv, fd_set* waitSend)
{
	for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
	{
		if (FD_ISSET(sockets[i].id, waitRecv))
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
		if (FD_ISSET(sockets[i].id, waitSend))
		{
			nfd--;
			switch (sockets[i].send)
			{
			case SEND:
				sendMessage(i);
				break;
			}
		}
	}
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
		cout << "Web Server: Error at accept(): " << WSAGetLastError() << endl;
		return;
	}
	cout << "Web Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;

	if (addSocket(msgSocket, RECEIVE) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
	return;
}

void findFirstReqLen(int index)
{
	int i = 0;
	while (sockets[index].buffer[i] != EndOfRequest)
	{
		i++;
	}

	sockets[index].request.requestLen = i;
}

int numberOfDigits(int number)
{
	int count = 0;
	while (number > 0)
	{
		count++;
		number /= 10;
	}
	return count;
}

void receiveMessage(int index)
{
	SOCKET msgSocket = sockets[index].id;
	char tempBuff[MAX_LEN] = { "\0" };
	time(&sockets[index].request.startTime); 

	int bytesRecv = recv(msgSocket, tempBuff, sizeof(sockets[index].buffer) - sockets[index].len, 0);
	CheckMessage(bytesRecv, msgSocket, "recv");
	if (bytesRecv == 0)
	{
		closesocket(msgSocket);
		removeSocket(index);
		exit(1);
	}
	int reqLen = numberOfDigits(bytesRecv);
	sockets[index].request.requestLen = reqLen;
	strcat(sockets[index].buffer, tempBuff);
	sockets[index].len += reqLen + 1;
	sockets[index].buffer[sockets[index].len] = EndOfRequest;

	findFirstReqLen(index);
	getRequestFromBuffer(index);
	sockets[index].send = SEND;
}

void getRequestFromBuffer(int socket_index)
{
	const char del[9] = "\r\n";
	char buffer[MAX_LEN];
	char* token;
	char* requestLine = nullptr;
	int bodyIndex = 9, strtok_count = 0; //0=requestLine, 2=header, 3=body

	strncpy(buffer, sockets[socket_index].buffer, sockets[socket_index].request.requestLen);
	buffer[sockets[socket_index].request.requestLen] = '\0';
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

string GetTime()
{
	time_t timer;
	string time_str;
	time(&timer);
	time_str = ctime(&timer);
	return (time_str.substr(0, time_str.length() - 1));
}

void createResponseHeader(Response* response, int socket_index)
{
	string contentLength =  "";
	response->header += response->statusLine.version;
	response->header += response->statusLine.status;
	if (sockets[socket_index].request.requestLine.method == "OPTIONS")
	{
		response->header += "Allow: "; 
		response->header += Options() + "\r\n";
	}
	response->header += "Server:Apache\r\n";
	response->header += "Connection: keep - alive\r\n";
	response->header += "Connection: keep - alive\r\n";

	string currTime = GetTime();
	response->header += "Date:";
	response->header += currTime + "\r\n" + "Content-length:";
	contentLength = to_string(response->body.length());
	response->header += contentLength + "\r\n\r\n";
}

string setRespondInBuffer(Response* response)
{
	string sendBuff = { response->statusLine.version + response->statusLine.status + "\r\n" + response->header
	+ response->body };

	return sendBuff;
}

void deleteReqFromBuffer(int index)
{
	int reqLen = sockets[index].request.requestLen;
	if (sockets[index].request.requestLen <= sockets[index].len)
	{
		sockets[index].buffer[0] = '\0';
	}
	else 
	{
		memcpy(sockets[index].buffer, sockets[index].buffer + reqLen + 1, sockets[index].len - reqLen);
	}
	sockets[index].len -= reqLen - 1; 
} 

void clearCurrRequest(int index)
{
	sockets[index].request.body.clear();
	sockets[index].request.header.clear();
	sockets[index].request.requestLen = 0;
	sockets[index].request.requestLine.lang[0] = '\0';
	sockets[index].request.requestLine.uri[0] = '\0';
}

response createTimeOutResponse(int socket_index)
{
	response newResponse;
	newResponse.statusLine.status = status[408];
	newResponse.body.clear();
	createResponseHeader(&newResponse, socket_index);
	return newResponse;
}

void sendMessage(int socket_index)
{
	Response response; 
	char sendBuffer[MAX_LEN];
	int bytesSent = 0;
	SOCKET socket = sockets[socket_index].id;
	time_t currTime;

	if (difftime(time(&currTime), sockets[socket_index].request.startTime) > TIMEOUT)
	{
		response = createTimeOutResponse(socket_index);
		removeSocket(socket_index);
	}
	else
	{
		string method = sockets[socket_index].request.requestLine.method;

		if (method == "GET")
		{
			Get(socket_index, &response);
		}
		else if (method == "POST")
		{
			Post(socket_index, response);
		}
		else if (method == "PUT")
		{
			Put(socket_index, &response);
		}
		else if (method == "DELETE")
		{
			Delete_func(socket_index, response);
		}
		else if (method == "OPTION")
		{
			response.statusLine.status = status[200];
		}
		else if (method == "TREACE")
		{
			Trace(socket_index, &response);
		}

		createResponseHeader(&response, socket_index);
	}

	string sendBuff = setRespondInBuffer(&response);
	strcpy(sendBuffer,sendBuff.c_str());
	bytesSent = send(socket, sendBuffer, (int)strlen(sendBuffer), 0);
	CheckMessage(bytesSent, socket, "send");
	cout << "Web Server: Sent: " << bytesSent << "\\" << strlen(sendBuffer) << " bytes\n";
	deleteReqFromBuffer(socket_index);
	clearCurrRequest(socket_index);

	if (sockets[socket_index].len == 0)
	{
		sockets[socket_index].send = IDLE;
	}
	else
	{
		sockets[socket_index].send = SEND;
	}
}

void main()
{
	WSAData wsaData = InitWinsock();
	SOCKET listenSocket = CreateSocket();
	sockaddr_in serverService = CreateSocketAdd(listenSocket);

	if (SOCKET_ERROR == listen(listenSocket, 5))
	{
		cout << "Web Server: Error at listen(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}
	addSocket(listenSocket, LISTEN);

	while (true)
	{
		fd_set waitRecv = createRecvSet();
		fd_set waitSend = createSendSet();

		int nfd = checkEvents(&waitRecv, &waitSend);
		handleEvents(nfd, &waitRecv, &waitSend);
	}

	// Closing connections and Winsock.
	cout << "Web Server: Closing Connection.\n";
	closesocket(listenSocket);
	WSACleanup();
}
