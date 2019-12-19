#pragma once

#include "IRenderableItem.h"
#include "ResetUtils.h"
#include "Simulation.h"

class HairSimulated : public IRenderableItem
{
public:
	HairSimulated(ID3D11Device *device, ID3D11DeviceContext *context);
	~HairSimulated();

	void Draw(ID3D11DeviceContext *context);

	void UpdateCamera(ID3D11DeviceContext *context, XMMATRIX view, XMMATRIX proj);

private:

	struct HairSimulatedConstantBuffer
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
	};

	Simulation *mSimulation;
	HairSimulatedConstantBuffer mConstantBufferData;
	UINT mVertexCount;

	bool mIsUpdated = false;
};

