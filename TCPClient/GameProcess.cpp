#include <WinSock2.h>
#include <Windows.h>
//#define NDEBUG
#include <cassert>
#include <WS2tcpip.h>

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

	ConnectServer(lpCmdLine);

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

	return S_OK;
}

HRESULT GameProcess::Finalize()
{
	bool bSuccess = true;

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

void GameProcess::ConnectServer(LPSTR lpCmdLine)
{
	char* ptrCmdLine = lpCmdLine;
	char serverIp[16]{};
	int index = 0;

	while (*ptrCmdLine != ' ')
	{
		serverIp[index] = *ptrCmdLine;

		++index;
		++ptrCmdLine;
	}

	serverIp[index] = '\0';
	index = 0;

	// 공백 하나 건너 뛰기
	++ptrCmdLine;

	char serverPort[6]{};

	while (*ptrCmdLine != '\0')
	{
		serverPort[index] = *ptrCmdLine;

		++index;
		++ptrCmdLine;
	}

	serverPort[index] = '\0';

	WSADATA wsaData;
	int wsaErrorCode = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaErrorCode != 0)
	{
		assert(wsaErrorCode == 0);
		exit(EXIT_FAILURE);
	}

	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET)
	{
		WSAErrorHandling();
	}

	SOCKADDR_IN serverAddress;
	memset(&serverAddress, 0x00, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	// ip주소는 inet_pton() 내부에서 네트워크 바이트 순서(Big endian)로 변경
	inet_pton(AF_INET, &serverIp[0], &serverAddress.sin_addr.S_un.S_addr);
	serverAddress.sin_port = htons(atoi(serverPort));

	// 괄호 잘못 닫는 거 조심해야함
	// 아래에서 connect() 괄호 닫을 때 SOCKET_ERROR 밖에서 닫아서 sizeof(serverAddress) == SOCKET_ERROR가 0으로 계산되었고
	// 세번째 파라미터 값이 0으로 전달되어서 WSAEFAULT(10014) 오류 발생(구조체의 인수 길이가 sizeof(sockaddr)보다 작음)
	//if (connect(clientSocket, reinterpret_cast<SOCKADDR*>(&serverAddress), sizeof(serverAddress) == SOCKET_ERROR))

	// connect()를 통해 지정한 ip주소의 server computer에서 지정한 port에 해당하는 프로세스(애플리케이션)에게 연결 요청 시도
	// 이 때 운영체제는 이 client 프로세스의 포트 번호를 임의로 지정해주면서 네트워크로 패킷을 송신
	if (connect(clientSocket, reinterpret_cast<SOCKADDR*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR)
	{
		WSAErrorHandling();
	}

	char cRecvMessageLength{};

	// 서버와의 통신 규칙: 가장 첫 바이트에 수신할 문자열의 길이가 있음
	int currRecv = recv(clientSocket, &cRecvMessageLength, 1, 0);
	if (currRecv < 0)
	{
		WSAErrorHandling();
	}

	int iRecvMessageLength = static_cast<int>(cRecvMessageLength);
	char* recvMessage = new char[iRecvMessageLength + 1] {};

	int totalRecv = 0;
	while (totalRecv < iRecvMessageLength)
	{
		// 수신할 버퍼의 위치와 몇 바이트를 수신할지 잘 넣어주자
		currRecv = recv(clientSocket, recvMessage + totalRecv, iRecvMessageLength - totalRecv, 0);

		// closesocket()을 통해 연결을 종료하면 recv() 반환 값이 0
		if (currRecv == 0)
		{
			break;
		}
		else if (currRecv < 0)
		{
			WSAErrorHandling();
		}

		totalRecv += currRecv;
	}

	recvMessage[totalRecv] = '\0';

	if (closesocket(clientSocket) == SOCKET_ERROR)
	{
		WSAErrorHandling();
	}

	if (WSACleanup() == SOCKET_ERROR)
	{
		WSAErrorHandling();
	}
}

void GameProcess::WSAErrorHandling()
{
	int wsaErrorCode = WSAGetLastError();
	assert(wsaErrorCode == 0);
	exit(EXIT_FAILURE);
}
