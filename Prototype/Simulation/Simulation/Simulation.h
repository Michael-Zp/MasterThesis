#pragma once

#include <DirectXMath.h>
#include <vector>
#include <d3d11.h>
#include "ComputeShader.h"

using namespace DirectX;

class Simulation
{
public:

	struct Particle
	{
		XMFLOAT3 Position;
		float Parameter;
	};

	Simulation(std::vector<XMFLOAT3> positions, ID3D11Device *device, ID3D11DeviceContext *context);
	~Simulation();

	void Simulate(ID3D11DeviceContext *context);

	ID3D11ShaderResourceView** GetSRVPtr() { return &mSRV; };

private:
	ComputeShader *mComputeShader;
	ID3D11Buffer *mStructuredBuffer;
	ID3D11UnorderedAccessView *mUAV;
	ID3D11ShaderResourceView *mSRV;
};

