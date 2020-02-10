#include "BSpline.h"



BSpline::BSpline(ID3D11Device *device)
{
	std::vector<GeometryGenerator::MeshData> meshData;
	GeometryGenerator generator;

	std::vector<std::vector<XMFLOAT3>> strandPoints;
	strandPoints.resize(1);
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

	HR(device->CreateShaderResourceView(mStructuredBuffer, &srvDesc, &mSRV));


	D3D11_BUFFER_DESC constantBuffer;
	ZeroMemory(&srvDesc, sizeof(constantBuffer));
	constantBuffer.ByteWidth = sizeof(HairConstantBuffer);
	constantBuffer.Usage = D3D11_USAGE_DYNAMIC;
	constantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBuffer.MiscFlags = 0;
	constantBuffer.StructureByteStride = 0;

	HR(device->CreateBuffer(&constantBuffer, NULL, &mConstantBuffer));


	D3D11_BUFFER_DESC cubicSplineDesc;
	ZeroMemory(&srvDesc, sizeof(cubicSplineDesc));
	cubicSplineDesc.ByteWidth = sizeof(CubicSpline);
	cubicSplineDesc.Usage = D3D11_USAGE_IMMUTABLE;
	cubicSplineDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cubicSplineDesc.CPUAccessFlags = 0;
	cubicSplineDesc.MiscFlags = 0;
	cubicSplineDesc.StructureByteStride = 0;

	mVertexCount = 120;

	CubicSpline cubicSpline;
	cubicSpline.vertexCount = (float)mVertexCount;

	D3D11_SUBRESOURCE_DATA cubSplineSubData;
	cubSplineSubData.pSysMem = &cubicSpline;

	HR(device->CreateBuffer(&cubicSplineDesc, &cubSplineSubData, &mCubicSplineCB));


	mVertexShader = new VertexShader(L"./Shader/vgpBSpline.hlsl", "HairVS", true);
	mVertexShader->prepare(device);

	mGeometryShader = new GeometryShader(L"./Shader/vgpBSpline.hlsl", "HairGS", true);
	mGeometryShader->prepare(device);

	mPixelShader = new PixelShader(L"./Shader/vgpBSpline.hlsl", "HairPS", true);
	mPixelShader->prepare(device);
}


BSpline::~BSpline()
{

}

void BSpline::Draw(float deltaTime, ID3D11DeviceContext *context)
{
	if (!mIsUpdated)
	{
		DebugBreak();
	}

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);


	mVertexShader->activate(context);
	mGeometryShader->activate(context);
	mPixelShader->activate(context);

	mVertexShader->activateInputLayout(context);
	context->VSSetShaderResources(0, 1, &mSRV);

	ID3D11Buffer **constBuffers;
	constBuffers = (ID3D11Buffer**)malloc(sizeof(ID3D11Buffer*) * 2);
	constBuffers[0] = mConstantBuffer;
	constBuffers[1] = mCubicSplineCB;

	context->VSSetConstantBuffers(0, 2, constBuffers);

	free(constBuffers);


	context->Draw(mVertexCount, 0);

	ResetUtils::ResetShaders(context);
	ResetUtils::ResetVertexShaderResources(context);
	ResetUtils::ResetAllConstantBuffers(context);


	mIsUpdated = false;
}

void BSpline::UpdateCamera(ID3D11DeviceContext *context, XMMATRIX view, XMMATRIX proj)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HR(context->Map(mConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	HairConstantBuffer *matrix = (HairConstantBuffer*)mappedResource.pData;
	matrix->view = XMMatrixTranspose(view);
	matrix->proj = XMMatrixTranspose(proj);
	matrix->world = XMMatrixTranspose(XMMatrixIdentity());

	context->Unmap(mConstantBuffer, 0);

	mIsUpdated = true;
}
