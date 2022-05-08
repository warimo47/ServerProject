#include "CharactersManager.h"

CharactersManager::CharactersManager()
{
	// Load obstacle information by file
	std::ifstream map("MapObstacles.bin", std::ios::binary);
	map.read(reinterpret_cast<char*>(&obstacles), sizeof(obstacles));
	map.close();
	
	int lv = 0;
	int posX = 0;
	int posY = 0;
	int mType = 0;

	Monster* tempMon = nullptr;

	// Load monster information by file
	for (int i = 0; i < 500; ++i)
	{
		lv = rand() % 3 + 1;
		posX = rand() % 298 + 1;
		posY = rand() % 298 + 1;
		mType = rand() % 4 + 2; // 2, 3, 4, 5

		while (obstacles[posY][posX] == true)
		{
			posX = rand() % 298 + 1;
			posY = rand() % 298 + 1;
		}

		tempMon = new Monster(lv, posX, posY, mType);

		monsters.push_back(tempMon);
	}
}

CharactersManager::~CharactersManager()
{
	for (Monster* m : monsters)
	{
		delete m;
	}
}

void CharactersManager::UserLogIn(std::wstring id, int level, int posX, int posY, int exp, int hp, int cKey)
{
	std::shared_ptr<User> tempUser = std::make_shared<User>(id, level, posX, posY, exp, hp);
	mutexUsers.lock();
	users.insert(std::pair<int, std::shared_ptr<User>>(cKey, tempUser));
	mutexUsers.unlock();
}

void CharactersManager::UserLogOut(int uKey)
{
	// Remove log out user from monsters view list
	std::unordered_set<int>::iterator removeTargetIter;

	for (int i = 0; i < monsters.size(); ++i)
	{
		monsters[i]->mutexUserVL.lock();
		removeTargetIter = monsters[i]->userViewList.find(uKey);
		if (removeTargetIter != monsters[i]->userViewList.end())
		{
			monsters[i]->userViewList.erase(removeTargetIter);
		}
		monsters[i]->mutexUserVL.unlock();
	}

	// Remove log out user from other users view list
	std::map<int, std::shared_ptr<User>>::iterator usersLoopIter = users.begin();

	while (usersLoopIter != users.end())
	{
		usersLoopIter->second->mutexUserVL.lock();
		removeTargetIter = usersLoopIter->second->userViewList.find(uKey);
		if (removeTargetIter != usersLoopIter->second->userViewList.end())
		{
			usersLoopIter->second->userViewList.erase(removeTargetIter);
		}
		usersLoopIter->second->mutexUserVL.unlock();
		usersLoopIter++;
	}

	users[uKey].reset();
}

CH_Info CharactersManager::GetUserInfo(int uKey)
{
	CH_Info tempUserInfo;
	tempUserInfo.objectType = OT_USER;
	tempUserInfo.level = users[uKey]->level;
	tempUserInfo.x = users[uKey]->positionX;
	tempUserInfo.y = users[uKey]->positionY;
	tempUserInfo.exp = users[uKey]->experiencePoint;
	tempUserInfo.hp = users[uKey]->healthPoint;
	return tempUserInfo;
}

std::wstring CharactersManager::GetUserWstrID(int uKey)
{
	return users[uKey]->idStr;
}

CH_Info CharactersManager::GetMonsterInfo(int mKey)
{
	CH_Info tempMonsterInfo;
	tempMonsterInfo.objectType = monsters[mKey]->monsterType;
	tempMonsterInfo.level = monsters[mKey]->level;
	tempMonsterInfo.x = monsters[mKey]->positionX;
	tempMonsterInfo.y = monsters[mKey]->positionY;
	tempMonsterInfo.exp = 0;
	tempMonsterInfo.hp = monsters[mKey]->healthPoint;
	return tempMonsterInfo;
}

int CharactersManager::GetMonsterType(int mKey)
{
	return monsters[mKey]->monsterType;
}

