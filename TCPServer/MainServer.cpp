#include <WinSock2.h>
//#define NDEBUG
#include <cassert>
#include <cstdio>
#include <mutex>
#include <exception>
#include <thread>
#include <cstring>
#include <functional>

#include "MainServer.h"
#include "../Inc/Player.h"

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "../Lib/CommonLibrary")

MainServer::MainServer()
	: mWSAData{}
	, mServerSocket{}
	, mServerAddress{}
	, mClientAddress{}
	, mClientAddressSize{}
	, mClientInfo{}
	, mClientMutex{} // 기본 생성은 mutex를 소유하지 않은 상태
	, mThreadList{}
{
	mThreadList.reserve(MainServer::MaxThreadCount);
}

MainServer::~MainServer()
{
	mClientInfo.clear();

	mThreadList.clear();
}

void MainServer::Initialize(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("usage: <%s> <port>\n", argv[0]);
		assert(argc == 2 && "usage: <filename.exe> <port>");
		exit(EXIT_FAILURE);
	}

	InitWSAData();

	MakeServerSocket();

	BindServerSocket(argv);

	ChangeToListeningState();

	AcceptClient();
}

void MainServer::Finalize()
{
	// 생성한 모든 스레드의 종료를 기다리고
	// 안전하게 프로세스를 정리하자
	for (auto* pThread : mThreadList)
	{
		pThread->join();
	}

	for (auto* pThread : mThreadList)
	{
		delete pThread;
	}

	// 동적 할당한 player 정보를 힙 메모리에서 해제하는 코드.
	// 정상적으로 모든 thread가 종료되어 client가 접속 종료했다면
	// player 정보는 메모리에서 해제되고 client socket 정보는 hash table에서 지워졌을 것이다.
	// 그렇기에 이 루프는 한번도 돌면 안된다.
	for (auto it = mClientInfo.begin(); it != mClientInfo.end(); ++it)
	{
		delete it->second;
	}

	if (WSACleanup() == SOCKET_ERROR)
	{
		printf("WSACleanup() error\n");
		WSAErrorHandling();
	}
}

void MainServer::InitWSAData()
{
	int wsaErrorCode = WSAStartup(MAKEWORD(2, 2), &mWSAData);

	if (wsaErrorCode != 0)
	{
		printf("WSAStartup() error\n");
		assert(wsaErrorCode == 0);
		exit(EXIT_FAILURE);
	}
}

void MainServer::MakeServerSocket()
{
	// ip version 4, tcp socket
	mServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mServerSocket == INVALID_SOCKET)
	{
		printf("socket() error\n");
		WSAErrorHandling();
	}
}

void MainServer::BindServerSocket(char** argv)
{
	memset(&mServerAddress, 0x00, sizeof(mServerAddress));
	mServerAddress.sin_family = AF_INET;

	// host byte order -> network byte order
	mServerAddress.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	mServerAddress.sin_port = htons(atoi(argv[1]));

	// 운영체제에게 mServerSocket이
	// 지정한 서버 주소와 포트 번호로 넘어오는 데이터를 수신하겠다고 등록
	if (bind(
		mServerSocket,
		reinterpret_cast<SOCKADDR*>(&mServerAddress),
		sizeof(mServerAddress))
		== SOCKET_ERROR)
	{
		printf("bind() error\n");
		WSAErrorHandling();
	}
}

void MainServer::ChangeToListeningState()
{
	if (listen(mServerSocket, 5) == SOCKET_ERROR)
	{
		printf("listen() error\n");
		WSAErrorHandling();
	}
}

