#pragma once
#include "server.h"

typedef void(PacketFunction)(CLIENT* client);

void ClientPacketDoNothing(CLIENT* client)
{
	// Do nothing
}

void ClientPacket00(CLIENT* client)
{
	unsigned short pType = client->inbuf.getType();

	switch (pType)
	{
	case 0x0002:
		// Ping
		return;
	default:
		return;
	}
}

void ClientPacketAuthentication(CLIENT* client)
{
	unsigned short pType = client->inbuf.getType();

	switch (pType)
	{
	case 0x100:
	{
		if (client->inbuf.getVer() == 0x53427B00 /* new client */)
		{
			if (client->setUsername((char*)&client->inbuf.buffer[0x08]))
			{
				client->SendAuthError(STRINGS[STR_INVALID_UNPW]);
				//client->Disconnect();
				return;
			}
			else
			{
				client->outbuf.clearBuffer();
				client->outbuf.setSize(0x06);
				client->outbuf.setOffset(0x06);
				client->outbuf.setType(0x100);
				client->outbuf.setSubType(0x180);
				client->outbuf.appendByte(0x01); // Doesn't seem to matter
				client->outbuf.setString("AA111AAA", client->outbuf.getOffset()); // Doesn't seem to matter
				client->outbuf.addOffset(0x09);
				client->outbuf.addSize(0x09);
			}
		}
		else
		{
			client->SendAuthError(STRINGS[STR_UNKNOWN]);
			//client->Disconnect();
			return;
		}
	}
	break;
	case 0x102:
	{
		// Check username and password here
		Server* server = (Server*)client->server;

		server->managementserver.outbuf.clearBuffer();
		server->managementserver.outbuf.setSize(0x06);
		server->managementserver.outbuf.setOffset(0x06);
		server->managementserver.outbuf.setType(0x0001);
		server->managementserver.outbuf.setSubType(0x0000);
		server->managementserver.outbuf.appendSInt(client->plySockfd);
		server->managementserver.outbuf.setArray((uint8_t*)client->username.data(), client->username.size(), server->managementserver.outbuf.getOffset());
		server->managementserver.outbuf.addOffset(0x10);
		server->managementserver.outbuf.appendArray((uint8_t*)&client->inbuf.buffer[0x04], 32);
		if (server->managementserver.Send())
		{
			client->SendAuthError(STRINGS[STR_UNKNOWN]);
			//client->Disconnect();
		}
		return;
	}
	break;
	default:
		client->SendAuthError("Unknown packet");
		client->Disconnect();
		return;
	}
	client->Send();
}

