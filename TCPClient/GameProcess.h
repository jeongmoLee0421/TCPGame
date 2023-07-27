#pragma once

#include <Windows.h>

class IRenderer;

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
	void ConnectServer(LPSTR lpCmdLine);
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
	HWND mHwnd;
	MSG mMsg;

	LONG mClientWidth;
	LONG mClientHeight;
};