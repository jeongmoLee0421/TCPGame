//#define NDEBUG

#include <cstdio>
#include <cassert>
#include <stdlib.h>
#include <cstring>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

// 2023 07 25 이정모 home

// 접속하는 1명의 클라이언트를 1개의 스레드가 담당하는 방식의 서버를 만들 예정

void WSAErrorHandling(const char* errorFunctionName)
{
	printf("%s() Error\n", errorFunctionName);

	printf("WSAErrorName: ");

	int wsaErrorCode = WSAGetLastError();

	switch (wsaErrorCode)
	{
	case WSASYSNOTREADY:
		printf("WSASYSNOTREADY");
		break;

	case WSAVERNOTSUPPORTED:
		printf("WSAVERNOTSUPPORTED");
		break;

	case WSAEINPROGRESS:
		printf("WSAEINPROGRESS");
		break;

	case WSAEPROCLIM:
		printf("WSAEPROCLIM");
		break;

	case WSAEFAULT:
		printf("WSAEFAULT");
		break;

	case WSANOTINITIALISED:
		printf("WSANOTINITIALISED");
		break;

	case WSAENETDOWN:
		printf("WSAENETDOWN");
		break;

	case WSAEAFNOSUPPORT:
		printf("WSAEAFNOSUPPORT");
		break;

	case WSAEMFILE:
		printf("WSAEMFILE");
		break;

	case WSAEINVAL:
		printf("WSAEINVAL");
		break;

	case WSAEINVALIDPROVIDER:
		printf("WSAEINVALIDPROVIDER");
		break;

	case WSAEINVALIDPROCTABLE:
		printf("WSAEINVALIDPROCTABLE");
		break;

	case WSAENOBUFS:
		printf("WSAENOBUFS");
		break;

	case WSAEPROTONOSUPPORT:
		printf("WSAEPROTONOSUPPORT");
		break;

	case WSAEPROTOTYPE:
		printf("WSAEPROTOTYPE");
		break;

	case WSAEPROVIDERFAILEDINIT:
		printf("WSAEPROVIDERFAILEDINIT");
		break;

	case WSAESOCKTNOSUPPORT:
		printf("WSAESOCKTNOSUPPORT");
		break;

	case WSAEACCES:
		printf("WSAEACCES");
		break;

	case WSAEADDRINUSE:
		printf("WSAEADDRINUSE");
		break;

	case WSAEADDRNOTAVAIL:
		printf("WSAEADDRNOTAVAIL");
		break;

	case WSAENOTSOCK:
		printf("WSAENOTSOCK");
		break;

	case WSAEISCONN:
		printf("WSAEISCONN");
		break;

	case WSAEOPNOTSUPP:
		printf("WSAEOPNOTSUPP");
		break;

	case WSAECONNRESET:
		printf("WSAECONNRESET");
		break;

	case WSAEINTR:
		printf("WSAEINTR");
		break;

	case WSAEWOULDBLOCK:
		printf("WSAEWOULDBLOCK");
		break;

	case WSAENETRESET:
		printf("WSAENETRESET");
		break;

	case WSAENOTCONN:
		printf("WSAENOTCONN");
		break;

	case WSAESHUTDOWN:
		printf("WSAESHUTDOWN");
		break;

	case WSAEMSGSIZE:
		printf("WSAEMSGSIZE");
		break;

	case WSAEHOSTUNREACH:
		printf("WSAEHOSTUNREACH");
		break;

	case WSAECONNABORTED:
		printf("WSAECONNABORTED");
		break;

	case WSAETIMEDOUT:
		printf("WSAETIMEDOUT");
		break;

	default:
		printf("Do not fine error name");
		break;
	}

	printf("\nWSAErrorCode: %d\n", wsaErrorCode);

	assert(wsaErrorCode == 0);

	exit(EXIT_FAILURE);
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("usage: <%s> <port>\n", argv[0]);
		assert(argc == 2 && "usage: <filename.exe> <port>");
		exit(EXIT_FAILURE);
	}

	WSADATA wsaData;
	//int wsaErrorCode = WSAStartup(MAKEWORD(0, 0), &wsaData); // error test
	int wsaErrorCode = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsaErrorCode != 0)
	{
		printf("WSAStartup() Error\n");
		assert(wsaErrorCode == 0);
		exit(EXIT_FAILURE);
	}

	// ip version 4, TCP 소켓을 만들자
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET)
	{
		WSAErrorHandling("socket");
	}

	// 서버 소켓을 만들기 위해서 주소, 포트를 지정
	SOCKADDR_IN serverAddress;
	memset(&serverAddress, 0x00, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	// sin_family를 제외한 나머지 데이터는 네트워크 바이트 순서(big-endian)로 지정
	// 네트워크 바이트 순서를 big-endian으로 정했기 때문에
	// 내가 받은 데이터가 big-endian인지 little-endian인지 검사할 필요가 없음
	// host(나의 byte order)가 big-endian이면 바이트 순서에 변경이 없을 것이고
	// little endian이면 바이트 순서를 뒤집을 것임
	serverAddress.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(atoi(argv[1]));

	// bind()는 소켓에 주소와 포트번호를 할당하는 것임
	// 운영체제에 소켓을 등록한다고 볼 수 있는데
	// 이러한 주소(PC)와 포트 번호(애플리케이션)를 목적지로 하는 데이터가 전송되면
	// 이 프로세스(애플리케이션)에 데이터를 넘겨달라고 운영체제에게 등록하는 것
	if (bind(serverSocket, reinterpret_cast<SOCKADDR*>(&serverAddress), sizeof(serverAddress)) == SOCKET_ERROR)
	{
		WSAErrorHandling("bind");
	}

	// 문득 든 생각은 여러 client가 접속한 상태라면 과연 어느 client socket에 데이터를 넘겨야 하는지?인데
	// socket을 만들면 고유한 unsigned __int64형 정수 값이 할당되는데
	// 아마도 이것으로 구분하지 않을까? 라는 추측

	if (listen(serverSocket, 5) == SOCKET_ERROR)
	{
		WSAErrorHandling("listen");
	}

	SOCKADDR_IN clientAddress;
	int clientAddressSize = sizeof(clientAddress);

	// client로부터 연결 요청이 들어오면 수락하면서 해당 클라이언트와 통신하는 소켓을 새롭게 생성
	SOCKET clientSocket = accept(serverSocket, reinterpret_cast<SOCKADDR*>(&clientAddress), &clientAddressSize);
	if (clientSocket == INVALID_SOCKET)
	{
		WSAErrorHandling("accept");
	}

	char message[30]{ "hello client\n" };
	int messageLength = static_cast<int>(strlen(message));
	int totalSendByte = 0;

	// tcp 통신이기 때문에 내가 전송하고자 하는 바이트를 전부 전송할때까지 send()를 반복 호출
	while (totalSendByte < messageLength)
	{
		// send()는 전송할 데이터를 어느 위치에서 몇 바이트를 보낼지에 대한 정보를 잘 넣어줘야 함
		int currentSendByte = send(clientSocket, message + totalSendByte, messageLength - totalSendByte, 0);
		if (currentSendByte == SOCKET_ERROR)
		{
			WSAErrorHandling("send");
		}

		totalSendByte += currentSendByte;
	}

	if (closesocket(clientSocket) == SOCKET_ERROR)
	{
		WSAErrorHandling("closesocket");
	}
	if (closesocket(serverSocket) == SOCKET_ERROR)
	{
		WSAErrorHandling("closesocket");
	}

	if (WSACleanup() == SOCKET_ERROR)
	{
		WSAErrorHandling("WSACleanup");
	}

	return 0;
}