void ClientPacket02(CLIENT* client)
{
	unsigned short pType = client->inbuf.getType();

	switch (pType)
	{
	case 0x200:
	{
		for (uint32_t i = 0; i < GARAGE_LIMIT; i++)
		{
			if (client->garagedata.car[i].carID != 0xFFFFFFFF)
			{
				client->outbuf.clearBuffer();
				client->outbuf.setSize(0x06);
				client->outbuf.setOffset(0x06);
				client->outbuf.setType(0x200);
				client->outbuf.setSubType(0x280);
				client->outbuf.appendByte(1); // byte moved to stack if not set process the 8 byte packet else structure continues
				client->outbuf.appendInt(0); // int moved to stack. if below value is 0 then this value is shifted left by 4 then added to pointer address at 0x6d1318 + 04 (crashes if greater than 0)
				client->outbuf.appendByte(i); // byte moved to stack another function is run 41fda0, Garage Bay number
				client->outbuf.appendInt(i + 1); // Active Car Slot
				client->Send();
			}
		}
		return;
	}
	break;
	case 0x201:
	{	// Car Settings
		if (!client->garagedata.activeCar)
		{
			client->Disconnect();
			return;
		}
		else if (client->garagedata.activeCar->carID == 0xFFFFFFFF || client->getCarCount() < 1)
		{
			client->Disconnect();
			return;
		}
		for (uint32_t i = 0; i < GARAGE_LIMIT; i++)
		{
			if (client->garagedata.car[i].carID != 0xFFFFFFFF)
			{
				client->outbuf.clearBuffer();
				client->outbuf.setSize(0x06);
				client->outbuf.setOffset(0x06);
				client->outbuf.setType(0x200);
				client->outbuf.setSubType(0x281);
				client->outbuf.appendInt(i + 1); // Active car slot from e80 on new accounts and 182 on existing
				client->outbuf.appendInt(client->garagedata.car[i].carID); // Car ID
				client->outbuf.appendInt(client->garagedata.car[i].KMs); // KMs
				client->outbuf.appendByte(0);
				client->outbuf.appendShort(static_cast<uint16_t>(client->garagedata.car[i].engineCondition)); // Engine Condition
				client->outbuf.appendByte(0);
				client->outbuf.appendArray((uint8_t*)&client->garagedata.car[i].carmods, sizeof(client->garagedata.car[i].carmods));
				client->outbuf.appendShort(0); // Status. 1: for sale
				client->Send();
			}
		}
		return;
	}
	break;
	case 0x202:
	{	// Client Switched cars
		uint32_t selectedCar = client->inbuf.getInt(0x04);
		if (selectedCar > GARAGE_LIMIT || selectedCar < 1)
		{
			client->Disconnect();
			return;
		}
		if (client->garagedata.car[selectedCar - 1].carID == 0xFFFFFFFF)
		{
			client->Disconnect();
			return;
		}
		client->setActiveCar(selectedCar - 1);
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x200);
		client->outbuf.setSubType(0x282);
		client->outbuf.appendInt(selectedCar);
	}
	break;
	case 0x203:
	{	// Purchase Car
		// 0x04: Car ID
		// 0x08: Color1 R
		// 0x0C: Color1 G
		// 0x10: Color1 B
		// 0x14: Color1 R
		// 0x18: Color1 G
		// 0x1C: Color1 B
		// 0x20: Cost
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x200);
		client->outbuf.setSubType(0x283);
		client->outbuf.appendInt(1); // Available Slot
		client->outbuf.appendInt(2); // ??
		client->outbuf.appendInt(3); // CP Remaining
	}
	break;
	case 0x204:
	{	// ???
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x200);
		client->outbuf.setSubType(0x284);
		client->outbuf.appendInt(0);
		client->outbuf.appendInt(0);
		client->outbuf.appendInt(0);
	}
	break;
	case 0x205:
	{	// Occurs painting car. Car slot at 0x04, Then 6 floats for colour, cost at 0x20
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x200);
		client->outbuf.setSubType(0x285);
		client->outbuf.appendInt(0);
		client->outbuf.appendInt(0);
		client->outbuf.appendInt(0);
	}
	break;
	case 0x206:
	{	// Engine Rehaul
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x200);
		client->outbuf.setSubType(0x286);
		client->outbuf.appendByte(1); // Success
		client->outbuf.appendShort(100);  // Engine Condition
		client->outbuf.appendByte(1); // Message?
		client->outbuf.appendString("ENGINE REPAIRED", 96);
	}
	break;
	case 0x207:
	{	// ????
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x200);
		client->outbuf.setSubType(0x287);
		client->outbuf.appendInt(0);
		client->outbuf.appendInt(0);
	}
	break;
	case 0x208:
	{	// Occurs when entering shop. Crashed when more than 1 car was in garage and only 1 value was sent here so suspect it linked to car bay number to display relevant parts
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x200);
		client->outbuf.setSubType(0x288);
		client->outbuf.appendShort(client->getCarCount());
		for (uint32_t i = 0; i < GARAGE_LIMIT; i++)
		{
			if (client->garagedata.car[i].carID != 0xFFFFFFFF)
				client->outbuf.appendInt(i);
		}
	}
	break;
	case 0x209:
	{	// Occurs when body paint shop.
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x200);
		client->outbuf.setSubType(0x289);
		client->outbuf.appendInt(client->getActiveCar() + 1);
		client->outbuf.appendInt(client->getCarCount());
	}
	break;
	case 0x20A:
	{	// Engine Overhaul
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x08);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x200);
		client->outbuf.setSubType(0x28A);
		client->outbuf.appendInt(0); // ????
		client->outbuf.appendInt(0); // ????
	}
	break;
	case 0x20C:
	{	// Tuned Car Purchase list
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x08);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x200);
		client->outbuf.setSubType(0x28C);
		client->outbuf.appendInt(0); // ????
		client->outbuf.appendInt(0); // ????
		client->outbuf.appendShort(0); // Car count
		//client->outbuf.appendInt(3); // ????
		//client->outbuf.appendInt(4); // ????
		//client->outbuf.appendInt(5); // ????
		//client->outbuf.appendInt(6); // ????
		//client->outbuf.addOffset(0x20);
		//client->outbuf.addSize(0x20);
		//client->outbuf.appendByte(0);
	}
	break;
	case 0x210:
	{	// Returning to highway. inform client of recent occurance
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x200);
		client->outbuf.setSubType(0x290);
		client->outbuf.appendInt64(client->careerdata.CP); // CP
		client->outbuf.appendByte(0); // Structure Count. Set 0 if no information
		break;
		//client->outbuf.appendInt(0); // ????
		//client->outbuf.appendShort(0); // Switch: O: Car Sold, 1: ?, 2: Car failed to sell, 3: ?
		//client->outbuf.appendInt(0); // CP Credited/Taken
	}
	break;
	case 0x220:
	{	// Car setting change - Handle at 0x08, Car slot at 0x04
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x08);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x200);
		client->outbuf.setSubType(0x2A0);
	}
	break;
	case 0x221:
	{	// Swapping car slot in garage (from slot at 0x05 and to at 0x0A)
		uint32_t fromBay = client->inbuf.getInt(0x05);
		uint32_t toBay = client->inbuf.getInt(0x0A);
		uint8_t result = 0;
		if (toBay == client->getActiveCar() ||
			toBay > GARAGE_LIMIT ||
			fromBay > GARAGE_LIMIT ||
			client->garagedata.car[fromBay].carID == 0xFFFFFFFF ||
			client->getCarCount() < 2 ||
			fromBay == toBay)
			result = 1;
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x08);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x200);
		client->outbuf.setSubType(0x2A1);
		client->outbuf.appendByte(result); // 0: Success 1: Failure
		if (!result)
		{
			uint8_t isSwapping = 0;
			if (client->garagedata.car[toBay].carID != 0xFFFFFFFF) isSwapping = 1;
			iter_swap(client->garagedata.car.begin() + (fromBay), client->garagedata.car.begin() + (toBay));

			Server* server = (Server*)client->server;
			server->managementserver.outbuf.clearBuffer();
			server->managementserver.outbuf.setSize(0x06);
			server->managementserver.outbuf.setOffset(0x06);
			server->managementserver.outbuf.setType(0x0002);
			server->managementserver.outbuf.setSubType(0x0001);
			server->managementserver.outbuf.appendInt(client->driverslicense);
			server->managementserver.outbuf.appendByte(isSwapping);
			server->managementserver.outbuf.appendInt(fromBay);
			server->managementserver.outbuf.appendInt(toBay);
			server->managementserver.Send();
		}
	}
	break;
	default:
		return;
	}
	client->Send();
}

