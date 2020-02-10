#pragma once

#include "RenderItem.h"
#include "ResetUtils.h"
#include "IDrawable.h"

class Hair : public IDrawable, RenderItem
{
public:
	Hair(ID3D11Device *device);
	~Hair();

	void Draw(float deltaTime, ID3D11DeviceContext *context);

	void UpdateCamera(ID3D11DeviceContext *context, XMMATRIX view, XMMATRIX proj);

private:
	struct HairVertexData
	{
		XMFLOAT3 Position;
		XMFLOAT4 Color;
	};

	struct HairConstantBuffer
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
	};

	HairConstantBuffer mConstantBufferData;
	UINT mVertexCount;

	bool mIsUpdated = false;
};