bool CharactersManager::CanSeeU(int uKey_a, int uKey_b)
{
	int distX = users[uKey_a]->positionX - users[uKey_b]->positionX;
	int distY = users[uKey_a]->positionY - users[uKey_b]->positionY;

	return ((VIEW_RADIUS >= distX * distX) && (VIEW_RADIUS >= distY * distY));
}

bool CharactersManager::CanSeeM(int uKey, int mKey)
{
	int distX = users[uKey]->positionX - monsters[mKey]->positionX;
	int distY = users[uKey]->positionY - monsters[mKey]->positionY;

	return ((VIEW_RADIUS >= distX * distX) && (VIEW_RADIUS >= distY * distY));
}

void CharactersManager::InsertViewList(int uKey_a, int uKey_b)
{
	users[uKey_a]->mutexUserVL.lock();
	users[uKey_a]->userViewList.insert(uKey_b);
	users[uKey_a]->mutexUserVL.unlock();

	users[uKey_b]->mutexUserVL.lock();
	users[uKey_b]->userViewList.insert(uKey_a);
	users[uKey_b]->mutexUserVL.unlock();
}

std::vector<CH_Info> CharactersManager::GetMonstersInSight(int uID)
{
	std::vector<CH_Info> ret;

	CH_Info temp;
	
	for (int i = 0; i < monsters.size(); ++i)
	{
		if (CanSeeM(uID, i) == false) continue;
		
		users[uID]->mutexMonsterVL.lock();
		users[uID]->monsterViewList.insert(i);
		users[uID]->mutexMonsterVL.unlock();

		monsters[i]->mutexUserVL.lock();
		monsters[i]->userViewList.insert(uID);
		monsters[i]->mutexUserVL.unlock();

		temp.objectType = monsters[i]->monsterType;
		temp.x = monsters[i]->positionX;
		temp.y = monsters[i]->positionY;
		temp.hp = monsters[i]->healthPoint;
		ret.push_back(temp);
	}

	return ret;
}

void CharactersManager::InsertUserToMonsterVL(int uKey, int mKey)
{
	monsters[mKey]->mutexUserVL.lock();

	// 내가 상대의 ViewList에 없는 경우
	if (monsters[mKey]->userViewList.find(uKey) == monsters[mKey]->userViewList.end())
	{
		monsters[mKey]->userViewList.insert(uKey);

		monsters[mKey]->mutexUserVL.unlock();

		// CheckNPCMove(ob);
	}
	// 내가 상대의 ViewList에 있는 경우
	else
	{
		monsters[mKey]->mutexUserVL.unlock();
	}
}

void CharactersManager::RemoveUserToMonsterVL(int uKey, int mKey)
{
	monsters[mKey]->mutexUserVL.lock();

	std::unordered_set<int>::iterator usIter =  monsters[mKey]->userViewList.find(uKey);

	if (usIter != monsters[mKey]->userViewList.end())
	{
		monsters[mKey]->userViewList.erase(usIter);
	}

	monsters[mKey]->mutexUserVL.unlock();
}

void CharactersManager::GetNewVLUU(int uKey, std::unordered_set<int>* nUserVL, std::unordered_set<int>* nUserRemoveVL)
{
	for (std::pair<int, std::shared_ptr<User>> us : users)
	{
		if (us.first == uKey) continue;
		if (CanSeeU(uKey, us.first) == true)
		{
			nUserVL->insert(us.first);
		}
		else
		{
			nUserRemoveVL->insert(us.first);
		}
	}
}

void CharactersManager::GetNewVLUM(int uKey, std::unordered_set<int>* nMonsterVL, std::unordered_set<int>* nMonsterRemoveVL)
{
	for (int mID = 0; mID < monsters.size(); ++mID) // Monster ID
	{
		if (CanSeeM(uKey, mID) == true)
		{
			nMonsterVL->insert(mID);
		}
		else
		{
			nMonsterRemoveVL->insert(mID);
		}
	}
}

