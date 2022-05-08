#include "NetworkManager.h"

#include "CharactersManager.h"
#include "DBManager.h"
#include "Timer.h"

NetworkManager::NetworkManager()
{
	hIOCP = nullptr;

	charactersManager = nullptr;
	dbManager = nullptr;

	clientsKeyCount = 0;
}

NetworkManager::~NetworkManager() {}

void NetworkManager::SetIOCPHandle(HANDLE hiocp)
{
	hIOCP = hiocp;
}

void NetworkManager::SetCharactersManagerPointer(CharactersManager* cm)
{
	charactersManager = cm;
}

void NetworkManager::SetDBManagerPointer(DBManager* dbm)
{
	dbManager = dbm;
}

void NetworkManager::SetTimerPointer(Timer* tm)
{
	timer = tm;
}

int NetworkManager::ClientConnect(SOCKET s)
{
	std::shared_ptr<Client> tempClient = std::make_shared<Client>(s);

	int key = clientsKeyCount++;

	mutexClients.lock();
	clients.insert(std::pair<int, std::shared_ptr<Client>>(key, tempClient));
	mutexClients.unlock();

	return key;
}

void NetworkManager::ClientDisconnect(int cKey)
{
	std::wstring userWstrID = charactersManager->GetUserWstrID(cKey); // cKey == uKey
	dbManager->SaveDB(userWstrID);

	charactersManager->UserLogOut(cKey); // cKey == uKey

	sc_remove_object remove_object_p;
	remove_object_p.size = sizeof(remove_object_p);
	remove_object_p.type = SC_REMOVE_OBJECT;
	remove_object_p.targetKey = cKey;
	remove_object_p.objectType = OT_USER;

	mutexClients.lock();
	std::map<int, std::shared_ptr<Client>> tempClients = clients;
	mutexClients.unlock();

	for (std::pair<int, std::shared_ptr<Client>> c : tempClients)
	{
		if (c.first == cKey) continue;

		SendPacket(c.first, &remove_object_p);
	}

	mutexClients.lock();
	closesocket(clients[cKey]->socket);
	clients[cKey].reset();
	clients.erase(cKey);
	mutexClients.unlock();

	std::cout << "Log out client [ " << cKey << " ].\n";
}

void NetworkManager::StartRecv(int cKey)
{
	unsigned long flags = 0;

	ZeroMemory(&clients[cKey]->exOver.over, sizeof(WSAOVERLAPPED));

	int wsaRecvReturn = WSARecv(clients[cKey]->socket, &clients[cKey]->exOver.wsaBuffer, 1, NULL, &flags, &clients[cKey]->exOver.over, NULL);

	if (wsaRecvReturn != 0)
	{
		ErrorDisplay(L"Recv Error ");
	}
}

