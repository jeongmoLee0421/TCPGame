#include <algorithm>
#include <windows.h>
#include <utility>

#include "Input.h"
#include "../Inc/EnumKeyInput.h"

Input::Input()
	: mKeyState{}
{
}

Input::~Input()
{
}

void Input::Initialize()
{
	mKeyState.reserve(static_cast<int>(eKeyType::End));

	// key type을 인덱스로 해서 key state를 참조하기 때문에
	// virtual key를 key type 순서에 맞게 넣으면
	// 반복문으로 CheckKeyState() 수행 가능
	mKeyState.emplace_back(eKeyState::None, VK_LEFT);
	mKeyState.emplace_back(eKeyState::None, VK_UP);
	mKeyState.emplace_back(eKeyState::None, VK_RIGHT);
	mKeyState.emplace_back(eKeyState::None, VK_DOWN);
}

void Input::Finalize()
{
	// 소멸자가 있는 자료형이 아니라서
	// 굳이 하지 않아도 되지만
	// Finalize()에서 마무리 작업을 한다는 것을 명시
	mKeyState.clear();
}

void Input::Update()
{
	int keyTypeStart = static_cast<int>(eKeyType::Left);
	int keyTypeEnd = static_cast<int>(eKeyType::End);

	for (int i = keyTypeStart; i < keyTypeEnd; ++i)
	{
		CheckKeyState(i);
	}
}

bool Input::GetKeyUp(eKeyType keyType)
{
	eKeyState keyState = mKeyState[static_cast<int>(keyType)].first;

	if (keyState == eKeyState::Away)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Input::GetKeyDown(eKeyType keyType)
{
	eKeyState keyState = mKeyState[static_cast<int>(keyType)].first;

	if (keyState == eKeyState::Press)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Input::GetKey(eKeyType keyType)
{
	eKeyState keyState = mKeyState[static_cast<int>(keyType)].first;

	if (keyState == eKeyState::Hold)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Input::CheckKeyState(int iKeyType)
{
	eKeyState prevKeyState = mKeyState[iKeyType].first;
	int virtualKey = mKeyState[iKeyType].second;

	// 이번 호출에서 키가 눌렸는지 여부(0x8000)가 중요한거지
	// 직전 호출과 이번 호출 사이에 키가 눌렸는지 여부(0x0001)는 중요하지 않음
	if (GetAsyncKeyState(virtualKey) & 0x8000)
	{
		// 뗀 상태에서 지금 막 누름
		if (prevKeyState == eKeyState::None
			|| prevKeyState == eKeyState::Away)
		{
			mKeyState[iKeyType].first = eKeyState::Press;
		}
		// 누르고 있는 상태에서 계속 누르고 있음
		else if (prevKeyState == eKeyState::Press
			|| prevKeyState == eKeyState::Hold)
		{
			mKeyState[iKeyType].first = eKeyState::Hold;
		}
	}
	else
	{
		// 뗸 상태에서 누르지 않음
		if (prevKeyState == eKeyState::None
			|| prevKeyState == eKeyState::Away)
		{
			mKeyState[iKeyType].first = eKeyState::None;
		}
		// 누르고 있는 상태에서 지금 막 뗌
		else if (prevKeyState == eKeyState::Press
			|| prevKeyState == eKeyState::Hold)
		{
			mKeyState[iKeyType].first = eKeyState::Away;
		}
	}
}
