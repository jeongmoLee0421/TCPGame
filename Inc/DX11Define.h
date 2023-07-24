#pragma once

// 2023 07 24 ������ home

// dx11 ���� define

#include <d3d11.h>
#include "../Inc/ConceptDefine.h"

#pragma comment(lib, "d3d11")

// com��ü�� �� �̻� ������� ���� ��
// Release�� ȣ�������ν� reference count�� ����
// iunknown concept�� ����Ͽ� ���ø� �Ű������� ���������� �ɾ���
template <iunknown T>
void ReleaseCom(T* com)
{
	com->Release();
	com = nullptr;
}