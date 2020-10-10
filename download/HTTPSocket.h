#ifndef __HTTP_SOCKET_H__
#define __HTTP_SOCKET_H__

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "FileWriter.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

using namespace std;

class HTTPSocket
{
private:
	string HOST;
	string PORT;
	string PARAM;
	long long START;
	long long END;
	FileWriter fwriter;

	SOCKET nSocket;
	addrinfo* pList;
	string headers;
	char buffer[2];

public:
	~HTTPSocket();
	HTTPSocket(string, string, string, shared_ptr<FileGuard>, long long, long long);

	int InitWinSocket();
	int ResolveAddress();
	int CreateNewSocket();
	int RequestContent();
	long long GetContentSize();
	void GetContentHead();
	void DownloadContent();
	void CleanUpSocket();

};

#endif

