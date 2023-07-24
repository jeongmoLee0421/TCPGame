#pragma once

#include <Windows.h>

// 2023 07 24 이정모 home

// 윈도우 생성, 게임 루프

class GameProcess
{
public:
	GameProcess();
	~GameProcess();

	HRESULT Initialize(HINSTANCE hInstance, int nCmdShow);
	HRESULT Finalize();

public:
	void Loop();
	static LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Render();

private:
	HWND mHwnd;
	MSG mMsg;

	LONG mClientWidth;
	LONG mClientHeight;
};