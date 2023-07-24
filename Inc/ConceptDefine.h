#pragma once

#include <guiddef.h>

// 2023 07 24 이정모 home

// template 매개변수에 제약을 걸기위해서 제약조건들을 묶어서 concept을 정의

// iunknown이 구현된 타입만 들어올 수 있는 concept
template <typename T>
concept iunknown = requires(T * ptr)
{
	// 실제 객체가 생성되는 것도 아니고
	// 실제 함수가 호출되는 것도 아님
	// 컴파일타임에 T에 대해서 이런 함수가 호출 가능한지 여부를 판단
	ptr->QueryInterface(IID{}, nullptr);
	ptr->AddRef();
	ptr->Release();
};