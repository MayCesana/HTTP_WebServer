#pragma once
#include "SocketsState.h"
#include "Declarations.h"

using namespace std;

const string methods[NUM_OPTIONS] = { "OPTIONS", "GET", "HEAD", "POST", "PUT", "DELETE", "TRACE" };

//Class represents the response
//creates the response message according to the method in the request 
class Response
{
private:
	typedef struct responseLine
	{
		const char* version = "HTTP/1.1 ";
		string status;
	}ResponseLine;

	ResponseLine statusLine;
	string header;
	string body;
public:
	Response() : header(""), body("") {}
	Response(const Response& r) = default;

	const char* getVersion() { return this->statusLine.version; }
	string getStatus() { return this->statusLine.status; }
	string getHeader() { return this->header; }
	string getBody() { return this->body; }

	void setStatus(string status) { this->statusLine.status = status; }
	void setBody(string body) { this->body = body; }
	void clearBody() { this->body.clear(); }
	void createResponseHeader(SocketsState& sockets, int socket_index);

	Response createTimeOutResponse(SocketsState& sockets, int socket_index);
	string returnRespondAsBuffer();

	string Options();
	void Get(SocketsState& sockets, int socket_index);
	void Trace(SocketsState& sockets, int index);
	void Head(SocketsState& sockets, int index);
	void Put(SocketsState& sockets, int index);
	void Delete_func(SocketsState& sockets, int index);
	void Post(SocketsState& sockets, int index);

	void selectMethod(SocketsState& sockets, int socket_index);
	char* createFilePath(SocketsState& sockets, int index);
	string GetTime();
};

