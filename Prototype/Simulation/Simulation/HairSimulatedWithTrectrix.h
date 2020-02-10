#pragma once

#include "RenderItem.h"
#include "ResetUtils.h"
#include "TractrixSimulation.h"
#include "IDrawable.h"

class HairSimulatedWithTrectrix : public IDrawable, RenderItem
{
public:
	HairSimulatedWithTrectrix(ID3D11Device *device, ID3D11DeviceContext *context);
	~HairSimulatedWithTrectrix();

	void Draw(const float deltaTime, ID3D11DeviceContext *context);

	void UpdateCamera(ID3D11DeviceContext *context, XMMATRIX view, XMMATRIX proj);

private:

	struct HairSimulatedConstantBuffer
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
	};

	TractrixSimulation *mSimulation;
	HairSimulatedConstantBuffer mConstantBufferData;
	UINT mVertexCount;

	bool mIsUpdated = false;
};

