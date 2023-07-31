#define _WINSOCKAPI_
#include <Windows.h>
#include <WinSock2.h>

//#define NDEBUG
#include <cassert>
#include <WS2tcpip.h>
#include <thread>
#include <cstring>

#include "GameProcess.h"
#include "../Inc/IRenderer.h"

#pragma comment(lib, "ws2_32")

GameProcess::GameProcess()
	: mHwnd{}
	, mMsg{}
	, mClientWidth{}
	, mClientHeight{}
	, mRenderer{ nullptr }
	, mRendererModule{}
	, GetRenderer{ nullptr }
	, ReleaseRenderer{ nullptr }
	, mServerIp{}
	, mServerPort{}
	, mWSAData{}
	, mClientSocket{}
	, mServerAddress{}
	, mSendThread{ nullptr }
	, mRecvThread{ nullptr }
{
}

GameProcess::~GameProcess()
{
}

HRESULT GameProcess::Initialize(HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
		return E_FAIL;
	}

	// 명시적 링킹을 하면 내가 원하는 시점에 dll을 메모리에 올렸다 내렸다가 가능
	// exe파일이 실행되어 있는 상태에서 dll 교체도 가능
	mRendererModule = LoadLibrary(L"../Dll/3DRenderer.dll");
	assert(mRendererModule);

	// C style 이름으로 함수를 찾자
	GetRenderer = reinterpret_cast<GETRENDERER>(GetProcAddress(mRendererModule, "GetRenderer"));
	assert(GetRenderer);

	ReleaseRenderer = reinterpret_cast<RELEASERENDERER>(GetProcAddress(mRendererModule, "ReleaseRenderer"));
	assert(ReleaseRenderer);

	mRenderer = GetRenderer();
	mRenderer->Initialize(mHwnd, mClientWidth, mClientHeight);

	InitSocketCommunicate(lpCmdLine);

	return S_OK;
}

HRESULT GameProcess::Finalize()
{
	bool bSuccess = true;

	// 스레드가 완전히 종료되길 기다림
	WaitForThreadToEnd();
	CleanWSAData();

	// 스레드 정보(스레드 핸들 값, id 값)가 힙 메모리에 할당되어 있기 때문에
	// 메모리 해제
	delete mSendThread;
	delete mRecvThread;

	mRenderer->Finalize();
	ReleaseRenderer(mRenderer);

	bSuccess = FreeLibrary(mRendererModule);
	assert(bSuccess);

	return S_OK;
}

void GameProcess::Loop()
{
	while (true)
	{
		// GetMessage()는 큐에 메시지가 없으면 메시지가 들어올 때 까지 대기
		// PeekMessage()는 큐에 메시지가 없으면 0을 반환하면서 진행
		if (PeekMessage(&mMsg, NULL, 0, 0, PM_REMOVE))
		{
			if (mMsg.message == WM_QUIT) break;

			TranslateMessage(&mMsg);
			DispatchMessage(&mMsg);
		}
		else
		{
			Update();
			Render();
		}
	}
}

LRESULT GameProcess::WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HRESULT GameProcess::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	const wchar_t className[]{ L"tcp client" };

	WNDCLASS wc{};
	wc.style = WS_OVERLAPPED;
	wc.lpfnWndProc = WinProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = className;

	RegisterClass(&wc);

	mHwnd = CreateWindow(className, className, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	if (mHwnd == NULL)
	{
		return E_FAIL;
	}

	ShowWindow(mHwnd, nCmdShow);

	RECT rc;
	GetClientRect(mHwnd, &rc);
	mClientWidth = rc.right - rc.left;
	mClientHeight = rc.bottom - rc.top;

	return S_OK;
}

void GameProcess::Update()
{
}

void GameProcess::Render()
{
	// dll을 사용하는 입장에서 IRenderer interface만 알고 있고
	// 특정 함수를 호출하면 가상함수 테이블을 참조하여 dll에서 dx11로 구현된 함수가 호출됨
	mRenderer->BeginRender();

	mRenderer->Render();

	mRenderer->EndRender();
}

