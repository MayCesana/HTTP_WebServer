#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "functions.h"

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