void CharactersManager::GetNewVLMU(int mKey, std::unordered_set<int>* nUserVL, std::unordered_set<int>* nUserRemoveVL)
{
	for (std::pair<int, std::shared_ptr<User>> us : users)
	{
		if (CanSeeM(us.first, mKey) == true)
		{
			nUserVL->insert(us.first);
		}
		else
		{
			nUserRemoveVL->insert(us.first);
		}
	}
}

std::vector<ES> CharactersManager::TryMove(int uKey, int dir)
{
	std::vector<ES> ret;
	
	int x = -1;
	int y = -1;
	std::chrono::steady_clock::time_point nowTime;

	if (CanMove(uKey, dir, &x, &y, &nowTime) == false)
	{
		return ret;
	}

	users[uKey]->positionX = x;
	users[uKey]->positionY = y;
	users[uKey]->tpMoveCoolTime = nowTime;

	ES es;
	es.eventType = ET_POSITIONINFO;
	es.sendTargetKey = uKey;
	es.objectKey = uKey;
	es.objectType = -1; // Do not use

	ret.push_back(es);

	/********** Processing visibility between users **********/

	std::unordered_set<int> newUserVL;
	std::unordered_set<int> newUserRemoveVL;

	GetNewVLUU(uKey, &newUserVL, &newUserRemoveVL);
	std::shared_ptr<User> me = users[uKey];
	std::shared_ptr<User> opponent = nullptr;
	CH_Info tempCharInfo;

	// If have anything to add to my user view list
	for (int nuk : newUserVL) // new user key
	{
		opponent = users[nuk];

		me->mutexUserVL.lock();

		// If the opponent is in my new view list, but not in the old view list
		if (me->userViewList.find(nuk) == me->userViewList.end())
		{
			me->userViewList.insert(nuk);

			me->mutexUserVL.unlock();

			tempCharInfo = GetUserInfo(nuk);

			es.eventType = ET_ADDOBJECT;
			es.sendTargetKey = uKey;
			es.objectKey = nuk;
			es.objectType = OT_USER;
			ret.push_back(es);

			es.eventType = ET_POSITIONINFO;
			es.sendTargetKey = uKey;
			es.objectKey = nuk;
			es.objectType = OT_USER;
			ret.push_back(es);

			opponent->mutexUserVL.lock();

			// If I am not in the opponent's view list
			if (opponent->userViewList.find(uKey) == opponent->userViewList.end())
			{
				opponent->userViewList.insert(uKey);

				opponent->mutexUserVL.unlock();

				es.eventType = ET_ADDOBJECT;
				es.sendTargetKey = nuk;
				es.objectKey = uKey;
				es.objectType = OT_USER;
				ret.push_back(es);
			}
			// If I am in the other person's view list
			else
			{
				opponent->mutexUserVL.unlock();
			}
		}
		// If the opponent is in both my new view list and old view list
		else
		{
			me->mutexUserVL.unlock();

			tempCharInfo = GetUserInfo(uKey);
		}

		es.eventType = ET_POSITIONINFO;
		es.sendTargetKey = nuk;
		es.objectKey = uKey;
		es.objectType = OT_USER;
		ret.push_back(es);
	}

	std::unordered_set<int>::iterator vlIter;

	// If have anything to remove to my user view list
	for (int nurk : newUserRemoveVL) // new user remove key
	{
		me->mutexUserVL.lock();

		vlIter = me->userViewList.find(nurk);

		// If the opponent is in my new remove view list and is also in old view list
		if (vlIter != me->userViewList.end())
		{
			me->userViewList.erase(vlIter);

			me->mutexUserVL.unlock();

			opponent = users[nurk];

			es.eventType = ET_REMOVEOBJECT;
			es.sendTargetKey = uKey;
			es.objectKey = nurk;
			es.objectType = OT_USER;
			ret.push_back(es);

			opponent->mutexUserVL.lock();

			vlIter = opponent->userViewList.find(uKey);

			if (vlIter != opponent->userViewList.end())
			{
				opponent->userViewList.erase(vlIter);
			}

			opponent->mutexUserVL.unlock();

			es.eventType = ET_REMOVEOBJECT;
			es.sendTargetKey = nurk;
			es.objectKey = uKey;
			es.objectType = OT_USER;
			ret.push_back(es);

			me->mutexUserVL.lock();
		}

		me->mutexUserVL.unlock();
	}

	/********** Processing visibility between user and monster **********/

	std::unordered_set<int> newMonstersVL;
	std::unordered_set<int> newMonstersRemoveVL;

	GetNewVLUM(uKey, &newMonstersVL, &newMonstersRemoveVL);

	// If have anything to add to my monster view list
	for (int nmk : newMonstersVL) // New monster key
	{
		me->mutexMonsterVL.lock();

		// If the opponent monster is in my new view list but not in my old view list
		if (me->monsterViewList.find(nmk) == me->monsterViewList.end())
		{
			me->monsterViewList.insert(nmk);

			me->mutexMonsterVL.unlock();

			tempCharInfo = GetMonsterInfo(nmk);

			es.eventType = ET_ADDOBJECT;
			es.sendTargetKey = uKey;
			es.objectKey = nmk;
			es.objectType = tempCharInfo.objectType;
			ret.push_back(es);

			es.eventType = ET_POSITIONINFO;
			es.sendTargetKey = uKey;
			es.objectKey = nmk;
			es.objectType = tempCharInfo.objectType;
			ret.push_back(es);

			InsertUserToMonsterVL(uKey, nmk);
		}
		// If the opponent monster is in my new view list and is also in old view list
		else
		{
			me->mutexMonsterVL.unlock();
		}
	}

	// If have anything to remove to my monster view list
	for (int nmrk : newMonstersRemoveVL) // New monster remove key
	{
		me->mutexMonsterVL.lock();

		vlIter = me->monsterViewList.find(nmrk);

		// If the opponent monster is in my new remove view list and is also in old view list
		if (vlIter != me->monsterViewList.end())
		{
			me->monsterViewList.erase(vlIter);

			me->mutexMonsterVL.unlock();

			tempCharInfo = GetMonsterInfo(nmrk);

			es.eventType = ET_REMOVEOBJECT;
			es.sendTargetKey = uKey;
			es.objectKey = nmrk;
			es.objectType = tempCharInfo.objectType;
			ret.push_back(es);

			RemoveUserToMonsterVL(uKey, nmrk);

			me->mutexMonsterVL.lock();
		}

		me->mutexMonsterVL.unlock();
	}

	return ret;
}

