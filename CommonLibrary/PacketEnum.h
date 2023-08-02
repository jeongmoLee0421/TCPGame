#pragma once

// 2023 08 01 이정모 home

// client와 server가 주고 받는 패킷의 종류를 구분하는 enum

enum class ePacketHeader : unsigned short
{
	None,

	C2S_LoadData,
	S2C_LoadData,

	C2S_NewClientConnection,
	S2C_NewClientConnection,

	end,
};
