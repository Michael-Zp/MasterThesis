#pragma once

#include "My_EI_LayoutManager.h"
#include "My_EI_BindLayout.h"
#include "DeviceAndContext.h"
#include <DirectXMath.h>
#include "d3dApp.h"

using namespace DirectX;


class EI_PSO : public D3DApp
{
public:
	EI_PSO(EI_LayoutManager &vertexManager, EI_LayoutManager &pixelManager, HINSTANCE hInstance, DeviceAndContext &dac);

	BindSlot mVertexConstantBindSlot;
	ID3D11Buffer *mVertexConstantBuffer;

	BindSlot mPixelConstantBindSlot;
	ID3D11Buffer *mPixelConstantBuffer;

	int FetchMessages();
	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

	void UpdateConstantBuffers(DeviceAndContext &dac);

private:

	POINT mLastMousePos;
	float mRadius = 15.0f;
	float mPhi = DirectX::XM_PIDIV2;
	float mTheta = 3 * DirectX::XM_PIDIV2;
	
	struct MatrixBufferType
	{
		XMFLOAT3 eye;
		XMFLOAT2 winSize;
		XMMATRIX view;
		XMMATRIX proj;
	};

	bool InitializeConstantBuffers(DeviceAndContext &dac, EI_LayoutManager &vertexManager, EI_LayoutManager &pixelManager);

};