#pragma once

#include "IRenderableItem.h"
#include "ResetUtils.h"
#include "MassSpringSimulation.h"

class HairSimulatedWithMassSpring : public IRenderableItem
{
public:
	HairSimulatedWithMassSpring(ID3D11Device *device, ID3D11DeviceContext *context);
	~HairSimulatedWithMassSpring();

	void Draw(const float deltaTime, ID3D11DeviceContext *context);

	void UpdateCamera(ID3D11DeviceContext *context, XMMATRIX view, XMMATRIX proj);

private:

	struct HairSimulatedConstantBuffer
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
	};

	MassSpringSimulation *mSimulation;
	HairSimulatedConstantBuffer mConstantBufferData;
	UINT mVertexCount;

	bool mIsUpdated = false;
};

