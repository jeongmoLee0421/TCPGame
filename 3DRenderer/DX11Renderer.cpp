//#define NDEBUG
#include <cassert>

#include "DX11Renderer.h"
#include "../Inc/DX11Define.h"
#include "../Inc/ShaderDefine.h"
#include "../Inc/Vector3.h"

#pragma comment(lib, "../Lib/CommonLibrary")

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
	, mVBCube{ nullptr }
	, mIBCube{ nullptr }
	, mCBCube{ nullptr }
	, mVSCube{ nullptr }
	, mILCube{ nullptr }
	, mPSCube{ nullptr }
	, mViewTM{}
	, mProjTM{}
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

	MakeViewTM();
	MakeProjTM();

	GenerateCubeData();
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
	FLOAT clearColor[]{ 0.f, 0.f, 0.f, 1.f };

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

void DX11Renderer::DrawCube(math::Vector3 position)
{
	ConstantBuffer cb;
	// 전치해서 넣지 않으면 shader에서 계산이 제대로 되지 않음
	// 전치하는 이유가
	// 전치하지 않으면 행백터 * 열벡터 계산을 하는데 요소마다 곱한 뒤 더하기를 수행하기 때문에 명령어가 많음
	// 그런데 전치하면 행벡터(XMVECTOR) * 행벡터(XMVECTOR) 계산이 가능하고
	// 이 때 내적 연산을 통해 4바이트 레지스터와 4바이트 레지스터가 병렬로 연산되어 명령어 개수가 매우 줄어든다.
	// 명령어 개수가 적기 때문에 당연히 연산 속도가 빠르다.
	cb.World = DirectX::XMMatrixTranspose(MakeWorldTM(position));
	cb.View = DirectX::XMMatrixTranspose(mViewTM);
	cb.Proj = DirectX::XMMatrixTranspose(mProjTM);

	// object마다 worldTM이 다르니까
	// DrawIndexed() 전에 상수 버퍼 update
	mDeviceContext->UpdateSubresource(mCBCube, 0, nullptr, &cb, 0, 0);

	UINT stride{ sizeof(ColorVertex) };
	UINT offset{ 0 };

	mDeviceContext->IASetVertexBuffers(0, 1, &mVBCube, &stride, &offset);
	mDeviceContext->IASetInputLayout(mILCube);
	mDeviceContext->IASetIndexBuffer(mIBCube, DXGI_FORMAT_R16_UINT, 0);
	mDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mDeviceContext->VSSetShader(mVSCube, nullptr, 0);
	mDeviceContext->VSSetConstantBuffers(0, 1, &mCBCube);
	mDeviceContext->PSSetShader(mPSCube, nullptr, 0);
	mDeviceContext->DrawIndexed(36, 0, 0);
}

DirectX::XMMATRIX DX11Renderer::MakeWorldTM(math::Vector3 position)
{
	// object마다 position이 다르기 때문에
	// worldTM의 dx, dy, dz를 달리 해줘야
	// 각자 자신의 위치에 맞게 그릴 수 있다.

	DirectX::XMFLOAT4X4 worldTM
	{
	1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	position.GetX(), position.GetY(), position.GetZ(), 1.f
	};

	return DirectX::XMLoadFloat4x4(&worldTM);
}

void DX11Renderer::MakeViewTM()
{
	DirectX::XMVECTOR eye = DirectX::XMVectorSet(2.f, 3.f, -8.f, 0.f);
	DirectX::XMVECTOR at = DirectX::XMVectorSet(0.f, 0.f, 0.f, 0.f);
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);

	mViewTM = DirectX::XMMatrixLookAtLH(eye, at, up);
}

void DX11Renderer::MakeProjTM()
{
	mProjTM = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XM_PIDIV2,
		static_cast<float>(mClientWidth) / static_cast<float>(mClientHeight),
		0.01f,
		1000.f);
}