void NetworkManager::IOTypeVerification(EXOVER* exOver, unsigned long ioSize, int key)
{
	// std::cout << "WT: Is Recv ? [" << p_over->is_recv << "]\n\n";

	std::vector<ES> transmitWaitPackets;

	CH_Info tempCharacterInfo;

	if (exOver->ioType == IOT_RECV)
	{
		// std::cout << "WT: Packet From Client [" << key << "]\n\n";
		int workSize = ioSize;
		char* recvBuffer = exOver->wsaBuffer.buf;
		while (workSize)
		{
			int p_size;
			if (clients[key]->packetSize)
			{
				p_size = clients[key]->packetSize;
			}
			else
			{
				p_size = recvBuffer[0];
				clients[key]->packetSize = p_size;
			}

			int needSize = p_size - clients[key]->prevPacketSize;

			if (needSize <= workSize)
			{
				memcpy(clients[key]->packetBuffer + clients[key]->prevPacketSize, recvBuffer, needSize);

				ProcessPacket(key, clients[key]->packetBuffer);
				clients[key]->prevPacketSize = 0;
				clients[key]->packetSize = 0;
				workSize -= needSize;
				recvBuffer += needSize;
			}
			else
			{
				memcpy(clients[key]->packetBuffer + clients[key]->prevPacketSize, recvBuffer, workSize);
				clients[key]->prevPacketSize += workSize;
				workSize = 0;
			}
		}

		StartRecv(key);
	}
	else if (exOver->ioType == IOT_SEND) // Send after process
	{
		// cout << "WT: A packet was sent to Client[" << key << "]\n\n";
		delete exOver;
	}
	else if (exOver->ioType == IOT_MONSTER_MOVE) // NPC Move event
	{
		transmitWaitPackets = charactersManager->MonsterMove(key);

		for (const ES& es : transmitWaitPackets)
		{
			tempCharacterInfo = charactersManager->GetMonsterInfo(es.sendTargetKey);
			if (es.eventType == ET_POSITIONINFO)
			{
				SendPositionInfoPacket(es.sendTargetKey, es.objectKey, es.objectType, tempCharacterInfo.x, tempCharacterInfo.y);
			}
			else if (es.eventType == ET_ADDOBJECT)
			{
				SendAddObjectPacket(es.sendTargetKey, es.objectKey, es.objectType, tempCharacterInfo.hp);
			}
			else if (es.eventType == ET_REMOVEOBJECT)
			{
				SendRemoveObjectPacket(es.sendTargetKey, es.objectKey, es.objectType);
			}
		}

		timer->PushEvent(key, IOT_MONSTER_MOVE, 0);

		delete exOver;
	}
	else if (exOver->ioType == IOT_MONSTER_ATTACK) // Monster attack event
	{
		transmitWaitPackets = charactersManager->MonsterAttack(key);

		for (const ES& es : transmitWaitPackets)
		{
			if (es.eventType == ET_POSITIONINFO)
			{
				cs_move packet;
				packet.size = sizeof(cs_move);
				packet.type = CS_MOVE;
				packet.dir = -1;

				EXOVER* postOver = new EXOVER;

				memcpy_s(postOver->ioBuffer, sizeof(cs_move), &packet, sizeof(cs_move));
				postOver->over.hEvent = 0;
				postOver->over.Internal = 0;
				postOver->over.InternalHigh = 0;
				postOver->over.Offset = 0;
				postOver->over.OffsetHigh = 0;
				postOver->over.Pointer = 0;
				postOver->ioType = IOT_RECV;
				postOver->wsaBuffer.buf = postOver->ioBuffer;
				postOver->wsaBuffer.len = 0;

				PostQueuedCompletionStatus(hIOCP, 1, es.sendTargetKey, &postOver->over);
			}
			else if (es.eventType == ET_STATCHANGE)
			{
				tempCharacterInfo = charactersManager->GetUserInfo(es.sendTargetKey);
				SendStatChangePacket(es.sendTargetKey, es.objectKey, OT_USER, tempCharacterInfo.level, tempCharacterInfo.exp, tempCharacterInfo.hp);
			}
			else if (es.eventType == ET_HEALSTART)
			{
				timer->PushEvent(es.objectKey, IOT_USER_HEAL, 0);
			}
		}

		timer->PushEvent(key, IOT_MONSTER_ATTACK, 0);

		delete exOver;
	}
	else if (exOver->ioType == IOT_MONSTER_RESPAWN)
	{
		transmitWaitPackets = charactersManager->MonsterRespawn(key);

		for (const ES& es : transmitWaitPackets)
		{
			tempCharacterInfo = charactersManager->GetMonsterInfo(es.sendTargetKey);
			if (es.eventType == ET_POSITIONINFO)
			{
				SendPositionInfoPacket(es.sendTargetKey, es.objectKey, es.objectType, tempCharacterInfo.x, tempCharacterInfo.y);
			}
			else if (es.eventType == ET_ADDOBJECT)
			{
				SendAddObjectPacket(es.sendTargetKey, es.objectKey, es.objectType, tempCharacterInfo.hp);
			}
			else if (es.eventType == ET_REMOVEOBJECT)
			{
				SendRemoveObjectPacket(es.sendTargetKey, es.objectKey, es.objectType);
			}
		}

		timer->PushEvent(key, IOT_MONSTER_MOVE, 0);
		timer->PushEvent(key, IOT_MONSTER_ATTACK, 500);

		delete exOver;
	}
	else if (exOver->ioType == IOT_USER_HEAL) // User heal event
	{
		transmitWaitPackets = charactersManager->UserHeal(key);

		for (const ES& es : transmitWaitPackets)
		{
			if (es.eventType == ET_HEALSTART)
			{
				timer->PushEvent(es.objectKey, IOT_USER_HEAL, 0);
			}
		}

		delete exOver;
	}
	else
	{
		std::cout << "Packet Type Error!\n";
	}
}

