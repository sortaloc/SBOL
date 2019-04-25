#define _WINSOCKAPI_
#include <Windows.h>
#include "serverpacket.h"

SERVERPACKET::SERVERPACKET()
{
	pOffset = 0;
	pSize = 0;
	ZeroMemory(buffer, sizeof(buffer));
}

SERVERPACKET::~SERVERPACKET()
{
}

void SERVERPACKET::appendByte(uint8_t in)
{
	if (pOffset < CLIENT_BUFFER_SIZE)
	{
		buffer[pOffset] = in;
		pOffset += sizeof(in);
		setSize(getSize() + sizeof(in));
	}
}

void SERVERPACKET::setByte(uint8_t in, uint32_t offset)
{
	if (offset + sizeof(in) < CLIENT_BUFFER_SIZE)
	{
		buffer[offset] = in;
	}
}

void SERVERPACKET::appendSByte(int8_t in)
{
	if (pOffset < CLIENT_BUFFER_SIZE)
	{
		buffer[pOffset] = in;
		pOffset += sizeof(in);
		setSize(getSize() + sizeof(in));
	}
}


void SERVERPACKET::setSByte(int8_t in, uint32_t offset)
{
	if (offset + sizeof(in) < CLIENT_BUFFER_SIZE)
	{
		buffer[offset] = in;
	}
}

