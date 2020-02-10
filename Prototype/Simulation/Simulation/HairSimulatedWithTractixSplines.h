#pragma once

#include "IDrawable.h"
#include "ResetUtils.h"
#include "RenderItem.h"

class HairSimulatedWithTractixSplines : public IDrawable
{
public:
	HairSimulatedWithTractixSplines(ID3D11Device *device, ID3D11DeviceContext *context);
	~HairSimulatedWithTractixSplines();

	void Draw(float deltaTime, ID3D11DeviceContext *context);

	void UpdateCamera(ID3D11DeviceContext *context, XMMATRIX view, XMMATRIX proj);

private:

	struct CameraConstantBuffer
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
	};

	struct SplineConstantBuffer
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
	UINT mVertexCount = 120;
	UINT mStrandsCount = 1;

	struct Strand
	{
		float ParticlesCount;
		Particle particles[MAX_PARTICLE_COUNT];
	};

	void InitializeSharedBuffers(ID3D11Device *device, ID3D11DeviceContext *context);
	void InitializeSplineRenderItem(ID3D11Device *device, ID3D11DeviceContext *context);
	void InitializeControlPolygonRenderItem(ID3D11Device *device, ID3D11DeviceContext *context);

	void DrawSplines(const float deltaTime, ID3D11DeviceContext *context);
	void DrawControlPolygon(const float deltaTime, ID3D11DeviceContext *context);

	RenderItem splineRenderItem;
	RenderItem controlPolygonRenderItem;

	ID3D11Buffer *mStructuredBuffer;
	ID3D11UnorderedAccessView *mUAV;
	ID3D11ShaderResourceView *mStrandsSRV;

	CameraConstantBuffer mConstantBufferData;
	ID3D11Buffer *mCameraCB;

	bool mIsUpdated = false;
};

