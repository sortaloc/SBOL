#define _WINSOCKAPI_
#include <Windows.h>
#include "globals.h"
#include "Logger.h"

const wchar_t* LOGFILES[]{
	nullptr,
	L"packet",
	L"error",
	L"debug",
	L"comm",
	L"client",
	L"management"
};

Logger::Logger()
{
	logpath = ".\\log";
}
Logger::Logger(char* path)
{
	logpath = path;
}
Logger::~Logger()
{
}
void Logger::Log(LOGTYPE type, const wchar_t* in, ...)
{
	try
	{
		bool isDebug = false;
#ifdef _DEBUG
		isDebug = true;
#endif
		va_list args;
		wchar_t text[MAX_MESG_LEN];
		wchar_t logbuf[FILENAME_MAX];
		wchar_t buf[MAX_MESG_LEN];

		SYSTEMTIME rawtime;

		GetLocalTime(&rawtime);
		va_start(args, in);
		vswprintf(text, MAX_MESG_LEN - 10, in, args);
		va_end(args);

		wcscat_s(text, L"\n");
		swprintf(&buf[0], MAX_MESG_LEN, L"[%02u-%02u-%u, %02u:%02u:%02u] %s", rawtime.wMonth, rawtime.wDay, rawtime.wYear, rawtime.wHour, rawtime.wMinute, rawtime.wSecond, text);

		wprintf(buf);

		if ((type == LOGTYPE_CLIENT || type == LOGTYPE_ERROR || type == LOGTYPE_PACKET || (type == LOGTYPE_DEBUG && isDebug)) && logpath.length() > 0)
		{
			swprintf(&logbuf[0], MAX_MESG_LEN, L"%s\\%s%02u%02u%04u.log", toWide(logpath).c_str(), LOGFILES[type], rawtime.wMonth, rawtime.wDay, rawtime.wYear);
			wfstream logFile(logbuf, ios::app | ios::ate);
			if (logFile.is_open())
			{
				logFile << buf;
				logFile.close();
			}
			else
			{
				wprintf(L"Error opening logfile: %s", logbuf);
			}
		}
	}
	catch (exception ex)
	{
		wprintf(L"Error writing to logfile: %s", ex.what());
	}
}
wstring Logger::toWide(string in)
{
	wstring temp(in.length(), L' ');
	copy(in.begin(), in.end(), temp.begin());
	return temp;
}
string Logger::toNarrow(wstring in)
{
	string temp(in.length(), ' ');
	copy(in.begin(), in.end(), temp.begin());
	return temp;
}
void Logger::setLogPath(char* in)
{
	logpath = in;
}
void Logger::setLogPath(string in)
{
	logpath = in;
}
string Logger::getLogPath()
{
	return logpath;
}
bool Logger::isLogPathSet()
{
	return (logpath.length() > 0) ? true : false;
}
wchar_t * Logger::packet_to_text(unsigned char* buf, uint32_t len)
{
	try
	{
		if (len > PACKET_BUFFER_SIZE)
		{
			ZeroMemory(&dp_w, PACKET_BUFFER_SIZE);
			return (wchar_t*)&dp_w[0];
		}
		int c, c2, c3, c4;

		c = c2 = c3 = c4 = 0;
		swprintf(&dp_w[c2++], CLIENT_BUFFER_SIZE - c2, L"\n");
		for (c = 0; c < len; c++)
		{
			if (c3 == 16)
			{
				for (; c4 < c; c4++)
					if (buf[c4] >= 0x20)
						dp_w[c2++] = buf[c4];
					else
						dp_w[c2++] = 0x002E;
				c3 = 0;
				swprintf(&dp_w[c2++], PACKET_BUFFER_SIZE - c2, L"\n");
			}

			if ((c == 0) || !(c % 16))
			{
				swprintf(&dp_w[c2], PACKET_BUFFER_SIZE - c2, L"(%04X) ", c);
				c2 += 7;
			}

			swprintf(&dp_w[c2], PACKET_BUFFER_SIZE - c2, L"%02X ", buf[c]);
			c2 += 3;
			c3++;
		}

		if (len % 16)
		{
			c3 = len;
			while (c3 % 16)
			{
				swprintf(&dp_w[c2], PACKET_BUFFER_SIZE - c2, L"   ");
				c2 += 3;
				c3++;
			}
		}

		for (; c4 < c; c4++)
			if (buf[c4] >= 0x20)
				dp_w[c2++] = buf[c4];
			else
				dp_w[c2++] = 0x002E;
		swprintf(&dp_w[c2++], PACKET_BUFFER_SIZE - c2, L"\n");
		dp_w[c2] = 0;
		return (wchar_t*)&dp_w[0];
	}
	catch (exception ex) { }
}