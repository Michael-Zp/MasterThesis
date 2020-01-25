#pragma once

#include "IRenderableItem.h"
#include "ResetUtils.h"
#include "OneDFlexObjSimulation.h"

class HairSimulatedWithOneDFlexObj : public IRenderableItem
{
public:
	HairSimulatedWithOneDFlexObj(ID3D11Device *device, ID3D11DeviceContext *context);
	~HairSimulatedWithOneDFlexObj();

	void Draw(const float deltaTime, ID3D11DeviceContext *context);

	void UpdateCamera(ID3D11DeviceContext *context, XMMATRIX view, XMMATRIX proj);

private:

	struct HairSimulatedConstantBuffer
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
	};

	OneDFlexObjSimulation *mSimulation;
	HairSimulatedConstantBuffer mConstantBufferData;
	UINT mVertexCount;

	bool mIsUpdated = false;
};

