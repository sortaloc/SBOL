#pragma once
#include "Logger.h"
#include "serverpacket.h"
#include <mutex>

typedef struct st_messagequeue {
	int32_t socket;
	uint8_t buffer[CLIENT_BUFFER_SIZE];
} MESSAGE_QUEUE;
typedef struct st_sendmanagementqueue {
	uint8_t sndbuf[CLIENT_BUFFER_SIZE];
} SEND_MANAGEMENT_QUEUE;

class ManagementServer
{
	mutex _muManagement;
	mutex _muPackets;
public:
	ManagementServer();
	~ManagementServer();
	uint32_t Start(void* serverptr);
	void Restart();
	uint32_t Stop();
	static void managementServerThread(void* parg);
	void CheckManagementPackets(unsigned rcvcopy, unsigned char * tmprcv);
	int plySockfd;
	BOOL todc;
	BOOL isAuth;
	void setAuth() { isAuth = TRUE; }
	void clearAuth() { isAuth = FALSE; }
	int32_t snddata;
	uint32_t connection_index;
	uint16_t rcvread;
	uint16_t packetSize;
	uint16_t expect;
	uint8_t IP_Address[16]; // Text version
	uint8_t ipaddr[4]; // Binary version
	uint8_t rcvbuf[CLIENT_BUFFER_SIZE];
	uint8_t sndbuf[CLIENT_BUFFER_SIZE];
	uint8_t key[32];
	uint8_t iv[16];
	SERVERPACKET inbuf;
	SERVERPACKET outbuf;
	void initialize();
	uint32_t Send(SERVERPACKET* src = nullptr);
	void Disconnect();
	void ProcessPacket();
	string compress(string in);
	string decompress(string in);
	bool isRunning() { return running; }
	bool isConfigured() { return configured; }
	bool shouldRetry();
	void setServerHandle(HANDLE* ptr) { serverHandle = ptr; }
	Logger* logger;
	void addToMessageQueue(MESSAGE_QUEUE in);
	MESSAGE_QUEUE getTopFromMessageQueue();
	uint32_t messagesInQueue();
	void addToSendQueue(SEND_MANAGEMENT_QUEUE in);
	SEND_MANAGEMENT_QUEUE getFromSendQueue();
	uint32_t messagesInSendQueue();
#pragma region Packets
	void SendAuth();
#pragma endregion
private:
	vector<MESSAGE_QUEUE> messagequeue;
	vector<SEND_MANAGEMENT_QUEUE> sendQueue;
	uint32_t LoadKey();
	HANDLE serverHandle;
	time_t lastConnectAttempt;
	wstring toWide(string in);
	string toNarrow(wstring in);
	int32_t tcp_set_nonblocking(int fd);
	void* server;
	bool running;
	bool configured;
};

