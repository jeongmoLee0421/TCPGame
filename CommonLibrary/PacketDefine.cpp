#include "PacketDefine.h"
#include "../Inc/Player.h"

PacketHeader::PacketHeader()
	: mPacketHeader{}
{
}

PacketHeader::PacketHeader(ePacketHeader packetHeader)
	: mPacketHeader{ packetHeader }
{
}

PacketHeader::~PacketHeader()
{
}

char* PacketHeader::Serialize(char* buf)
{
	memcpy(buf, &mPacketHeader, sizeof(mPacketHeader));
	buf += sizeof(mPacketHeader);

	return buf;
}

void PacketHeader::Deserialize(char* buf)
{
	memcpy(&mPacketHeader, &buf[0], sizeof(mPacketHeader));
	buf += sizeof(mPacketHeader);
}

PacketPlayer::PacketPlayer()
	: mPlayer{}
{
}

PacketPlayer::~PacketPlayer()
{
}

void PacketPlayer::Serialize(char* buf)
{
	buf = PacketHeader::Serialize(buf);
	mPlayer.Serialize(buf);
}

void PacketPlayer::Deserialize(char* buf)
{
	mPlayer.Deserialize(buf);
}