bool CharactersManager::CanMove(int uKey, int dir, int* xp, int* yp, std::chrono::steady_clock::time_point* now)
{
	std::chrono::steady_clock::time_point nowTime = std::chrono::high_resolution_clock::now();
	std::chrono::nanoseconds nanoCoolTime = nowTime - users[uKey]->tpMoveCoolTime;
	long long coolTime = std::chrono::duration_cast<std::chrono::milliseconds>(nanoCoolTime).count();

	if (static_cast<int>(coolTime) < 1000)
	{
		return false;
	}

	int x = users[uKey]->positionX;
	int y = users[uKey]->positionY;

	switch (dir)
	{
	case 0: // Go up
		if (y > 0)
		{
			y--;
		}
		else
		{
			return false;
		}
		break;
	case 1: // Go down
		if (y < BOARD_HEIGHT - 1)
		{
			y++;
		}
		else
		{
			return false;
		}
		break;
	case 2: // Go left
		if (x > 0)
		{
			x--;
		}
		else
		{
			return false;
		}
		break;
	case 3: // Go right
		if (x < BOARD_WIDTH - 1)
		{
			x++;
		}
		else
		{
			return false;
		}
		break;
	default:
		std::wcout << L"Unkown Direction from user key [ " << uKey << L" ]\n";
		return false;
	}

	if (obstacles[y][x] == true) return false;

	*xp = x;
	*yp = y;
	*now = nowTime;

	return true;
}

