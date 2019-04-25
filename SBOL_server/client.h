#pragma once
#include "packet.h"
#include "serverpacket.h"
#include <mutex>
#include <vector>

typedef uint16_t ITEMBOX_ITEM;
#pragma region Structures
typedef struct st_careerdata {
	uint8_t level;
	uint8_t experiencePercent;
	uint32_t experiencePoints;
	uint16_t rivalWin;
	uint16_t rivalLose;
	uint16_t playerWin;
	uint16_t playerLose;
	uint64_t CP;
} CAREERDATA;
typedef struct st_teamdata {
	uint32_t teamID;
	uint8_t unknown1[5];
	char name[18];
	char comment[41];
	uint8_t memberCount;
	uint8_t unknown3;
	uint8_t unknown4;
	uint8_t unknown5;
	uint32_t survivalWins;
	uint32_t survivalLoses;
	char leaderName[16];
	uint8_t unknown2[12];
} TEAMDATA;
typedef struct st_carmods {
	float color1_R;
	float color1_G;
	float color1_B;
	float color2_R;
	float color2_G;
	float color2_B;
	int16_t engine;
	int16_t unknown1;
	int16_t unknown2;
	int16_t unknown3;
	int16_t arches;
	int16_t bonnet;
	int16_t sideskirts;
	int16_t rearwing;
	int16_t headlights;
	int16_t unknown4;
	int16_t unknown5;
	int16_t wheels;
	int16_t unknown6;
	int16_t unknown7;
	int16_t unknown8;
	int16_t unknown9;
	int16_t unknown10;
	int16_t unknown11;
	int16_t unknown12;
	int16_t unknown13;
	int16_t unknown14;
	int16_t unknown15;
	int16_t unknown16;
	int16_t unknown17;
	int16_t unknown18;
	int16_t unknown19;
	int16_t stickers;
	int16_t unknown20;
	int32_t unknown21;
	int32_t unknown22;
	int32_t unknown23;
	int32_t unknown24;
} CARMODS;
typedef struct st_car {
	uint32_t carID;
	uint32_t KMs; // Divided by 10.
	uint32_t engineCondition;
	CARMODS carmods;
} CAR;
typedef struct st_garage {
	CAR* activeCar;
	uint32_t activeCarBay;
	vector<CAR> car;
} GARAGE;
typedef struct st_sendqueue {
	uint8_t sndbuf[CLIENT_BUFFER_SIZE];
} SEND_QUEUE;
#pragma endregion

class CLIENT
{
	mutex _muClient;
public:
	CLIENT();
	~CLIENT();
	Logger* logger;
	void* server;
	int32_t plySockfd;
	BOOL todc;
	BOOL canSave;
	int32_t snddata;
	uint32_t connection_index;
	uint32_t driverslicense;
	uint8_t privileges;
	uint32_t timeoutCount;
	uint32_t ping;
	uint32_t pokes;
	uint16_t rcvread;
	uint16_t packetSize;
	uint16_t expect;
	uint8_t IP_Address[16]; // Text version
	uint8_t ipaddr[4]; // Binary version
	string handle;
	string username;
	TEAMDATA teamdata;
	GARAGE garagedata;
	CAREERDATA careerdata;
	vector<ITEMBOX_ITEM> itembox;
	PACKET inbuf;
	PACKET outbuf;
	SERVERPACKET serverbuf;
	uint8_t rcvbuf[CLIENT_BUFFER_SIZE];
	uint8_t sndbuf[CLIENT_BUFFER_SIZE];
	void addToSendQueue(SEND_QUEUE in);
	SEND_QUEUE getFromSendQueue();
	uint32_t messagesInSendQueue();
	void initialize();
	void initializeGarage();
	int8_t getCarCount();
	int32_t setActiveCar(int32_t slot);
	int32_t getActiveCar() { return garagedata.activeCarBay; };
	int32_t setHandle(string in);
	int32_t setUsername(string in);
	void Send(PACKET* src = nullptr);
	void Disconnect();
	void ProcessPacket();
	void ProcessManagementPacket();
	int test1;
	int test2;
#pragma region Client Packets
	void SendAuthError(string cmd);
	void SendAuthError(uint8_t cmd);
#pragma endregion
private:
	vector<SEND_QUEUE> sendQueue;
};