void MainServer::AcceptClient()
{
	mClientAddressSize = sizeof(mClientAddress);

	// defer_lock으로 unique_lock을 생성하면
	// 생성할 때는 mutex를 소유하지 않고 있다가
	// 내가 원하는 타이밍에 lock()을 호출해서 mutex를 소유할 수 있다.
	std::unique_lock clientInfoLock{ mClientMutex, std::defer_lock };

	try
	{
		while (true)
		{
			// 연결 요청 수락
			// 해당 client와 통신 가능한 새로운 socket 생성
			mClientSocket = accept(
				mServerSocket,
				reinterpret_cast<SOCKADDR*>(&mClientAddress),
				&mClientAddressSize);

			if (mClientSocket == INVALID_SOCKET)
			{
				printf("accept() error\n");
				WSAErrorHandling();
			}

			Player* newPlayer = new Player{ mClientSocket, 0.f, 0.f, 0.f };

			clientInfoLock.lock();

			mClientInfo.emplace(mClientSocket, newPlayer);

			clientInfoLock.unlock();

			// thread는 멤버 함수를 수행할 수 없음
			// 멤버 함수란 this가 있어야 호출 가능한 함수인데
			// 그렇다면 thread가 작업할 함수가 멤버 함수면 항상 this가 필요하다는 것이고
			// 이런 방식은 말이 되지 않는다.
			// 그래서 static을 붙여서 class의 instance 여부와 상관없는 함수로 만들었다.
			std::thread* newThread = new std::thread{ ProcessClient, mClientSocket, this };

			// thread list 데이터 수정은 이곳뿐이어서
			// 굳이 lock을 걸지 않았음
			mThreadList.push_back(newThread);
		}
	}
	// lock() 호출 후 mutex를 소유한 상태에서 예외가 발생했을 때
	// 이를 catch하면 함수 호출이 종료되면서 스택 프레임이 해제되는데
	// 이 때 지역변수인 unique_lock이 소멸하면서 mutex를 소유한 상태라면
	// unlock()을 호출해서 mutex 소유권을 해제함.
	// 즉, mutex를 소유한 상태에서 예외가 발생하더라도 안전하게 mutex 소유권을 해제할 수 있다.
	catch (std::exception& e)
	{
		printf("exception: %s", e.what());
	}
}

void MainServer::ProcessClient(SOCKET clientSocket, MainServer* pMainServer)
{
	// 연결된 client로부터 패킷을 수신하면
	// 이를 구분해서 작업을 처리하는 함수

	char cMessageLength{};
	int recvByte = recv(clientSocket, &cMessageLength, 1, 0);
	if (recvByte == SOCKET_ERROR)
	{
		printf("recv() error\n");
		pMainServer->WSAErrorHandling();
	}

	int iMessageLength = static_cast<int>(cMessageLength);
	int totalByteReceived = iMessageLength;
	int currentByteReceived = 0;
	char recvMessage[30]{};

	while (currentByteReceived < totalByteReceived)
	{
		recvByte = recv(clientSocket, &recvMessage[currentByteReceived], totalByteReceived - currentByteReceived, 0);

		// 일단 지금은 문자열을 받는 상황이기 때문에
		// 문자열 길이를 다 받으면 while문을 탈출하게 되어있음.
		// 추후에는 계속 패킷을 주고 받다가 client가 접속 종료하면 while문을 탈출할 것임
		if (recvByte == 0)
		{
			break;
		}
		else if (recvByte == SOCKET_ERROR)
		{
			printf("recv() error");
			pMainServer->WSAErrorHandling();
		}

		currentByteReceived += recvByte;
	}

	recvMessage[currentByteReceived] = '\0';
	printf("%s\n", recvMessage);

	char sendMessage[30]{};
	memcpy(&sendMessage[0], &cMessageLength, 1);
	memcpy(&sendMessage[1], &recvMessage[0], iMessageLength);

	// 전송할 문자열 길이 +1비트
	int totalByteToSend = currentByteReceived + 1;
	int currentByteToSend = 0;
	while (currentByteToSend < totalByteToSend)
	{
		int sendByte = send(clientSocket, &sendMessage[currentByteToSend], totalByteToSend - currentByteToSend, 0);
		if (sendByte == SOCKET_ERROR)
		{
			printf("send() error\n");
			pMainServer->WSAErrorHandling();
		}

		currentByteToSend += sendByte;
	}

	std::unique_lock clientInfoLock{ pMainServer->GetClientMutex() };

	auto it = pMainServer->GetClientInfo().find(clientSocket);
	if (it == pMainServer->GetClientInfo().end())
	{
		assert(nullptr && "do not find client info");
	}

	// 접속을 종료한 client의 Player 정보 제거 후 hash table에서 삭제
	delete it->second;
	pMainServer->GetClientInfo().erase(it);

	clientInfoLock.unlock();

	// table에서 client 정보를 제거했기 때문에
	// 해당 client에는 더 이상 패킷을 전송되지 않음
	// 안전하게 socket만 닫아주면 되기 때문에
	// closesocket()은 lock() 밖에서 수행했음
	if (closesocket(clientSocket) == SOCKET_ERROR)
	{
		printf("closesocket() error\n");
		pMainServer->WSAErrorHandling();
	}
}

std::mutex& MainServer::GetClientMutex()
{
	return mClientMutex;
}

std::unordered_map<SOCKET, Player*>& MainServer::GetClientInfo()
{
	return mClientInfo;
}

void MainServer::WSAErrorHandling()
{
	int wsaErrorCode = WSAGetLastError();
	printf("WSAErrorCode: %d\n", wsaErrorCode);

	assert(wsaErrorCode == 0);
	exit(EXIT_FAILURE);
}
