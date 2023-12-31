﻿#include <WinSock2.h>
#include <cstring>

#include "Player.h"

Player::Player()
	: mSocket{}
	, mPosition{}
{
}

Player::Player(SOCKET socket,
	float x, float y, float z)
	: mSocket{ socket }
	, mPosition{ x,y,z }
{
}

Player::Player(const Player& ref)
	: mSocket{ ref.mSocket }
	, mPosition{ ref.mPosition }
{
}

Player::Player(Player&& ref) noexcept
	: mSocket{ ref.mSocket }
	, mPosition{ ref.mPosition }
{
	// Player class 멤버 변수로
	// 동적 할당한 데이터에 대한 포인터 변수가 있었다면
	// 해당 데이터에 대한 소유권을 이전함과 동시에
	// 더 이상 참조하지 않겠다는 nullptr 대입이 필수
}

Player& Player::operator=(const Player& ref)
{
	mSocket = ref.mSocket;
	mPosition = ref.mPosition;

	return *this;
}

Player& Player::operator=(Player&& ref) noexcept
{
	mSocket = ref.mSocket;
	mPosition = ref.mPosition;

	return *this;
}

Player::~Player()
{
}

char* Player::Serialize(char* buf)
{
	// 데이터를 바이트 단위로 copy하고
	// 데이터 크기(sizeof)만큼 복사할 위치(buf)를 누적해서 이동

	memcpy(buf, &mSocket, sizeof(mSocket));
	buf += sizeof(mSocket);

	memcpy(buf, &mPosition, sizeof(mPosition));
	buf += sizeof(mPosition);

	return buf;
}

char* Player::Deserialize(char* buf)
{
	memcpy(&mSocket, buf, sizeof(mSocket));
	buf += sizeof(mSocket);

	memcpy(&mPosition, buf, sizeof(mPosition));
	buf += sizeof(mPosition);

	return buf;
}

SOCKET Player::GetSocket()
{
	return mSocket;
}

math::Vector3 Player::GetPosition()
{
	return mPosition;
}
