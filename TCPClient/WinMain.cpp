#include <Windows.h>

#include "../Inc/GameProcess.h"

// 2023 07 23 이정모 home

// tcp client 진입점

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	GameProcess* pGameProcess = new GameProcess{};

	if (FAILED(pGameProcess->Initialize(hInstance, lpCmdLine, nCmdShow)))
	{
		return -1;
	}

	pGameProcess->Loop();

	if (FAILED(pGameProcess->Finalize()))
	{
		return -1;
	}

	delete pGameProcess;

	return 0;
}