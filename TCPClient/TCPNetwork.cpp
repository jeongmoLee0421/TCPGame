//#define NDEBUG
#include <cassert>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <cstring>
#include <exception>

#include "TCPNetwork.h"
#include "../Inc/PacketEnum.h"
#include "../Inc/PacketDefine.h"
#include "../Inc/IRenderer.h"

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "../Lib/CommonLibrary")

TCPNetwork::TCPNetwork()
	: mServerIp{}
	, mServerPort{}
	, mWSAData{}
	, mClientSocket{}
	, mServerAddress{}
	, mSendThread{ nullptr }
	, mRecvThread{ nullptr }
	, mMyPlayerData{ nullptr }
{
}

TCPNetwork::~TCPNetwork()
{
}

void TCPNetwork::Initialize(LPSTR lpCmdLine)
{
	InitSocketCommunicate(lpCmdLine);
}

void TCPNetwork::Finalize()
{
	delete mMyPlayerData;

	// 스레드가 완전히 종료되길 기다림
	WaitForThreadToEnd();
	CleanWSAData();

	// 스레드 정보(스레드 핸들 값, id 값)가 힙 메모리에 할당되어 있기 때문에
	// 메모리 해제
	delete mSendThread;
	delete mRecvThread;
}

void TCPNetwork::Render(IRenderer* pRenderer)
{
	// 접속해 있는 client의 position을 가지고
	// 각각의 worldTM을 만들어서
	// 3d 세상의 cube로 묘사했다.
	for (auto& pair : mClientInfo)
	{
		pRenderer->DrawCube(pair.second->GetPosition());
	}
}

void TCPNetwork::SendPacketToServer(TCPNetwork* pTCPNetwork)
{

}

void TCPNetwork::RecvPacketFromServer(TCPNetwork* pTCPNetwork)
{
	// packet의 header를 먼저 받아서 ProcessPacket()으로 보낸다.

	PacketHeader packetHeader{};
	char packetHeaderData[sizeof(packetHeader)]{};

	int totalByteReceived = sizeof(packetHeader);
	int currentByteReceived = 0;

	while (currentByteReceived < totalByteReceived)
	{
		int recvByte = recv(
			pTCPNetwork->mClientSocket,
			&packetHeaderData[0] + currentByteReceived,
			totalByteReceived - currentByteReceived,
			0
		);

		if (recvByte == SOCKET_ERROR)
		{
			assert(nullptr && "recv() error");
			pTCPNetwork->WSAErrorHandling();
		}

		currentByteReceived += recvByte;
	}

	packetHeader.Deserialize(&packetHeaderData[0]);

	ProcessPacket(packetHeader.mPacketHeader, pTCPNetwork);
}

void TCPNetwork::ProcessPacket(ePacketHeader packetHeader, TCPNetwork* pTCPNetwork)
{
	switch (packetHeader)
	{
	case ePacketHeader::S2C_LoadData:
	{
		// 패킷 헤더는 이미 위에서 한번 받았음
		int totalByteRecevied{ sizeof(PacketPlayer) - sizeof(PacketHeader) };
		int currentByteRecevied{ 0 };

		PacketPlayer packetPlayer{};
		char recvBuf[512]{};

		while (currentByteRecevied < totalByteRecevied)
		{
			int recvByte = recv(
				pTCPNetwork->mClientSocket,
				&recvBuf[0] + currentByteRecevied,
				totalByteRecevied - currentByteRecevied,
				0);

			if (recvByte == SOCKET_ERROR)
			{
				printf("ProcessPacket() -> recv() error\n");
				assert(nullptr && "ProcessPacket() -> recv() error");
				pTCPNetwork->WSAErrorHandling();
			}

			currentByteRecevied += recvByte;
		}

		packetPlayer.Deserialize(&recvBuf[0]);

		// server로부터 본인 데이터를 받으면 clientInfo에 넣어주자.
		pTCPNetwork->mMyPlayerData = new Player{ packetPlayer.mPlayer };
		pTCPNetwork->GetClientInfo().emplace(pTCPNetwork->mMyPlayerData->GetSocket(), pTCPNetwork->mMyPlayerData);
	}
	break;

	case ePacketHeader::S2C_NewClientConnection:
	{

	}
	break;

	default:
		break;
	}
}

void TCPNetwork::InitSocketCommunicate(LPSTR lpCmdLine)
{
	ParsingServerInformation(lpCmdLine);

	InitWSAData();

	MakeClientSocket();

	FillServerInformation();

	ConnectServer();

	MakeThread();

	// 처음 접속한 신규 유저라고 가정하고 초기 데이터를 받아옴
	LoadDataFromServer();

	// 기존 유저와 동기화
	NotifyNewClientConnection();
}

