#include "My_EI_PSO.h"

#include <exception>

EI_PSO::EI_PSO(EI_LayoutManager & vertexManager, EI_LayoutManager & pixelManager, HINSTANCE hInstance, DeviceAndContext &dac) : D3DApp(hInstance, dac)
{
	if (!D3DApp::Init())
	{
		throw EXCEPTION_ILLEGAL_INSTRUCTION;
	}


	InitializeConstantBuffers(dac, vertexManager, pixelManager);
}

int EI_PSO::FetchMessages()
{
	MSG msg = { 0 };

	// If there are Window messages then process them.
	if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	// Otherwise, do animation/game stuff.
	else
	{
		mTimer.Tick();

		//if (!mAppPaused)
		if(true)
		{
			CalculateFrameStats();
		}
		else
		{
			Sleep(100);
		}
	}

	return (int)msg.wParam;
}


void EI_PSO::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void EI_PSO::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

template<typename T>
static T Clamp(const T& x, const T& low, const T& high)
{
	return x < low ? low : (x > high ? high : x);
}

void EI_PSO::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;

		// Restrict the angle mPhi.
		mPhi = Clamp(mPhi, 0.1f, DirectX::XM_PI - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.005f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f*static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = Clamp(mRadius, 3.0f, 15.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void EI_PSO::UpdateConstantBuffers(DeviceAndContext &dac)
{
	float x = mRadius * sinf(mPhi)*cosf(mTheta);
	float y = mRadius * cosf(mPhi);
	float z = mRadius * sinf(mPhi)*sinf(mTheta);

	XMMATRIX mView = XMMatrixLookAtLH(XMVectorSet(x, y, z, 1.0f), XMVectorZero(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	XMMATRIX mProj = XMMatrixPerspectiveFovLH(0.25f * DirectX::XM_PI, 800.f / 600.f, 1.0f, 1000.0f);



	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HR(dac.context->context->Map(mVertexConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	MatrixBufferType *matrix = (MatrixBufferType*)mappedResource.pData;
	matrix->eye = XMFLOAT3(x, y, z);
	matrix->winSize = XMFLOAT2(800, 600);
	matrix->view = mView;
	matrix->proj = mProj;

	dac.context->context->Unmap(mVertexConstantBuffer, 0);


	HR(dac.context->context->Map(mPixelConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	matrix = (MatrixBufferType*)mappedResource.pData;
	matrix->eye = XMFLOAT3(x, y, z);
	matrix->winSize = XMFLOAT2(800, 600);
	matrix->view = XMMatrixTranspose(mView);
	matrix->proj = XMMatrixTranspose(mProj);

	dac.context->context->Unmap(mPixelConstantBuffer, 0);
}


bool EI_PSO::InitializeConstantBuffers(DeviceAndContext &dac, EI_LayoutManager &vertexManager, EI_LayoutManager &pixelManager)
{
	mVertexConstantBindSlot = vertexManager.GetBindSlotByName("constants");

	mPixelConstantBindSlot = pixelManager.GetBindSlotByName("constants");


	D3D11_BUFFER_DESC constantBuffer;
	constantBuffer.ByteWidth = sizeof(MatrixBufferType);
	constantBuffer.Usage = D3D11_USAGE_DYNAMIC;
	constantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBuffer.MiscFlags = 0;
	constantBuffer.StructureByteStride = 0;

	dac.device->device->CreateBuffer(&constantBuffer, NULL, &mVertexConstantBuffer);


	ZeroMemory(&constantBuffer, sizeof(D3D11_BUFFER_DESC));
	constantBuffer.ByteWidth = sizeof(MatrixBufferType);
	constantBuffer.Usage = D3D11_USAGE_DYNAMIC;
	constantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBuffer.MiscFlags = 0;
	constantBuffer.StructureByteStride = 0;

	dac.device->device->CreateBuffer(&constantBuffer, NULL, &mPixelConstantBuffer);

	UpdateConstantBuffers(dac);

	return true;
}