#pragma once

#include "RenderItem.h"
#include "ResetUtils.h"
#include "IDrawable.h"

class HairBase : public IDrawable, RenderItem
{
public:
	HairBase(ID3D11Device *device);
	~HairBase();

	void Draw(float deltaTime, ID3D11DeviceContext *context);

	void UpdateCamera(ID3D11DeviceContext *context, XMMATRIX view, XMMATRIX proj);

private:
	struct HairBaseVertexData
	{
		XMFLOAT3 Position;
		XMFLOAT4 Color;
	};

	struct HairBaseConstantBuffer
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
	};

	HairBaseConstantBuffer mConstantBufferData;
	UINT mIndexCount;

	bool mIsUpdated = false;
};

