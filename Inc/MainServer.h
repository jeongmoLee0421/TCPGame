#pragma once

#include <WinSock2.h>
#include <unordered_map>
#include <mutex>
#include <vector>

class Player;
class std::thread;

// 2023 07 27 이정모 home

// server 기능을 class로 묶었다.
// main thread가 접속을 요청하는 client를 받아서 client socket을 생성하고
// 나머지 thread는 접속한 client에 대응하는 서버

class MainServer
{
public:
	MainServer();
	~MainServer();

	void Initialize(int argc, char** argv);
	void Finalize();

public:
	void AcceptClient();

private:
	static void ProcessClient(SOCKET clientSocket, MainServer* pMainServer);

public:
	// mutex class는 복사 생성/대입을 delete했고 -> 값 복사 불가
	// 이동 생성/대입을 정의하지 않았기 때문에 -> 이동 생성/대입이 정의되지 않으면 ravlue 생성/대입이 복사 생성/대입으로 호출되는데 delete임
	// lvalue, rvalue 모두 값 복사가 막혔기 때문에 원본을 참조로 넘겼음
	std::mutex& GetClientMutex();

	std::unordered_map<SOCKET, Player*>& GetClientInfo();

private:
	void WSAErrorHandling();

private:
	// player 정보는 우선 순위가 없어서 순서를 따지는 순회가 중요하지 않기 때문에 hash table을 사용했음.
	// 만약 client에서 최적화를 위해 (VB, IB등의 자원이 같은)동일한 object를 몰아서 rendering하는 경우,
	// client에서 직접 순서를 가진 자료구조를 사용하도록 하자.
	std::unordered_map<SOCKET, Player*> mClientInfo;

	std::vector<std::thread*> mThreadList;

private:
	// constexpr을 사용하여 compile time에 상수 값을 확정하기 위해서
	// static을 사용하여 compile time에 데이터 영역에 메모리를 잡자.
	static int constexpr MaxThreadCount = 10;

private:
	WSAData mWSAData;
	SOCKET mServerSocket;

	SOCKADDR_IN mServerAddress;
	SOCKADDR_IN mClientAddress;
	int mClientAddressSize;

	SOCKET mClientSocket;

	std::mutex mClientMutex;
};