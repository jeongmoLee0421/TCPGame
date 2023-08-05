//#define NDEBUG
#include <cassert>

#define _WINSOCKAPI_
#include <Windows.h>

#include "GameProcess.h"
#include "../Inc/IRenderer.h"
#include "../Inc/TCPNetwork.h"
#include "../Inc/input.h"

GameProcess::GameProcess()
	: mHwnd{}
	, mMsg{}
	, mClientWidth{}
	, mClientHeight{}
	, mRenderer{ nullptr }
	, mRendererModule{}
	, GetRenderer{ nullptr }
	, ReleaseRenderer{ nullptr }
	, mTCPNetwork{ nullptr }
	, mInput{ nullptr }
{
}

GameProcess::~GameProcess()
{
	delete mTCPNetwork;
	delete mInput;
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

	mInput = new Input();
	mInput->Initialize();

	mTCPNetwork = new TCPNetwork();
	mTCPNetwork->Initialize(lpCmdLine);

	return S_OK;
}

HRESULT GameProcess::Finalize()
{
	bool bSuccess = true;

	mRenderer->Finalize();
	ReleaseRenderer(mRenderer);

	bSuccess = FreeLibrary(mRendererModule);
	assert(bSuccess);

	mInput->Finalize();

	// 소켓 통신에 사용되는 thread가 종료될 때까지 기다림
	mTCPNetwork->Finalize();

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
	// input class update()를 통해
	// key state가 결정되면
	mInput->Update();

	// tcp network class에서
	// 결정된 key state를 확인하여
	// server에 패킷 전송
	mTCPNetwork->Update(mInput);
}

void GameProcess::Render()
{
	// dll을 사용하는 입장에서 IRenderer interface만 알고 있고
	// 특정 함수를 호출하면 가상함수 테이블을 참조하여 dll에서 dx11로 구현된 함수가 호출됨
	mRenderer->BeginRender();

	mRenderer->Render();

	mTCPNetwork->Render(mRenderer);

	mRenderer->EndRender();
}