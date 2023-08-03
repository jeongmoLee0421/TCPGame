#pragma once

#include <WinSock2.h>

#include "../Inc/Vector3.h"

// 2023 07 28 이정모 home

// client가 접속하면
// 해당 client를 3d 공간에 배치하기 위한 정보를 담은 class

// Player class도 1바이트 정렬을 반드시 해야함.
// 1바이트 정렬을 하지 않으면, 패딩이 들어간 상태로
// PacketPlayer에 들어가게 되는데
// PacketPlayer에서 1바이트 정렬을 했을지언정
// Player는 이미 패딩이 들어간 상태임
#pragma pack(push, 1)
class Player
{
public:
	Player();
	Player(SOCKET socket,
		float x, float y, float z);

	// player class에 포인터 변수가 없어서
	// 아직까지는 복사/이동 연산 사이에 차이는 없음.
	// 동적할당한 내부 멤버 변수가 생긴다면
	// 그것을 복사할 것인지, 아니면 소유권을 넘길 것인지에 대한 처리도 중요
	Player(const Player& ref);
	Player(Player&& ref) noexcept;

	Player& operator=(const Player& ref);
	Player& operator=(Player&& ref) noexcept;

	~Player();

public:
	char* Serialize(char* buf);
	char* Deserialize(char* buf);

public:
	SOCKET GetSocket();
	math::Vector3 GetPosition();

private:
	SOCKET mSocket;
	math::Vector3 mPosition;
};
#pragma pack(pop)