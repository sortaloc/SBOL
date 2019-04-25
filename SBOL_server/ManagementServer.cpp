#define _WINSOCKAPI_
#include <crytopp\cryptlib.h>
#include <crytopp\gzip.h>
#include <crytopp\modes.h>
#include <crytopp\filters.h>
#include <crytopp\aes.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <stdint.h>
#include <time.h>
#include "globals.h"
#include "server.h"
#include "serverpackets.h"
#include "ManagementServer.h"
#include <fstream>

ManagementServer::ManagementServer()
{
	server = nullptr;
	serverHandle = nullptr;
	plySockfd = -1;
	todc = FALSE;
	isAuth = FALSE;
	running = false;
	snddata = 0;
	connection_index = 0;
	rcvread = 0;
	packetSize = 0;
	expect = 0;
	ZeroMemory(&IP_Address[0], sizeof(IP_Address));
	ZeroMemory(&ipaddr[0], sizeof(ipaddr));
	inbuf.clearBuffer();
	outbuf.clearBuffer();
	ZeroMemory(&rcvbuf[0], sizeof(rcvbuf));
	ZeroMemory(&sndbuf[0], sizeof(sndbuf));
	ZeroMemory(&key[0], sizeof(key));
	ZeroMemory(&iv[0], sizeof(iv));
}
ManagementServer::~ManagementServer()
{
	messagequeue.clear();
	sendQueue.clear();
}
uint32_t ManagementServer::Start(void* serverptr)
{
	initialize();
	if (LoadKey())
	{
		logger->Log(LOGTYPE_MANAGEMENT, L"Management key failed to load.");
		return 1;
	}

	server = serverptr;
	if (!server)
		return 1;

	logger = &((Server*)server)->logger;
	configured = true;
	running = true;
	thread mThread(managementServerThread, this);
	mThread.detach();
	this_thread::sleep_for(10ms);
	logger->Log(LOGTYPE_MANAGEMENT, L"Management Server Thread Started.");


	return 0;
}
void ManagementServer::Restart()
{
	if (serverHandle) return;
	lastConnectAttempt = time(0);
	running = true;
	thread mThread(managementServerThread, this);
	mThread.detach();
	this_thread::sleep_for(10ms);
}
uint32_t ManagementServer::Stop()
{
	running = false;
	return 0;
}
uint32_t ManagementServer::LoadKey()
{
	ifstream keyfile(".\\serverkey.bin", ios::binary);
	if (!keyfile.is_open()) return 1;
	size_t size = (size_t)keyfile.tellg();
	keyfile.seekg(0, ios::end);
	size = (uint32_t)keyfile.tellg() - size;
	if (size != 48)
		return 1;
	keyfile.seekg(ios::beg);
	keyfile.read((char*)&key[0], 32);
	keyfile.read((char*)&iv[0], 16);
	keyfile.close();
	logger->Log(LOGTYPE_MANAGEMENT, L"Management key loaded.");
	return 0;
}
void ManagementServer::initialize()
{
	plySockfd = -1;
	todc = FALSE;
	isAuth = FALSE;
	snddata = 0;
	connection_index = 0;
	rcvread = 0;
	packetSize = 0;
	expect = 0;
	ZeroMemory(&IP_Address[0], sizeof(IP_Address));
	ZeroMemory(&ipaddr[0], sizeof(ipaddr));
	inbuf.clearBuffer();
	outbuf.clearBuffer();
	ZeroMemory(&rcvbuf[0], sizeof(rcvbuf));
	ZeroMemory(&sndbuf[0], sizeof(sndbuf));
	ZeroMemory(&key[0], sizeof(key));
	ZeroMemory(&iv[0], sizeof(iv));
	messagequeue.clear();
	sendQueue.clear();
}
uint32_t ManagementServer::Send(SERVERPACKET* src)
{
	if (src == nullptr)
		src = &outbuf;
	if (plySockfd >= 0)
	{
		if (isAuth == FALSE && src->getType() > 0x0000)
		{
			logger->Log(LOGTYPE_ERROR, L"Unabled to send packet to management server %s as not authenticated.", logger->toWide((char*)IP_Address).c_str());
			return 2;
		}
		if (CLIENT_BUFFER_SIZE < ((int32_t)src->getSize() + 15))
		{
			Disconnect();
			return 1;
		}
		else
		{
			uint16_t size = src->getSize();
			uint16_t newsize = 0;
			if ((size - 4) < 0 || size > CLIENT_BUFFER_SIZE)
			{
				logger->Log(LOGTYPE_ERROR, L"Unabled to send packet to client %s as packet will be %ubytes", logger->toWide((char*)IP_Address).c_str(), size);
				return 1;
			}
			src->setSize(size - 2);
			newsize = src->getSize();
			// Compress Packet
#ifndef DISABLE_COMPRESSION
			string compressed;	
			compressed.resize(newsize);
			memcpy((char*)compressed.data(), &src->buffer[0x02], newsize);
			compressed = compress(compressed);
			newsize = compressed.size();
			if (newsize > CLIENT_BUFFER_SIZE)
			{
				logger->Log(LOGTYPE_ERROR, L"Unabled to send packet to client %s as packet will be %ubytes", logger->toWide((char*)IP_Address).c_str(), newsize);
				return 1;
			}
			if (newsize)
			{
				inbuf.clearBuffer();
				inbuf.setArray((uint8_t*)compressed.data(), newsize, 0x02);
			}
			src->setSize(newsize);
#endif
			while (src->getSize() % 16 || src->getSize() <= 16)
				src->appendByte(0);
			newsize = src->getSize();
			if (isAuth == TRUE)
			{
				if (newsize + 2 + CryptoPP::AES::BLOCKSIZE > CLIENT_BUFFER_SIZE)
				{
					logger->Log(LOGTYPE_ERROR, L"Unabled to send packet to client %s as packet will be %ubytes", logger->toWide((char*)IP_Address).c_str(), newsize + 2 + CryptoPP::AES::BLOCKSIZE);
					return 1;
				}
				CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption enc(key, sizeof(key), iv);
				enc.ProcessData(&src->buffer[0x02], &src->buffer[0x02], newsize);
			}
			*(uint16_t*)&src->buffer[0x00] = newsize;
			SEND_MANAGEMENT_QUEUE entry = { 0 };
			memcpy(&entry.sndbuf[0], &src->buffer[0x00], min(newsize + 2, sizeof(sndbuf) - 2));
			addToSendQueue(entry);
			//memcpy(&sndbuf[0x00], &src->buffer[0x00], min(newsize + 2, sizeof(sndbuf) - 2));
			//snddata += newsize + 2;
		}
	}
	return 0;
}
void ManagementServer::Disconnect()
{
	todc = TRUE;
}
bool ManagementServer::shouldRetry()
{
	time_t now = time(0);
	time_t result = now - lastConnectAttempt;
	return (result > 10);
}
void ManagementServer::ProcessPacket()
{
	if (!todc)
	{
		try {
			int16_t size = inbuf.getSize();
			if (isAuth == TRUE)
			{
				CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption dec(key, sizeof(key), iv);
				dec.ProcessData(&inbuf.buffer[0x02], &inbuf.buffer[0x02], size);
			}

			// Decompress Packet
#ifndef DISABLE_COMPRESSION
			string decompressed;
			decompressed.resize(size);
			memcpy((char*)decompressed.data(), &inbuf.buffer[0x02], size);
			decompressed = decompress(decompressed);
			size = decompressed.size();
			if (size)
			{
				inbuf.clearBuffer();
				inbuf.setArray((uint8_t*)decompressed.data(), size, 0x02);
			}
			else
				return;
			inbuf.setSize(size);
#endif
		}
		catch (exception ex)
		{
			logger->Log(LOGTYPE_COMM, L"Invalid Packet Message: %04X from client %s", inbuf.getType(), logger->toWide((char*)IP_Address).c_str());
			Disconnect();
			return;
		}

		if (inbuf.getType() > sizeof(ManagementPacketFunctions) / 4 || (isAuth == FALSE && inbuf.getType() != PACKETTYPE_AUTH))
		{
			logger->Log(LOGTYPE_COMM, L"Invalid Packet Message: %04X from client %s", inbuf.getType(), logger->toWide((char*)IP_Address).c_str());
			Disconnect();
			return;
		}
#ifdef PACKET_OUTPUT
		logger->Log(LOGTYPE_MANAGEMENT, L"Packet: Management -> Server");
		logger->Log(LOGTYPE_MANAGEMENT, logger->packet_to_text(&inbuf.buffer[0], inbuf.getSize() + 2));
#endif
		ManagementPacketFunctions[inbuf.getType()](this);
	}
}
void ManagementServer::managementServerThread(void* parg)
{
	ManagementServer* managementserver = (ManagementServer*)parg;
	Server* gameserver = (Server*)managementserver->server;
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL, *ptr = NULL, hints;
	uint8_t sendbuf[CLIENT_BUFFER_SIZE];
	uint8_t recvbuf[CLIENT_BUFFER_SIZE];
	int32_t iResult, wserror;
	string managementPort;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		managementserver->logger->Log(LOGTYPE_MANAGEMENT, L"WSAStartup failed for management server with error %u", iResult);
		goto end;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	managementPort = static_cast<ostringstream*>(&(ostringstream() << gameserver->getManagementServerPort()))->str();
	iResult = getaddrinfo(gameserver->getManagementServerAddress().c_str(), managementPort.c_str(), &hints, &result);
	if (iResult != 0) {
		managementserver->logger->Log(LOGTYPE_MANAGEMENT, L"getaddrinfo failed for management server with error %u", iResult);
		goto end;
	}

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			managementserver->logger->Log(LOGTYPE_MANAGEMENT, L"socket failed for management server with error %ld", WSAGetLastError());
			goto end;
		}
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		managementserver->logger->Log(LOGTYPE_MANAGEMENT, L"unable to connect to management server on port %u", gameserver->getManagementServerPort());
		goto end;
	}
	managementserver->logger->Log(LOGTYPE_MANAGEMENT, L"Connected to management server on port %u", gameserver->getManagementServerPort());
	int32_t recv_len = 0;
	int32_t send_len = 0;
	int32_t max_send = 0;

	managementserver->plySockfd = ConnectSocket;
	managementserver->SendAuth();
	managementserver->tcp_set_nonblocking(ConnectSocket);

	managementserver->running = true;

	while (managementserver->isRunning())
	{
		// Send
		if (managementserver->messagesInSendQueue())
		{
			SEND_MANAGEMENT_QUEUE entry = managementserver->getFromSendQueue();
			managementserver->snddata = *(int16_t*)&entry.sndbuf[0] + 2;

			if (managementserver->snddata > TCP_BUFFER_SIZE)
				max_send = TCP_BUFFER_SIZE;
			else
				max_send = managementserver->snddata;

			memcpy(&managementserver->sndbuf[0], &entry.sndbuf[0], max_send);

			if ((send_len = send(managementserver->plySockfd, (const char *)&managementserver->sndbuf[0], max_send, 0)) == SOCKET_ERROR)
			{
				wserror = WSAGetLastError();
				if (wserror != WSAEWOULDBLOCK)
				{
					managementserver->logger->Log(LOGTYPE_MANAGEMENT, L"Could not send data to management server. Socket Error %d", wserror);
					break;
				}
			}
			else
			{
				managementserver->snddata -= send_len;

				if (managementserver->snddata > 0) // We didn't send it all?
				{
					memcpy(&sendbuf[0], &managementserver->sndbuf[send_len], managementserver->snddata);
					memcpy(&managementserver->sndbuf[0], &sendbuf[0], managementserver->snddata);
				}
			}
		}

		// Receive
		recv_len = recv(ConnectSocket, (char*)&managementserver->rcvbuf[managementserver->rcvread], TCP_BUFFER_SIZE, 0);
		if (recv_len <= 0)
		{
			wserror = WSAGetLastError();
			if (wserror != WSAEWOULDBLOCK)
			{
				managementserver->logger->Log(LOGTYPE_MANAGEMENT, L"Could not read data from management server. Socket Error %d", wserror);
				break;
			}
		}
		else
		{
			managementserver->rcvread += (uint32_t)recv_len;
			if (managementserver->rcvread >= 2)
			{
				uint16_t rcvcopy = managementserver->rcvread;
				memcpy(&recvbuf[0], &managementserver->rcvbuf[0], rcvcopy);
				managementserver->CheckManagementPackets(rcvcopy, &recvbuf[0]);
				managementserver->rcvread -= rcvcopy;

				if (managementserver->rcvread > 0)
				{
					// Anything still left in the receive buffer?  Copy it to the beginning.
					memcpy(&recvbuf[0], &managementserver->rcvbuf[rcvcopy], managementserver->rcvread);
					memcpy(&managementserver->rcvbuf[0], &recvbuf[0], managementserver->rcvread);
				}
			}
		}
		//this_thread::sleep_for(1ms);
	}
