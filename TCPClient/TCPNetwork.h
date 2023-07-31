#pragma once

#define _WINSOCKAPI_
#include <Windows.h>
#include <WinSock2.h>

namespace std
{
	class thread;
}

// 2023 07 31 이정모 home

// TCP 기능의 소켓을 생성해서
// server와 통신하는 class

class TCPNetwork
{
public:
	TCPNetwork();
	~TCPNetwork();

	void Initialize(LPSTR lpCmdLine);
	void Finalize();

private:
	// 송신 thread와 수신 thread를 분리하여
	// 병렬적으로 송수신 처리
	static void SendPacketToServer(TCPNetwork* pTCPNetwork);
	static void RecvPacketFromServer(TCPNetwork* pTCPNetwork);

private:
	void InitSocketCommunicate(LPSTR lpCmdLine);

private:
	void ParsingServerInformation(LPSTR lpCmdLine);
	void SkipWhitespaceCharacters(const char* str);

private:
	void InitWSAData();
	void MakeClientSocket();
	void FillServerInformation();
	void ConnectServer();
	void WaitForThreadToEnd();
	void CleanWSAData();

private:
	void WSAErrorHandling();

private:
	char mServerIp[16];
	char mServerPort[6];

	WSADATA mWSAData;
	SOCKET mClientSocket;
	SOCKADDR_IN mServerAddress;

	std::thread* mSendThread;
	std::thread* mRecvThread;
};