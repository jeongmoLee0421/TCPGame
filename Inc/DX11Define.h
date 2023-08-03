#pragma once

// 2023 07 24 이정모 home

// dx11 관련 define

#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

#include "../Inc/ConceptDefine.h"

#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

// com객체를 더 이상 사용하지 않을 때
// Release를 호출함으로써 reference count를 감소
// iunknown concept을 사용하여 템플릿 매개변수에 제약조건을 걸어줌
template <iunknown T>
void ReleaseCom(T* com)
{
	com->Release();
	com = nullptr;
}