#include "globals.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "server.h"

Server::Server()
{
	managementServerAddress = "localhost";
	managementServerPort = 7946;
	serverPort = 47701;
	serverClientLimit = 100;
	serverWelcomeMessage = "";
	running = false;
	serverNumConnections = 0;
	server_sockfd = -1;
	temp_sockfd = -1;

	startingCars = {
		/*AE86_T_3_1985,
		SILVIA_Q_1988,*/
		AE86_L_3_1985,
		AE86_T_3_1985,
		AE86_L_2_1985,
		AE86_T_2_1985,
		MR2_GT_1997,
		SUPRA_RZ_1993,
		SUPRA_RZ_1997,
		SUPRA_SZR_1997,
		ALTEZZA_R200_Z_1997,
		CHASER_TOURER_V_1996,
		CHASER_TOURER_V_1998,
		MARKII_TOURER_V_1998,
		SILVIA_K_1988,
		SILVIA_Q_1988,
		SILVIA_K_1991,
		SILVIA_K_1993,
		SILVIA_K_1996,
		SILVIA_SPECR_1999,
		SILVIA_SPECS_1999,
		S180SX_TYPEIII_1994,
		SKYLINE_GTR_VSPECII_1994,
		SKYLINE_GTR_1989,
		SKYLINE_GTST_TYPEM_1991,
		SKYLINE_GTR_VSPEC_1997,
		SKYLINE_25GT_TURBO_1998,
		SKYLINE_25GT_TURBO_2000,
		FAIRLADY_Z_S_TT_1998,
		FAIRLADY_Z_TBAR_1998,
		CEDRIC_BROUGHAM_VIP_1997,
		GLORIA_GRANTURISMO_ULTIMA_1997,
		LANCER_GSR_EVOIII_1995,
		LANCER_GSR_EVOIV_1996,
		SAVANNA_RX7_INFIIII_1989,
		RX7_TYPERS_1999,
		RX7_TYPERZ_1998,
		IMPREZA_WRX_STI_VERIV_1997,
		IMPREZA_WRX_STI_VERV_1998,
		IMPREZA_WRX_STI_VERVI_1999,
		FAIRLADY_Z_240ZG_1972,
		MX5_ROADSTER_RS_1998,
		MX5_ROADSTER_RS_2000,
		CELICA_GTFOUR_1996,
		CELICA_SSIII_1996,
		GTO_TT_1993,
		FTO_GP_VERR_1997,
		PULSAR_SERIE_VZN1_VER2_1998,
		LEGACY_TOURING_WAGON_GTB_1998,
		CELICA_SSII_SSPACK_1999,
		MRS_1999,
		CEDRIC_300VIP_1999,
		CLORIA_300_ULTIMA_1999,
		AZ1_1992,
		CAPPUCCINO_1991,
		CEFIRO_CRUISING_1990,
		STARION_GSRVR_1988,
		SAVANNA_RX3_GT_1973,
		SKYLINE_2000GTR_1971,
		SAVANNA_RX7_GTX_1983,
		ECLIPSE_GST_1999,
		BB15Z_2000,
		SUPRA_30GT_T_1989,
		SUPRA_25GT_TT_1991,
		LANCER_GSR_EVOVII_2001,
		IMPREZA_WRX_NB_2000,
		IMPREZA_WRX_STI_2000,
		LANTIS_COUPE_TYPER_1993
	};
}
Server::~Server()
{
	for (uint32_t i = 0; i < connections.size(); i++)
	{
		delete connections[i];
	}
}
void Server::initialize() {
	connections.resize(serverClientLimit);
	serverConnections.resize(serverClientLimit);
	for (uint32_t i = 0; i < connections.size(); i++)
	{
		connections[i] = new CLIENT();
		connections[i]->logger = &logger;
		connections[i]->server = this;
		serverConnections[i] = 0;
	}
}
char* Server::HEXString(uint8_t* in, char* out, uint32_t length = 0)
{
	uint32_t offset = 0;
	uint8_t* getLength = in;
	while (*getLength && (getLength - in) < MAX_MESG_LEN)
	{
		sprintf(&out[offset], "%02X", getLength[0]);
		offset += 2;
		getLength++;
		if (length && (offset / 2 >= length)) break;
	}
	out[offset] = 0;
	return out;
}
char* Server::HEXString(uint8_t* in, uint32_t length = 0)
{
	char* out = &hexStr[0];
	uint32_t offset = 0;
	uint8_t* getLength = in;
	while (*getLength && (getLength - in) < MAX_MESG_LEN)
	{
		sprintf(&out[offset], "%02X", getLength[0]);
		offset += 2;
		getLength++;
		if (length && (offset / 2 >= length)) break;
	}
	out[offset] = 0;
	return out;
}
int32_t Server::Start()
{
	if (VerifyConfig())
		return 1;

	initialize();

	thread sThread(serverThread, this);
	sThread.detach();
	logger.Log(LOGTYPE_NONE, L"Server Started.");
	this_thread::sleep_for(10ms);
	managementserver.Start(this);
	return 0;
}
int32_t Server::Stop()
{
	if (running == false)
		return 1;

	for (uint32_t i = 0; i < connections.size(); i++)
	{
		if (connections[i]->plySockfd < 0)
			initialize_connection(connections[i]);
	}
	running = false;
	return 0;
}
int32_t Server::LoadConfig(const wchar_t* filename)
{
	bool malformed = false;
	wstring error = L"";
	ifstream inFile(filename);
	if (!inFile.is_open())
	{
		logger.Log(LOGTYPE_ERROR, L"Error Loading configuration file: %s.", filename);
		return 1;
	}

	json in;

	try {
		inFile >> in;
	}
	catch (json::parse_error) {
		logger.Log(LOGTYPE_ERROR, L"JSON Parse Error\n");
		inFile.close();
		return 1;
	}

	inFile.close();

	if (in.empty())
	{
		error = L"file is empty";
		malformed = true;
	}
	else
	{
		if (in["serverport"].is_number_integer())
		{
			try {
				serverPort = in["serverport"].get<unsigned short>();
				logger.Log(LOGTYPE_DEBUG, L"CONFIG: Server port is %u.", serverPort);
			}
			catch (json::parse_error) {
				logger.Log(LOGTYPE_ERROR, L"JSON Parse Error\n");
				return 1;
			}
		}
		else
		{
			error = L"serverport value is not integer";
			malformed = true;
		}

		if (in["servername"].is_string())
		{
			try {
				serverName = in["servername"].get<string>();
				logger.Log(LOGTYPE_DEBUG, L"CONFIG: Server Name is %s.", toWide(serverName).c_str());
			}
			catch (json::parse_error) {
				logger.Log(LOGTYPE_ERROR, L"JSON Parse Error\n");
				return 1;
			}
		}
		else
		{
			error = L"servername value is not string";
			malformed = true;
		}

		if (in["serverclientlimit"].is_number_integer())
		{
			try {
				serverClientLimit = in["serverclientlimit"].get<unsigned short>();
				logger.Log(LOGTYPE_DEBUG, L"CONFIG: Server Client Limit is %u.", serverClientLimit);
			}
			catch (json::parse_error) {
				logger.Log(LOGTYPE_ERROR, L"JSON Parse Error\n");
				return 1;
			}
		}
		else
		{
			error = L"serverclientlimit value is not integer";
			malformed = true;
		}

		if (in["serverwelcomemessage"].is_string())
		{
			try {
				serverWelcomeMessage = in["serverwelcomemessage"].get<string>();
				logger.Log(LOGTYPE_DEBUG, L"CONFIG: Server Welcome Message is %s.", toWide(serverWelcomeMessage).c_str());
			}
			catch (json::parse_error) {
				logger.Log(LOGTYPE_ERROR, L"JSON Parse Error\n");
				return 1;
			}
		}
		else
		{
			error = L"serverwelcomemessage value is not string";
			malformed = true;
		}

		if (in["managementserverport"].is_number_integer())
		{
			try {
				managementServerPort = in["managementserverport"].get<unsigned short>();
				logger.Log(LOGTYPE_DEBUG, L"CONFIG: Management Server port is %u.", managementServerPort);
			}
			catch (json::parse_error) {
				logger.Log(LOGTYPE_ERROR, L"JSON Parse Error\n");
				return 1;
			}
		}
		else
		{
			error = L"managementserverport value is not integer";
			malformed = true;
		}

		if (in["managementserveraddress"].is_string())
		{
			try {
				managementServerAddress = in["managementserveraddress"].get<string>();
				logger.Log(LOGTYPE_DEBUG, L"CONFIG: Management Server Address is %s.", toWide(managementServerAddress).c_str());
			}
			catch (json::parse_error) {
				logger.Log(LOGTYPE_ERROR, L"JSON Parse Error\n");
				return 1;
			}
		}
		else
		{
			error = L"managementserveraddress value is not string";
			malformed = true;
		}

		if (in["logpath"].is_string())
		{
			try {
				logger.setLogPath(in["logpath"].get<string>());
				logger.Log(LOGTYPE_DEBUG, L"CONFIG: Log path is %s.", logger.isLogPathSet() ? toWide(logger.getLogPath()).c_str() : L"EMPTY");
			}
			catch (json::parse_error) {
				logger.Log(LOGTYPE_ERROR, L"JSON Parse Error\n");
				return 1;
			}
		}
		else
		{
			error = L"logpath value is not string";
			malformed = true;
		}
	}

	if (malformed)
	{
		logger.Log(LOGTYPE_ERROR, L"Configuration file: %s is malformed (%s).", filename, error.c_str());
		return 1;
	}
	else
		return 0;
}
wstring Server::toWide(string in)
{
	wstring temp(in.length(), L' ');
	copy(in.begin(), in.end(), temp.begin());
	return temp;
}
string Server::toNarrow(wstring in)
{
	string temp(in.length(), ' ');
	copy(in.begin(), in.end(), temp.begin());
	return temp;
}
void Server::CheckClientPackets(CLIENT* client, unsigned rcvcopy, unsigned char * tmprcv)
{
	unsigned p = 0;

	while (rcvcopy)
	{
		// Copy it to the decryption buffer.
		client->inbuf.buffer[client->packetSize++] = tmprcv[p++];
		if ((client->expect == 0) && (client->packetSize == 4))
		{
			unsigned short size = client->inbuf.getSizeFromBuffer();
			client->inbuf.setSize(SWAP_SHORT(size) + 2);
			client->expect = client->inbuf.getSize();
			if (client->expect > CLIENT_BUFFER_SIZE)
			{
				// Packet too big, disconnect the client.
				wstring ip = toWide((char*)client->IP_Address);
				logger.Log(LOGTYPE_COMM, L"Client %s sent a invalid packet.", ip.c_str());
				client->Disconnect();
				break;
			}
		}

		// Process the packet if we've decrypted it all.
		if (client->packetSize == client->expect)
		{
			client->ProcessPacket();

			// Reset variables.
			client->packetSize = 0;
			client->expect = 0;
		}
		rcvcopy--;
	}
}
void Server::initialize_connection(CLIENT* connect)
{
	if (connect->plySockfd >= 0)
	{
		saveClientData(connect);
		wstring ip = toWide((char*)connect->IP_Address);
		logger.Log(LOGTYPE_COMM, L"Client %s has disconnected.", ip.c_str());

		int j = 0;
		for (uint32_t i = 0; i < serverNumConnections; i++)
		{
			if (serverConnections[i] != connect->connection_index)
				serverConnections[j++] = serverConnections[i];
		}
		closesocket(connect->plySockfd);
	}
	connect->initialize();
}
void Server::saveClientData(CLIENT* client)
{
	if (client->canSave == FALSE) return;

	managementserver.outbuf.clearBuffer();
	managementserver.outbuf.setSize(0x06);
	managementserver.outbuf.setOffset(0x06);
	managementserver.outbuf.setType(0x0002);
	managementserver.outbuf.setSubType(0x0000);
	managementserver.outbuf.appendInt(client->driverslicense);
	managementserver.outbuf.setString(client->handle, managementserver.outbuf.getOffset());
	managementserver.outbuf.addOffset(0x10);
	managementserver.outbuf.addSize(0x10);
	managementserver.outbuf.appendInt64(client->careerdata.CP);
	managementserver.outbuf.appendByte(client->careerdata.level);
	managementserver.outbuf.appendInt(client->careerdata.experiencePoints);
	managementserver.outbuf.appendInt(client->careerdata.playerWin);
	managementserver.outbuf.appendInt(client->careerdata.playerLose);
	managementserver.outbuf.appendInt(client->careerdata.rivalWin);
	managementserver.outbuf.appendInt(client->careerdata.rivalLose);
	managementserver.outbuf.appendByte(static_cast<uint8_t>(client->getActiveCar()));
	managementserver.outbuf.appendByte(client->privileges);
	managementserver.outbuf.appendInt(client->teamdata.teamID);

	int8_t carCount = client->getCarCount();
	managementserver.outbuf.appendByte(carCount);
	for (int32_t i = 0; i < GARAGE_LIMIT; i++)
	{
		if (client->garagedata.car[i].carID != 0xFFFFFFFF)
		{
			managementserver.outbuf.appendInt(client->garagedata.car[i].carID);
			managementserver.outbuf.appendInt(client->garagedata.car[i].KMs);
			managementserver.outbuf.appendArray((uint8_t*)&client->garagedata.car[i].carmods, sizeof(client->garagedata.car[i].carmods));
			managementserver.outbuf.appendInt(client->garagedata.car[i].engineCondition);
			managementserver.outbuf.appendInt(i); // Bay
		}
	}

	managementserver.outbuf.appendInt(client->itembox.size());
	managementserver.outbuf.appendArray((uint8_t*)client->itembox.data(), sizeof(ITEMBOX_ITEM) * client->itembox.size());

	managementserver.Send();
}
int32_t Server::tcp_sock_open(struct in_addr ip, uint16_t port)
{
	int32_t fd, bufsize, turn_on_option_flag = 1, rcSockopt;
	struct sockaddr_in sa;

	memset((void *)&sa, 0, sizeof(sa));

	fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (fd < 0) {
		return -1;
	}

	sa.sin_family = AF_INET;
	memcpy((void *)&sa.sin_addr, (void *)&ip, sizeof(struct in_addr));
	sa.sin_port = htons(port);

	/* Reuse port */

	rcSockopt = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&turn_on_option_flag, sizeof(turn_on_option_flag));

	/* Increase the TCP/IP buffer size */
	bufsize = TCP_BUFFER_SIZE;
	setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&bufsize, sizeof(bufsize));
	setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char *)&bufsize, sizeof(bufsize));

	/* bind() the socket to the interface */
	if (::bind(fd, (struct sockaddr *)&sa, sizeof(struct sockaddr)) < 0) {
		return -2;
	}
	return fd;
}
int32_t Server::tcp_set_nonblocking(int fd)
{
	u_long flags = 1;
	if (ioctlsocket(fd, FIONBIO, &flags))
		return 1;
	return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char*)&flags, sizeof(flags));
}
int32_t Server::tcp_listen(int sockfd)
{
	if (listen(sockfd, 10) < 0)
	{
		return 1;
	}
	tcp_set_nonblocking(sockfd);
	return 0;
}
int32_t Server::tcp_accept(int sockfd, struct sockaddr *client_addr, int *addr_len)
{
	int fd, bufsize;

	if ((fd = accept(sockfd, client_addr, addr_len)) != INVALID_SOCKET)
	{
		bufsize = TCP_BUFFER_SIZE;
		setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char*)&bufsize, sizeof(bufsize));
		setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (const char*)&bufsize, sizeof(bufsize));
		tcp_set_nonblocking(fd);
		return fd;
	}
	else
	{
		bufsize = WSAGetLastError();
		if (bufsize != WSAEWOULDBLOCK)
			logger.Log(LOGTYPE_COMM, L"Could not accept connection.");
		return -1;
	}
}
int32_t Server::free_connection()
{
	CLIENT* wc;

	for (uint32_t fc = 0; fc < serverClientLimit; fc++)
	{
		wc = connections[fc];
		if (wc->plySockfd<0)
			return fc;
	}
	return 0xFFFF;
}
int32_t Server::VerifyConfig()
{
	if (LoadConfig(CONFIG_FILENAME))
		return 1;

	if (serverName == "" || serverPort == 0 || managementServerPort == 0 || managementServerAddress == "")
		return 1;

	return 0;
}
void Server::serverThread(void* parg)
{
	Server* server = (Server*)parg;
	int32_t connectNum;
	struct in_addr server_in;
	WSADATA winsock_data;
	struct sockaddr_in listen_in;
	int32_t listen_length;
	CLIENT* workConnect;
	int32_t pkt_len, wserror, max_send, bytes_sent;
	fd_set ReadFDs, WriteFDs;
	uint8_t tmprcv[CLIENT_BUFFER_SIZE];
	uint8_t tmpsnd[CLIENT_BUFFER_SIZE];
	WSAStartup(MAKEWORD(2, 2), &winsock_data);

	struct timeval select_timeout = {
		1,
		0
	};

	if (!server)
		return;

	for (uint32_t i = 0; i < server->serverClientLimit; i++)
	{
		server->initialize_connection(server->connections[i]);
	}

	server_in.s_addr = INADDR_ANY;
	server->server_sockfd = server->tcp_sock_open(server_in, server->serverPort);

	if (server->server_sockfd < 0)
	{
		switch (server->server_sockfd)
		{
		case -1:
			server->logger.Log(LOGTYPE_ERROR, L"Could not create socket.");
			break;
		case -2:
			server->logger.Log(LOGTYPE_ERROR, L"Could not bind to port %u.", server->serverPort);
			break;
		default:
			break;
		}
		server->Stop();
		return;
	}

	if (server->tcp_listen(server->server_sockfd))
		server->logger.Log(LOGTYPE_ERROR, L"Could not listen to port %u.", server->serverPort);
	else
		server->logger.Log(LOGTYPE_COMM, L"Listening on port %u.", server->serverPort);

	server->running = true;

	while (server->running)
	{
		// Zero descriptors.

		FD_ZERO(&ReadFDs);
		FD_ZERO(&WriteFDs);

		// Accept connections to the server port.

		FD_SET(server->server_sockfd, &ReadFDs);

		// Read/write data from clients.

		for (uint32_t i = 0; i < server->serverNumConnections; i++)
		{
			connectNum = server->serverConnections[i];
			workConnect = server->connections[connectNum];

			if ((workConnect) && (workConnect->plySockfd >= 0))
			{
				if (!workConnect->todc)
					FD_SET(workConnect->plySockfd, &ReadFDs);
				if (workConnect->snddata)
					FD_SET(workConnect->plySockfd, &WriteFDs);
			}
		}

		select(0, &ReadFDs, &WriteFDs, NULL, &select_timeout);

		if (FD_ISSET(server->server_sockfd, &ReadFDs))
		{
			listen_length = sizeof(listen_in);

			while ((server->temp_sockfd = server->tcp_accept(server->server_sockfd, (struct sockaddr*) &listen_in, &listen_length)) != -1)
			{
				int ch = server->free_connection();
				if (ch != 0xFFFF)
				{
					workConnect = server->connections[ch];
					if (!workConnect) break;
					workConnect->connection_index = ch;
					server->serverConnections[server->serverNumConnections++] = ch;
					workConnect->plySockfd = server->temp_sockfd;
					InetNtopA(AF_INET, &listen_in.sin_addr, (char*)&workConnect->IP_Address[0], 16);
					*(unsigned *)&workConnect->ipaddr = *(unsigned *)&listen_in.sin_addr;
					wstring ip = server->toWide((char*)workConnect->IP_Address);
					server->logger.Log(LOGTYPE_COMM, L"Accepted CLIENT connection from %s:%u", ip.c_str(), listen_in.sin_port);
				}
			}
		}

		for (uint32_t i = 0; i < server->serverNumConnections; i++)
		{
			connectNum = server->serverConnections[i];
			workConnect = server->connections[connectNum];

			if ((workConnect) && (workConnect->plySockfd >= 0))
			{
				if (FD_ISSET(workConnect->plySockfd, &ReadFDs))
				{
					if ((pkt_len = recv(workConnect->plySockfd, (char *)&workConnect->rcvbuf[workConnect->rcvread], TCP_BUFFER_SIZE, 0)) <= 0)
					{
						wserror = WSAGetLastError();
						if (wserror != WSAEWOULDBLOCK)
						{
							wstring ip = server->toWide((char*)workConnect->IP_Address);
							if (wserror != 0)
								server->logger.Log(LOGTYPE_COMM, L"Could not read data from client %s. Socket Error %d", ip.c_str(), wserror);
							workConnect->todc = 1;
						}
					}
					else
					{
						workConnect->rcvread += (unsigned)pkt_len;
						if (workConnect->rcvread >= 2) // Have we read enough bytes?
						{
							unsigned short rcvcopy = workConnect->rcvread;

							memcpy(&tmprcv[0], &workConnect->rcvbuf[0], rcvcopy);
							server->CheckClientPackets(workConnect, rcvcopy, &tmprcv[0]);

							workConnect->rcvread -= rcvcopy;

							if (workConnect->rcvread > 0)
							{
								// Anything still left in the receive buffer?  Copy it to the beginning.

								memcpy(&tmprcv[0], &workConnect->rcvbuf[rcvcopy], workConnect->rcvread);
								memcpy(&workConnect->rcvbuf[0], &tmprcv[0], workConnect->rcvread);
							}
						}
					}
				}

				if ((FD_ISSET(workConnect->plySockfd, &WriteFDs)) || (workConnect->messagesInSendQueue()))
				{
					SEND_QUEUE entry = workConnect->getFromSendQueue();
					workConnect->snddata = SWAP_SHORT(*(int16_t*)&entry.sndbuf[0]) + 2;
					if (workConnect->snddata > TCP_BUFFER_SIZE)
						max_send = TCP_BUFFER_SIZE;
					else
						max_send = workConnect->snddata;

					memcpy(&workConnect->sndbuf[0], &entry.sndbuf[0], max_send);

					bytes_sent = send(workConnect->plySockfd, (const char *)&workConnect->sndbuf[0], max_send, 0);

					if (bytes_sent == SOCKET_ERROR)
					{
						wserror = WSAGetLastError();
						if (wserror != WSAEWOULDBLOCK)
						{
							wstring ip = server->toWide((char*)workConnect->IP_Address);
							if (wserror != 0)
								server->logger.Log(LOGTYPE_COMM, L"Could not write data to client %s. Socket Error %X08", ip.c_str(), wserror);
							workConnect->todc = 1;
						}
					}
					else
					{
						workConnect->snddata -= bytes_sent;

						if (workConnect->snddata > 0) // We didn't send it all?
						{
							memcpy(&tmpsnd[0], &workConnect->sndbuf[bytes_sent], workConnect->snddata);
							memcpy(&workConnect->sndbuf[0], &tmpsnd[0], workConnect->snddata);
						}
					}
				}
				if (workConnect->todc)
					server->initialize_connection(workConnect);
			}
		}

		if (server->managementserver.messagesInQueue())
		{
			MESSAGE_QUEUE message = server->managementserver.getTopFromMessageQueue();
			for (uint32_t i = 0; i < server->connections.size(); i++)
			{
				if (server->connections[i]->plySockfd == message.socket)
				{
					workConnect = server->connections[i];
					workConnect->serverbuf.clearBuffer();
					workConnect->serverbuf.appendArray(&message.buffer[0x00], *(uint16_t*)&message.buffer[0x00] + 2);
					workConnect->ProcessManagementPacket();
					break;
				}
			}
		}
		//this_thread::sleep_for(1ms);
	}
	server->running = false;
}

vector<string> Server::split(string in, string delimit)
{
	vector<string> out;
	uint32_t offset = 0;
	while (in.find(delimit, offset) != string::npos)
	{
		out.push_back(in.substr(offset, in.find(delimit, offset)));
		offset += in.find(delimit, offset);
	}
	if (offset == 0 && in.length() > 0) out.push_back(in);
	return out;
}