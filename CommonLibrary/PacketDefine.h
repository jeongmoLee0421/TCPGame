#pragma once

#include "../Inc/PacketEnum.h"
#include "../Inc/Player.h"

// 2023 08 01 이정모 home

// client와 server가 주고 받는 패킷 내부

// 프로그래머 규칙대로(1바이트) 구조체를 정렬했다.
// 컴파일러 규칙대로 구조체에 패딩이 들어가게 되면,
// server를 A컴파일러로 컴파일하고
// client를 B컴파일러로 컴파일했을 때
// 컴파일러마다 패딩 규칙이 달라서
// 동일한 데이터에 대해 다른 해석이 가능할 수 있다.
// 이를 방지하기 위해서 패딩을 넣지 않고
// 멤버 변수의 크기를 그대로 사용도록 했다.
#pragma pack(push, 1)
struct PacketHeader
{
	// 모든 패킷에는 어떤 패킷인지에 대한 정보가 공통으로 들어감
	// 추상화를 통해 다형성을 구현하는 것이 목적이 아니라
	// 공통된 데이터가 있어서 부모 class로 추출한 것

	PacketHeader();
	PacketHeader(ePacketHeader packetHeader);
	~PacketHeader();

	// PacketHeader 직렬화의 경우 PacketHeader 이후에 오는
	// 다른 데이터들도 직렬화를 해야하기 때문에
	// 직렬화를 해야할 갱신된 위치를 반환하도록 했지만
	char* Serialize(char* buf);

	// 역직렬화의 경우
	// PacketHeader만 먼저 받아서
	// 분기 처리에 사용하고
	// 나머지 데이터는 새로운 buffer에 받기 때문에
	// 갱신된 위치를 반환할 필요가 없음
	void Deserialize(char* buf);

	ePacketHeader mPacketHeader;
};

struct PacketPlayer : public PacketHeader
{
	PacketPlayer();
	~PacketPlayer();

	// 새로 정의한 class, struct가 다양한 자료형들로 구성되어 있는데
	// 이를 char 배열로 직렬화시켜서
	// 데이터의 구조를 생각하지 않고 처리 가능
	void Serialize(char* buf);

	// 수신하는 쪽에서도
	// 어떤 class형으로 받아야 할지 생각하지 않고
	// 일단 char 배열로 받아서
	// 역직렬화를 통해 데이터를 세팅
	void Deserialize(char* buf);

	// 데이터를 송신하는 입장에서는 바이트 배열로만 보내면 되고
	// 데이터를 수신하는 입장에서는 바이트 배열로만 받으면 됨
	// 송수신 과정에서 어떤 데이터의 형태를 생각할 필요가 없어짐.
	// 바이트 배열로 규칙이 정해졌으니 송수신하기도 좋고 DB나 파일에 저장하기에도 용이하다.

	Player mPlayer;
};

#pragma pack(pop)