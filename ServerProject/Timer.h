#pragma once

// #include <chrono>
#include <queue>
#include <mutex>

struct Event
{
	long long time;
	int key;
	unsigned char eventType;
};

struct E_Min
{
	bool operator()(Event a, Event b)
	{
		return a.time < b.time;
	}
};

class NetworkManager;
class CharactersManager;

class Timer
{
private:
	NetworkManager* networkManager;
	CharactersManager* charactersManager;

	std::chrono::high_resolution_clock::time_point timerStartTime;

	std::priority_queue<Event, std::vector<Event>, E_Min> timerQueue;
	std::mutex mutexTimer;

protected:

public:

private:

protected:

public:
	Timer();
	~Timer();

	void SetNetworkManagerPointer(NetworkManager* nm);
	void SetCharactersManagerPointer(CharactersManager* cm);

	void MonsterEventSetting();

	void CheckEvent();

	void PushEvent(int key, unsigned char eventType, long long delay);
};

