#include "Client.h"

Client::Client()
{
	socket = -1;

	ZeroMemory(&exOver.over, sizeof(WSAOVERLAPPED));
	exOver.wsaBuffer.buf = exOver.ioBuffer;
	exOver.wsaBuffer.len = sizeof(exOver.ioBuffer);
	exOver.ioType = IOT_RECV;

	packetSize = 0;
	prevPacketSize = 0;
	ZeroMemory(&packetBuffer, sizeof(packetBuffer));
}

Client::Client(SOCKET s)
{
	socket = s;

	ZeroMemory(&exOver.over, sizeof(WSAOVERLAPPED));
	exOver.wsaBuffer.buf = exOver.ioBuffer;
	exOver.wsaBuffer.len = sizeof(exOver.ioBuffer);
	exOver.ioType = IOT_RECV;

	packetSize = 0;
	prevPacketSize = 0;
	ZeroMemory(&packetBuffer, sizeof(packetBuffer));
}

Client::~Client() {}
