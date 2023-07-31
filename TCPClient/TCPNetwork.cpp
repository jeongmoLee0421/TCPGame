//#define NDEBUG
#include <cassert>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>

#include "TCPNetwork.h"

#pragma comment(lib, "ws2_32")

TCPNetwork::TCPNetwork()
	: mServerIp{}
	, mServerPort{}
	, mWSAData{}
	, mClientSocket{}
	, mServerAddress{}
	, mSendThread{ nullptr }
	, mRecvThread{ nullptr }
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
	// 스레드가 완전히 종료되길 기다림
	WaitForThreadToEnd();
	CleanWSAData();

	// 스레드 정보(스레드 핸들 값, id 값)가 힙 메모리에 할당되어 있기 때문에
	// 메모리 해제
	delete mSendThread;
	delete mRecvThread;
}

void TCPNetwork::SendPacketToServer(TCPNetwork* pTCPNetwork)
{
	const char* message = "hello server";

	int iMessageLength = static_cast<int>(strlen(message));
	char cMessageLength = static_cast<char>(iMessageLength);

	char sendData[30]{};
	memcpy(&sendData[0], &cMessageLength, 1);

	// 그럴일은 없지만 보낼 바이트 수에 int가 음수로 들어가게 되면
	// unsigned long long을 받는 strcpy_s()는 MSB가 1로 세팅된 이 수를 매우 큰 수로 해석하기 때문에 문제가 발생할 수 있음
	strcpy_s(&sendData[1], static_cast<rsize_t>(30 - 1), message);

	// 첫번째 비트는 message의 길이 정보
	int totalByteToSend = iMessageLength + 1;
	int currentByteToSend = 0;

	while (currentByteToSend < totalByteToSend)
	{
		// 정적 멤버 변수이기 때문에 this가 존재하지 않는다.
		// this를 매개 변수로 받아서 사용하자.
		int sendByte = send(pTCPNetwork->mClientSocket, &sendData[currentByteToSend], totalByteToSend - currentByteToSend, 0);

		if (sendByte == SOCKET_ERROR)
		{
			pTCPNetwork->WSAErrorHandling();
		}

		currentByteToSend += sendByte;
	}

	// 더 이상 보낼 데이터가 없고
	// 수신할 데이터만 존재하기 때문에
	// 출력 스트림만 닫는다.
	if (shutdown(pTCPNetwork->mClientSocket, SD_SEND) == SOCKET_ERROR)
	{
		pTCPNetwork->WSAErrorHandling();
	}
}

void TCPNetwork::RecvPacketFromServer(TCPNetwork* pTCPNetwork)
{
	char cMessageLength{};
	int recvByte = recv(pTCPNetwork->mClientSocket, &cMessageLength, 1, 0);
	if (recvByte == SOCKET_ERROR)
	{
		pTCPNetwork->WSAErrorHandling();
	}

	int iMessageLength = static_cast<int>(cMessageLength);

	char sendData[30]{};
	int totalByteReceived = iMessageLength;
	int currentByteReceived = 0;
	while (currentByteReceived < totalByteReceived)
	{
		recvByte = recv(pTCPNetwork->mClientSocket, &sendData[currentByteReceived], totalByteReceived - currentByteReceived, 0);

		// 연결이 정상적으로 닫힌 경우
		if (recvByte == 0)
		{
			break;
		}
		else if (recvByte == SOCKET_ERROR)
		{
			pTCPNetwork->WSAErrorHandling();
		}

		currentByteReceived += recvByte;
	}

	sendData[currentByteReceived] = '\0';

	if (shutdown(pTCPNetwork->mClientSocket, SD_RECEIVE) == SOCKET_ERROR)
	{
		pTCPNetwork->WSAErrorHandling();
	}
}

void TCPNetwork::InitSocketCommunicate(LPSTR lpCmdLine)
{
	ParsingServerInformation(lpCmdLine);

	InitWSAData();

	MakeClientSocket();

	FillServerInformation();

	ConnectServer();

	mSendThread = new std::thread{ SendPacketToServer, this };
	mRecvThread = new std::thread{ RecvPacketFromServer, this };
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
		assert(wsaErrorCode == 0);
		exit(EXIT_FAILURE);
	}
}

void TCPNetwork::MakeClientSocket()
{
	mClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (mClientSocket == INVALID_SOCKET)
	{
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
		WSAErrorHandling();
	}
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
		WSAErrorHandling();
	}
}

void TCPNetwork::WSAErrorHandling()
{
	int wsaErrorCode = WSAGetLastError();

	assert(wsaErrorCode == 0);
	exit(EXIT_FAILURE);
}
