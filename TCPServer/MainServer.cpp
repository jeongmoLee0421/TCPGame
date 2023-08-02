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
#include "../Inc/PacketEnum.h"
#include "../Inc/PacketDefine.h"

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
		assert(nullptr && "Finalize() -> WSACleanup() error");
		WSAErrorHandling();
	}
}

void MainServer::InitWSAData()
{
	int wsaErrorCode = WSAStartup(MAKEWORD(2, 2), &mWSAData);

	if (wsaErrorCode != 0)
	{
		printf("WSAStartup() error\n");
		assert(nullptr && "InitWSAData() -> WSAStartup() error");
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
		assert(nullptr && "MakeServerSocket() -> socket() error");
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
		assert(nullptr && "BindServerSocket() -> bind() error");
		WSAErrorHandling();
	}
}

void MainServer::ChangeToListeningState()
{
	if (listen(mServerSocket, 5) == SOCKET_ERROR)
	{
		printf("listen() error\n");
		assert(nullptr && "ChangeToListeningState() -> listen() error");
		WSAErrorHandling();
	}
}

void MainServer::AcceptClient()
{
	mClientAddressSize = sizeof(mClientAddress);

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
				assert(nullptr && "AcceptClient() -> accept() error");
				WSAErrorHandling();
			}

			// thread는 멤버 함수를 수행할 수 없음
			// 멤버 함수란 this가 있어야 호출 가능한 함수인데
			// 그렇다면 thread가 작업할 함수가 멤버 함수면 항상 this가 필요하다는 것이고
			// 이런 방식은 말이 되지 않는다.
			// 그래서 static을 붙여서 class의 instance 여부와 상관없는 함수로 만들었다.
			std::thread* newThread = new std::thread{ MainServer::ProcessClient, mClientSocket, this };

			// thread list 데이터 수정은 이곳뿐이어서
			// 굳이 lock을 걸지 않았음
			mThreadList.push_back(newThread);
		}
	}
	catch (std::exception& e)
	{
		printf("AcceptClient() -> exception: %s", e.what());
	}
}

void MainServer::ProcessClient(SOCKET clientSocket, MainServer* pMainServer)
{
	// 연결된 client로부터 패킷을 수신하면
	// 이를 구분해서 작업을 처리하는 함수
	// client당 thread가 1개씩 대응되기 때문에
	// socket 번호는 이미 알고 있음
	// 다만 작업 단위로 thread가 구성되지 않고
	// client 단위로 thread가 구성된다면
	// thread의 최대 생성 개수로 clinet의 최대 접속자 수가 제한됨

	PacketHeader packetHeader{};
	char packetHeaderData[sizeof(PacketHeader)]{};

	try
	{
		int totalByteRecevied{ sizeof(packetHeader) };
		int currentByteReceived{ 0 };

		while (currentByteReceived < totalByteRecevied)
		{
			int recvByte = recv(
				clientSocket,
				&packetHeaderData[0] + currentByteReceived,
				totalByteRecevied - currentByteReceived,
				0);

			if (recvByte == SOCKET_ERROR)
			{
				printf("ProcessClient() -> recv() error\n");
				assert(nullptr && "ProcessClient() -> recv() error");
				pMainServer->WSAErrorHandling();
			}

			currentByteReceived += recvByte;
		}

		packetHeader.Deserialize(&packetHeaderData[0]);

		MainServer::ProcessPacket(packetHeader.mPacketHeader, clientSocket, pMainServer);
	}
	catch (std::exception& e)
	{
		printf("ProcessClient() -> exception: %s", e.what());
	}
}

void MainServer::ProcessPacket(ePacketHeader packetHeader, SOCKET clientSocket, MainServer* pMainServer)
{
	// 참조로 받지 않으면 복사본이 된다.
	auto& clientInfo{ pMainServer->GetClientInfo() };

	// defer_lock을 통해 내가 원하는 타이밍에 lock을 걸 수 있다.
	std::unique_lock<std::mutex> clientLock{ pMainServer->GetClientMutex(), std::defer_lock };

	try
	{
		switch (packetHeader)
		{
		case ePacketHeader::C2S_LoadData:
		{
			// 일단은 모든 client가 새롭게 계정을 생성해서 접속했다고 가정
			// 추후에는 기존 유저인지 아닌지 판단해서 DB에서 데이터를 꺼내주도록 변경하자

			Player* newPlayer = new Player{ clientSocket, 0.f, 0.f, 0.f };

			clientLock.lock();

			pMainServer->GetClientInfo().emplace(clientSocket, newPlayer);

			clientLock.unlock();

			PacketPlayer packetPlayer{};
			packetPlayer.mPacketHeader = ePacketHeader::S2C_LoadData;

			// player 구조체에 포인터 변수가 없기 때문에
			// 복사 대입이나 이동 대입이나 아직은 차이가 없음
			packetPlayer.mPlayer = *newPlayer;

			int totalByteToSend = sizeof(packetPlayer);
			int currentByteToSend = 0;

			char sendBuf[512]{};
			packetPlayer.Serialize(&sendBuf[0]);

			while (currentByteToSend < totalByteToSend)
			{
				int sendByte = send(
					clientSocket,
					&sendBuf[0] + currentByteToSend,
					totalByteToSend - currentByteToSend,
					0);

				if (sendByte == SOCKET_ERROR)
				{
					printf("ProcessPacket() -> send() error\n");
					assert(nullptr && "ProcessPacket() -> send() error");
					pMainServer->WSAErrorHandling();
				}

				currentByteToSend += sendByte;
			}
		}
		break;

		case ePacketHeader::C2S_NewClientConnection:
		{

		}
		break;

		default:
			break;
		}
	}
	// 예외가 발생했을 때 잡는다면
	// 이 함수의 스택 메모리가 정상적으로 해제되고
	// unique_lock의 소멸자가 호출되는데, 이 때 mutex의 소유권을 가지고 있다면
	// mutex의 소유권을 해제(unlock())하기 때문에
	// 안전하게 mutex를 사용할 수 있음
	catch (std::exception& e)
	{
		printf("ProcessPacket() -> exception: %s", e.what());
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
	exit(EXIT_FAILURE);
}
