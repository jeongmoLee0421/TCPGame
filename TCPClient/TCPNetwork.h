#pragma once

#define _WINSOCKAPI_
#include <Windows.h>
#include <WinSock2.h>

#include <unordered_map>

namespace std
{
	class thread;
}

enum class ePacketHeader : unsigned short;
class Player;
class IRenderer;
class Input;

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

	// Input class는 Update()에서만 사용되고
	// IRenderer class는 Render()에서만 사용된다.
	// 여러 곳에서 사용되는 것이 아니고 사용되는 곳이 명확하기 때문에
	// 멤버 변수로 들고 있지 않고
	// 함수 호출 때 매개변수로 넘겼다.
public:
	// 어떤 Key가 눌렸는가? 등을 확인
	void Update(Input* mInput);

	// 자신을 포함한 server에 접속한 유저들을 그리기 위함
	void Render(IRenderer* pRenderer);

private:
	// 송신 thread와 수신 thread를 분리하여
	// 병렬적으로 송수신 처리
	static void SendPacketToServer(TCPNetwork* pTCPNetwork);
	static void RecvPacketFromServer(TCPNetwork* pTCPNetwork);
	static void ProcessPacket(ePacketHeader packetHeader, TCPNetwork* pTCPNetwork);

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
	void MakeThread();

	// client가 접속했을 때
	// 새롭게 계정을 생성한 client면 초기값으로 세팅된 데이터를 받을 것이고
	// 이미 계정이 있는 client면 DB에 있는 데이터를 받을 것임
	void LoadDataFromServer();

	// client가 server로부터 본인의 데이터를 받은 후
	// 기존에 server에 접속해 있던 유저들과 동기화하기 위한 패킷 전송
	void NotifyNewClientConnection();

private:
	void WaitForThreadToEnd();
	void CleanWSAData();

private:
	void WSAErrorHandling();

private:
	// server에 접속한 client에 변경이 생기면
	// thread에서 clientInfo 데이터를 참조해서 동기화
	std::unordered_map<SOCKET, Player*>& GetClientInfo();

private:
	// 서버와 client 목록 동기화
	std::unordered_map<SOCKET, Player*> mClientInfo;
	Player* mMyPlayerData;

private:
	char mServerIp[16];
	char mServerPort[6];

	WSADATA mWSAData;
	SOCKET mClientSocket;
	SOCKADDR_IN mServerAddress;

	std::thread* mSendThread;
	std::thread* mRecvThread;
};