void NetworkManager::ProcessPacket(int cKey, unsigned char* packet)
{
	switch (packet[1])
	{
	case CS_LOGIN:
	{
		wchar_t tempIDStr[ID_STR_LENGTH];
		memcpy(tempIDStr, &packet[2], sizeof(wchar_t) * ID_STR_LENGTH);

		if (dbManager->TryLogin(tempIDStr, cKey) == false)
		{
			sc_login_fail loginFailPacket;
			loginFailPacket.size = sizeof(loginFailPacket);
			loginFailPacket.type = SC_LOGIN_FAIL;
			SendPacket(cKey, &loginFailPacket);
			std::wcout << L"client [ " << cKey << L" ] login fail.\n";
			break;
		}

		std::wcout << L"user [ " << tempIDStr << L" ] login.\n";

		CH_Info tempUserInfo = charactersManager->GetUserInfo(cKey);

		// Send login ok packet
		sc_login_ok logi_ok_p;
		logi_ok_p.size = sizeof(logi_ok_p);
		logi_ok_p.type = SC_LOGIN_OK;
		logi_ok_p.myKey = cKey;
		logi_ok_p.level = tempUserInfo.level;
		logi_ok_p.x = tempUserInfo.x;
		logi_ok_p.y = tempUserInfo.y;
		logi_ok_p.exp = tempUserInfo.exp;
		logi_ok_p.hp = tempUserInfo.hp;
		SendPacket(cKey, &logi_ok_p);

		sc_add_object add_object_p;
		add_object_p.size = sizeof(add_object_p);
		add_object_p.type = SC_ADD_OBJECT;
		add_object_p.targetKey = cKey;
		add_object_p.objectType = OT_USER;
		add_object_p.hp = tempUserInfo.hp;

		sc_position_info position_info_p;
		position_info_p.size = sizeof(position_info_p);
		position_info_p.type = SC_POSITION_INFO;
		position_info_p.targetKey = cKey;
		position_info_p.x = tempUserInfo.x;
		position_info_p.y = tempUserInfo.y;
		position_info_p.objectType = OT_USER;

		CH_Info loopCHInfo;

		mutexClients.lock();
		std::map<int, std::shared_ptr<Client>> tempClients = clients;
		mutexClients.unlock();

		// Notify the world of new users' connections
		for (std::pair<int, std::shared_ptr<Client>> c : tempClients)
		{
			if (c.first == cKey)
			{
				continue;
			}

			if (charactersManager->CanSeeU(cKey, c.first) == false)
			{
				continue;
			}

			charactersManager->InsertViewList(cKey, c.first); // cKey == uKey

			loopCHInfo = charactersManager->GetUserInfo(c.first);

			SendAddObjectPacket(cKey, c.first, OT_USER, loopCHInfo.hp);
			SendPositionInfoPacket(cKey, c.first, OT_USER, loopCHInfo.x, loopCHInfo.y);
			SendPacket(c.first, &add_object_p);
			SendPacket(c.first, &position_info_p);
		}

		std::vector<CH_Info> monstersInSight = charactersManager->GetMonstersInSight(cKey);

		for (int i = 0; i < monstersInSight.size(); ++i)
		{
			loopCHInfo = monstersInSight[i];

			SendAddObjectPacket(cKey, loopCHInfo.key, loopCHInfo.objectType, loopCHInfo.hp);
			SendPositionInfoPacket(cKey, loopCHInfo.key, loopCHInfo.objectType, loopCHInfo.x, loopCHInfo.y);
		}
	}
	break;
	case CS_LOGOUT:
	{
		ClientDisconnect(cKey);
	}
	break;
	case CS_MOVE:
	{
		std::vector<ES> transmitWaitPackets = charactersManager->TryMove(cKey, static_cast<int>(packet[2])); // cKey == uKey

		CH_Info tempCharInfo;

		for (const ES &es : transmitWaitPackets)
		{
			if (es.eventType == ET_REMOVEOBJECT)
			{
				SendRemoveObjectPacket(cKey, es.objectKey, es.objectType);
			}
			else if (es.eventType == ET_STATCHANGE)
			{
				if (es.objectType == OT_USER)
				{
					tempCharInfo = charactersManager->GetUserInfo(cKey);
					SendStatChangePacket(cKey, cKey, OT_USER, tempCharInfo.level, tempCharInfo.exp, tempCharInfo.hp);
				}
				else
				{
					tempCharInfo = charactersManager->GetMonsterInfo(es.objectKey);
					SendStatChangePacket(cKey, tempCharInfo.key, tempCharInfo.objectType, tempCharInfo.level, tempCharInfo.exp, tempCharInfo.hp);
				}
			}
			else if (es.eventType == ET_ADDOBJECT)
			{
				tempCharInfo = charactersManager->GetUserInfo(cKey);
				SendAddObjectPacket(es.sendTargetKey, es.objectKey, OT_USER, tempCharInfo.hp);
			}
			else if (es.eventType == ET_POSITIONINFO)
			{
				tempCharInfo = charactersManager->GetUserInfo(cKey);
				SendPositionInfoPacket(cKey, cKey, OT_USER, tempCharInfo.x, tempCharInfo.y);
			}
		}
	}
	break;
	case CS_ATTACK:
	{
		std::vector<ES> transmitWaitPackets = charactersManager->TryAttack(cKey); // cKey == uKey

		CH_Info tempCharInfo;

		for (const ES &es : transmitWaitPackets)
		{
			if (es.eventType == ET_REMOVEOBJECT)
			{
				SendRemoveObjectPacket(cKey, es.objectKey, es.objectType);
			}
			else if (es.eventType == ET_STATCHANGE)
			{
				if (es.objectType == OT_USER)
				{
					tempCharInfo = charactersManager->GetUserInfo(cKey); // cKey == uKey
					SendStatChangePacket(cKey, cKey, OT_USER, tempCharInfo.level, tempCharInfo.exp, tempCharInfo.hp);
				}
				else
				{
					tempCharInfo = charactersManager->GetMonsterInfo(es.objectKey);
					SendStatChangePacket(cKey, tempCharInfo.key, tempCharInfo.objectType, tempCharInfo.level, tempCharInfo.exp, tempCharInfo.hp);
				}
			}
			else if (es.eventType == ET_RESPAWNMONSTER)
			{
				timer->PushEvent(es.objectKey, IOT_MONSTER_RESPAWN, 0);
			}
		}
	}
	break;
	case CS_CHAT:
	{
		cs_chat* p_chat = reinterpret_cast<cs_chat*>(packet);
		sc_chat chat_p;
		chat_p.size = static_cast<unsigned char>(sizeof(sc_chat));
		chat_p.type = SC_CHAT;
		chat_p.speakerKey = cKey;
		wcscpy_s(chat_p.chat_str, p_chat->chat_str);

		for (int cKeyloop = 0; cKeyloop < clients.size(); ++cKeyloop)
		{
			SendPacket(cKeyloop, &chat_p);
		}
	}
	break;
	default:
		std::cout << "Unkown Packet Type from Client [" << cKey << "]\n\n";
		return;
	}
}

