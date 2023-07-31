// <Windows.h>는 <winsock.h>를 include하고 있다.
// 그래서 <windows.h> 아래에 <winsock2.h>를 include하면
// <winsock.h> 정의한 것들이 <winsock2.h>에서도 똑같이 정의하고 있기 때문에 재정의 error가 발생한다.
// 전처리기 지시문 #define을 통해 _WINSOCKAPI_를 정의하면 <windows.h> 내부에서 <winsock.h>를 include하지 않는다.
// 이로 인해 <windows.h> 아래에서 <winsock2.h>를 include 하더라도 문제가 없다.
#define _WINSOCKAPI_
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