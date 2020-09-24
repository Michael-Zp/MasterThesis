#include "HairSimulatedWithTractixSplines.h"



void HairSimulatedWithTractixSplines::InitializeSharedBuffers(ID3D11Device * device, ID3D11DeviceContext * context)
{
	D3D11_BUFFER_DESC cameraCBDesc;
	ZeroMemory(&cameraCBDesc, sizeof(cameraCBDesc));
	cameraCBDesc.ByteWidth = sizeof(CameraConstantBuffer);
	cameraCBDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraCBDesc.MiscFlags = 0;
	cameraCBDesc.StructureByteStride = 0;

	HR(device->CreateBuffer(&cameraCBDesc, NULL, &mCameraCB));
}

void HairSimulatedWithTractixSplines::InitializeSplineRenderItem(ID3D11Device * device, ID3D11DeviceContext * context)
{
	D3D11_BUFFER_DESC splinesDesc;
	ZeroMemory(&splinesDesc, sizeof(splinesDesc));
	splinesDesc.ByteWidth = sizeof(SplineConstantBuffer);
	splinesDesc.Usage = D3D11_USAGE_IMMUTABLE;
	splinesDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	splinesDesc.CPUAccessFlags = 0;
	splinesDesc.MiscFlags = 0;
	splinesDesc.StructureByteStride = 0;

	SplineConstantBuffer cubicSpline;
	cubicSpline.vertexCount = (float)mVertexCount;

	D3D11_SUBRESOURCE_DATA splinesSubData;
	splinesSubData.pSysMem = &cubicSpline;

	HR(device->CreateBuffer(&splinesDesc, &splinesSubData, &mSplineRenderItem.mConstantBuffer));


	mSplineRenderItem.mVertexShader = new VertexShader(L"./Shader/vgpTractrixSplinesSplines.hlsl", "HairVS", true);
	mSplineRenderItem.mVertexShader->prepare(device);

	mSplineRenderItem.mGeometryShader = new GeometryShader(L"./Shader/vgpTractrixSplinesSplines.hlsl", "HairGS", true);
	mSplineRenderItem.mGeometryShader->prepare(device);

	mSplineRenderItem.mPixelShader = new PixelShader(L"./Shader/vgpTractrixSplinesSplines.hlsl", "HairPS", true);
	mSplineRenderItem.mPixelShader->prepare(device);
}

void HairSimulatedWithTractixSplines::InitializeControlPolygonRenderItem(ID3D11Device * device, ID3D11DeviceContext * context)
{
	mControlPolygonRenderItem.mVertexShader = new VertexShader(L"./Shader/vgpTractrixSplinesControlPolygon.hlsl", "HairVS", true);
	mControlPolygonRenderItem.mVertexShader->prepare(device);

	mControlPolygonRenderItem.mGeometryShader = new GeometryShader(L"./Shader/vgpTractrixSplinesControlPolygon.hlsl", "HairGS", true);
	mControlPolygonRenderItem.mGeometryShader->prepare(device);

	mControlPolygonRenderItem.mPixelShader = new PixelShader(L"./Shader/vgpTractrixSplinesControlPolygon.hlsl", "HairPS", true);
	mControlPolygonRenderItem.mPixelShader->prepare(device);
}



//HairSimulatedWithTractixSplines::HairSimulatedWithTractixSplines(ID3D11Device *device, ID3D11DeviceContext *context, TractrixSimulation *simulation)
HairSimulatedWithTractixSplines::HairSimulatedWithTractixSplines(ID3D11Device *device, ID3D11DeviceContext *context, ThirdSimulation *simulation)
{
	mSimulation = simulation;
	InitializeSharedBuffers(device, context);
	InitializeSplineRenderItem(device, context);
	InitializeControlPolygonRenderItem(device, context);
}


HairSimulatedWithTractixSplines::~HairSimulatedWithTractixSplines()
{
	free(mSimulation);
}

void HairSimulatedWithTractixSplines::Draw(float deltaTime, ID3D11DeviceContext *context)
{
	if (!mIsUpdated)
	{
		DebugBreak();
	}

	mSimulation->Simulate(deltaTime, context);
	DrawSplines(deltaTime, context);
	//DrawControlPolygon(deltaTime, context);

	mIsUpdated = false;
}


void HairSimulatedWithTractixSplines::DrawSplines(const float deltaTime, ID3D11DeviceContext * context)
{
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	mSplineRenderItem.mVertexShader->activate(context);
	mSplineRenderItem.mGeometryShader->activate(context);
	mSplineRenderItem.mPixelShader->activate(context);

	mSplineRenderItem.mVertexShader->activateInputLayout(context);
	context->VSSetShaderResources(0, 1, mSimulation->GetSRVPtr());

	ID3D11Buffer **constBuffers;
	constBuffers = (ID3D11Buffer**)malloc(sizeof(ID3D11Buffer*) * 2);
	constBuffers[0] = mCameraCB;
	constBuffers[1] = mSplineRenderItem.mConstantBuffer;

	context->VSSetConstantBuffers(0, 2, constBuffers);

	free(constBuffers);

	context->Draw(mVertexCount * mSimulation->GetStrandsCount() * 2, 0); //*2 because of LINELIST, LINESTRIP would require seperate draw calls or alpha blending to achieve seperate lines

	ResetUtils::ResetShaders(context);
	ResetUtils::ResetVertexShaderResources(context);
	ResetUtils::ResetAllConstantBuffers(context);
}

void HairSimulatedWithTractixSplines::DrawControlPolygon(const float deltaTime, ID3D11DeviceContext * context)
{
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	mControlPolygonRenderItem.mVertexShader->activate(context);
	mControlPolygonRenderItem.mGeometryShader->activate(context);
	mControlPolygonRenderItem.mPixelShader->activate(context);

	mControlPolygonRenderItem.mVertexShader->activateInputLayout(context);
	context->VSSetShaderResources(0, 1, mSimulation->GetSRVPtr());

	ID3D11Buffer **constBuffers;
	constBuffers = (ID3D11Buffer**)malloc(sizeof(ID3D11Buffer*));
	constBuffers[0] = mCameraCB;

	context->VSSetConstantBuffers(0, 1, constBuffers);

	free(constBuffers);

	context->Draw(mSimulation->GetParticlesCount() * 2, 0); //*2 because of LINELIST, LINESTRIP would require seperate draw calls or alpha blending to achieve seperate lines

	ResetUtils::ResetShaders(context);
	ResetUtils::ResetVertexShaderResources(context);
	ResetUtils::ResetAllConstantBuffers(context);
}

void HairSimulatedWithTractixSplines::UpdateCamera(ID3D11DeviceContext *context, XMMATRIX view, XMMATRIX proj)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HR(context->Map(mCameraCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	CameraConstantBuffer *matrix = (CameraConstantBuffer*)mappedResource.pData;
	matrix->view = XMMatrixTranspose(view);
	matrix->proj = XMMatrixTranspose(proj);
	matrix->world = XMMatrixTranspose(XMMatrixIdentity());

	context->Unmap(mCameraCB, 0);
	   	 
	mIsUpdated = true;
}
