#pragma once
#include <nlohmann\json.hpp>
#include <crytopp\cryptlib.h>
#include <crytopp\sha.h>
#include <crytopp\hex.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <exception>
#include <thread>
#include "globals.h"
#include "Logger.h"
#include "client.h"
#include "ManagementServer.h"

using namespace std;
using json = nlohmann::json;

extern int tempValue1;
extern int tempValue2;
extern int tempValue3;
extern int tempValue4;

class Server
{
public:
	Server();
	~Server();
	vector<CLIENT*> connections;
	Logger logger;
	ManagementServer managementserver;
	char* HEXString(uint8_t* in, char* out, uint32_t length);
	char* HEXString(uint8_t* in, uint32_t length);
	int32_t Start();
	int32_t Stop();
	void initialize();
	int32_t LoadConfig(const wchar_t* filename);
	bool isRunning() { return running; }
	static void serverThread(void* parg);
	void CheckClientPackets(CLIENT* client, unsigned rcvcopy, unsigned char * tmprcv);
	void initialize_connection(CLIENT* connect);
	int32_t tcp_sock_open(struct in_addr ip, uint16_t port);
	int32_t tcp_set_nonblocking(int fd);
	int32_t tcp_listen(int sockfd);
	int32_t tcp_accept(int sockfd, struct sockaddr *client_addr, int *addr_len);
	int32_t free_connection();
	int32_t server_sockfd;
	int32_t temp_sockfd;
	uint32_t getServerNumConnections() { return serverNumConnections; }
	vector<uint32_t> startingCars;
	string getManagementServerAddress() { return managementServerAddress; }
	uint16_t getManagementServerPort() { return managementServerPort; }
	void saveClientData(CLIENT* client);
	vector<string> split(string in, string delimit);
private:
	vector<uint32_t> serverConnections;
	uint32_t serverNumConnections;
	bool running;
	wstring toWide(string in);
	string toNarrow(wstring in);
	char hexStr[(MAX_MESG_LEN + 1) * 2];
	const wchar_t* CONFIG_FILENAME = L"config.json";
	uint16_t serverPort;
	string serverName;
	uint32_t serverClientLimit;
	string serverWelcomeMessage;
	uint16_t managementServerPort;
	string managementServerAddress;
	int32_t VerifyConfig();
};