void GameProcess::InitSocketCommunicate(LPSTR lpCmdLine)
{
	ParsingServerInformation(lpCmdLine);

	InitWSAData();

	MakeClientSocket();

	FillServerInformation();

	ConnectServer();

	mSendThread = new std::thread{ SendPacketToServer, this };
	mRecvThread = new std::thread{ RecvPacketFromServer, this };
}

void GameProcess::InitWSAData()
{
	int wsaErrorCode = WSAStartup(MAKEWORD(2, 2), &mWSAData);
	if (wsaErrorCode != 0)
	{
		assert(wsaErrorCode == 0);
		exit(EXIT_FAILURE);
	}
}

void GameProcess::MakeClientSocket()
{
	mClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mClientSocket == INVALID_SOCKET)
	{
		WSAErrorHandling();
	}
}

void GameProcess::FillServerInformation()
{
	memset(&mServerAddress, 0x00, sizeof(mServerAddress));
	mServerAddress.sin_family = AF_INET;

	// ip주소는 inet_pton() 내부에서 네트워크 바이트 순서(Big endian)로 변경
	inet_pton(AF_INET, &mServerIp[0], &mServerAddress.sin_addr.S_un.S_addr);
	mServerAddress.sin_port = htons(atoi(mServerPort));
}

void GameProcess::ConnectServer()
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

void GameProcess::WaitForThreadToEnd()
{
	mSendThread->join();
	mRecvThread->join();
}

void GameProcess::CleanWSAData()
{
	if (WSACleanup() == SOCKET_ERROR)
	{
		WSAErrorHandling();
	}
}

void GameProcess::SendPacketToServer(GameProcess* pGameProcess)
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
		int sendByte = send(pGameProcess->mClientSocket, &sendData[currentByteToSend], totalByteToSend - currentByteToSend, 0);

		if (sendByte == SOCKET_ERROR)
		{
			pGameProcess->WSAErrorHandling();
		}

		currentByteToSend += sendByte;
	}

	// 더 이상 보낼 데이터가 없고
	// 수신할 데이터만 존재하기 때문에
	// 출력 스트림만 닫는다.
	if (shutdown(pGameProcess->mClientSocket, SD_SEND) == SOCKET_ERROR)
	{
		pGameProcess->WSAErrorHandling();
	}
}

void GameProcess::RecvPacketFromServer(GameProcess* pGameProcess)
{
	char cMessageLength{};
	int recvByte = recv(pGameProcess->mClientSocket, &cMessageLength, 1, 0);
	if (recvByte == SOCKET_ERROR)
	{
		pGameProcess->WSAErrorHandling();
	}

	int iMessageLength = static_cast<int>(cMessageLength);

	char sendData[30]{};
	int totalByteReceived = iMessageLength;
	int currentByteReceived = 0;
	while (currentByteReceived < totalByteReceived)
	{
		recvByte = recv(pGameProcess->mClientSocket, &sendData[currentByteReceived], totalByteReceived - currentByteReceived, 0);

		// 연결이 정상적으로 닫힌 경우
		if (recvByte == 0)
		{
			break;
		}
		else if (recvByte == SOCKET_ERROR)
		{
			pGameProcess->WSAErrorHandling();
		}

		currentByteReceived += recvByte;
	}

	sendData[currentByteReceived] = '\0';

	if (shutdown(pGameProcess->mClientSocket, SD_RECEIVE) == SOCKET_ERROR)
	{
		pGameProcess->WSAErrorHandling();
	}
}

void GameProcess::ParsingServerInformation(LPSTR lpCmdLine)
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

void GameProcess::SkipWhitespaceCharacters(const char* str)
{
	while (*str == ' ')
	{
		++str;
	}
}

void GameProcess::WSAErrorHandling()
{
	int wsaErrorCode = WSAGetLastError();
	assert(wsaErrorCode == 0);
	exit(EXIT_FAILURE);
}
