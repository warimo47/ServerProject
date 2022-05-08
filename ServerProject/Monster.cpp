#include "Monster.h"

Monster::Monster(int lv, int posX, int posY, int mType)
{
	level = lv;
	positionX = posX;
	positionY = posY;
	healthPoint = mType;

	spawnX = posX;
	spawnY = posY;

	monsterType = mType; // 2, 3, 4, 5
	isDead = false;
	focusKey = -1;
}

Monster::~Monster() {}