std::vector<ES> CharactersManager::TryAttack(int uKey)
{
	std::vector<ES> ret;
	std::chrono::steady_clock::time_point nowTime = std::chrono::high_resolution_clock::now();
	std::chrono::nanoseconds nanoCoolTime = nowTime - users[uKey]->tpAttackCoolTime;
	long long coolTime = std::chrono::duration_cast<std::chrono::milliseconds>(nanoCoolTime).count();

	if (static_cast<int>(coolTime) < 1000)
	{
		return ret;
	}

	std::unordered_set<int> monsterVL;

	users[uKey]->mutexMonsterVL.lock();
	monsterVL = users[uKey]->monsterViewList;
	users[uKey]->mutexMonsterVL.unlock();

	std::unordered_set<int>::iterator eraseIter;
	ES es;

	for (int vmk : monsterVL) // Visible monster key
	{
		if (CanAttackUM(uKey, vmk) == false) continue;
		
		monsters[vmk]->healthPoint -= users[uKey]->level * 2;

		// Kill monster
		if (monsters[vmk]->healthPoint < 1)
		{
			// Remove monster
			monsters[vmk]->isDead = true;

			es.eventType = ET_REMOVEOBJECT; // SendRemoveObjectPacket
			es.sendTargetKey = uKey;
			es.objectKey = vmk;
			es.objectType = monsters[vmk]->monsterType;
			ret.push_back(es);

			es.eventType = ET_RESPAWNMONSTER; // Respawn monster
			es.sendTargetKey = -1; // Do not use
			es.objectKey = vmk;
			es.objectType = monsters[vmk]->monsterType;
			ret.push_back(es);

			// Change user state
			switch (monsters[vmk]->monsterType)
			{
			case OT_MONSTER1: users[uKey]->experiencePoint += monsters[vmk]->level * 5; break;
			case OT_MONSTER2: users[uKey]->experiencePoint += monsters[vmk]->level * 10; break;
			case OT_MONSTER3: users[uKey]->experiencePoint += monsters[vmk]->level * 10; break;
			case OT_MONSTER4: users[uKey]->experiencePoint += monsters[vmk]->level * 20; break;
			}

			if (users[uKey]->experiencePoint >= (static_cast<int>(pow(2, users[uKey]->level - 1) * 100)))
			{
				users[uKey]->experiencePoint -= (static_cast<int>(pow(2, users[uKey]->level - 1) * 100));
				users[uKey]->level++;
				users[uKey]->healthPoint = users[uKey]->level * 100;
			}

			users[uKey]->mutexMonsterVL.lock();
			eraseIter = users[uKey]->monsterViewList.find(vmk);
			if (eraseIter != users[uKey]->monsterViewList.end())
			{
				users[uKey]->monsterViewList.erase(eraseIter);
			}
			users[uKey]->mutexMonsterVL.unlock();

			es.eventType = ET_STATCHANGE; // SendStatChangePacket
			es.sendTargetKey = uKey;
			es.objectKey = uKey;
			es.objectType = OT_USER;
			ret.push_back(es);
		}
		else
		{
			monsters[vmk]->focusKey = uKey;

			es.eventType = ET_STATCHANGE; // SendStatChangePacket
			es.sendTargetKey = uKey;
			es.objectKey = vmk;
			es.objectType = monsters[vmk]->monsterType;
			ret.push_back(es);
		}

		// CheckNPCAttack(ob);
	}

	users[uKey]->tpAttackCoolTime = nowTime;
	
	return ret;
}

bool CharactersManager::CanAttackUM(int uKey, int mKey)
{
	int userX = users[uKey]->positionX;
	int userY = users[uKey]->positionY;
	int monstarX = monsters[mKey]->positionX;
	int monstarY = monsters[mKey]->positionY;

	if ((userX - 1 == monstarX) && (userY == monstarY)) return true;
	if ((userX + 1 == monstarX) && (userY == monstarY)) return true;
	if ((userX == monstarX) && (userY - 1 == monstarY)) return true;
	if ((userX == monstarX) && (userY + 1 == monstarY)) return true;
	return false;
}


