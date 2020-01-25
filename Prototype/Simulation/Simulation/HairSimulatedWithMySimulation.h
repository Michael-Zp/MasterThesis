#pragma once

#include "IRenderableItem.h"
#include "ResetUtils.h"
#include "MySimulation.h"

class HairSimulatedWithMySimulation : public IRenderableItem
{
public:
	HairSimulatedWithMySimulation(ID3D11Device *device, ID3D11DeviceContext *context);
	~HairSimulatedWithMySimulation();

	void Draw(const float deltaTime, ID3D11DeviceContext *context);

	void UpdateCamera(ID3D11DeviceContext *context, XMMATRIX view, XMMATRIX proj);

private:

	struct HairSimulatedConstantBuffer
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
	};

	MySimulation *mSimulation;
	HairSimulatedConstantBuffer mConstantBufferData;
	UINT mVertexCount;

	bool mIsUpdated = false;
};