void ClientPacket03(CLIENT* client)
{
	unsigned short pType = client->inbuf.getType();

	switch (pType)
	{
	case 0x300:
	//case 0x302:
	{	// if byte at 0x06 is 0 can just send 0x08 byte packet else the packet is quite large...
		/*
		if (LOGIN_SUCCESS)
		{
			client->outbuf.clearBuffer();
			client->outbuf.setSize(0x06);
			client->outbuf.setOffset(0x06);
			client->outbuf.setType(0x300);
			client->outbuf.setSubType(0x380);
			client->outbuf.appendByte(1);
			for (int i = 0; i < 1; i++)
			{
				client->outbuf.setString("Course name", client->outbuf.getOffset()); // Course name
				client->outbuf.addOffset(0x10);
				client->outbuf.addSize(0x10);
				client->outbuf.appendByte(0);
				client->outbuf.appendByte(1);
				client->outbuf.appendShort(1);
				client->outbuf.appendShort(1);
				client->outbuf.appendShort(1);
				client->outbuf.appendShort(1);
				client->outbuf.appendInt(1);
				client->outbuf.setString("Some string", client->outbuf.getOffset());
				client->outbuf.addOffset(0x10);
				client->outbuf.addSize(0x10);
				client->outbuf.appendInt(1);
				client->outbuf.appendShort(1);
				for (int i = 0; i < 204; i++) client->outbuf.appendByte(0x00);
			}
		}
		else
		{*/
		uint32_t count = 16;
		char *coursenames[] = {
			"Main Course",
			"Car Shop Course",
			"Parts Course",
			"Freeway A",
			"Freeway B",
			"Survival Course",
			"Time Attack A",
			"Time Attack B",
			"Test Server 9",
			"Test Server 10",
			"Test Server 11",
			"Test Server 12",
			"Test Server 13",
			"Test Server 14",
			"Test Server 15",
			"Test Server 16",
		};
		// Name of courses
			client->outbuf.clearBuffer();
			client->outbuf.setSize(0x06);
			client->outbuf.setOffset(0x06);
			client->outbuf.setType(0x300);
			client->outbuf.setSubType(0x380);
			client->outbuf.appendByte(static_cast<uint8_t>(count)); // Number of courses
			
			for (uint32_t i = 0; i < count; i++)
			{
				client->outbuf.appendString(coursenames[i % count], 0x10);
				client->outbuf.appendByte(1);
				client->outbuf.appendByte(2);
				client->outbuf.appendShort(1);
				client->outbuf.appendShort(2);
				client->outbuf.appendShort(3);
				client->outbuf.appendShort(4);
				client->outbuf.appendInt(1);
				client->outbuf.appendString("aaaaaaaaaaaaaaaa", 0x10);
				client->outbuf.appendInt(2);
				client->outbuf.appendShort(0); // If 0 additional 0xE4 Bytes
				for(int i = 0; i < 0xe4; i++) client->outbuf.appendByte(0x44);
				/*for (uint32_t j = 0; j < 19; j++)
				{
					client->outbuf.appendShort(0, FALSE);
					client->outbuf.appendShort(0, FALSE);
					client->outbuf.appendShort(0, FALSE);
					client->outbuf.appendShort(0, FALSE);
					client->outbuf.appendShort(0, FALSE);
					client->outbuf.appendShort(0, FALSE);
				}*/
				//for (int j = 0; j < 51; j++) client->outbuf.appendInt(0, false);
				//client->outbuf.appendInt(client->garagedata.activeCar->carID); // Carid?
				//client->outbuf.appendArray((uint8_t*)&client->garagedata.activeCar->carmods, sizeof(client->garagedata.activeCar->carmods)); // Car data?
				
			}
		//}
	}
	break;
	case 0x301:
		//case 0x302:
	{	// if login success course needs to be 3,4,5,6?
		client->outbuf.clearBuffer();
		client->outbuf.setOffset(0x06);
		client->outbuf.setSize(0x06);
		client->outbuf.setType(0x300);
		client->outbuf.setSubType(0x381);
		client->outbuf.appendShort(0x00);
		client->outbuf.appendInt(0x01);
		client->outbuf.appendShort(0x00); // if 0 0xE4 bytes sent below
		for (int i = 0; i < 114; i++) client->outbuf.appendShort(0, false);
	}
	break;
	/*case 0x302:
	{
	client->outbuf.clearBuffer();
	client->outbuf.setSize(0x06);
	client->outbuf.setOffset(0x06);
	client->outbuf.setType(0x300);
	client->outbuf.setSubType(0x380);
	client->outbuf.appendByte(0x01);
	client->outbuf.setString("Course name2", client->outbuf.getOffset()); // Course name
	client->outbuf.addOffset(0x10);
	client->outbuf.addSize(0x10);
	client->outbuf.appendByte(10);
	client->outbuf.appendByte(10);
	client->outbuf.appendShort(10);
	client->outbuf.appendShort(10);
	client->outbuf.appendShort(10);
	client->outbuf.appendShort(10);
	client->outbuf.appendInt(10);
	client->outbuf.setString("Some string", client->outbuf.getOffset());
	client->outbuf.addOffset(0x10);
	client->outbuf.addSize(0x10);
	client->outbuf.appendInt(10);
	client->outbuf.appendShort(10);
	for (int i = 0; i < 204; i++) client->outbuf.appendByte(0x41);
	}
	break;*/
	default:
		return;
	}
	client->Send();
}

void ClientPacket04(CLIENT* client)
{
	unsigned short pType = client->inbuf.getType();
	unsigned int command;
	switch (pType)
	{
	case 0x400:
	{
		return;
		// crashu. I dont think anything should be sent back to 0x400 packets...
		/*client->outbuf.clearBuffer();
		client->outbuf.setSize(0x08);
		client->outbuf.setType(0x400);
		client->outbuf.setSubType(0x480);
		for(int i = 0; i < 800; i++) client->outbuf.appendByte(0);*/
	}
	break;
	case 0x401:
	{
		command = client->inbuf.getInt(0x04);
		switch (command)
		{
		case 0x01:
#ifdef PACKET_OUTPUT
			client->logger->Log(LOGTYPE_CLIENT, L"(%u) %s: Switched to driving mode.", client->driverslicense, client->logger->toWide(client->handle).c_str());
#endif
			break;
		case 0x05:
			// Entering Garage
			break;
			client->outbuf.clearBuffer();
			client->outbuf.setSize(0x08);
			client->outbuf.setType(0x400);
			client->outbuf.setSubType(0x481);
			break;
		case 0x09:
#ifdef PACKET_OUTPUT
			client->logger->Log(LOGTYPE_CLIENT, L"(%u) %s: Switched to auto mode.", client->driverslicense, client->logger->toWide(client->handle).c_str());
#endif
			break;
		default:
			return;
		}
	}
	break;
	case 0x402:
	{

		return;
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x08);
		client->outbuf.setType(0x400);
		client->outbuf.setSubType(0x482);

	}
	break;
	default:
		return;
	}
	client->Send();
}