void NetworkManager::SendPacket(int cKey, void *ptr)
{
	char* packet = reinterpret_cast<char *>(ptr);
	EXOVER *over = new EXOVER;
	over->ioType = IOT_SEND;
	memcpy(over->ioBuffer, packet, packet[0]);
	over->wsaBuffer.buf = over->ioBuffer;
	over->wsaBuffer.len = over->ioBuffer[0];

	ZeroMemory(&over->over, sizeof(WSAOVERLAPPED));

	int wsaSendReturn = WSASend(clients[cKey]->socket, &over->wsaBuffer, 1, NULL, 0, &over->over, NULL);

	if (wsaSendReturn != 0)
	{
		ErrorDisplay(L"Send Error! ");
	}
}

void NetworkManager::SendAddObjectPacket(int cKey, int targetKey, int objectType, int hp)
{
	sc_add_object p;
	p.size = sizeof(p);
	p.type = SC_ADD_OBJECT;
	p.targetKey = targetKey;
	p.objectType = objectType;
	p.hp = hp;
	SendPacket(cKey, &p);
}

void NetworkManager::SendPositionInfoPacket(int cKey, int targetKey, int objectType, int x, int y)
{
	sc_position_info p;
	p.size = sizeof(sc_position_info);
	p.type = SC_POSITION_INFO;
	p.targetKey = targetKey;
	p.objectType = objectType;
	p.x = x;
	p.y = y;
	SendPacket(cKey, &p);
}

void NetworkManager::SendRemoveObjectPacket(int cKey, int targetKey, int objectType)
{
	sc_remove_object p;
	p.size = sizeof(sc_remove_object);
	p.type = SC_REMOVE_OBJECT;
	p.targetKey = targetKey;
	p.objectType = objectType;
	SendPacket(cKey, &p);
}

void NetworkManager::SendStatChangePacket(int cKey, int targetKey, int objectType, int lv, int exp, int hp)
{
	sc_stat_change p;
	p.size = sizeof(sc_stat_change);
	p.type = SC_STAT_CHANGE;
	p.targetKey = targetKey;
	p.objectType = objectType;
	p.level = lv;
	p.exp = exp;
	p.hp = hp;
	SendPacket(cKey, &p);
}

void NetworkManager::InputEventToIOCP(int key, int eventType)
{
	EXOVER* postOver = new EXOVER;

	postOver->over.hEvent = 0;
	postOver->over.Internal = 0;
	postOver->over.InternalHigh = 0;
	postOver->over.Offset = 0;
	postOver->over.OffsetHigh = 0;
	postOver->over.Pointer = 0;
	postOver->ioType = eventType;
	postOver->wsaBuffer.buf = postOver->ioBuffer;
	postOver->wsaBuffer.len = 0;

	PostQueuedCompletionStatus(hIOCP, 1, key, &postOver->over);
}