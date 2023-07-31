#include "../Inc/MainServer.h"

// 2023 07 25 이정모 home

// 접속하는 1명의 클라이언트를 1개의 스레드가 담당하는 방식의 서버를 만들 예정

int main(int argc, char* argv[])
{
	MainServer* pMainServer = new MainServer{};

	pMainServer->Initialize(argc, argv);

	pMainServer->AcceptClient();

	pMainServer->Finalize();

	delete pMainServer;

	return 0;
}