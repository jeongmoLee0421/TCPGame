#include <WinSock2.h>

#include "Player.h"

Player::Player(SOCKET socket,
	float x, float y, float z)
	: mSocket(socket)
	, mX(x), mY(y), mZ(z)
{
}

Player::~Player()
{
}
