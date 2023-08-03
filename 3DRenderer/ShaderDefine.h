#pragma once

#include "../Inc/DX11Define.h"

// 2023 08 02 이정모 home

// shader를 제작하기 위해서 필요한 자료형 모음

struct ColorVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT4 Color;
};

struct ConstantBuffer
{
	DirectX::XMMATRIX World;
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Proj;
};