void ClientPacket06(CLIENT* client)
{
	unsigned short pType = client->inbuf.getType();
	Server* server = (Server*)client->server;
	switch (pType)
	{
	case 0x601:
	{
		if (client->privileges)
		{
			// Command being sent
			vector<string> command = server->split(client->inbuf.getString(0x14), " ");
			if (command.size() == 0) return;

			if (command[0].compare("ping") == 0)
			{
				client->outbuf.clearBuffer();
				client->outbuf.setSize(0x06);
				client->outbuf.setOffset(0x06);
				client->outbuf.setType(0xA00);
				client->outbuf.setSubType(0xA80);
				client->outbuf.appendInt(0);
				client->Send();

				client->outbuf.clearBuffer();
				client->outbuf.setSize(0x06);
				client->outbuf.setOffset(0x06);
				client->outbuf.setType(0x600);
				client->outbuf.setSubType(0x601);
				client->outbuf.setString(client->handle, client->outbuf.getOffset());
				client->outbuf.setOffset(client->outbuf.getOffset() + 0x10);
				client->outbuf.setSize(client->outbuf.getSize() + 0x10);
				client->outbuf.setString("Sent Ping Command!", client->outbuf.getOffset());
				client->outbuf.setOffset(client->outbuf.getOffset() + 0x4E);
				client->outbuf.setSize(client->outbuf.getSize() + 0x4E);
				break;
			}
			if (command[0].compare("test") == 0)// || client->test1 == 0)
			{
				//client->test1 = 1;
				client->outbuf.clearBuffer();
				client->outbuf.setSize(0x06);
				client->outbuf.setOffset(0x06);
				client->outbuf.setType(0x400);
				client->outbuf.setSubType(0x480);

				client->outbuf.appendShort(1); // ????

				//client->outbuf.appendString("test", 0x10); // Player Name
				client->outbuf.addOffset(0x10);
				client->outbuf.addSize(0x10);

				client->outbuf.appendInt(1); // ????

				// 0x01 byte array
				client->outbuf.appendByte(0xFF); // ????

				// 0x6C byte array
				//for (int i = 0; i < 27 * 4; i++) client->outbuf.appendByte(0);
				//for (int i = 0; i < 27 * 4; i++) client->outbuf.appendByte(rand() % 0xFF);
				//for (int i = 0; i < 27; i++) client->outbuf.appendInt(i, false);
				
				// TESTING
				client->outbuf.appendInt(1, false); // ???
				client->outbuf.appendInt(2, false); // ???
				client->outbuf.appendInt(3, false); // ???
				client->outbuf.appendArray((uint8_t*)&client->garagedata.activeCar->carmods, sizeof(client->garagedata.activeCar->carmods)); // Car Data
				// TESTING

				// 0x0E byte array
				// Positioning here somewhere
				client->outbuf.appendShort(0x0000, false); // Junction
				client->outbuf.appendShort(0x0000, false); // ?? Direction ?
				client->outbuf.appendShort(0x0000, false); // ??
				client->outbuf.appendShort(0x0000, false); // ??
				
				client->outbuf.appendByte(tempValue1); // npc stuff must be below 0x20. 0x02 = NPC
				client->outbuf.appendByte(0); // When 1 message with name displayed
				client->outbuf.appendByte(1); // When 1 message with name displayed
				client->outbuf.appendByte(0); // When 1 message with name displayed
				client->outbuf.appendByte(0); // When 1 message with name displayed
				client->outbuf.appendByte(0); // When 1 message with name displayed

				// 0x6C byte array
				client->outbuf.appendInt(1, false); // Rival Team
				client->outbuf.appendInt(2, false); // ???
				client->outbuf.appendInt(3, false);// tempValue2++ % 8, FALSE); // Rival Number
				//for (int i = 0; i < 24 * 4; i++) client->outbuf.appendByte(0);
				//for (int i = 0; i < 24; i++) client->outbuf.appendInt(i, false);
				//for (int i = 0; i < 24 * 4; i++) client->outbuf.appendByte(rand() % 0xFF);
				client->outbuf.appendArray((uint8_t*)&client->garagedata.activeCar->carmods, sizeof(client->garagedata.activeCar->carmods)); // Car Data
				
				client->outbuf.appendByte(1); // ????
				client->outbuf.appendShort(0); // Count for something requires the shorts as many as these
				//client->outbuf.appendShort(10);
				//client->outbuf.appendShort(11);
				//client->outbuf.appendShort(12);
				//client->outbuf.appendShort(13);
				//client->outbuf.appendShort(14);
				client->outbuf.appendInt(1);
				client->Send();

				stringstream ss;
				ss << "Command data: " << tempValue1 << ", " << tempValue2;
				client->outbuf.clearBuffer();
				client->outbuf.setSize(0x06);
				client->outbuf.setOffset(0x06);
				client->outbuf.setType(0x600);
				client->outbuf.setSubType(0x601);
				client->outbuf.appendString("Server", 0x10);
				client->outbuf.appendString(ss.str(), 0x4E);

				tempValue1 += 1;// 0x80;

				break;
				
			}
			if (command[0].compare("setsticker") == 0)
			{
				if (command.size() > 1)
				{
					char* p;
					int sticker = strtol(command[1].c_str(), &p, 10);
					client->garagedata.activeCar->carmods.stickers = sticker;
					stringstream ss;
					ss << "Sticker changed to " << sticker;
					client->outbuf.clearBuffer();
					client->outbuf.setSize(0x06);
					client->outbuf.setOffset(0x06);
					client->outbuf.setType(0x600);
					client->outbuf.setSubType(0x601);
					client->outbuf.appendString("Server", 0x10);
					client->outbuf.appendString(ss.str(), 0x4E);
					break;
				}
				else
				{
					stringstream ss;
					ss << "setsticker requires value: !setsticker 1";
					client->outbuf.clearBuffer();
					client->outbuf.setSize(0x06);
					client->outbuf.setOffset(0x06);
					client->outbuf.setType(0x600);
					client->outbuf.setSubType(0x601);
					client->outbuf.appendString("Server", 0x10);
					client->outbuf.appendString(ss.str(), 0x4E);
					break;
				}
			}
		}
	}
	break;
	case 0x609:
	{	// Chat
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x600);
		client->outbuf.setSubType(0x604);
		client->outbuf.appendArray(&client->inbuf.buffer[0x04], 0x10);
		int offset = (client->inbuf.getShort(0x14) * 4) + 0x16;
		client->outbuf.appendArray(&client->inbuf.buffer[offset], 0x4E);
	}
	break;
	default:
		return;
	}
	client->Send();
}

