#pragma once

#include "RenderItem.h"
#include "ResetUtils.h"

class BSpline : public IDrawable, RenderItem
{
public:
	BSpline(ID3D11Device *device);
	~BSpline();

	void Draw(float deltaTime, ID3D11DeviceContext *context);

	void UpdateCamera(ID3D11DeviceContext *context, XMMATRIX view, XMMATRIX proj);

private:

	struct HairConstantBuffer
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
	};

	struct CubicSpline
	{
		float vertexCount;
		XMFLOAT3 padding;
	};

	struct Particle
	{
		XMFLOAT3 Position;
		XMFLOAT4 Color;
	};

	static const int MAX_PARTICLE_COUNT = 16;

	struct Strand
	{
		float ParticlesCount;
		Particle particles[MAX_PARTICLE_COUNT];
	};

	ID3D11Buffer *mStructuredBuffer;
	ID3D11UnorderedAccessView *mUAV;
	ID3D11ShaderResourceView *mSRV;
	ID3D11Buffer *mCubicSplineCB;
	HairConstantBuffer mConstantBufferData;
	UINT mVertexCount;

	bool mIsUpdated = false;
};