void DX11Renderer::GenerateCubeData()
{
	HRESULT hr = S_OK;

	ColorVertex vertices[] =
	{
		// front
		{DirectX::XMFLOAT3{-1.f, 1.f, -1.f}, DirectX::XMFLOAT4{1.f, 1.f, 1.f, 1.f}},
		{DirectX::XMFLOAT3{1.f, 1.f, -1.f}, DirectX::XMFLOAT4{1.f, 1.f, 1.f, 1.f}},
		{DirectX::XMFLOAT3{1.f, -1.f, -1.f}, DirectX::XMFLOAT4{1.f, 1.f, 1.f, 1.f}},
		{DirectX::XMFLOAT3{-1.f, -1.f, -1.f}, DirectX::XMFLOAT4{1.f, 1.f, 1.f, 1.f}},

		// back
		{DirectX::XMFLOAT3{1.f, 1.f, 1.f}, DirectX::XMFLOAT4{1.f, 1.f, 1.f, 1.f}},
		{DirectX::XMFLOAT3{-1.f, 1.f, 1.f}, DirectX::XMFLOAT4{1.f, 1.f, 1.f, 1.f}},
		{DirectX::XMFLOAT3{-1.f, -1.f, 1.f}, DirectX::XMFLOAT4{1.f, 1.f, 1.f, 1.f}},
		{DirectX::XMFLOAT3{1.f, -1.f, 1.f}, DirectX::XMFLOAT4{1.f, 1.f, 1.f, 1.f}},
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ColorVertex) * 8;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.pSysMem = &vertices[0];

	hr = mD3DDevice->CreateBuffer(&bd, &initData, &mVBCube);
	if (FAILED(hr))
	{
		assert(nullptr && "CreateBuffer() error");
		exit(EXIT_FAILURE);
	}

	// DX는 삼각형을 clock wise로 정의
	WORD indices[] =
	{
		// front
		0,1,2,
		0,2,3,

		// back
		4,5,6,
		4,6,7,

		// left
		5,0,3,
		5,3,6,

		// right
		1,4,7,
		1,7,2,

		// up
		5,4,1,
		5,1,0,

		// down
		3,2,7,
		3,7,6,
	};

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 36;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	initData.pSysMem = &indices[0];

	hr = mD3DDevice->CreateBuffer(&bd, &initData, &mIBCube);
	if (FAILED(hr))
	{
		assert(nullptr && "CreateBuffer() error");
		exit(EXIT_FAILURE);
	}

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;

	hr = mD3DDevice->CreateBuffer(&bd, NULL, &mCBCube);
	if (FAILED(hr))
	{
		assert(nullptr && "CreateBuffer() error");
		exit(EXIT_FAILURE);
	}

	ID3DBlob* pBlob{ nullptr };
	hr = D3DReadFileToBlob(L"../Shader/Color_VS.cso", &pBlob);
	if (FAILED(hr))
	{
		assert(nullptr && "D3DReadFileToBlob() error");
		exit(EXIT_FAILURE);
	}

	hr = mD3DDevice->CreateVertexShader(
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		nullptr,
		&mVSCube);
	if (FAILED(hr))
	{
		pBlob->Release();
		assert(nullptr && "CreateVertexShader() error");
		exit(EXIT_FAILURE);
	}

	// gpu가 셰이더 실행할 때 처음에 정점 데이터 받아서 변환을 수행하는데 정점 데이터의 구조을 알려주기 위함
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	UINT numElements = ARRAYSIZE(layout);

	hr = mD3DDevice->CreateInputLayout(layout, numElements, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &mILCube);

	// pBlob이 가리키고 있던 .cso 메모리를 기반으로 vertex shader를 만들었다.
	// 앞으로는 vertex shader를 통해서 메모리에 load된 셰이더를 수행할 것이다.
	// 그리고 pBlob이 가리키는 .cso 메모리는 이제 참조할 필요가 없기 때문에
	// Release()를 호출해서 reference count를 감소시켜준다(ref cnt가 0이 되면 메모리에서 내려감).
	pBlob->Release();

	if (FAILED(hr))
	{
		assert(nullptr && "CreateInputLayout() error");
		exit(EXIT_FAILURE);
	}

	hr = D3DReadFileToBlob(L"../Shader/Color_PS.cso", &pBlob);
	if (FAILED(hr))
	{
		assert(nullptr && "D3DReadFileToBlob() error");
		exit(EXIT_FAILURE);
	}

	hr = mD3DDevice->CreatePixelShader(
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		nullptr,
		&mPSCube);
	pBlob->Release();

	if (FAILED(hr))
	{
		assert(nullptr && "CreatePixelShader() error");
		exit(EXIT_FAILURE);
	}
}

RENDERER_API IRenderer* GetRenderer()
{
	return new DX11Renderer();
}

RENDERER_API void ReleaseRenderer(IRenderer* pRenderer)
{
	delete pRenderer;
}