std::vector<ES> CharactersManager::UserHeal(int uKey)
{
	int maxHP = users[uKey]->level * 100;

	std::vector<ES> ret;

	if (users[uKey]->healthPoint >= maxHP)
	{
		return ret;
	}

	users[uKey]->healthPoint += maxHP / 10;

	if (users[uKey]->healthPoint >= maxHP)
	{
		users[uKey]->healthPoint = maxHP;
	}

	if (users[uKey]->healthPoint == maxHP)
	{
		return ret;
	}

	ES es;
	es.eventType = ET_HEALSTART;
	es.sendTargetKey = uKey; // Do not use
	es.objectKey = uKey;
	es.objectType = OT_USER;
	ret.push_back(es);

	return ret;
}

std::vector<ES> CharactersManager::MonsterMove(int mKey)
{
	std::vector<ES> ret;

	if (monsters[mKey]->isDead == true)
	{
		return ret;
	}

	int x = monsters[mKey]->positionX;
	int y = monsters[mKey]->positionY;

	int dir = FindDirection(mKey);

	if (dir == 1) // Go up
	{
		if (y > 0)
		{
			y--;
		}
		else
		{
			return ret;
		}
	}
	else if (dir == 2) // Go down
	{
		if (y < BOARD_HEIGHT - 1)
		{
			y++;
		}
		else
		{
			return ret;
		}
	}
	else if (dir == 3) // Go left
	{
		if (x > 0)
		{
			x--;
		}
		else
		{
			return ret;
		}
	}
	else if (dir == 4) // Go right
	{
		if(x < BOARD_WIDTH - 1)
		{
			x++;
		}
		else
		{
			return ret;
		}
	}

	if (obstacles[y][x] == true)
	{
		return ret;
	}

	monsters[mKey]->positionX = x;
	monsters[mKey]->positionY = y;

	std::unordered_set<int> newVL;
	std::unordered_set<int> removeVL;

	GetNewVLMU(mKey, &newVL, &removeVL);

	ES es;

	// If have anything to add to this monster view list
	for (int nukey : newVL) // New user key
	{
		monsters[mKey]->mutexUserVL.lock();

		// If the other person is in my new view liset but not in old view list
		if (monsters[mKey]->userViewList.find(nukey) == monsters[mKey]->userViewList.end())
		{
			monsters[mKey]->userViewList.insert(nukey);

			monsters[mKey]->mutexUserVL.unlock();

			users[nukey]->mutexMonsterVL.lock();

			// If I am not in the opponent's view list
			if (users[nukey]->monsterViewList.count(mKey) == 0)
			{
				users[nukey]->monsterViewList.insert(mKey);

				users[nukey]->mutexMonsterVL.unlock();

				es.eventType = ET_ADDOBJECT;
				es.sendTargetKey = nukey;
				es.objectKey = mKey;
				es.objectType = monsters[mKey]->monsterType;
				ret.push_back(es);
			}
			// If I'm on the other person's view list
			else
			{
				users[nukey]->mutexMonsterVL.unlock();
			}
		}
		// If the other person is in my new view list and old view list
		else
		{
			monsters[mKey]->mutexUserVL.unlock();
		}

		es.eventType = ET_POSITIONINFO;
		es.sendTargetKey = nukey;
		es.objectKey = mKey;
		es.objectType = monsters[mKey]->monsterType;
		ret.push_back(es);
	}

	std::unordered_set<int>::iterator vlIter;

	// If have anything to remove to my user view list
	for (int nurk : removeVL) // New user remove key
	{
		monsters[mKey]->mutexUserVL.lock();

		vlIter = monsters[mKey]->userViewList.find(nurk);

		// If the other person is in my remove view list and old view list
		if (vlIter != monsters[mKey]->userViewList.end())
		{
			monsters[mKey]->userViewList.erase(vlIter);

			monsters[mKey]->mutexUserVL.unlock();

			bool isErase = false;

			users[nurk]->mutexMonsterVL.lock();

			vlIter = users[nurk]->monsterViewList.find(nurk);

			if (vlIter != users[nurk]->monsterViewList.end())
			{
				users[nurk]->monsterViewList.erase(vlIter);
				isErase = true;
			}

			users[nurk]->mutexMonsterVL.unlock();

			if (isErase == true)
			{
				es.eventType = ET_REMOVEOBJECT;
				es.sendTargetKey = nurk;
				es.objectKey = mKey;
				es.objectType = monsters[mKey]->monsterType;
				ret.push_back(es);
			}

			monsters[mKey]->mutexUserVL.lock();
		}

		monsters[mKey]->mutexUserVL.unlock();
	}

	return ret;
}

