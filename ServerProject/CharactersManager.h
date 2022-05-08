#pragma once

#include <iostream>
#include <fstream>
#include <map>
#include <vector>

#include "Monster.h"
#include "User.h"
#include "Timer.h"

#define VIEW_RADIUS 49
#define OT_NONE 0
#define OT_USER 1
#define OT_MONSTER1 2
#define OT_MONSTER2 3
#define OT_MONSTER3 4
#define OT_MONSTER4 5
#define BOARD_WIDTH 300
#define BOARD_HEIGHT 300
#define ET_REMOVEOBJECT 1
#define ET_STATCHANGE 2
#define ET_ADDOBJECT 3
#define ET_POSITIONINFO 4
#define ET_POSTIOCP 5
#define ET_RESPAWNMONSTER 6
#define ET_HEALSTART 7

struct CH_Info
{
	int key;
	int objectType;
	int level;
	int exp;
	int x;
	int y;
	int hp;
};

struct ES // Event struct
{
	int eventType;
	int sendTargetKey;
	int objectKey;
	int objectType;
};

class CharactersManager
{
private:
	std::map<int, std::shared_ptr<User>> users;
	std::mutex mutexUsers;

	std::vector<Monster*> monsters;

	bool obstacles[300][300];

protected:

public:

private:

protected:

public:
	CharactersManager();
	~CharactersManager();

	void UserLogIn(std::wstring, int, int, int, int, int, int);
	void UserLogOut(int uKey);

	CH_Info GetUserInfo(int uKey);
	std::wstring GetUserWstrID(int uKey);

	CH_Info GetMonsterInfo(int mKey);
	int GetMonsterType(int mKey);

	// Field of view processing
	bool CanSeeU(int uKeyA, int uKeyB);
	bool CanSeeM(int uKey, int mKey);
	void InsertViewList(int uKeyA, int uKeyB);
	std::vector<CH_Info> GetMonstersInSight(int uKey);
	void InsertUserToMonsterVL(int uKey, int mKey);
	void RemoveUserToMonsterVL(int uKey, int mKey);
	void GetNewVLUU(int uKey, std::unordered_set<int>* nUserVL, std::unordered_set<int>* nUserRemoveVL);
	void GetNewVLUM(int uKey, std::unordered_set<int>* nMonsterVL, std::unordered_set<int>* nMonsterRemoveVL);
	void GetNewVLMU(int mKey, std::unordered_set<int>* nUserVL, std::unordered_set<int>* nUserRemoveVL);
	
	// User move processing
	std::vector<ES> TryMove(int uKey, int dir);
	bool CanMove(int uKey, int dir, int* xp, int* yp, std::chrono::steady_clock::time_point* now);
	
	// Uesr attack processing
	std::vector<ES> TryAttack(int uKey);
	bool CanAttackUM(int uKey, int mKey);

	// Timer processing
	std::vector<ES> UserHeal(int uKey);
	
	// Monster move processing
	std::vector<ES> MonsterMove(int mkey);
	int FindDirection(int mKey);

	// Monster attack processing
	std::vector<ES> MonsterAttack(int mkey);
	bool CanAttackMU(int mKey, int uKey);

	// Monster respawn processing
	std::vector<ES> MonsterRespawn(int mKey);
};

