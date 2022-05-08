#pragma comment ( lib , "ws2_32.lib" )

#include <thread>
#include <vector>
#include <fstream>
#include <ctime>

#include <locale.h>

#include "NetworkManager.h"
#include "DBManager.h"
#include "CharactersManager.h"
#include "Timer.h"

void WorkerThreadsFunc(HANDLE hIOCP, NetworkManager* networkManager)
{
	while (true)
	{
		unsigned long ioSize;
		unsigned long long iocpKey; // It is 64 bit, because of compile to 64 bit
		WSAOVERLAPPED* wsaOverlapped;
		int ret = GetQueuedCompletionStatus(hIOCP, &ioSize, &iocpKey, &wsaOverlapped, INFINITE);
		int key = static_cast<int>(iocpKey);
		// std::cout << "WT: Network I/O with Client [" << key << "]\n\n";

		if (ret == 0)
		{
			std::cout << "WT: Error in GQCS\n";
			networkManager->ClientDisconnect(key);
			continue;
		}

		if (ioSize == 0)
		{
			networkManager->ClientDisconnect(key);
			continue;
		}

		networkManager->IOTypeVerification(reinterpret_cast<EXOVER*>(wsaOverlapped), ioSize, key);
	}
}

void AcceptThreadFunc(HANDLE hIOCP, NetworkManager* networkManager, CharactersManager* charactersManager)
{
	SOCKET s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN bind_addr;
	ZeroMemory(&bind_addr, sizeof(bind_addr));
	bind_addr.sin_family = AF_INET;
	bind_addr.sin_port = htons(MY_SERVER_PORT);
	bind_addr.sin_addr.s_addr = INADDR_ANY;	// 0.0.0.0 // It means from anywhere

	bind(s, reinterpret_cast<sockaddr *>(&bind_addr), sizeof(bind_addr));
	listen(s, 1000);

	std::cout << "Server Ready!\n\n";

	while (true)
	{
		SOCKADDR_IN c_addr;
		ZeroMemory(&c_addr, sizeof(c_addr));
		c_addr.sin_family = AF_INET;
		c_addr.sin_port = htons(MY_SERVER_PORT);
		c_addr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0 // It means from anywhere
		int addr_size = sizeof(sockaddr);

		SOCKET newAcceptedSocket = WSAAccept(s, reinterpret_cast<sockaddr *>(&c_addr), &addr_size, NULL, NULL);

		if (newAcceptedSocket == INVALID_SOCKET)
		{
			ErrorDisplay(L"In Accept Thread: WSAAccept()");
			continue;
		}

		int newClientKey = networkManager->ClientConnect(newAcceptedSocket);

		CreateIoCompletionPort(reinterpret_cast<HANDLE>(newAcceptedSocket), hIOCP, newClientKey, 0);

		networkManager->StartRecv(newClientKey);
	}
}

void TimerThreadFunc(HANDLE hIOCP, Timer* timer, std::chrono::steady_clock::time_point timerStartTime)
{
	std::cout << "Timer thread Start!\n\n";

	while (true)
	{
		timer->CheckEvent();

		Sleep(1000);
	}
}

int main()
{
	std::srand(static_cast<unsigned int>(std::time(nullptr)));

	std::wcout.imbue(std::locale("korean"));
	setlocale(LC_ALL, "korean");

	NetworkManager* networkManager = new NetworkManager();
	CharactersManager* charactersManager = new CharactersManager();
	DBManager* dbManager = new DBManager();
	Timer timer;

	HANDLE hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	networkManager->SetCharactersManagerPointer(charactersManager);
	networkManager->SetDBManagerPointer(dbManager);
	networkManager->SetTimerPointer(&timer);
	networkManager->SetIOCPHandle(hIOCP);

	dbManager->SetCharactersManagerPointer(charactersManager);
		
	timer.SetNetworkManagerPointer(networkManager);
	timer.SetCharactersManagerPointer(charactersManager);
	timer.MonsterEventSetting();

	WSADATA	wsadata;
	int ret = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (ret != 0)
	{
		std::wcout << L"WSAStartup return " << ret << " \n";
		ret = WSAGetLastError();
		std::wcout << L"WSAGetLastError return " << ret << " \n";
	}

	// Create worker threads 
	std::vector <std::thread> w_threads;
	unsigned int cts = std::thread::hardware_concurrency(); // Concurrent Threads Supported
	for (unsigned int i = 0; i < cts; ++i)
	{
		w_threads.push_back(std::thread{ WorkerThreadsFunc, hIOCP, networkManager });
	}

	// Create accept thread
	std::thread a_thread{ AcceptThreadFunc, hIOCP, networkManager, charactersManager };

	// Create timer thread
	std::thread t_thread{ TimerThreadFunc, hIOCP, &timer, std::chrono::high_resolution_clock::now() };

	// Join worker threads
	for (auto& th : w_threads)
	{
		th.join();
	}

	// Join accept thread
	a_thread.join();

	// Join timer thread
	t_thread.join();

	delete dbManager;
	delete charactersManager;
	delete networkManager;

	return 0;
}