#pragma once

#include "Character.h"

class Monster : public Character
{
private:

protected:

public:
	int spawnX;
	int spawnY;

	int monsterType;
	std::atomic<bool> isDead;
	std::atomic<int> focusKey;

private:

protected:

public:
	Monster(int lv, int posX, int posY, int mType);
	~Monster();
};

