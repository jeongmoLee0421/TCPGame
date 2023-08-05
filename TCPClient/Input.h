#pragma once

#include <vector>
#include <utility>

#include "../Inc/EnumKeyInput.h"

// 2023 08 04 이정모 home

// 키 입력 상태를 저장하기 위한 class
// 키 입력 상태에 따라서 패킷을 보내야 하기 때문에
// (방향키를 누르면 해당 방향으로 이동하라는 패킷 등)
// 정밀한 입력 감지가 필요

class Input
{
public:
	Input();
	~Input();

	void Initialize();
	void Finalize();

public:
	void Update();

public:
	bool GetKeyUp(eKeyType keyType);
	bool GetKeyDown(eKeyType keyType);
	bool GetKey(eKeyType keyType);

private:
	// 키의 눌림, 뗌 여부를 확인해서
	// 키 상태 저장
	void CheckKeyState(int iKeyType);

private:
	// index = key type
	// pair.first = key state
	// pair.second = virtual key
	std::vector<std::pair<eKeyState, int>> mKeyState;
};