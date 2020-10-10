#ifndef __FILE_WRITER_H__
#define __FILE_WRITER_H__

#include <stdio.h>
#include <string>
#include <mutex>

using namespace std;

class FileGuard
{
private:
	FILE* file;
	mutex entry;

public:
	FileGuard(string path)
	{
		file = fopen(path.c_str(), "wb+");
	}

	~FileGuard()
	{
		fclose(file);
	}

	void write(char* buffer, long long pos)
	{
		lock_guard<mutex> lock(entry);

		_fseeki64(file, pos, SEEK_SET);

		fwrite(buffer, sizeof(char), 1, file);
	}
};

class FileWriter
{
private:
	shared_ptr<FileGuard> pFile;

public:
	FileWriter(shared_ptr<FileGuard> syncfile)
	{
		pFile = syncfile;
	}

	void fwrite(char* buffer, long long pos)
	{
		pFile->write(buffer, pos);
	}
};

#endif // !__FILE_WRITER_H__