void SERVERPACKET::appendShort(uint16_t in)
{
	if ((pOffset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		*(uint16_t*)&buffer[pOffset] = in;
		pOffset += sizeof(in);
		setSize(getSize() + sizeof(in));
	}
}

void SERVERPACKET::setShort(uint16_t in, uint32_t offset)
{
	if ((offset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		*(uint16_t*)&buffer[offset] = in;
	}
}

void SERVERPACKET::appendSShort(int16_t in)
{
	if ((pOffset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		*(int16_t*)&buffer[pOffset] = in;
		pOffset += sizeof(in);
		setSize(getSize() + sizeof(in));
	}
}

void SERVERPACKET::setSShort(int16_t in, uint32_t offset)
{
	if ((offset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		*(int16_t*)&buffer[offset] = in;
	}
}

void SERVERPACKET::appendInt(uint32_t in)
{
	if ((pOffset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		*(uint32_t*)&buffer[pOffset] = in;
		pOffset += sizeof(in);
		setSize(getSize() + sizeof(in));
	}
}

void SERVERPACKET::setInt(uint32_t in, uint32_t offset)
{
	if ((offset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		*(uint32_t*)&buffer[offset] = in;
	}
}

void SERVERPACKET::appendSInt(int32_t in)
{
	if ((pOffset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		*(int32_t*)&buffer[pOffset] = in;
		pOffset += sizeof(in);
		setSize(getSize() + sizeof(in));
	}
}

void SERVERPACKET::setSInt(int32_t in, uint32_t offset)
{
	if ((offset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		*(int32_t*)&buffer[offset] = in;
	}
}

void SERVERPACKET::appendInt64(uint64_t in)
{
	if ((pOffset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		*(uint64_t*)&buffer[pOffset] = in;
		pOffset += sizeof(in);
		setSize(getSize() + sizeof(in));
	}
}

void SERVERPACKET::setInt64(uint64_t in, uint32_t offset)
{
	if ((offset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		*(uint64_t*)&buffer[offset] = in;
	}
}

void SERVERPACKET::appendFloat(float in)
{
	if ((pOffset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		int32_t tmp = *(int32_t*)&in;
		*(float*)&buffer[pOffset] = *(float*)&tmp;
		*(float*)&buffer[pOffset] = in;
		pOffset += sizeof(in);
		setSize(getSize() + sizeof(in));
	}
}

void SERVERPACKET::setFloat(float in, uint32_t offset)
{
	if ((offset + sizeof(in)) < CLIENT_BUFFER_SIZE)
	{
		int32_t tmp = *(int32_t*)&in;
		*(float*)&buffer[pOffset] = *(float*)&tmp;
	}
}

void SERVERPACKET::appendArray(uint8_t* in, uint32_t size)
{
	if ((pOffset + size) < CLIENT_BUFFER_SIZE)
	{
		memcpy(&buffer[pOffset], in, size);
		pOffset += size;
		setSize(getSize() + size);
	}
}

void SERVERPACKET::setArray(uint8_t* in, uint32_t size, uint32_t offset)
{
	if ((offset + size) < CLIENT_BUFFER_SIZE)
	{
		memcpy(&buffer[offset], in, size);
	}
}

void SERVERPACKET::getArray(uint8_t * in, uint32_t size, uint32_t offset)
{
	if ((offset + size) < CLIENT_BUFFER_SIZE)
	{
		memcpy(&in[0], &buffer[offset], size);
	}
}

void SERVERPACKET::getArray(uint8_t * in, uint32_t size)
{
	if ((pOffset + size) < CLIENT_BUFFER_SIZE)
	{
		memcpy(&in[0], &buffer[pOffset], size);
		pOffset += size;
	}
}

int32_t SERVERPACKET::appendString(string cmd)
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

int32_t SERVERPACKET::setString(string cmd, uint32_t offset)
{
	int32_t size = cmd.length();
	if (offset + size < CLIENT_BUFFER_SIZE)
	{
		strcpy_s((char *)&buffer[offset], size + 1, &cmd[0]);
		return size + 1;
	}
	return 0;
}

string SERVERPACKET::getString(uint32_t offset)
{
	string tempString;
	tempString.assign((char*)&buffer[offset]);
	return tempString;
}

string SERVERPACKET::getString(uint32_t offset, uint32_t size)
{
	string tempString;
	if (offset + size > CLIENT_BUFFER_SIZE)
		return "";
	tempString.assign((char*)&buffer[offset], offset + size);
	if (tempString.find("\0") != string::npos)
		tempString.assign((char*)&buffer[offset]);
	return tempString;
}

string SERVERPACKET::getStringA(uint32_t size)
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

void SERVERPACKET::clearBuffer()
{
	pOffset = 0;
	pSize = 0;
	memset(&buffer[0x00], 0, sizeof(buffer));
}

uint8_t SERVERPACKET::getByte(uint32_t offset)
{
	if (offset + sizeof(uint8_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
		return *(uint8_t*)&buffer[offset];
}

uint8_t SERVERPACKET::getByte()
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

int8_t SERVERPACKET::getSByte(uint32_t offset)
{
	if (offset + sizeof(int8_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
		return *(int8_t*)&buffer[offset];
}

int8_t SERVERPACKET::getSByte()
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

uint16_t SERVERPACKET::getShort(uint32_t offset)
{
	if (offset + sizeof(uint16_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
		return (*(uint16_t*)&buffer[offset]);
}

uint16_t SERVERPACKET::getShort()
{
	if (pOffset + sizeof(uint16_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
	{
		uint16_t value = *(uint16_t*)&buffer[pOffset];
		pOffset += sizeof(uint16_t);
		return value;
	}
}

int16_t SERVERPACKET::getSShort(uint32_t offset)
{
	if (offset + sizeof(int16_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
		return (*(int16_t*)&buffer[offset]);
}

int16_t SERVERPACKET::getSShort()
{
	if (pOffset + sizeof(int16_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
	{
		int16_t value = *(int16_t*)&buffer[pOffset];
		pOffset += sizeof(int16_t);
		return value;
	}
}

uint32_t SERVERPACKET::getInt(uint32_t offset)
{
	if (offset + sizeof(uint32_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
		return (*(uint32_t*)&buffer[offset]);
}

uint32_t SERVERPACKET::getInt()
{
	if (pOffset + sizeof(uint32_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
	{
		uint32_t value = *(uint32_t*)&buffer[pOffset];
		pOffset += sizeof(uint32_t);
		return value;
	}
}

int32_t SERVERPACKET::getSInt(uint32_t in)
{
	if (in + sizeof(int32_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
		return (*(int32_t*)&buffer[in]);
}

int32_t SERVERPACKET::getSInt()
{
	if (pOffset + sizeof(int32_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
	{
		int32_t value = *(int32_t*)&buffer[pOffset];
		pOffset += sizeof(int32_t);
		return value;
	}
}

uint64_t SERVERPACKET::getInt64(uint32_t offset)
{
	if (offset + sizeof(uint64_t) > CLIENT_BUFFER_SIZE)
		return 0;
	else
		return *(uint64_t*)&buffer[offset];
}

uint64_t SERVERPACKET::getInt64()
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

float SERVERPACKET::getFloat(uint32_t offset)
{
	int32_t tmp;
	if (offset + sizeof(float) > CLIENT_BUFFER_SIZE)
		return 0.0f;
	else
	{
		tmp = (*(int32_t*)&buffer[offset]);
		return *(float *)&tmp;
	}
}

float SERVERPACKET::getFloat()
{
	int32_t tmp;
	if (pOffset + sizeof(float) > CLIENT_BUFFER_SIZE)
		return 0.0f;
	else
	{
		tmp = (*(int32_t*)&buffer[pOffset]);
		pOffset += sizeof(int32_t);
		return *(float *)&tmp;
	}
}

uint16_t SERVERPACKET::getType()
{
	return (*(uint16_t*)&buffer[0x02]);
}

void SERVERPACKET::setType(uint16_t in)
{
	*(uint16_t*)&buffer[0x02] = (in);
}

uint16_t SERVERPACKET::getSubType()
{
	return (*(uint16_t*)&buffer[0x04]);
}

void SERVERPACKET::setSubType(uint16_t in)
{
	*(uint16_t*)&buffer[0x04] = (in);
}


