#pragma once

#include <DirectXMath.h>
#include <vector>
#include <d3d11.h>
#include "ComputeShader.h"
#include "ITractrixSimulation.h"

using namespace DirectX;

class TractrixSplineSimulation : public ITractrixSimulation
{
private:

	struct TimeConstBuf
	{
		float DeltaTime;
		float TotalTime;
		XMFLOAT2 PADDING;
	};


public:

	struct PropertiesConstBuf
	{
		float DoTractrix;
		float DoKnotInsertion;
		float DoKnotRemoval;
		float StopIfKnotChanged;
		float UsePhysics;
		XMFLOAT3 Padding;

		PropertiesConstBuf(bool doTractrix, bool doKnotInsertion, bool doKnotRemoval, bool stopIfKnotChanged, bool usePhysics) :
			DoTractrix(doTractrix ? 1.0f : 0.0f), DoKnotInsertion(doKnotInsertion ? 1.0f : 0.0f), DoKnotRemoval(doKnotRemoval ? 1.0f : 0.0f), 
			StopIfKnotChanged(stopIfKnotChanged ? 1.0f : 0.0f), UsePhysics(usePhysics ? 1.0f : 0.0f) { };
	};

	struct Particle
	{
		XMFLOAT3 Position;
		XMFLOAT4 Color;
	};

	struct Strand
	{
		int ParticlesCount;
		int StrandIdx;
		XMFLOAT3 HairRoot;
		XMFLOAT3 DesiredHeadMovement;
		XMFLOAT3 OriginalHeadPosition;
		XMFLOAT3 HeadVelocity;
		float HeadMass;
		Particle Particles[MAX_PARTICLE_COUNT];
		float Knot[MAX_KNOT_SIZE];
		float KnotValues[MAX_KNOT_SIZE];
		float MaxKnotValue;
		float KnotHasChangedOnce;
	};

	enum class Configuration
	{
		Z4Points,
		ZReverse4Points,
		Z4PointsStretch,
		I4Points,
		Z5Points,
		ZReverse5Points,
		Z5PointsStretch
	};

	TractrixSplineSimulation(ID3D11Device *device, ID3D11DeviceContext *context, PropertiesConstBuf props, XMFLOAT4 strandColor, Configuration config);
	~TractrixSplineSimulation();

	void Simulate(const float deltaTime, ID3D11DeviceContext *context);

	ID3D11ShaderResourceView** GetSRVPtr() { return &mSRV; };
	int GetParticlesCount() { return mStrandsCount * MAX_PARTICLE_COUNT; }


private:

	enum class ConstBufSlots {
		TIME_CONST_BUF = 0,
		PROPERTEIS_CONST_BUF = 1,
	};

	float ElapsedTimeInSimulation = 0;


	ComputeShader *mComputeShader;
	ID3D11Buffer *mTimeConstBuf;
	ID3D11Buffer *mPropertiesConstBuf;
	ID3D11Buffer *mStructuredBuffer;
	ID3D11UnorderedAccessView *mUAV;
};

