#include <cassert>

#include "DX11Renderer.h"
#include "../Inc/DX11Define.h"

DX11Renderer::DX11Renderer()
	: mD3DDevice{ nullptr }
	, mDeviceContext{ nullptr }
	, mSwapChain{ nullptr }
	, mDriverType{ D3D_DRIVER_TYPE_NULL }
	, mFeatureLevel{ D3D_FEATURE_LEVEL_11_0 }
	, mRenderTargetView{ nullptr }
	, mDepthStencil{ nullptr }
	, mDepthStencilView{ nullptr }
	, mViewPort{}
	, mClientWidth{}
	, mClientHeight{}
{
}

DX11Renderer::~DX11Renderer()
{
}

void DX11Renderer::Initialize(void* hWnd, long clientWidth, long clientHeight)
{
	HRESULT hr = S_OK;
	mClientWidth = clientWidth;
	mClientHeight = clientHeight;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = mClientWidth;
	sd.BufferDesc.Height = mClientHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = (HWND)hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; ++driverTypeIndex)
	{
		mDriverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain
		(
			NULL, mDriverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &mSwapChain, &mD3DDevice, &mFeatureLevel, &mDeviceContext
		);

		if (SUCCEEDED(hr))
		{
			break;
		}
	}
	assert(SUCCEEDED(hr));

	ID3D11Texture2D* pBackBuffer{ nullptr };
	hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
	assert(SUCCEEDED(hr));

	hr = mD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &mRenderTargetView);
	assert(SUCCEEDED(hr));

	// GetBuffer()를 통해서 pBackBuffer를 생성할 때 reference count 1증가
	// pBackBuffer가 가리키는 메모리에 대해서 mRenderTargetView를 세팅했기 때문에 reference count 1 증가
	// 앞으로는 mRenderTargetView로만 참조할 것이고 pBackBuffer는 사용하지 않기 때문에 reference count 감소 시킴
	pBackBuffer->Release();

	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = mClientWidth;
	descDepth.Height = mClientHeight;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	hr = mD3DDevice->CreateTexture2D(&descDepth, NULL, &mDepthStencil);
	assert(SUCCEEDED(hr));

	D3D11_DEPTH_STENCIL_VIEW_DESC descDsv;
	ZeroMemory(&descDsv, sizeof(descDsv));
	descDsv.Format = descDepth.Format;
	descDsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDsv.Texture2D.MipSlice = 0;

	hr = mD3DDevice->CreateDepthStencilView(mDepthStencil, &descDsv, &mDepthStencilView);
	assert(SUCCEEDED(hr));

	mDeviceContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);

	mViewPort.Width = static_cast<FLOAT>(mClientWidth);
	mViewPort.Height = static_cast<FLOAT>(mClientHeight);
	mViewPort.MinDepth = 0.f;
	mViewPort.MaxDepth = 1.f;
	mViewPort.TopLeftX = 0;
	mViewPort.TopLeftY = 0;

	mDeviceContext->RSSetViewports(1, &mViewPort);
}

void DX11Renderer::Finalize()
{
	ReleaseCom(mDepthStencilView);
	ReleaseCom(mDepthStencil);
	ReleaseCom(mRenderTargetView);
	ReleaseCom(mSwapChain);
	ReleaseCom(mDeviceContext);
	ReleaseCom(mD3DDevice);
}

void DX11Renderer::BeginRender()
{
	FLOAT clearColor[]{ 0.04f, 0.82f, 0.94f, 1.f };

	mDeviceContext->ClearRenderTargetView(mRenderTargetView, clearColor);
	mDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.f, NULL);
}

void DX11Renderer::Render()
{
}

void DX11Renderer::EndRender()
{
	mSwapChain->Present(0, 0);
}

RENDERER_API IRenderer* GetRenderer()
{
	return new DX11Renderer();
}

RENDERER_API void ReleaseRenderer(IRenderer* pRenderer)
{
	delete pRenderer;
}
