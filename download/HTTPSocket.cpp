#include "HTTPSocket.h"

HTTPSocket::HTTPSocket(string host, string port, string param, shared_ptr<FileGuard> syncfile, long long start, long long end)
	: fwriter(syncfile)
{
	HOST = host;
	PORT = port;
	PARAM = param;

	START = start;
	END = end;

	headers = "";
	nSocket = INVALID_SOCKET;
	pList = NULL;
}
HTTPSocket::~HTTPSocket()
{
	CleanUpSocket();
}


int HTTPSocket::InitWinSocket()
{
	WSADATA wsData;

	int error = WSAStartup(MAKEWORD(2, 2), &wsData);

	if (error != 0) throw "WSAStartup Failed!";

	return 0;
}

int HTTPSocket::ResolveAddress()
{
	struct addrinfo pHints;

	ZeroMemory(&pHints, sizeof(pHints));
	pHints.ai_family = AF_INET;
	pHints.ai_socktype = SOCK_STREAM;
	pHints.ai_protocol = IPPROTO_TCP;

	int error = getaddrinfo(HOST.c_str(), PORT.c_str(), &pHints, &pList);

	if (error != 0) throw "Error Resolving Address!";

	return 0;
}

int HTTPSocket::CreateNewSocket()
{
	int error = SOCKET_ERROR;

    for (addrinfo* it = pList; it != NULL; it = it->ai_next) 
    {
        nSocket = socket(it->ai_family, it->ai_socktype,it->ai_protocol);

        if (nSocket == INVALID_SOCKET) throw "Error Creating Socket!";

        error  = connect(nSocket, it->ai_addr, (int)it->ai_addrlen);

		if (error != SOCKET_ERROR) break;
    }

	freeaddrinfo(pList);

	if (error == SOCKET_ERROR) throw "Error Connecting To Server!";

	return 0;
}

int HTTPSocket::RequestContent()
{
	string request = "GET " + PARAM + "\r\n" + "HOST: " + HOST + "\r\n";

	string RANGE = "bytes=" + to_string(START) + '-' + to_string(END);

	if (END) request += "RANGE: " + RANGE + "\r\n";

	request += "\r\n\r\n";

	int error = send(nSocket, request.c_str(), (int)request.length(), 0);

	if (error == SOCKET_ERROR) throw "Error Sending Request!";

	return 0;
}

void HTTPSocket::GetContentHead()
{
	while (1)
	{
		int byte = recv(nSocket, buffer, 1, 0);

		buffer[byte] = 0;

		headers += string(buffer);

		if (headers.length() < 4) continue;

		size_t pos = headers.find("\r\n\r\n");

		if (pos != -1) break;
	}
}

long long HTTPSocket::GetContentSize()
{
	GetContentHead();

	size_t pos = headers.find("Content-Length");

	string bsize = headers.substr(pos + 16);

	pos = bsize.find("\r\n");

	return stoll(bsize.erase(pos));
}

void HTTPSocket::DownloadContent()
{
	GetContentHead();

	for (long long i = START; i <= END; i++)
	{
		int byte = recv(nSocket, buffer, 1, 0);

		buffer[byte] = 0;

		fwriter.fwrite(buffer, i);
	}
}

void HTTPSocket::CleanUpSocket()
{
	if (nSocket != INVALID_SOCKET) closesocket(nSocket);

	WSACleanup();
}