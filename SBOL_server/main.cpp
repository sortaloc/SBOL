#define _WINSOCKAPI_
#include <Windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

int tempValue1 = 0;
int tempValue2 = 0;
int tempValue3 = 0;
int tempValue4 = 0;

#include "strings.h"
#include "globals.h"
#include "server.h"
#include "main.h"

using namespace std;

int main()
{
	Server server;
	server.logger.Log(LOGTYPE_NONE, L"SBOL Server version %s by Tofuman", VERSION_STRING);
	server.logger.Log(LOGTYPE_NONE, L"-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");
	if (server.Start())
	{
		server.logger.Log(LOGTYPE_ERROR, L"Server failed to start");
		return 1;
	}

	while (server.isRunning())
	{
		if (server.managementserver.isConfigured() && !server.managementserver.isRunning() && server.managementserver.shouldRetry())
			server.managementserver.Restart();
		this_thread::sleep_for(1ms);
	}
	return 0;
}

//TODO: Add Rival List status to client data & database
//TODO: Add Team Items to client data & database
//TODO: Support the team garages/extended garages and store in database
//TODO: retrieve the S.DAT and P.DAT from management server to use when client purchases something from shop

