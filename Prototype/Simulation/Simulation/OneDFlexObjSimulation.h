#pragma once

#include <DirectXMath.h>
#include <vector>
#include <d3d11.h>
#include "ComputeShader.h"

using namespace DirectX;

class OneDFlexObjSimulation
{
public:

	struct Particle
	{
		XMFLOAT3 Position;
		XMFLOAT3 Color;
	};

	const static int MAXIMUM_NUMBER_OF_PARTICLES = 16;

	struct Strand
	{
		int NumberOfParticles;
		int StrandIdx;
		float SimulatedTime;
		XMFLOAT3 DesiredHeadPosition;
		Particle Particles[MAXIMUM_NUMBER_OF_PARTICLES];
	};

	OneDFlexObjSimulation(std::vector<std::vector<XMFLOAT3>*> positions, ID3D11Device *device, ID3D11DeviceContext *context);
	~OneDFlexObjSimulation();

	void Simulate(const float deltaTime, ID3D11DeviceContext *context);

	ID3D11ShaderResourceView** GetSRVPtr() { return &mSRV; };

private:

	enum ConstBufSlots {
		TIME_CONST_BUF = 0,
		PROPERTIES_CONST_BUF = 1
	};

	float ElapsedTimeInSimulation = 0;

	struct TimeConstBuf
	{
		float DeltaTime;
		float TotalTime;
		XMFLOAT2 PADDING;
	};

	struct PropertiesConstBuf
	{
		float drag;
		float stiffness;
		XMFLOAT2 PADDING;
	};

	ComputeShader *mComputeShader;
	ID3D11Buffer *mTimeConstBuf;
	ID3D11Buffer *mPropertiesConstBuf;
	ID3D11Buffer *mStructuredBuffer;
	ID3D11UnorderedAccessView *mUAV;
	ID3D11ShaderResourceView *mSRV;
};
