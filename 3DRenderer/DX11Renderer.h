#pragma once

#ifdef MY3DRENDERER_EXPORTS
#define RENDERER_API __declspec(dllexport)
#else
#define RENDERER_API __declspec(import)
#endif

#include "../Inc/IRenderer.h"
#include "../Inc/DX11Define.h"
#include "../Inc/Vector3.h"

// 2023 07 24 이정모 home

// dx11로 구성된 renderer

class DX11Renderer : public IRenderer
{
public:
	DX11Renderer();
	~DX11Renderer() override;

	void Initialize(void* hWnd, long clientWidth, long clientHeight) override;
	void Finalize() override;

public:
	void BeginRender() override;
	void Render() override;
	void EndRender() override;

public:
	// 특정 위치에 오직 cube만을 그린다.
	void DrawCube(math::Vector3 position) override;

private:
	// worldTM은 object마다 다르고
	DirectX::XMMATRIX MakeWorldTM(math::Vector3 position);

	// view와 proj는 일단 고정
	void MakeViewTM();
	void MakeProjTM();

private:
	void GenerateCubeData();

private:
	ID3D11Device* mD3DDevice;
	ID3D11DeviceContext* mDeviceContext;
	IDXGISwapChain* mSwapChain;

	D3D_DRIVER_TYPE mDriverType;
	D3D_FEATURE_LEVEL mFeatureLevel;

private:
	ID3D11RenderTargetView* mRenderTargetView;
	ID3D11Texture2D* mDepthStencil;
	ID3D11DepthStencilView* mDepthStencilView;
	D3D11_VIEWPORT mViewPort;

private:
	DirectX::XMMATRIX mViewTM;
	DirectX::XMMATRIX mProjTM;

private:
	ID3D11Buffer* mVBCube;
	ID3D11Buffer* mIBCube;
	ID3D11Buffer* mCBCube;
	ID3D11VertexShader* mVSCube;
	ID3D11InputLayout* mILCube;
	ID3D11PixelShader* mPSCube;

private:
	long mClientWidth;
	long mClientHeight;
};

// extern "C"를 쓰는 이유
// C++에는 함수 오버로딩이란 개념이 있는데 함수 이름이 같더라도 매개변수 타입이나 개수가 다르면 이를 허용해주는 것을 말한다.
// 이름이 같기 때문에 컴파일러는 이를 구분하기 위해서 컴파일러 자체적으로 정의한 규칙에 의해 이름을 바꾸는 네임 맹글링을 수행하게 된다.
// DLL 프로젝트에서 네임맹글링을 허용하여 함수를 export했다고 해보자
// 외부에서 DLL을 명시적으로 링킹하기 위해 GetProcAddress()를 호출하면서 링킹하기 위한 함수의 이름을 C style로 넘긴다면
// DLL에서 실제로 그 함수가 있음에도 불구하고 함수의 이름이 달라서 찾지 못한다.
// 하지만 extern "C"를 사용함으로써 네임 맹글링을 수행하지 않고 순수하게 C style 이름으로 함수를 컴파일한다면 컴파일러가 달라도 찾을 수 있다.
// C style로 export하고 C style로 찾기 때문이다.
extern "C" RENDERER_API IRenderer * GetRenderer();
extern "C" RENDERER_API void ReleaseRenderer(IRenderer* pRenderer);