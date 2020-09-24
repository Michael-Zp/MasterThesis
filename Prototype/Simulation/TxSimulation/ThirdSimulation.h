#pragma once

#include <DirectXMath.h>
#include <vector>
#include <d3d11.h>
#include "ComputeShader.h"

using namespace DirectX;
class ThirdSimulation
{
private:

	struct SimulationConstBuf
	{
		float DeltaTime;
		float TotalTime;
		float StrandsCount;
		float PADDING;
		XMUINT3 DispatchSize;
		float PADDING2;
	};

	static const int MAX_PARTICLE_COUNT = 32;
	static const int MAX_KNOT_SIZE = MAX_PARTICLE_COUNT * 2;
	UINT mStrandsCount = 1;
	int mNumberOfSegments = 31;
	XMUINT3 mDispatchSize = XMUINT3(128, 128, 4);

public:

	struct PropertiesConstBuf
	{
		float DoTractrix;
		float DoKnotInsertion;
		float DoKnotRemoval;
		float StopIfKnotChanged;
		XMFLOAT4 Padding;

		PropertiesConstBuf(bool doTractrix, bool doKnotInsertion, bool doKnotRemoval, bool stopIfKnotChanged) :
			DoTractrix(doTractrix ? 1.0f : 0.0f), DoKnotInsertion(doKnotInsertion ? 1.0f : 0.0f), DoKnotRemoval(doKnotRemoval ? 1.0f : 0.0f),
			StopIfKnotChanged(stopIfKnotChanged ? 1.0f : 0.0f) { };
	};

	struct PhysicalParticle
	{
		XMFLOAT3 Position;
		XMFLOAT3 Velocity;
	};


	struct PhysicalStrand 
	{
		PhysicalParticle PhysicalParticles[MAX_PARTICLE_COUNT];
	};


	struct Particle
	{
		XMFLOAT3 Position;
	};


	struct Strand
	{
		int ParticlesCount;
		int StrandIdx;
		XMFLOAT3 HairRoot;
		XMFLOAT3 OriginalHeadPosition;
		XMFLOAT3 DesiredSegmentDirections[MAX_PARTICLE_COUNT - 1];
		Particle Particles[MAX_PARTICLE_COUNT];
		XMFLOAT4 Color;
		float Knot[MAX_KNOT_SIZE];
		float KnotValues[MAX_KNOT_SIZE];
		float MaxKnotValue;
		float KnotHasChangedOnce;
	};

	enum class Configuration
	{
		Z4Points,
		Z12Points,
		Z31Points,
		LoadHair,
		Random
	};

	ThirdSimulation(ID3D11Device *device, ID3D11DeviceContext *context, PropertiesConstBuf props, XMFLOAT4 strandColor, Configuration config);
	~ThirdSimulation();

	void Simulate(const float deltaTime, ID3D11DeviceContext *context);

	ID3D11ShaderResourceView** GetSRVPtr() { return &mSRV; };
	int GetParticlesCount() { return mStrandsCount * MAX_PARTICLE_COUNT; };
	int GetStrandsCount() { return mStrandsCount; };

private:

	enum class ConstBufSlots {
		TIME_CONST_BUF = 0,
		PROPERTEIS_CONST_BUF = 1,
	};

	float ElapsedTimeInSimulation = 0;


	ID3D11ShaderResourceView *mSRV;
	ID3D11ShaderResourceView *mPhysicalSRV;

	ComputeShader *mComputeShader;
	ID3D11Buffer *mTimeConstBuf;
	ID3D11Buffer *mPropertiesConstBuf;
	ID3D11Buffer *mStructuredBuffer;
	ID3D11Buffer *mPhysicalStructuredBuffer;
	ID3D11UnorderedAccessView *mUAV;
	ID3D11UnorderedAccessView *mPhysicalUAV;
};

