#pragma once
#include "ManagementServer.h"
typedef void(PacketFunction)(ManagementServer* managementserver);


void ServerAuth(ManagementServer* managementserver)
{
	switch (managementserver->inbuf.getSubType())
	{
		case 0x0000: // Server
		{
			if (managementserver->inbuf.getInt(0x0A) == managementserver->inbuf.getInt(0x06) / 8)
			{
				managementserver->logger->Log(LOGTYPE_MANAGEMENT, L"Authenticated with management server");
			}
			else
			{
				managementserver->isAuth = FALSE;
				managementserver->Disconnect();
				managementserver->logger->Log(LOGTYPE_MANAGEMENT, L"Failed to authenticate with management server");
			}
		}
		break;
	}
}
void ClientAuth(ManagementServer* managementserver)
{
	CLIENT* client = nullptr;
	MESSAGE_QUEUE entry = { 0 };
	switch (managementserver->inbuf.getSubType())
	{
		case 0x0000: // Client authentication
		case 0x0001: // Check Handle
		{
			entry.socket = managementserver->inbuf.getSInt(0x06);
			memcpy(&entry.buffer[0x00], &managementserver->inbuf.buffer[0x00], managementserver->inbuf.getSize() + 2);
			managementserver->addToMessageQueue(entry);
		}
		break;
	}
}
void ClientRequests(ManagementServer* managementserver)
{
}
void ClientOperations(ManagementServer* managementserver)
{
}
void ServerOperations(ManagementServer* managementserver)
{
}

void ClientPacketDoNothing(ManagementServer* managementserver)
{
	// Do nothing
}

PacketFunction* ManagementPacketFunctions[] =
{
	&ServerAuth,				// 0x0000 - Handshake: Verify keys and encryption
	&ClientAuth,				// 0x0100 - Verify client login: Check login. Including if banned and reason.
	&ClientRequests,			// 0x0200 - Request client data: Retrieve car, garage and team information.
	&ClientPacketDoNothing,		// 0x0300 - TBC
	&ClientOperations,			// 0x0400 - Client operations: Kick, ban message clients
	&ServerOperations,			// 0x0500 - Get Server Stats: Total player count, races in progress ....
};