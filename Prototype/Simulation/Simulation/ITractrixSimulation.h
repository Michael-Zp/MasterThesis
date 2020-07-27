#pragma once

#include <d3d11.h>

class ITractrixSimulation
{
public:
	virtual void Simulate(const float deltaTime, ID3D11DeviceContext *context) = 0;

	ID3D11ShaderResourceView** GetSRVPtr() { return &mSRV; };
	int GetParticlesCount() { return mStrandsCount * MAX_PARTICLE_COUNT; };

protected:

	static const int MAX_PARTICLE_COUNT = 16;
	static const int MAX_KNOT_SIZE = MAX_PARTICLE_COUNT * 2;
	UINT mStrandsCount = 1;

	ID3D11ShaderResourceView *mSRV;
};