#pragma once
#define _WINSOCKAPI_
#include <string>
#include <Windows.h>
#include "globals.h"

using namespace std;

class PACKET
{
public:
	PACKET();
	~PACKET();
	uint8_t buffer[CLIENT_BUFFER_SIZE];
	void setOffset(uint16_t in) { pOffset = in; }
	uint32_t getOffset() { return pOffset; }
	void addOffset(uint16_t in) { pOffset += in; }
	uint16_t getSize() { return pSize; }
	uint16_t getSizeFromBuffer() { return *(uint16_t*)&buffer[0]; }
	void setSize(uint16_t in) { pSize = in; }
	void addSize(uint16_t in) { pSize += in; }
	uint16_t getType();
	void setType(uint16_t in);
	uint16_t getSubType();
	void setSubType(uint16_t in);
	uint32_t getVer();
	void setVer(uint32_t in);
	void appendByte(uint8_t in);
	void setByte(uint8_t in, uint32_t offset);
	uint8_t getByte(uint32_t offset);
	uint8_t getByte();
	void appendSByte(int8_t in);
	void setSByte(int8_t in, uint32_t offset);
	int8_t getSByte(uint32_t offset);
	int8_t getSByte();
	void appendShort(uint16_t in, bool SWAP = TRUE);
	void setShort(uint16_t in, uint32_t offset, bool SWAP = TRUE);
	uint16_t getShort(uint32_t offset);
	uint16_t getShort();
	void appendSShort(int16_t in, bool SWAP = TRUE);
	void setSShort(int16_t in, uint32_t offset, bool SWAP = TRUE);
	int16_t getSShort(uint32_t offset);
	int16_t getSShort();
	void appendInt(uint32_t in, bool SWAP = TRUE);
	void setInt(uint32_t in, uint32_t offset, bool SWAP = TRUE);
	uint32_t getInt(uint32_t offset);
	uint32_t getInt();
	void appendSInt(int32_t in, bool SWAP = TRUE);
	void setSInt(int32_t in, uint32_t offset, bool SWAP = TRUE);
	int32_t getSInt(uint32_t offset);
	int32_t getSInt();
	void appendInt64(uint64_t in);
	void setInt64(uint64_t in, uint32_t offset);
	uint64_t getInt64(uint32_t offset);
	uint64_t getInt64();
	void appendFloat(float in, bool SWAP = TRUE);
	void setFloat(float in, uint32_t offset, bool SWAP = TRUE);
	float getFloat(uint32_t offset);
	float getFloat();
	void appendArray(uint8_t * in, uint32_t size);
	void setArray(uint8_t * in, uint32_t size, uint32_t offset);
	void getArray(uint8_t * in, uint32_t size, uint32_t offset);
	void getArray(uint8_t * in, uint32_t size);
	int32_t appendString(string cmd);
	int32_t appendString(string cmd, uint32_t addToSize);
	int32_t setString(string cmd, uint32_t offset);
	string getString(uint32_t offset);
	string getString(uint32_t offset, uint32_t size);
	string getStringA(uint32_t size);
	void clearBuffer();
private:
	uint16_t pOffset;
	uint16_t pSize;
};

