#include "User.h"

User::User(std::wstring id, int lv, int posX, int posY, int exp, int hp)
{
	idStr = id;
	level = lv;
	positionX = posX;
	positionY = posY;
	experiencePoint = exp;
	healthPoint = hp;
}

User::~User() {}