int CharactersManager::FindDirection(int mKey)
{
	int targetUserKey = monsters[mKey]->focusKey;

	if (targetUserKey == -1)
	{
		return rand() % 4 + 1;
	}
	
	int diffX = users[targetUserKey]->positionX - monsters[mKey]->positionX;
	int diffY = users[targetUserKey]->positionX - monsters[mKey]->positionX;

	if (diffX * diffX > diffY * diffY) // Move x axis direction
	{
		if (diffX > 0)
		{
			return 4; // Go right
		}
		else
		{
			return 3; // Go left
		}
	}
	else // Move y axis direction
	{
		if (diffY > 0)
		{
			return 2; // Go down
		}
		else
		{
			return 1; // Go up
		}
	}
}

std::vector<ES> CharactersManager::MonsterAttack(int mKey)
{
	std::vector<ES> ret;

	if (monsters[mKey]->isDead == true)
	{
		return ret;
	}

	if (monsters[mKey]->focusKey == -1)
	{
		return ret;
	}

	if (CanAttackMU(mKey, monsters[mKey]->focusKey) == false)
	{
		return ret;
	}

	ES es;

	users[monsters[mKey]->focusKey]->healthPoint -= monsters[mKey]->level * 5;

	if (users[monsters[mKey]->focusKey]->healthPoint < 1)
	{
		users[monsters[mKey]->focusKey]->positionX = 150;
		users[monsters[mKey]->focusKey]->positionY = 150;
		users[monsters[mKey]->focusKey]->experiencePoint = users[monsters[mKey]->focusKey]->experiencePoint / 2;
		users[monsters[mKey]->focusKey]->healthPoint = users[monsters[mKey]->focusKey]->level * 100;

		es.eventType = ET_POSTIOCP;
		es.sendTargetKey = monsters[mKey]->focusKey;
		es.objectKey = monsters[mKey]->focusKey;
		es.objectType = OT_USER;
		ret.push_back(es);
	}

	es.eventType = ET_STATCHANGE;
	es.sendTargetKey = monsters[mKey]->focusKey;
	es.objectKey = monsters[mKey]->focusKey;
	es.objectType = OT_USER;
	ret.push_back(es);
	
	es.eventType = ET_HEALSTART;
	es.sendTargetKey = monsters[mKey]->focusKey; // Do not use
	es.objectKey = monsters[mKey]->focusKey;
	es.objectType = OT_USER;
	ret.push_back(es);

	return ret;
}

bool CharactersManager::CanAttackMU(int mKey, int uKey)
{
	int monstarX = monsters[mKey]->positionX;
	int monstarY = monsters[mKey]->positionY;
	int userX = users[uKey]->positionX;
	int userY = users[uKey]->positionY;

	if ((monstarX - 1 == userX) && (monstarY == userY)) return true;
	if ((monstarX + 1 == userX) && (monstarY == userY)) return true;
	if ((monstarX == userX) && (monstarY - 1 == userY)) return true;
	if ((monstarX == userX) && (monstarY + 1 == userY)) return true;
	return false;
}

std::vector<ES> CharactersManager::MonsterRespawn(int mKey)
{
	std::vector<ES> ret;

	monsters[mKey]->isDead = false;
	monsters[mKey]->positionX = monsters[mKey]->spawnX;
	monsters[mKey]->positionY = monsters[mKey]->spawnY;
	monsters[mKey]->focusKey = -1;

	ret = MonsterMove(mKey);

	return ret;
}
