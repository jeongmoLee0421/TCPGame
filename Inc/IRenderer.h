#pragma once

#include "../Inc/Vector3.h"

// 2023 07 24 이정모 home

// renderer interface
// 하부가 dx11, dx12, opengl 등 어떤 그래픽 api로 구성되어있더라도 대응하는 interface

class IRenderer
{
public:
	IRenderer() {}
	virtual ~IRenderer() = 0 {}

	virtual void Initialize(void* hWnd, long clientWidth, long clientHeight) = 0;
	virtual void Finalize() = 0;

public:
	virtual void BeginRender() = 0;
	virtual void Render() = 0;
	virtual void EndRender() = 0;

public:
	virtual void DrawCube(math::Vector3 position) = 0;
};