
#include "HTTPSocket.h"
#include "ThreadPool.h"

string PORT;
string HOST;
string PARAM;
string FDIR;
string URL;

shared_ptr<FileGuard> SFILE = NULL;

long long fsize = 0;
int nThreads = 1;
int nConnect = 1;

void GetFileChunk(long long START, long long END)
{
	HTTPSocket* IDM = new HTTPSocket(HOST, PORT, PARAM, SFILE, START, END);

	try
	{
		IDM->InitWinSocket();
		IDM->ResolveAddress();
		IDM->CreateNewSocket();
		IDM->RequestContent();

		if (!END)
		{
			fsize = IDM->GetContentSize();
		}
		else IDM->DownloadContent();
	}
	catch (const char* error)
	{
		printf(error); printf("\n");
	}

	delete IDM;
}


void BuildThreadPool()
{
	GetFileChunk(0, 0);

	ThreadPool TPool(nThreads);

	long long size = fsize / (long long)nConnect;

	for (long long i = 0; i < nConnect; i++)
	{
		long long start = i * size;
		long long end = start + size - 1;

		if (i + 1 == nConnect) end = fsize - 1;

		TPool.Enqueue(GetFileChunk, start, end);
	}
}

void GetProtocol(string protocol)
{
	if (protocol == "http")
	{
		PORT = "80";
	}
	else if (protocol == "https")
	{
		PORT = "443";
	}
	else PORT = "0";
}

void ExtractURL()
{
	size_t pos = URL.find("://");

	PORT = URL.substr(0, pos);

	GetProtocol(PORT);

	URL = URL.erase(0, pos + 3);

	pos = URL.find("/");
	
	HOST = URL.substr(0, pos);

	PARAM = URL.erase(0, pos);

	PARAM = PARAM + " HTTP/1.0";
}


int main(int argc, char** argv)
{
	for (int i = 1; i < argc; i++)
	{
		string input = string(argv[i]);

		if (input.find("--url") == 0)
		{
			URL = input.substr(6);
		}
		else if (input.find("--out") == 0)
		{
			FDIR = input.substr(6);
		}
		else if (input.find("--thread") == 0)
		{
			nThreads = stoi(input.substr(9));
		}
		else if (input.find("--conn") == 0)
		{
			nConnect = stoi(input.substr(7));
		}
	}

	SFILE = make_shared<FileGuard>(FDIR);

	ExtractURL();

	BuildThreadPool();

	return 0;
}
