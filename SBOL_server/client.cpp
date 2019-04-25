#define _WINSOCKAPI_
#include <Windows.h>
#include "globals.h"
#include "Logger.h"
#include "client.h"
#include "packet.h"
#include "serverpacket.h"
#include "packets.h"
#include "managementpackets.h"

CLIENT::CLIENT()
{
	test1 = 0;
	test2 = 0;
	plySockfd = -1;
	todc = FALSE;
	canSave = FALSE;
	snddata = 0;
	connection_index = 0;
	driverslicense = 0;
	privileges = 0;
	rcvread = 0;
	packetSize = 0;
	expect = 0;
	server = nullptr;
	timeoutCount = 0;
	ping = 0;
	pokes = 0;
	ZeroMemory(&IP_Address[0], sizeof(IP_Address));
	ZeroMemory(&ipaddr[0], sizeof(ipaddr));
	handle = "";
	ZeroMemory((char *)&teamdata, sizeof(teamdata));
	inbuf.clearBuffer();
	outbuf.clearBuffer();
	ZeroMemory(&rcvbuf[0], sizeof(rcvbuf));
	ZeroMemory(&sndbuf[0], sizeof(sndbuf));
}
CLIENT::~CLIENT()
{
	itembox.clear();
	sendQueue.clear();
}
void CLIENT::initialize()
{
	test1 = 0;
	test2 = 0;
	plySockfd = -1;
	todc = FALSE;
	canSave = FALSE;
	snddata = 0;
	connection_index = 0;
	driverslicense = 0;
	privileges = 0;
	rcvread = 0;
	packetSize = 0;
	expect = 0;
	timeoutCount = 0;
	ping = 0;
	pokes = 0;
	ZeroMemory(&IP_Address[0], sizeof(IP_Address));
	ZeroMemory(&ipaddr[0], sizeof(ipaddr));
	handle = "";
	ZeroMemory((char *)&teamdata, sizeof(teamdata));
	inbuf.clearBuffer();
	outbuf.clearBuffer();
	ZeroMemory(&rcvbuf[0], sizeof(rcvbuf));
	ZeroMemory(&sndbuf[0], sizeof(sndbuf));
	itembox.clear();
	sendQueue.clear();
}
void CLIENT::initializeGarage()
{
	garagedata.activeCar = nullptr;
	garagedata.activeCarBay = 0;
	garagedata.car.resize(GARAGE_LIMIT);
	for (int i = 0; i < GARAGE_LIMIT; i++)
	{
		garagedata.car[i].carID = 0xFFFFFFFF;
	}
}
int8_t CLIENT::getCarCount()
{
	int8_t count = 0;
	for (int8_t i = 0; i < GARAGE_LIMIT; i++)
	{
		if (garagedata.car[i].carID != 0xFFFFFFFF) count++;
	}
	return count;
}
int32_t CLIENT::setActiveCar(int32_t slot)
{
	garagedata.activeCarBay = slot % GARAGE_LIMIT;
	garagedata.activeCar = &garagedata.car[garagedata.activeCarBay];

	return (garagedata.activeCar) ? 0 : 1;
}
int32_t CLIENT::setHandle(string in)
{
	handle = in.substr(0, 15);
	return 0;
}
int32_t CLIENT::setUsername(string in)
{
	username = in.substr(0, 15);
	return 0;
}
void CLIENT::SendAuthError(string cmd)
{
	outbuf.clearBuffer();
	outbuf.setSize(0x06);
	outbuf.setOffset(0x06);
	outbuf.setType(0x100);
	outbuf.setSubType(0x181);
	outbuf.appendByte(0);
	outbuf.appendString(cmd, 0x78);
	Send();
}
void CLIENT::SendAuthError(uint8_t cmd)
{
	outbuf.clearBuffer();
	outbuf.setSize(0x7B);
	outbuf.setOffset(0x06);
	outbuf.setType(0x100);
	outbuf.setSubType(0x181);
	outbuf.appendByte(cmd);
	Send();
}
void CLIENT::Send(PACKET* src)
{
	if (src == nullptr)
		src = &outbuf;
	if (plySockfd >= 0)
	{
		if (CLIENT_BUFFER_SIZE < ((int)src->getSize() + 15))
			Disconnect();
		else
		{
			uint16_t size = src->getSize();
			if (size == 0) return;
			*(uint16_t*)&src->buffer[0x00] = SWAP_SHORT(size - 2);
			SEND_QUEUE entry = { 0 };
			memcpy(&entry.sndbuf[0], &src->buffer[0x00], size);
			addToSendQueue(entry);
#ifdef PACKET_OUTPUT
			if (src->getType() != 0x0A00)
			{
				logger->Log(LOGTYPE_PACKET, L"Packet: Server -> Client");
				logger->Log(LOGTYPE_PACKET, logger->packet_to_text(&src->buffer[0x00], size));
			}
#endif
			src->setSize(0);
		}
	}
}
void CLIENT::Disconnect()
{
	todc = TRUE;
}
void CLIENT::ProcessPacket()
{
	if (!todc)
	{
		uint32_t packetType = (inbuf.getType() >> 8) & 0xFF;
		if (packetType > sizeof(MainPacketFunctions) / 4)
		{
			logger->Log(LOGTYPE_COMM, L"Invalid Packet Message: %04X from client %s", inbuf.getType(), logger->toWide((char*)IP_Address).c_str());
			Disconnect();
			return;
		}
#ifdef PACKET_OUTPUT
		//if(packetType == 0x07)
		if (packetType != 0x0A && packetType != 0x07 && packetType != 0x00 && packetType != 0x04)
		//if (packetType == 0x07 || packetType == 0x04)
		{
			logger->Log(LOGTYPE_PACKET, L"Packet: Client -> Server");
			logger->Log(LOGTYPE_PACKET, logger->packet_to_text(&inbuf.buffer[0x00], inbuf.getSize()));
		}
#endif
		MainPacketFunctions[packetType](this);
		timeoutCount = 0;
#ifdef PACKET_OUTPUT
		//if (packetType != 0x07) return;
		if (packetType == 0x07 || packetType == 0x00 || packetType == 0x0A || packetType == 0x04) return;
		//if (packetType != 0x07 || packetType != 0x04) return;
		else if ((inbuf.getType() & 0xFF00) == (outbuf.getType() & 0xFF00)) logger->Log(LOGTYPE_COMM, L"Packet Message: %04X -> %04X", inbuf.getType(), outbuf.getSubType());
		else logger->Log(LOGTYPE_COMM, L"Packet Message: %04X -> NO RESPONSE", inbuf.getType());
#endif
	}
}
void CLIENT::ProcessManagementPacket()
{
	if (!todc)
	{
		uint32_t packetType = serverbuf.getType();
		if (packetType > sizeof(ManagementPacketFunctions) / 4)
		{
			logger->Log(LOGTYPE_COMM, L"Invalid Packet Message: %04X from management server", serverbuf.getType());
			Disconnect();
			return;
		}
#ifdef PACKET_OUTPUT
		logger->Log(LOGTYPE_PACKET, L"Packet: Management -> Client");
		logger->Log(LOGTYPE_PACKET, logger->packet_to_text(&serverbuf.buffer[0x00], serverbuf.getSize()));
#endif
		ManagementPacketFunctions[packetType](this);
	}
}
void CLIENT::addToSendQueue(SEND_QUEUE in)
{
	lock_guard<mutex> locker(_muClient);
	sendQueue.push_back(in);
}
SEND_QUEUE CLIENT::getFromSendQueue()
{
	lock_guard<mutex> locker(_muClient);
	if (sendQueue.size())
	{
		SEND_QUEUE extract = sendQueue.at(0);
		sendQueue.erase(sendQueue.begin());
		return extract;
	}
	else
	{
		SEND_QUEUE extract = { 0 };
		return extract;
	}
}
uint32_t CLIENT::messagesInSendQueue()
{
	lock_guard<mutex> locker(_muClient);
	return sendQueue.size();
}