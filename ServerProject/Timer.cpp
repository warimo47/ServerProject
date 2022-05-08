#include "Timer.h"
#include "NetworkManager.h"
#include "CharactersManager.h"

Timer::Timer()
{
	networkManager = nullptr;
	charactersManager = nullptr;

	timerStartTime = std::chrono::high_resolution_clock::now();
}

Timer::~Timer() {}

void Timer::SetNetworkManagerPointer(NetworkManager* nm)
{
	networkManager = nm;
}

void Timer::SetCharactersManagerPointer(CharactersManager* cm)
{
	charactersManager = cm;
}

void Timer::MonsterEventSetting()
{
	std::chrono::high_resolution_clock::time_point nowTime = std::chrono::high_resolution_clock::now();
	std::chrono::nanoseconds nowNanoSec = nowTime - timerStartTime;
	long long nowCount = std::chrono::duration_cast<std::chrono::milliseconds>(nowNanoSec).count();

	Event tempEvent;

	int mType = -1;

	for (int mKey = 0; mKey < 500; ++mKey)
	{
		mType = charactersManager->GetMonsterType(mKey);

		if (mType == OT_MONSTER2 || mType == OT_MONSTER4)
		{
			tempEvent.key = mKey;
			tempEvent.eventType = IOT_MONSTER_MOVE;
			tempEvent.time = nowCount + 10000; // 10 Sec
			timerQueue.push(tempEvent);
		}

		tempEvent.eventType = IOT_MONSTER_ATTACK;
		tempEvent.time = nowCount + 10500; // 10.5 Sec
		timerQueue.push(tempEvent);
	}
}

void Timer::CheckEvent()
{
	while (true)
	{
		std::chrono::high_resolution_clock::time_point nowTime = std::chrono::high_resolution_clock::now();
		std::chrono::nanoseconds nowNanoSec = nowTime - timerStartTime;
		long long nowCount = std::chrono::duration_cast<std::chrono::milliseconds>(nowNanoSec).count();

		mutexTimer.lock();

		if (timerQueue.empty())
		{
			mutexTimer.unlock();
			break;
		}

		if (timerQueue.top().time > nowCount)
		{
			mutexTimer.unlock();
			break;
		}

		int key = timerQueue.top().key;
		unsigned char eventType = timerQueue.top().eventType;

		timerQueue.pop();

		mutexTimer.unlock();

		networkManager->InputEventToIOCP(key, eventType);
	}
}

void Timer::PushEvent(int key, unsigned char eventType, long long delay)
{
	std::chrono::high_resolution_clock::time_point nowTime = std::chrono::high_resolution_clock::now();
	std::chrono::nanoseconds nowNanoSec = nowTime - timerStartTime;
	long long nowCount = std::chrono::duration_cast<std::chrono::milliseconds>(nowNanoSec).count();
	nowCount += 1000;

	if (eventType == IOT_MONSTER_RESPAWN || eventType == IOT_USER_HEAL)
	{
		nowCount += 9000;
	}

	nowCount += delay;

	Event newEvent{ nowCount, key, eventType };

	mutexTimer.lock();
	timerQueue.push(newEvent);
	mutexTimer.unlock();
}