void ClientPacket07(CLIENT* client)
{
	unsigned short pType = client->inbuf.getType();

	switch (pType)
	{
	case 0x700:
	{	// Player location
		//return;"test
		//client->outbuf.clearBuffer();
		//client->outbuf.setSize(0x06);
		//client->outbuf.setOffset(0x06);
		//client->outbuf.setType(0x700);
		//client->outbuf.setSubType(0x780);
		//client->outbuf.appendInt(0x0B00);
		//client->outbuf.appendArray(&client->inbuf.buffer[0x08], 34);
		//client->logger->Log(LOGTYPE_PACKET, L"Packet: Client -> Server");
		//client->logger->Log(LOGTYPE_PACKET, client->logger->packet_to_text(&client->inbuf.buffer[0x00], client->inbuf.getSize()));
		if (client->test1)
		{
			//if (client->test1 < 20)
			//	client->test1++;
			//else
			//{
			//for (int i = 0; i < tempValue2; i ++)
			{
				tempValue2 += 1;
				time_t temp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) % 0xffffff;
				//if(((int)temp - client->test2) > 10)
				{
					int count = 3;
					client->test2 = temp;
					client->outbuf.clearBuffer();
					client->outbuf.setSize(0x06);
					client->outbuf.setOffset(0x06);
					client->outbuf.setType(0x500);
					client->outbuf.setSubType(0x580);
					//client->outbuf.appendByte(0); // Course Number
					//client->outbuf.appendShort(count); // Count
					/*
					for (int i = 0; i < count; i++)
					{
						client->outbuf.appendShort(i); // Player ID
						client->outbuf.appendSShort(0); // Area
						client->outbuf.appendSShort(tempValue2 % 20000); // client->test2); // Moves arrow on map
						client->outbuf.appendSShort(0); // ????
						client->outbuf.appendFloat(0); // ????
					}
					*/
					//client->outbuf.appendShort(1);
					client->Send();
				}
				return;
			}
			//}
		}
	}
	break;
	//case 0x701:
	//{
	//	client->outbuf.clearBuffer();
	//	client->outbuf.setSize(0x08);
	//	client->outbuf.setType(0x700);
	//	client->outbuf.setSubType(0x781);
	//}
	//break;
	default:
		return;
	}
	client->Send();
}

void ClientPacket09(CLIENT* client)
{
	unsigned short pType = client->inbuf.getType();

	switch (pType)
	{
	case 0x900: // Entered shop : Shop id at 0x04? 0x04 Parts
	{
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x900);
		client->outbuf.setSubType(0x980);
		client->outbuf.appendInt(client->inbuf.getInt(0x04));
	}
	break;
	case 0x901: // Purchased item from shop : Shop id at 0x04? Category at 0x08 (byte), item type 0x09 (byte), item id 0x0A (byte), cost at 0x0B (int)
	{
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x08);
		client->outbuf.setOffset(0x08);
		client->outbuf.setType(0x900);
		client->outbuf.setSubType(0x981); // Success???
		client->outbuf.appendInt(client->getActiveCar() + 1);
		client->outbuf.appendInt(0); // to 0x006d0e6c - 1 = Take item price from CP
		client->outbuf.appendInt(client->careerdata.CP); // to 0x006d0e68 - CP to take
	}
	break;
	case 0x902: // equipped item from shop : Shop id at 0x04? Category at 0x08 (byte), item type 0x09 (byte), item id 0x0A (byte)
	{
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x08);
		client->outbuf.setOffset(0x08);
		client->outbuf.setType(0x900);
		client->outbuf.setSubType(0x982);
		client->outbuf.appendInt(client->getActiveCar() + 1);
		client->outbuf.appendInt(0); // to 0x006d0e6c - 1 = Take item price from CP
		client->outbuf.appendInt(client->careerdata.CP); // to 0x006d0e68 - CP to take
	}
	break;
	case 0x903: // Purchasing wheels
	{	// Shop id at 0x04?
		// Part id at 0x08?
		// Buying wheels example
		// 0x00CC at 0x08
		// 0x00 at 0x0A
		// 0x0B Wheel Color 1 R
		// 0x0F Wheel Color 1 G
		// 0x13 Wheel Color 1 B
		// 0x17 Wheel Color 2 R
		// 0x1B Wheel Color 2 G
		// 0x1F Wheel Color 2 B
		// 0x23 Price
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x08);
		client->outbuf.setOffset(0x08);
		client->outbuf.setType(0x900);
		client->outbuf.setSubType(0x981); // Success???
		client->outbuf.appendInt(client->getActiveCar() + 1);
		client->outbuf.appendInt(0); // to 0x0x6d0e6c
		client->outbuf.appendInt(0); // to 0x0x6d0e68
	}
	break;
	case 0x904: // Occurs when entering parts shop send 0x983 packet (shop contents)
	{
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x900);
		client->outbuf.setSubType(0x983);
		client->outbuf.appendInt(client->getActiveCar() + 1);
		client->outbuf.appendByte(7); // repeat as many as this
		for (int i = 0; i < 8; i++)
		{
			client->outbuf.appendByte(1); // Switch case : shop category? (0 - 2)
			client->outbuf.appendByte(i); // tests if higher than 6/7 depending on switch case. (7, 6, 6);

			//Structure of 10 bytes: Flags for items
			for (int j = 0; j < 10; j++) client->outbuf.appendByte(1);
		}
	}
	break;
	default:
		return;
	}
	client->Send();
}

