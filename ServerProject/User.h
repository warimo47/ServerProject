#pragma once

#include <unordered_set>
#include <string>
#include "Character.h"

class User : public Character
{
private:
	
protected:

public:
	std::atomic<bool> isReadingNow;

	std::wstring idStr;
	std::atomic<int> experiencePoint;

	std::unordered_set<int> monsterViewList;
	std::mutex mutexMonsterVL;

	std::chrono::time_point<std::chrono::steady_clock> tpMoveCoolTime;
	std::chrono::time_point<std::chrono::steady_clock> tpAttackCoolTime;

private:

protected:

public:
	User(std::wstring, int, int, int, int, int);
	~User();

};

