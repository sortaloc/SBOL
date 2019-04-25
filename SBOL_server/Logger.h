#pragma once
#include "globals.h"
#include <iostream>
#include <fstream>
#include <string>

#define PACKET_BUFFER_SIZE 64000 * 16

using namespace std;

class Logger
{
public:
	Logger();
	Logger(char* path);
	~Logger();
	void Log(LOGTYPE type, const wchar_t* in, ...);
	void setLogPath(char* in);
	void setLogPath(string in);
	bool isLogPathSet();
	string getLogPath();
	wchar_t * packet_to_text(unsigned char* buf, uint32_t len);
	wstring toWide(string in);
	string toNarrow(wstring in);
private:
	string logpath;
	wchar_t dp_w[PACKET_BUFFER_SIZE];
};