void ClientPacket0A(CLIENT* client)
{
	unsigned short pType = client->inbuf.getType();

	switch (pType)
	{
	case 0xA00:
	{
		// Ping
		// TODO: Server to send ping every x seconds
		if (client->ping < 2)
		{
			client->outbuf.clearBuffer();
			client->outbuf.setSize(0x06);
			client->outbuf.setOffset(0x06);
			client->outbuf.setType(0xA00);
			client->outbuf.setSubType(0xA80);
			client->outbuf.appendInt(++client->pokes);
			client->ping++;
		}
		else
		{
			client->ping = 0;
			return;
		}
	}
	break;
	default:
		return;
	}
	client->Send();
}

void ClientPacket0C(CLIENT* client)
{
	unsigned short pType = client->inbuf.getType();

	switch (pType)
	{
	case 0xC00:
	{
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0xC00);
		client->outbuf.setSubType(0xC80);
		client->outbuf.appendShort(client->careerdata.playerWin + client->careerdata.playerLose); // Player total
		client->outbuf.appendShort(client->careerdata.playerWin); // VS Player Win
		client->outbuf.appendShort(client->careerdata.playerLose); // VS Player Lose
		client->outbuf.appendShort(client->careerdata.rivalWin + client->careerdata.rivalLose); // Rival total
		client->outbuf.appendShort(client->careerdata.rivalWin); // VS Rival Win
		client->outbuf.appendShort(client->careerdata.rivalLose); // VS Rival Lose
	}
	break;
	case 0xC01:
	{
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0xC00);
		client->outbuf.setSubType(0xC81);
		int count = 72;
		client->outbuf.appendByte(count); // 0x64); // Rival count (always 0x64
		// Struct Start
		for (int i = 0; i < count; i++)
		{
			client->outbuf.appendInt(i); // compare to 0x6ebe94 rival list must be loop index
			client->outbuf.appendByte(0x00); // Byte array size rival team count (always 8)
			// 0 - Not Seen
			// 1 - Lost
			// 2 - Won
			// 3 - Hide
			//for (int j = 0; j < 8; j++)
			{
				//client->outbuf.appendByte(0x00);
				//client->outbuf.appendByte(0x01);
				//client->outbuf.appendByte(0x02);
				//client->outbuf.appendByte(0x00);
				//client->outbuf.appendByte(0x01);
				//client->outbuf.appendByte(0x02);
				//client->outbuf.appendByte(0x00);
				//client->outbuf.appendByte(0x01);
			}
			client->outbuf.appendInt(0);// i + 1); // rival team count or number?
		}
	}
	break;
	case 0xC02:
	{
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x106);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0xC00);
		client->outbuf.setSubType(0xC82);
		client->outbuf.appendByte(0x00);
		client->outbuf.appendInt(0);
		client->outbuf.appendByte(0x01);
		client->outbuf.appendByte(0);
		client->outbuf.appendInt(1, FALSE);
	}
	break;
	default:
		return;
	}
	client->Send();
}

void ClientPacketPlayerCreation(CLIENT* client)
{
	unsigned short pType = client->inbuf.getType();
	Server* server = (Server*)client->server;

	if (!server)
	{
		client->logger->Log(LOGTYPE_ERROR, L"No Server Pointer for client %s", client->logger->toWide((char*)client->IP_Address).c_str());
		return;
	}

	switch (pType)
	{
	case 0xE00:
	{
		//Client requested if username is taken.
		string tempHandle;
		tempHandle.assign(client->inbuf.getString(0x14, 0x10));
		if (tempHandle.find(" ") != string::npos)
		{
			client->outbuf.clearBuffer();
			client->outbuf.setSize(0x06);
			client->outbuf.setOffset(0x06);
			client->outbuf.setType(0xE00);
			client->outbuf.setSubType(0xE80);
			client->outbuf.appendByte(0x04); // Spaces in handlename
		}
		client->setHandle(tempHandle);

		Server* server = (Server*)client->server;

		server->managementserver.outbuf.clearBuffer();
		server->managementserver.outbuf.setSize(0x06);
		server->managementserver.outbuf.setOffset(0x06);
		server->managementserver.outbuf.setType(0x0001);
		server->managementserver.outbuf.setSubType(0x0001);
		server->managementserver.outbuf.appendSInt(client->plySockfd);
		server->managementserver.outbuf.setArray((uint8_t*)tempHandle.data(), tempHandle.size(), server->managementserver.outbuf.getOffset());
		server->managementserver.outbuf.setSize(server->managementserver.outbuf.getSize() + 16);
		server->managementserver.outbuf.setOffset(server->managementserver.outbuf.getOffset() + 16);
		if (server->managementserver.Send())
		{
			client->SendAuthError(STRINGS[STR_UNKNOWN]);
			//client->Disconnect();
		}
		return;
	}
	break;
	case 0xE01:
	{
		//Client chosen starting car
		int selectedCar = client->inbuf.getInt(0x14);
		int found = 0;
		for (uint32_t i = 0; i < server->startingCars.size(); i++)
		{
			if (selectedCar == server->startingCars[i])
			{
				// Initialize Garage
				client->initializeGarage();
				if (client->setActiveCar(0))
				{
					client->Disconnect();
					return;
				}

				// Add car to garage
				client->inbuf.setOffset(0x18);
				client->garagedata.activeCar->carID = selectedCar;
				client->garagedata.activeCar->KMs = 0;
				client->garagedata.activeCar->carmods.color1_R = client->inbuf.getFloat();
				client->garagedata.activeCar->carmods.color1_G = client->inbuf.getFloat();
				client->garagedata.activeCar->carmods.color1_B = client->inbuf.getFloat();
				client->garagedata.activeCar->carmods.color2_R = client->inbuf.getFloat();
				client->garagedata.activeCar->carmods.color2_G = client->inbuf.getFloat();
				client->garagedata.activeCar->carmods.color2_B = client->inbuf.getFloat();
				client->garagedata.activeCar->engineCondition = 100;
				client->garagedata.activeCar->carmods.wheels = selectedCar;
				client->garagedata.activeCar->carmods.stickers = 3;

				client->outbuf.clearBuffer();
				client->outbuf.setSize(0x06);
				client->outbuf.setOffset(0x06);
				client->outbuf.setType(0xE00);
				client->outbuf.setSubType(0xE80);
				client->outbuf.appendByte(0x02);
				client->outbuf.appendInt(client->getActiveCar() + 1); // Active Car slot new accounts would be 0. Moved to 0x6d0ea4
				found = 1;
				client->canSave = TRUE;
				server->saveClientData(client);
				break;
			}
		}
		if (!found)
		{
			//Illegal car choice
			client->Disconnect();
			return;
		}
	}
	break;
	case 0xE02:
	{
		//Client is requesting Purchase list for first time play.
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0xE00);
		client->outbuf.setSubType(0xE81);
		client->outbuf.appendShort(static_cast<uint16_t>(server->startingCars.size()));
		for (uint32_t i = 0; i < server->startingCars.size(); i++) client->outbuf.appendInt(server->startingCars[i]);
	}
	break;
	default:
		return;
	}
	client->Send();
}