end:
	managementserver->isAuth = FALSE;
	managementserver->running = false;
	closesocket(ConnectSocket);
	WSACleanup();
	managementserver->setServerHandle(nullptr);
}
void ManagementServer::CheckManagementPackets(unsigned rcvcopy, unsigned char * tmprcv)
{
	if (rcvcopy >= 6)
	{
		inbuf.setArray(tmprcv, rcvcopy, 0x00);
		uint16_t size = inbuf.getSizeFromBuffer();
		inbuf.setSize(size);
		expect = inbuf.getSize();
		if (expect > CLIENT_BUFFER_SIZE)
		{
			// Packet too big, disconnect the client.
			logger->Log(LOGTYPE_MANAGEMENT, L"Management server %s sent a invalid packet.", toWide((char*)IP_Address).c_str());
			Disconnect();
		}
		else
		{
			ProcessPacket();

			// Reset variables.
			packetSize = 0;
			expect = 0;
		}
	}
}
wstring ManagementServer::toWide(string in)
{
	wstring temp(in.length(), L' ');
	copy(in.begin(), in.end(), temp.begin());
	return temp;
}
string ManagementServer::toNarrow(wstring in)
{
	string temp(in.length(), ' ');
	copy(in.begin(), in.end(), temp.begin());
	return temp;
}
string ManagementServer::compress(string in)
{
	string result("");
	CryptoPP::Gzip zipper(new CryptoPP::StringSink(result));
	zipper.Put((byte*)in.data(), in.size());
	zipper.MessageEnd();
	return result;
}
string ManagementServer::decompress(string in)
{
	string result("");
	CryptoPP::Gunzip unzipper(new CryptoPP::StringSink(result));
	unzipper.Put((byte*)in.data(), in.size());
	unzipper.MessageEnd();
	return result;
}
int32_t ManagementServer::tcp_set_nonblocking(int fd)
{
	u_long flags = 1;
	if (ioctlsocket(fd, FIONBIO, &flags))
		return 1;
	return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&flags, sizeof(flags));
}
void ManagementServer::addToMessageQueue(MESSAGE_QUEUE in)
{
	lock_guard<mutex> locker(_muManagement);
	messagequeue.push_back(in);
}
MESSAGE_QUEUE ManagementServer::getTopFromMessageQueue()
{
	lock_guard<mutex> locker(_muManagement);
	if (messagequeue.size())
	{
		MESSAGE_QUEUE extract = messagequeue.at(0);
		messagequeue.erase(messagequeue.begin());
		return extract;
	}
	else
	{
		MESSAGE_QUEUE extract = { 0 };
		return extract;
	}
}
uint32_t ManagementServer::messagesInQueue()
{
	lock_guard<mutex> locker(_muManagement);
	return messagequeue.size();
}
void ManagementServer::addToSendQueue(SEND_MANAGEMENT_QUEUE in)
{
	lock_guard<mutex> locker(_muPackets);
	sendQueue.push_back(in);
}
SEND_MANAGEMENT_QUEUE ManagementServer::getFromSendQueue()
{
	lock_guard<mutex> locker(_muPackets);
	if (sendQueue.size())
	{
		SEND_MANAGEMENT_QUEUE extract = sendQueue.at(0);
		sendQueue.erase(sendQueue.begin());
		return extract;
	}
	else
	{
		SEND_MANAGEMENT_QUEUE extract = { 0 };
		return extract;
	}
}
uint32_t ManagementServer::messagesInSendQueue()
{
	lock_guard<mutex> locker(_muPackets);
	return sendQueue.size();
}

#pragma region Packets
void ManagementServer::SendAuth()
{
	outbuf.clearBuffer();
	outbuf.setSize(0x06);
	outbuf.setOffset(0x06);
	outbuf.setType(0x0000);
	outbuf.setSubType(0x0000);
	outbuf.appendArray(&iv[0], sizeof(iv));
	Send();
	isAuth = TRUE;
}
#pragma endregion