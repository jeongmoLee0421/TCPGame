#pragma once

// 2023 08 01 이정모 home

// client와 server가 주고 받는 패킷의 종류를 구분하는 enum

enum class ePacketHeader : unsigned short
{
	None,

	// client가 처음 접속해서
	// server로 부터 데이터를 받아옴
	C2S_LoadData,
	S2C_LoadData,

	// 새로운 client가 접속하면
	// 기존 유저들과 동기화
	C2S_NewClientConnection,
	S2C_NewClientConnection,

	end,
};