void TCPNetwork::ParsingServerInformation(LPSTR lpCmdLine)
{
	const char* strServerInfo = lpCmdLine;
	int index = 0;

	// 불필요한 공백 제거
	SkipWhitespaceCharacters(strServerInfo);

	while (*strServerInfo != ' ')
	{
		mServerIp[index] = *strServerInfo;

		++index;
		++strServerInfo;
	}

	mServerIp[index] = '\0';
	index = 0;

	SkipWhitespaceCharacters(strServerInfo);

	while (*strServerInfo != '\0')
	{
		mServerPort[index] = *strServerInfo;

		++index;
		++strServerInfo;
	}

	mServerPort[index] = '\0';
}

void TCPNetwork::SkipWhitespaceCharacters(const char* str)
{
	while (*str == ' ')
	{
		++str;
	}
}

void TCPNetwork::InitWSAData()
{
	int wsaErrorCode = WSAStartup(MAKEWORD(2, 2), &mWSAData);

	if (wsaErrorCode != 0)
	{
		assert(nullptr && "InitWSAData() -> WSAStartup() error");
		exit(EXIT_FAILURE);
	}
}

void TCPNetwork::MakeClientSocket()
{
	mClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (mClientSocket == INVALID_SOCKET)
	{
		assert(nullptr && "MakeClientSocket() -> socket() error");
		WSAErrorHandling();
	}
}

void TCPNetwork::FillServerInformation()
{
	memset(&mServerAddress, 0x00, sizeof(mServerAddress));
	mServerAddress.sin_family = AF_INET;

	// ip주소는 inet_pton() 내부에서 네트워크 바이트 순서(Big endian)로 변경
	inet_pton(AF_INET, &mServerIp[0], &mServerAddress.sin_addr.S_un.S_addr);
	mServerAddress.sin_port = htons(atoi(mServerPort));
}

void TCPNetwork::ConnectServer()
{
	// 괄호 잘못 닫는 거 조심해야함
	// 아래에서 connect() 괄호 닫을 때 SOCKET_ERROR 밖에서 닫아서 sizeof(serverAddress) == SOCKET_ERROR가 0으로 계산되었고
	// 세번째 파라미터 값이 0으로 전달되어서 WSAEFAULT(10014) 오류 발생(구조체의 인수 길이가 sizeof(sockaddr)보다 작음)
	//if (connect(clientSocket, reinterpret_cast<SOCKADDR*>(&serverAddress), sizeof(serverAddress) == SOCKET_ERROR))

	// connect()를 통해 지정한 ip주소의 server computer에서 지정한 port 번호에 해당하는 프로세스(애플리케이션)에게 연결 요청 시도
	// 이 때 운영체제는 이 client 프로세스의 포트 번호를 임의로 지정해주면서 네트워크로 패킷을 송신
	if (connect(mClientSocket, reinterpret_cast<SOCKADDR*>(&mServerAddress), sizeof(mServerAddress)) == SOCKET_ERROR)
	{
		assert(nullptr && "ConnectServer() -> connect() error");
		WSAErrorHandling();
	}
}

void TCPNetwork::LoadDataFromServer()
{
	PacketHeader packetHeader{ ePacketHeader::C2S_LoadData };
	char sendBuf[sizeof(packetHeader)]{};
	packetHeader.Serialize(&sendBuf[0]);

	int totalByteToSend{ sizeof(packetHeader) };
	int currentByteToSend{ 0 };

	while (currentByteToSend < totalByteToSend)
	{
		int sendByte = send(
			mClientSocket,
			&sendBuf[0] + currentByteToSend,
			totalByteToSend - currentByteToSend,
			0);

		if (sendByte == SOCKET_ERROR)
		{
			assert(nullptr && "LoadDataFromServer() -> send() error");
			WSAErrorHandling();
		}

		currentByteToSend += sendByte;
	}
}

void TCPNetwork::MakeThread()
{
	mSendThread = new std::thread{ SendPacketToServer, this };
	mRecvThread = new std::thread{ RecvPacketFromServer, this };
}

void TCPNetwork::NotifyNewClientConnection()
{
	// 기존에 server에 접속해 있던 client와
	// player 데이터를 동기화
}

void TCPNetwork::WaitForThreadToEnd()
{
	mSendThread->join();
	mRecvThread->join();
}

void TCPNetwork::CleanWSAData()
{
	if (WSACleanup() == SOCKET_ERROR)
	{
		assert(nullptr && "CleanWSAData() -> WSACleanup() error");
		WSAErrorHandling();
	}
}

void TCPNetwork::WSAErrorHandling()
{
	int wsaErrorCode = WSAGetLastError();
	exit(EXIT_FAILURE);
}

std::unordered_map<SOCKET, Player*>& TCPNetwork::GetClientInfo()
{
	return mClientInfo;
}
