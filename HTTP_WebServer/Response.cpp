#include "Response.h"

void Response::createResponseHeader(SocketsState& sockets, int socket_index)
{
	string contentLength = "";
	
	string currTime = GetTime();
	this->header += "Date: ";
	this->header += currTime + "\r\n" + "Content-Length: ";
	contentLength = to_string(this->body.length());
	this->header += contentLength + "\r\n";
	if (sockets[socket_index].request.requestLine.method == "OPTIONS")
	{
		this->header += "Allow: ";
		this->header += Options() + "\r\n";
	}
	this->header += "\r\n";
}

string Response::GetTime()
{
	time_t timer;
	string time_str;
	time(&timer);
	time_str = ctime(&timer);
	return (time_str.substr(0, time_str.length() - 1));
}

char* Response::createFilePath(SocketsState& sockets, int socket_index)
{
	char fileName[MAX_LEN] = { "C:/TEMP/" };
	strcat(fileName, sockets[socket_index].request.requestLine.uri);
	if (sockets[socket_index].request.requestLine.lang[0] != '\0')
	{
		strcat(fileName, ".");
		strcat(fileName, sockets[socket_index].request.requestLine.lang);
	}
	strcat(fileName, ".html");
	return fileName;
}

void Response::Get(SocketsState& sockets, int socket_index)
{
	char bodyBuff[MAX_LEN];
	char* filePath = createFilePath(sockets, socket_index);
	FILE* file = nullptr;

	file = fopen(filePath, "r");
	if (file == nullptr)
	{
		setStatus(Status[404]);
	}
	else
	{
		setStatus(Status[200]);
		while (!feof(file))
		{
			fgets(bodyBuff, MAX_LEN, file);
			this->body += bodyBuff;
		}

		fclose(file);
	}
}

void Response::Post(SocketsState& sockets, int socket_index)
{
	cout << sockets[socket_index].request.reqBody << endl;
	setStatus(Status[200]);
}

void Response::Delete_func(SocketsState& sockets, int socket_index)
{
	int deletedSucceed;
	char* filePath = createFilePath(sockets, socket_index);

	deletedSucceed = remove(filePath);

	if (deletedSucceed == 0)
	{
		setStatus(Status[200]);
	}
	else
	{
		setStatus(Status[404]);
	}
}

void Response::Put(SocketsState& sockets, int socket_index)
{
	FILE* file = nullptr;
	char* filePath = createFilePath(sockets, socket_index);

	file = fopen(filePath, "r");
	if (file == nullptr)
	{
		this->setStatus(Status[201]);
	}
	else //file exist
	{
		this->setStatus(Status[200]);
		fclose(file);
	}
	file = fopen(filePath, "w");
	if (file == nullptr)
	{
		this->setStatus(Status[501]);
	}
	else
	{
		fputs((sockets[socket_index].request.reqBody).c_str(), file);
		fclose(file);
	}
}

void Response::Head(SocketsState& sockets, int socket_index)
{
	char* filePath = createFilePath(sockets, socket_index);
	FILE* file = nullptr;

	file = fopen(filePath, "r");
	if (file == nullptr)
	{
		this->setStatus(Status[404]);
	}
	else
	{
		this->setStatus(Status[200]);
		fclose(file);
	}
}

void Response::Trace(SocketsState& sockets, int socket_index)
{
	this->setBody(sockets[socket_index].buffer.front().requestBuffer);
	this->setStatus(Status[200]);
}

string Response::Options()
{
	string optionsStr;

	for (int i = 0; i < NUM_OPTIONS; i++)
	{
		optionsStr += methods[i];
		if(i != NUM_OPTIONS - 1)
			optionsStr += ", ";
	}

	optionsStr += "\n";

	return optionsStr;
}

Response Response::createTimeOutResponse(SocketsState& sockets, int socket_index)
{
	Response newResponse;
	newResponse.setStatus(Status[408]);
	newResponse.clearBody();
	newResponse.createResponseHeader(sockets, socket_index);
	return newResponse;
}

string Response::returnRespondAsBuffer()
{
	string sendBuff = { statusLine.version + statusLine.status + "\r\n" + header + body };
	return sendBuff;
}

void Response::selectMethod(SocketsState& sockets, int socket_index)
{
	string method = sockets[socket_index].request.requestLine.method;

	if (method == "GET")
	{
		Get(sockets, socket_index);
	}
	else if (method == "POST")
	{
		Post(sockets, socket_index);
	}
	else if (method == "PUT")
	{
		Put(sockets, socket_index);
	}
	else if (method == "DELETE")
	{
		Delete_func(sockets, socket_index);
	}
	else if (method == "OPTIONS")
	{
		setStatus(Status[200]);
	}
	else if (method == "HEAD")
	{
		Head(sockets, socket_index);
	}
	else if (method == "TRACE")
	{
		Trace(sockets, socket_index);
	}
}