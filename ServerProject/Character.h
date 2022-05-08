#pragma once

#include <unordered_set>
#include <mutex>
#include <atomic>

class Character
{
private:

protected:

public:
	std::atomic<int> level;
	std::atomic<int> positionX;
	std::atomic<int> positionY;
	std::atomic<int> healthPoint;

	std::unordered_set<int> userViewList;
	std::mutex mutexUserVL;
	
private:

protected:

public:
};