void ClientPacket10(CLIENT* client)
{
	unsigned short pType = client->inbuf.getType();

	switch (pType)
	{
	case 0x1000:
	{	// Players details
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x1000);
		client->outbuf.setSubType(0x1080);
		client->outbuf.appendInt(client->getActiveCar() + 1); // ???
		client->outbuf.appendInt(client->careerdata.CP); // CP
		client->outbuf.appendByte(client->careerdata.level); // Level
		client->outbuf.appendInt(client->getActiveCar() + 1); // ???
		client->outbuf.appendByte(0); // Exp %
	}
	break;
	default:
		return;
	}
	client->Send();
}

void ClientPacket11(CLIENT* client)
{
	unsigned short pType = client->inbuf.getType();

	switch (pType)
	{
	case 0x1100:
	{
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x1100);
		client->outbuf.setSubType(0x1180);
		client->outbuf.appendByte(0);
	}
	break;
	case 0x1101:
	{	// Item list
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x1100);
		client->outbuf.setSubType(0x1180);
		client->outbuf.appendByte(10);
		for (int i = 0; i<10; i++) client->outbuf.appendShort(rand() % 10);
	}
	break;
	/*case 0x1101:
	{	// Use Item : ID at 0x06

	}
	break;*/
	case 0x1107:
	{	// Trash Item: ID at 0x06
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x1100);
		client->outbuf.setSubType(0x1180);
		client->outbuf.appendByte(10);
		for (int i = 0; i<10; i++) client->outbuf.appendShort(0x22 * i);
	}
	break;
	default:
		return;
	}
	client->Send();
}

