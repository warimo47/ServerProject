#pragma once

#include <unordered_set>
#include <mutex>

#include "protocol.h"
#include "Error.h"

struct EXOVER
{
	WSAOVERLAPPED over;
	char ioBuffer[MAX_BUFF_SIZE];
	WSABUF wsaBuffer;
	unsigned char ioType;
};

class Client
{
private:

protected:

public:
	SOCKET socket;

	EXOVER exOver; // Client extension structure
	int packetSize; // Size of the packet that is being assembled
	int	prevPacketSize; // The size of the front part of the packet that was saved because it was not completed in the last recv
	unsigned char packetBuffer[MAX_PACKET_SIZE]; // Client Packet Buffer
	
private:

protected:

public:
	Client();
	Client(SOCKET s);
	~Client();
};

