#include <Windows.h>

#include "GameProcess.h"

GameProcess::GameProcess()
	: mHwnd{}
	, mMsg{}
	, mClientWidth{}
	, mClientHeight{}
{
}

GameProcess::~GameProcess()
{
}

HRESULT GameProcess::Initialize(HINSTANCE hInstance, int nCmdShow)
{
	if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT GameProcess::Finalize()
{
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
}
