#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <atomic>

#include "Client.h"

class CharactersManager;
class DBManager;
class Timer;

class NetworkManager
{
private:
	HANDLE hIOCP;
	CharactersManager* charactersManager;
	DBManager* dbManager;
	Timer* timer;

	std::map<int, std::shared_ptr<Client>> clients;
	std::mutex mutexClients;

	std::atomic<int> clientsKeyCount;

protected:

public:

private:

protected:

public:
	NetworkManager();
	~NetworkManager();

	void SetIOCPHandle(HANDLE hiocp);
	void SetCharactersManagerPointer(CharactersManager* cm);
	void SetDBManagerPointer(DBManager* dbm);
	void SetTimerPointer(Timer* tm);

	int ClientConnect(SOCKET s);
	void ClientDisconnect(int cKey);
	
	void StartRecv(int id);
	void IOTypeVerification(EXOVER* exOver, unsigned long ioSize, int key);
	void ProcessPacket(int cKey, unsigned char* packet);

	void SendPacket(int id, void *ptr);
	void SendAddObjectPacket(int cKey, int targetKey, int objectType, int hp);
	void SendPositionInfoPacket(int cKey, int targetKey, int objectType, int x, int y);
	void SendRemoveObjectPacket(int cKey, int targetKey, int objectType);
	void SendStatChangePacket(int cKey, int targetKey, int objectType, int lv, int exp, int hp);

	void InputEventToIOCP(int key, int eventType);
};

