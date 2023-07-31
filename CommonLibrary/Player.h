#pragma once

#include <WinSock2.h>

// 2023 07 28 이정모 home

// client가 접속하면
// 해당 client를 3d 공간에 배치하기 위한 정보를 담은 class

class Player
{
public:
	Player(SOCKET socket,
		float x, float y, float z);
	~Player();

private:
	SOCKET mSocket;
	float mX, mY, mZ;
};