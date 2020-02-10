#include "HairSimulatedWithTractixSplines.h"



void HairSimulatedWithTractixSplines::InitializeSharedBuffers(ID3D11Device * device, ID3D11DeviceContext * context)
{
	std::vector<GeometryGenerator::MeshData> meshData;
	GeometryGenerator generator;

	std::vector<std::vector<XMFLOAT3>> strandPoints;
	strandPoints.resize(mStrandsCount);
	strandPoints[0].push_back(XMFLOAT3(0, 1.25, 0));
	strandPoints[0].push_back(XMFLOAT3(1, 0, 0));
	strandPoints[0].push_back(XMFLOAT3(0, -1.25, 0));
	strandPoints[0].push_back(XMFLOAT3(-1, -1.5, 0));
	strandPoints[0].push_back(XMFLOAT3(1, -2.75, 0));
	strandPoints[0].push_back(XMFLOAT3(-1, -4.5, 0));


	std::vector<Strand> strands;


	strands.resize(strandPoints.size());
	meshData.resize(strandPoints.size());
	for (int i = 0; i < strandPoints.size(); i++)
	{
		generator.CreateLineStrip(strandPoints[i], meshData[i]);

		strands[i].ParticlesCount = strandPoints[i].size();

		for (int k = 0; k < strands[i].ParticlesCount; k++)
		{
			strands[i].particles[k] = {
				meshData[i].Vertices[k].Position,
				(XMFLOAT4)Colors::Magenta
			};
		}
	}


	D3D11_BUFFER_DESC structuredBufferDesc;
	structuredBufferDesc.ByteWidth = sizeof(Strand) * strands.size();
	structuredBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	structuredBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	structuredBufferDesc.CPUAccessFlags = 0;
	structuredBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	structuredBufferDesc.StructureByteStride = sizeof(Strand);


	D3D11_SUBRESOURCE_DATA subData;
	subData.pSysMem = strands.data();

	HR(device->CreateBuffer(&structuredBufferDesc, &subData, &mStructuredBuffer));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = 0;
	uavDesc.Buffer.NumElements = strands.size();

	HR(device->CreateUnorderedAccessView(mStructuredBuffer, &uavDesc, &mUAV));


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.ElementOffset = 0;
	srvDesc.Buffer.ElementWidth = sizeof(Strand);
	srvDesc.Buffer.NumElements = strands.size();

	HR(device->CreateShaderResourceView(mStructuredBuffer, &srvDesc, &mStrandsSRV));



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

	HR(device->CreateBuffer(&splinesDesc, &splinesSubData, &splineRenderItem.mConstantBuffer));


	splineRenderItem.mVertexShader = new VertexShader(L"./Shader/vgpTractrixSplinesSplines.hlsl", "HairVS", true);
	splineRenderItem.mVertexShader->prepare(device);

	splineRenderItem.mGeometryShader = new GeometryShader(L"./Shader/vgpTractrixSplinesSplines.hlsl", "HairGS", true);
	splineRenderItem.mGeometryShader->prepare(device);

	splineRenderItem.mPixelShader = new PixelShader(L"./Shader/vgpTractrixSplinesSplines.hlsl", "HairPS", true);
	splineRenderItem.mPixelShader->prepare(device);
}

void HairSimulatedWithTractixSplines::InitializeControlPolygonRenderItem(ID3D11Device * device, ID3D11DeviceContext * context)
{
	controlPolygonRenderItem.mVertexShader = new VertexShader(L"./Shader/vgpTractrixSplinesControlPolygon.hlsl", "HairVS", true);
	controlPolygonRenderItem.mVertexShader->prepare(device);

	controlPolygonRenderItem.mGeometryShader = new GeometryShader(L"./Shader/vgpTractrixSplinesControlPolygon.hlsl", "HairGS", true);
	controlPolygonRenderItem.mGeometryShader->prepare(device);

	controlPolygonRenderItem.mPixelShader = new PixelShader(L"./Shader/vgpTractrixSplinesControlPolygon.hlsl", "HairPS", true);
	controlPolygonRenderItem.mPixelShader->prepare(device);
}



HairSimulatedWithTractixSplines::HairSimulatedWithTractixSplines(ID3D11Device *device, ID3D11DeviceContext *context)
{
	InitializeSharedBuffers(device, context);
	InitializeSplineRenderItem(device, context);
	InitializeControlPolygonRenderItem(device, context);
}


HairSimulatedWithTractixSplines::~HairSimulatedWithTractixSplines()
{

}

void HairSimulatedWithTractixSplines::Draw(float deltaTime, ID3D11DeviceContext *context)
{
	if (!mIsUpdated)
	{
		DebugBreak();
	}

	DrawSplines(deltaTime, context);
	DrawControlPolygon(deltaTime, context);

	mIsUpdated = false;
}


void HairSimulatedWithTractixSplines::DrawSplines(const float deltaTime, ID3D11DeviceContext * context)
{
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	splineRenderItem.mVertexShader->activate(context);
	splineRenderItem.mGeometryShader->activate(context);
	splineRenderItem.mPixelShader->activate(context);

	splineRenderItem.mVertexShader->activateInputLayout(context);
	context->VSSetShaderResources(0, 1, &mStrandsSRV);

	ID3D11Buffer **constBuffers;
	constBuffers = (ID3D11Buffer**)malloc(sizeof(ID3D11Buffer*) * 2);
	constBuffers[0] = mCameraCB;
	constBuffers[1] = splineRenderItem.mConstantBuffer;

	context->VSSetConstantBuffers(0, 2, constBuffers);

	free(constBuffers);

	context->Draw(mVertexCount, 0);

	ResetUtils::ResetShaders(context);
	ResetUtils::ResetVertexShaderResources(context);
	ResetUtils::ResetAllConstantBuffers(context);
}

void HairSimulatedWithTractixSplines::DrawControlPolygon(const float deltaTime, ID3D11DeviceContext * context)
{
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	controlPolygonRenderItem.mVertexShader->activate(context);
	controlPolygonRenderItem.mGeometryShader->activate(context);
	controlPolygonRenderItem.mPixelShader->activate(context);

	controlPolygonRenderItem.mVertexShader->activateInputLayout(context);
	context->VSSetShaderResources(0, 1, &mStrandsSRV);

	ID3D11Buffer **constBuffers;
	constBuffers = (ID3D11Buffer**)malloc(sizeof(ID3D11Buffer*));
	constBuffers[0] = mCameraCB;

	context->VSSetConstantBuffers(0, 1, constBuffers);

	free(constBuffers);

	context->Draw(mStrandsCount * MAX_PARTICLE_COUNT, 0);

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
