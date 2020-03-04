#pragma once

#include <DirectXMath.h>
#include <vector>
#include <d3d11.h>
#include "ComputeShader.h"

using namespace DirectX;

class TractrixSplineSimulation
{
public:
	struct Particle
	{
		XMFLOAT3 Position;
		XMFLOAT4 Color;
	};

	static const int MAX_PARTICLE_COUNT = 16;
	static const int MAX_KNOT_SIZE = MAX_PARTICLE_COUNT + 4;
	UINT mStrandsCount = 1;


	struct Strand
	{
		int ParticlesCount;
		int StrandIdx;
		XMFLOAT3 DesiredHeadPosition;
		Particle Particles[MAX_PARTICLE_COUNT];
		float Knot[MAX_KNOT_SIZE];
	};

	TractrixSplineSimulation(ID3D11Device *device, ID3D11DeviceContext *context);
	~TractrixSplineSimulation();

	void Simulate(const float deltaTime, ID3D11DeviceContext *context);

	ID3D11ShaderResourceView** GetSRVPtr() { return &mSRV; };
	int GetParticlesCount() { return mStrandsCount * MAX_PARTICLE_COUNT; }

private:

	enum ConstBufSlots {
		TIME_CONST_BUF = 0,
	};

	float ElapsedTimeInSimulation = 0;

	struct TimeConstBuf
	{
		float DeltaTime;
		float TotalTime;
		XMFLOAT2 PADDING;
	};

	ComputeShader *mComputeShader;
	ID3D11Buffer *mTimeConstBuf;
	ID3D11Buffer *mStructuredBuffer;
	ID3D11UnorderedAccessView *mUAV;
	ID3D11ShaderResourceView *mSRV;
};