void ClientPacket12(CLIENT* client)
{
	unsigned short pType = client->inbuf.getType();

	switch (pType)
	{
	case 0x1201:
	{	// Team Creation - Name @ 0x04
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x1200);
		client->outbuf.setSubType(0x1281);
		client->outbuf.appendByte(0x00); // reason?
		client->outbuf.appendInt(0x01); // Team ID
	}
	break;
	case 0x1204:
	{   // When entering garage and highway
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x1200);
		client->outbuf.setSubType(0x1284);
		client->outbuf.appendByte(0x03); // Case switch
		//	0xFF: byte from 0x6d15aa moved to stack
		//	0x00 : 0 moved to 0x6d1423, byte from 0x6d15aa moved to stack
		//	0x01 : 0 moved to 0x6d1423, byte from 0x6d15aa moved to stack
		//	0x02 : 0 moved to 0x6d1423, byte from 0x6d15aa moved to stack
		//	0x03 : byte from 0x6d15aa moved to stack
		//	0x09 : 0 moved to 0x6d1423, byte from 0x6d15aa moved to stack
		//	0x0b : 0 moved to 0x6d1423, byte from 0x6d15aa moved to stack
		//	0x0d : 0 moved to 0x6d1423, byte from 0x6d15aa moved to stack
		//	0x0f : 0 moved to 0x6d1423, team name loaded from 0x6d1074
		//	0x11 : team name loaded from 0x6d1074 - Team name disappear. Team options in "DATA" disabled.
		//	0x12 : 0 moved to 0x6d1423, byte from 0x6d15aa moved to stack
		//	default: byte from 0x6d15aa moved to stack

		client->outbuf.appendByte(0); // ???
	}
	break;
	case 0x120C:
	{
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x1200);
		client->outbuf.setSubType(0x128C);
		client->outbuf.appendByte(0x00);
		client->outbuf.appendArray((unsigned char *)&client->teamdata, sizeof(client->teamdata));
	}
	break;
	case 0x120A:
	{	// Team Creation - Finalize?
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x07);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x1200);
		client->outbuf.setSubType(0x128A);
		client->outbuf.appendByte(0x00); // reason?
	}
	break;
	case 0x120B:
	{	// Team Creation - ID @ 0x04, Comment @ 0x08
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x07);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x1200);
		client->outbuf.setSubType(0x128B);
		client->outbuf.appendByte(0x00); // reason
	}
	break;
	case 0x120D:
	{	// Team List
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x1200);
		client->outbuf.setSubType(0x128D);
		client->outbuf.appendInt(0);
		client->outbuf.appendShort(0);
		client->outbuf.appendInt(0);
		client->outbuf.appendInt(0);
		client->outbuf.appendInt(0);
		client->outbuf.appendInt(0);
		client->outbuf.appendInt(0);
		client->outbuf.appendInt(0);
		client->outbuf.appendInt(0);
		client->outbuf.appendInt(0);
		client->outbuf.appendInt(0);
		client->outbuf.appendInt(0);
	}
	break;
	case 0x120E:
	{
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x1200);
		client->outbuf.setSubType(0x128E);
		client->outbuf.appendInt(5); // number of members out of limit
		client->outbuf.appendShort(5); // Number of members
		client->outbuf.appendInt(1);
		client->outbuf.setString("Testing 1", client->outbuf.getOffset()); // Member Name
		client->outbuf.addOffset(0x10);
		client->outbuf.addSize(0x10);
		client->outbuf.appendInt(1234); // Rank
		client->outbuf.appendByte(0x01);
		client->outbuf.appendByte(0x01);
		client->outbuf.appendInt(1);
		client->outbuf.setString("Testing 2", client->outbuf.getOffset()); // Member Name
		client->outbuf.addOffset(0x10);
		client->outbuf.addSize(0x10);
		client->outbuf.appendInt(1235); // Rank
		client->outbuf.appendByte(0x02);
		client->outbuf.appendByte(0x02);
		client->outbuf.appendInt(1);
		client->outbuf.setString("Testing 3", client->outbuf.getOffset()); // Member Name
		client->outbuf.addOffset(0x10);
		client->outbuf.addSize(0x10);
		client->outbuf.appendInt(1236); // Rank
		client->outbuf.appendByte(0x03);
		client->outbuf.appendByte(0x03);
		client->outbuf.appendInt(1);
		client->outbuf.setString("Testing 4", client->outbuf.getOffset()); // Member Name
		client->outbuf.addOffset(0x10);
		client->outbuf.addSize(0x10);
		client->outbuf.appendInt(1237); // Rank
		client->outbuf.appendByte(0x04);
		client->outbuf.appendByte(0x04);
		client->outbuf.appendInt(1);
		client->outbuf.setString("Testing 5", client->outbuf.getOffset()); // Member Name
		client->outbuf.addOffset(0x10);
		client->outbuf.addSize(0x10);
		client->outbuf.appendInt(1237); // Rank
		client->outbuf.appendByte(0x05);
		client->outbuf.appendByte(0x05);
	}
	break;
	case 0x1211:
	{
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x1200);
		client->outbuf.setSubType(0x1291);
		// Below is switched on byte. 0xff: return, 0x05: return, 0x07: return, 0x0d: move 0 to 0x6d1423, default: return.
		// 0xff: TEAM NAME with japanese below
		// 0x05: TEAM NAME with japanese below
		// 0x07: TEAM NAME with japanese below
		// 0x0d: Displays team data
		client->outbuf.appendByte(0x0D);
		client->outbuf.appendArray((unsigned char *)&client->teamdata, sizeof(client->teamdata));
	}
	break;
	case 0x121C:
	{ // Looks like it contains car details similar structure to 0281 Packet
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x1200);
		client->outbuf.setSubType(0x129C);
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x200);
		client->outbuf.setSubType(0x281);
		client->outbuf.appendInt(client->getActiveCar() + 1); // Active car slot from e80 on new accounts and 182 on existing
		client->outbuf.appendInt(client->garagedata.activeCar->carID); // Car ID
		client->outbuf.appendInt(client->garagedata.activeCar->KMs); // KMs
		client->outbuf.appendByte(0);
		client->outbuf.appendShort(static_cast<uint16_t>(client->garagedata.activeCar->engineCondition)); // Engine Condition
		client->outbuf.appendByte(0);
		client->outbuf.appendArray((uint8_t*)&client->garagedata.activeCar->carmods, sizeof(client->garagedata.activeCar->carmods));
		client->outbuf.appendShort(0); // ????
	}
	break;
	default:
		return;
	}
	client->Send();
}

void ClientPacket15(CLIENT* client)
{
	unsigned short pType = client->inbuf.getType();

	switch (pType)
	{
	case 0x1500:
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x1500);
		client->outbuf.setSubType(0x1580);
		client->outbuf.appendShort(0x00);
		client->outbuf.appendInt(0);
		break;
	}
	client->Send();
}

void ClientPacket16(CLIENT* client)
{
	unsigned short pType = client->inbuf.getType();

	switch (pType)
	{
	case 0x1600: // Icons
	{
		client->outbuf.clearBuffer();
		client->outbuf.setSize(0x06);
		client->outbuf.setOffset(0x06);
		client->outbuf.setType(0x1600);
		client->outbuf.setSubType(0x1680);
		client->outbuf.appendShort(0);
		for (int i = 0; i < 24; i++) client->outbuf.appendInt(11+i);
	}
	break;
	}
	client->Send();
}

PacketFunction* MainPacketFunctions[] =
{
	&ClientPacket00,				// 0x0000
	&ClientPacketAuthentication,	// 0x0100
	&ClientPacket02,				// 0x0200
	&ClientPacket03,				// 0x0300
	&ClientPacket04,				// 0x0400
	&ClientPacketDoNothing,			// 0x0500
	&ClientPacket06,				// 0x0600
	&ClientPacket07,				// 0x0700
	&ClientPacketDoNothing,			// 0x0800
	&ClientPacket09,				// 0x0900
	&ClientPacket0A,				// 0x0A00
	&ClientPacketDoNothing,			// 0x0B00
	&ClientPacket0C,				// 0x0C00
	&ClientPacketDoNothing,			// 0x0D00
	&ClientPacketPlayerCreation,	// 0x0E00
	&ClientPacketDoNothing,			// 0x0F00
	&ClientPacket10,				// 0x1000
	&ClientPacket11,				// 0x1100
	&ClientPacket12,				// 0x1200
	&ClientPacketDoNothing,			// 0x1300
	&ClientPacketDoNothing,			// 0x1400
	&ClientPacket15,				// 0x1500
	&ClientPacket16,				// 0x1600
	&ClientPacketDoNothing,			// 0x1700
	&ClientPacketDoNothing,			// 0x1800
	&ClientPacketDoNothing,			// 0x1900
	&ClientPacketDoNothing,			// 0x1A00
	&ClientPacketDoNothing,			// 0x1B00
	&ClientPacketDoNothing,			// 0x1C00
	&ClientPacketDoNothing,			// 0x1D00
	&ClientPacketDoNothing,			// 0x1E00
	&ClientPacketDoNothing,			// 0x1F00
};