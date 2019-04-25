#define _WINSOCKAPI_
#include <windows.h>
#include "packet.h"

PACKET::PACKET()
{
	pOffset = 0;
	pSize = 0;
	ZeroMemory(&buffer[0], sizeof(buffer));
}


PACKET::~PACKET()
{
}

void PACKET::appendByte(uint8_t in)
{
	if (pOffset < CLIENT_BUFFER_SIZE)
	{
		buffer[pOffset] = in;
		pOffset += sizeof(in);
		setSize(getSize() + sizeof(in));
	}
}

void PACKET::setByte(uint8_t in, uint32_t offset)
{
	if (offset + sizeof(in) < CLIENT_BUFFER_SIZE)
	{
		buffer[offset] = in;
	}
}

void PACKET::appendSByte(int8_t in)
{
	if (pOffset < CLIENT_BUFFER_SIZE)
	{
		buffer[pOffset] = in;
		pOffset += sizeof(in);
		setSize(getSize() + sizeof(in));
	}
}


void PACKET::setSByte(int8_t in, uint32_t offset)
{
	if (offset + sizeof(in) < CLIENT_BUFFER_SIZE)
	{
		buffer[offset] = in;
	}
}

void PACKET::appendShort(uint16_t in, bool SWAP)
{
	if ((pOffset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		*(uint16_t*)&buffer[pOffset] = SWAP ? SWAP_SHORT(in) : in;
		pOffset += sizeof(in);
		setSize(getSize() + sizeof(in));
	}
}

void PACKET::setShort(uint16_t in, uint32_t offset, bool SWAP)
{
	if ((offset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		*(uint16_t*)&buffer[offset] = SWAP ? SWAP_SHORT(in) : in;
	}
}

void PACKET::appendSShort(int16_t in, bool SWAP)
{
	if ((pOffset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		*(int16_t*)&buffer[pOffset] = SWAP ? SWAP_SHORT(in) : in;
		pOffset += sizeof(in);
		setSize(getSize() + sizeof(in));
	}
}

void PACKET::setSShort(int16_t in, uint32_t offset, bool SWAP)
{
	if ((offset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		*(int16_t*)&buffer[offset] = SWAP ? SWAP_SHORT(in) : in;
	}
}

void PACKET::appendInt(uint32_t in, bool SWAP)
{
	if ((pOffset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		*(uint32_t*)&buffer[pOffset] = SWAP ? SWAP_LONG(in) : in;
		pOffset += sizeof(in);
		setSize(getSize() + sizeof(in));
	}
}

void PACKET::setInt(uint32_t in, uint32_t offset, bool SWAP)
{
	if ((offset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		*(uint32_t*)&buffer[offset] = SWAP ? SWAP_LONG(in) : in;
	}
}

void PACKET::appendSInt(int32_t in, bool SWAP)
{
	if ((pOffset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		*(int32_t*)&buffer[pOffset] = SWAP ? SWAP_LONG(in) : in;
		pOffset += sizeof(in);
		setSize(getSize() + sizeof(in));
	}
}

void PACKET::setSInt(int32_t in, uint32_t offset, bool SWAP)
{
	if ((offset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		*(int32_t*)&buffer[offset] = SWAP ? SWAP_LONG(in) : in;
	}
}

void PACKET::appendInt64(uint64_t in)
{
	if ((pOffset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		*(uint64_t*)&buffer[pOffset] = in;
		pOffset += sizeof(in);
		setSize(getSize() + sizeof(in));
	}
}

void PACKET::setInt64(uint64_t in, uint32_t offset)
{
	if ((offset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		*(uint64_t*)&buffer[offset] = in;
	}
}

void PACKET::appendFloat(float in, bool SWAP)
{
	if ((pOffset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		int32_t tmp = SWAP ? SWAP_LONG(*(int32_t*)&in) : *(int32_t*)&in;
		*(float*)&buffer[pOffset] = *(float*)&tmp;
		*(float*)&buffer[pOffset] = in;
		pOffset += sizeof(in);
		setSize(getSize() + sizeof(in));
	}
}

void PACKET::setFloat(float in, uint32_t offset, bool SWAP)
{
	if ((offset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		int32_t tmp = SWAP ? SWAP_LONG(*(int32_t*)&in) : *(int32_t*)&in;
		*(float*)&buffer[pOffset] = *(float*)&tmp;
	}
}

void PACKET::appendArray(uint8_t* in, uint32_t size)
{
	if ((pOffset + size) < CLIENT_BUFFER_SIZE)
	{
		memcpy(&buffer[pOffset], in, size);
		pOffset += size;
		setSize(getSize() + size);
	}
}

void PACKET::setArray(uint8_t* in, uint32_t size, uint32_t offset)
{
	if ((offset + size) < CLIENT_BUFFER_SIZE)
	{
		memcpy(&buffer[offset], in, size);
	}
}

void PACKET::getArray(uint8_t * in, uint32_t size, uint32_t offset)
{
	if ((offset + size) < CLIENT_BUFFER_SIZE)
	{
		memcpy(&in[0], &buffer[offset], size);
	}
}

void PACKET::getArray(uint8_t * in, uint32_t size)
{
	if ((pOffset + size) < CLIENT_BUFFER_SIZE)
	{
		memcpy(&in[0], &buffer[pOffset], size);
		pOffset += size;
	}
}

int32_t PACKET::appendString(string cmd)
{
	int32_t size = cmd.length();
	if (pOffset + size < CLIENT_BUFFER_SIZE)
	{
		strcpy_s((char *)&buffer[pOffset], size + 1, &cmd[0]);
		pOffset += size;
		setSize(getSize() + size);
		return size + 1;
	}
	return 0;
}

int32_t PACKET::appendString(string cmd, uint32_t addToSize)
{
	int32_t size = cmd.length();
	if (pOffset + size < CLIENT_BUFFER_SIZE)
	{
		strcpy_s((char *)&buffer[pOffset], size + 1, &cmd[0]);
		pOffset += addToSize;
		setSize(getSize() + addToSize);
		return addToSize;
	}
	return 0;
}

int PACKET::setString(string cmd, uint32_t offset)
{
	int32_t size = cmd.length();
	if (offset + size < CLIENT_BUFFER_SIZE)
	{
		strcpy_s((char *)&buffer[offset], size + 1, &cmd[0]);
		return size + 1;
	}
	return 0;
}

string PACKET::getString(uint32_t offset)
{
	string tempString;
	tempString.assign((char*)&buffer[offset]);
	return tempString;
}

string PACKET::getString(uint32_t offset, uint32_t size)
{
	string tempString;
	if (offset + size > CLIENT_BUFFER_SIZE)
		return "";
	tempString.assign((char*)&buffer[offset], offset + size);
	if (tempString.find("\0") != string::npos)
		tempString.assign((char*)&buffer[offset]);
	return tempString;
}

string PACKET::getStringA(uint32_t size)
{
	string tempString;
	if (pOffset + size > CLIENT_BUFFER_SIZE)
		return "";
	tempString.assign((char*)&buffer[pOffset], pOffset + size);
	if (tempString.find("\0") != string::npos)
		tempString.assign((char*)&buffer[pOffset]);
	pOffset += size;
	return tempString;
}

void PACKET::clearBuffer()
{
	pOffset = 0;
	pSize = 0;
	ZeroMemory(&buffer[0], sizeof(buffer));
}

uint8_t PACKET::getByte(uint32_t offset)
{
	if (offset + sizeof(uint8_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
		return *(uint8_t*)&buffer[offset];
}

uint8_t PACKET::getByte()
{
	if (pOffset + sizeof(uint8_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
	{
		uint8_t value = *(uint8_t*)&buffer[pOffset];
		pOffset += sizeof(uint8_t);
		return value;
	}
}

int8_t PACKET::getSByte(uint32_t offset)
{
	if (offset + sizeof(int8_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
		return *(int8_t*)&buffer[offset];
}

int8_t PACKET::getSByte()
{
	if (pOffset + sizeof(int8_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
	{
		int8_t value = *(int8_t*)&buffer[pOffset];
		pOffset += sizeof(int8_t);
		return value;
	}
}

uint16_t PACKET::getShort(uint32_t offset)
{
	if (offset + sizeof(uint16_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
		return SWAP_SHORT(*(uint16_t*)&buffer[offset]);
}

uint16_t PACKET::getShort()
{
	if (pOffset + sizeof(uint16_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
	{
		uint16_t value = SWAP_SHORT(*(uint16_t*)&buffer[pOffset]);
		pOffset += sizeof(uint16_t);
		return value;
	}
}

int16_t PACKET::getSShort(uint32_t offset)
{
	if (offset + sizeof(int16_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
		return SWAP_SHORT(*(int16_t*)&buffer[offset]);
}

int16_t PACKET::getSShort()
{
	if (pOffset + sizeof(int16_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
	{
		int16_t value = SWAP_SHORT(*(int16_t*)&buffer[pOffset]);
		pOffset += sizeof(int16_t);
		return value;
	}
}

uint32_t PACKET::getInt(uint32_t offset)
{
	if (offset + sizeof(uint32_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
		return SWAP_LONG(*(uint32_t*)&buffer[offset]);
}

uint32_t PACKET::getInt()
{
	if (pOffset + sizeof(uint32_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
	{
		uint32_t value = SWAP_LONG(*(uint32_t*)&buffer[pOffset]);
		pOffset += sizeof(uint32_t);
		return value;
	}
}

int PACKET::getSInt(uint32_t offset)
{
	if (offset + sizeof(int32_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
		return SWAP_LONG(*(int32_t*)&buffer[offset]);
}

int PACKET::getSInt()
{
	if (pOffset + sizeof(int32_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
	{
		int32_t value = SWAP_LONG(*(int32_t*)&buffer[pOffset]);
		pOffset += sizeof(int32_t);
		return value;
	}
}

uint64_t PACKET::getInt64(uint32_t offset)
{
	if (offset + sizeof(uint64_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
		return *(uint64_t*)&buffer[offset];
}

uint64_t PACKET::getInt64()
{
	if (pOffset + sizeof(uint64_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
	{
		uint64_t value = *(uint64_t*)&buffer[pOffset];
		pOffset += sizeof(uint64_t);
		return value;
	}
}

float PACKET::getFloat(uint32_t offset)
{
	int tmp;
	if (offset + sizeof(float) > CLIENT_BUFFER_SIZE)
		return 0.0f;
	else
	{
		tmp = SWAP_LONG(*(int32_t*)&buffer[offset]);
		return *(float *)&tmp;
	}
}

float PACKET::getFloat()
{
	int tmp;
	if (pOffset + sizeof(float) > CLIENT_BUFFER_SIZE)
		return 0.0f;
	else
	{
		tmp = SWAP_LONG(*(int32_t*)&buffer[pOffset]);
		pOffset += sizeof(float);
		return *(float *)&tmp;
	}
}

uint16_t PACKET::getType()
{
	return SWAP_SHORT(*(uint16_t*)&buffer[0x02]);
}

void PACKET::setType(uint16_t in)
{
	*(uint16_t*)&buffer[0x02] = SWAP_SHORT(in);
}

uint16_t PACKET::getSubType()
{
	return SWAP_SHORT(*(uint16_t*)&buffer[0x04]);
}

void PACKET::setSubType(uint16_t in)
{
	*(uint16_t*)&buffer[0x04] = SWAP_SHORT(in);
}

uint32_t PACKET::getVer()
{
	return SWAP_LONG(*(uint32_t*)&buffer[0x04]);
}

void PACKET::setVer(uint32_t in)
{
	*(uint32_t*)&buffer[0x04] = SWAP_LONG(in);
}

