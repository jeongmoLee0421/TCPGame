#pragma once

#define _WINSOCKAPI_
#include <Windows.h>
#include <WinSock2.h>

class IRenderer;

namespace std
{
	class thread;
}

// 2023 07 24 이정모 home

// 윈도우 생성, 게임 루프

class GameProcess
{
public:
	GameProcess();
	~GameProcess();

	HRESULT Initialize(HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow);
	HRESULT Finalize();

public:
	void Loop();
	static LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Render();

private:
	void InitSocketCommunicate(LPSTR lpCmdLine);
	void InitWSAData();
	void MakeClientSocket();
	void FillServerInformation();
	void ConnectServer();
	void WaitForThreadToEnd();
	void CleanWSAData();

private:
	// 송신 thread와 수신 thread를 분리하여
	// 병렬적으로 송수신 처리
	static void SendPacketToServer(GameProcess* pGameProcess);
	static void RecvPacketFromServer(GameProcess* pGameProcess);

private:
	void ParsingServerInformation(LPSTR lpCmdLine);
	void SkipWhitespaceCharacters(const char* str);

private:
	void WSAErrorHandling();

public:
	using GETRENDERER = IRenderer * (*)();
	using RELEASERENDERER = void(*)(IRenderer*);

private:
	IRenderer* mRenderer; // 하부가 어떤 그래픽 api로 구현되었는지는 알 수 없으나 interface만 가지고 사용 가능
	HMODULE mRendererModule;

	GETRENDERER GetRenderer;
	RELEASERENDERER ReleaseRenderer;

private:
	char mServerIp[16];
	char mServerPort[6];

	WSADATA mWSAData;
	SOCKET mClientSocket;
	SOCKADDR_IN mServerAddress;

	std::thread* mSendThread;
	std::thread* mRecvThread;

private:
	HWND mHwnd;
	MSG mMsg;

	LONG mClientWidth;
	LONG mClientHeight;
};