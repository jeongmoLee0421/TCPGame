#pragma once

// 2023 08 04 이정모 home

// key 입력에 사용되는 enum 모음

enum class eKeyState : unsigned char
{
	Press, // 이전에 누르지 않았고 지금 막 누름
	Hold, // 이전에 눌렀고 지금도 누름
	Away, // 이전에 눌렀고 지금은 막 뗌
	None, // 이전에 누르지 않았고 지금도 누르지 않음
};

enum class eKeyType : unsigned short
{
	Left,
	Up,
	Right,
	Down,

	End,
};