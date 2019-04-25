#pragma once
#define _WINSOCKAPI_
#include <Windows.h>
#include "server.h"
typedef void(PacketFunction)(CLIENT* client);

void ManagementPacketDoNothing(CLIENT* client)
{
	// Do nothing
}
void ManagementPacketClientAuth(CLIENT* client)
{
	Server* server = (Server*)client->server;
	switch (client->serverbuf.getSubType())
	{
	case 0x0000:
		{
			client->serverbuf.setOffset(0x0A);
			int32_t license = client->serverbuf.getSInt();
			if (license == -1)
			{
				client->SendAuthError(STRINGS[STR_INVALID_UNPW]);
				return;
				//client->Disconnect();
			}
			else
			{
				uint8_t loggedIn = client->serverbuf.getByte();
				if (loggedIn)
				{
					client->SendAuthError(STRINGS[STR_LOGGED_IN]);
					client->Disconnect();
					return;
				}
				else
				{
					client->driverslicense = license;
					string handle = client->serverbuf.getStringA(0x10);
					client->careerdata.CP = client->serverbuf.getInt64();
					client->careerdata.level = client->serverbuf.getByte();
					client->careerdata.experiencePoints = client->serverbuf.getInt();
					client->careerdata.playerWin = client->serverbuf.getInt();
					client->careerdata.playerLose = client->serverbuf.getInt();
					client->careerdata.rivalWin = client->serverbuf.getInt();
					client->careerdata.rivalLose = client->serverbuf.getInt();
					int8_t activeCarBay = client->serverbuf.getSByte();
					client->privileges = client->serverbuf.getByte();
					client->teamdata.teamID = client->serverbuf.getInt();
					uint32_t flags = client->serverbuf.getInt();
					uint32_t ranking = client->serverbuf.getInt();
					if (flags & 1)
					{
						client->SendAuthError(STRINGS[STR_BANNED]);
						client->Disconnect();
						return;
					}
					else if (flags & 128)
					{
						client->SendAuthError(STRINGS[STR_PASS_RESET]);
						client->Disconnect();
						return;
					}
					else
					{
						if (handle == "")
						{ // Create new handle
							client->outbuf.clearBuffer();
							client->outbuf.setSize(0x06);
							client->outbuf.setOffset(0x06);
							client->outbuf.setType(0x100);
							client->outbuf.setSubType(0x182);
							client->outbuf.appendByte(0); // Players garage exists
							client->outbuf.appendByte(2); // if 2 additional int below. If 0 Team Center and Tuned Car Exchange is unavailable If anything else both available but no need for additional int
							client->outbuf.appendInt(0xffffffff); // Some int from above value If 0 client crashes at every shop
							client->outbuf.appendInt(1); // ???
							client->outbuf.appendInt(1); // ???
							// Team Data
							client->teamdata.teamID = 0xffffffff; //When no team set id -1 and all other data 0'd
							client->outbuf.appendArray((uint8_t*)&client->teamdata, sizeof(client->teamdata));
							// End of team data

							client->outbuf.appendShort(client->privileges ? 1 : 0); // Is GM? Give super speed
							client->outbuf.appendByte(0x00); // ????
							client->outbuf.appendInt(ranking); // Ranking number
							client->outbuf.appendByte(0); // Below value count // No purchased items
							//client->outbuf.appendInt(1); // ???? \				
							//client->outbuf.appendInt(1); // ???? | - values from above						
							
							client->outbuf.appendInt(0); // Time Played
							client->outbuf.appendShort(0x24); // Course Section
							client->outbuf.appendShort(0xffff); // ??? Both of these check for 0xffff
							client->outbuf.appendShort(0xffff); // ???
							client->outbuf.appendShort(0xffff); // appears to back the user out if lower than 1 - Doesn't any more when the additional byte below ranking number isn't set
							client->outbuf.appendByte(0); // Course number
						}
						else
						{ // Load Existing Account

							client->canSave = TRUE;
							client->initializeGarage();
							client->handle = handle;
							uint8_t carCount = client->serverbuf.getByte();
							for (uint32_t i = 0; i < carCount; i++)
							{
								uint8_t bay = client->serverbuf.getByte();
								client->garagedata.car[bay].carID = client->serverbuf.getInt();
								client->garagedata.car[bay].KMs = client->serverbuf.getInt();
								client->serverbuf.getArray((uint8_t*)&client->garagedata.car[bay].carmods, sizeof(client->garagedata.car[bay].carmods));
								client->garagedata.car[bay].engineCondition = client->serverbuf.getInt();
							}
							client->setActiveCar(activeCarBay);

							uint32_t itemCount = client->serverbuf.getInt();
							client->itembox.resize(itemCount);
							client->inbuf.getArray((uint8_t*)client->itembox.data(), sizeof(ITEMBOX_ITEM) * itemCount);

							// TODO: Process Team Data
							uint8_t inTeam = 0;//client->serverbuf.getByte();
							if (inTeam)
							{

							}

							client->outbuf.clearBuffer();
							client->outbuf.setSize(0x06);
							client->outbuf.setOffset(0x06);
							client->outbuf.setType(0x100);
							client->outbuf.setSubType(0x182);
							client->outbuf.appendByte(1); // Players garage exists
							client->outbuf.appendString(client->handle, 0x10);
							client->outbuf.appendInt(client->getActiveCar() + 1); // Active Car Slot
							client->outbuf.appendByte(5); // if 2 additional int below
							//client->outbuf.appendInt(1); // Some int from above value
							client->outbuf.appendInt(0); // ???
							client->outbuf.appendInt(0); // ???
							// Team Data
							if (!inTeam)
								client->teamdata.teamID = 0xffffffff; //When no team set id -1 and all other data 0'd
							else
							{
								client->teamdata.teamID = 1;
								strcpy(&client->teamdata.name[0], "Tofu Delivery");
								client->teamdata.survivalLoses = 0;
								client->teamdata.survivalWins = 0;
								client->teamdata.memberCount = 5;
								client->teamdata.unknown3 = 0;
								client->teamdata.unknown4 = 0;
								client->teamdata.unknown5 = 0;

								for (int i = 0; i < sizeof(client->teamdata.unknown1); i++) client->teamdata.unknown1[i] = 0x00;// 0x01 + i;
								for (int i = 0; i < sizeof(client->teamdata.unknown2); i++) client->teamdata.unknown2[i] = 0x00; //0x01 + i;
								
								strcpy(&client->teamdata.comment[0], "Comment!!!!!!");
								strcpy(&client->teamdata.leaderName[0], "THE DEVIL");

							}
							client->outbuf.appendArray((unsigned char *)&client->teamdata, sizeof(client->teamdata));
							// End of team data

							client->outbuf.appendShort(client->privileges ? 1 : 0); // Is GM? Give super speed
							client->outbuf.appendByte(0x00); // ????
							client->outbuf.appendInt(ranking); // Ranking number
							client->outbuf.appendByte(0); // Below value count 2 ints per value
							//client->outbuf.appendInt(0); // Garage number 0: main, 1: second
							//client->outbuf.appendInt(1); // Garage Type			
							//client->outbuf.appendInt(1); // 
							//client->outbuf.appendInt(1); // 

							client->outbuf.appendInt(0); // Time Played
							client->outbuf.appendShort(0x0024); // Course Section
							client->outbuf.appendShort(0xffff); // ??? Both of these check for 0xffff
							client->outbuf.appendShort(0xffff); // ???
							client->outbuf.appendShort(0xffff); // appears to back the user out if lower than 1 - Doesn't any more when the additional byte below ranking number isn't set
							client->outbuf.appendByte(0x00); // Course number
							
							/*
							0. Main Course
							1. Shop Course
							2. Car Parts Course
							3. Freeway A
							4. Freeway B
							5. Survival Course
							6. ???
							7. Time Attack
							*/
						}
					}
				}
				client->Send();
			}
		}
		break;
	case 0x0001:
		{
			int32_t clientSocket = client->serverbuf.getSInt(0x06);
			uint8_t result = client->serverbuf.getByte(0x0A);

			client->outbuf.clearBuffer();
			client->outbuf.setSize(0x06);
			client->outbuf.setOffset(0x06);
			client->outbuf.setType(0xE00);
			client->outbuf.setSubType(0xE80);
			client->outbuf.appendByte(result ? 1 : 0); // 0x00 OK, 0x01 Taken
			client->Send();
		}
		break;
	}
}

PacketFunction* ManagementPacketFunctions[] =
{
	&ManagementPacketDoNothing,			// 0x0000 - TBC
	&ManagementPacketClientAuth,		// 0x0001 - Client Authentication
	&ManagementPacketDoNothing,			// 0x0002 - TBC
	&ManagementPacketDoNothing,			// 0x0003 - TBC
	&ManagementPacketDoNothing,			// 0x0004 - TBC
	&ManagementPacketDoNothing,			// 0x0005 - TBC
};