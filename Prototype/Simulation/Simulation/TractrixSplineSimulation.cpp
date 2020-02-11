#include "TractrixSplineSimulation.h"

#include "ResetUtils.h"
#include "GeometryGenerator.h"


TractrixSplineSimulation::TractrixSplineSimulation(ID3D11Device *device, ID3D11DeviceContext *context)
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


	std::vector<TractrixSplineSimulation::Strand> strands;


	strands.resize(strandPoints.size());
	meshData.resize(strandPoints.size());
	for (int i = 0; i < strandPoints.size(); i++)
	{
		generator.CreateLineStrip(strandPoints[i], meshData[i]);

		strands[i].ParticlesCount = strandPoints[i].size();
		strands[i].StrandIdx = i;
		strands[i].DesiredHeadPosition = XMFLOAT3(0, 0, 0);


		for (int k = 0; k < strands[i].ParticlesCount; k++)
		{
			strands[i].Particles[k] = {
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



	D3D11_BUFFER_DESC timeConstBuf;
	timeConstBuf.ByteWidth = sizeof(TimeConstBuf);
	timeConstBuf.Usage = D3D11_USAGE_DYNAMIC;
	timeConstBuf.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	timeConstBuf.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	timeConstBuf.MiscFlags = 0;
	timeConstBuf.StructureByteStride = 0;

	HR(device->CreateBuffer(&timeConstBuf, NULL, &mTimeConstBuf));



	D3D11_BUFFER_DESC timeConstBufDesc;
	timeConstBufDesc.ByteWidth = sizeof(TimeConstBuf);
	timeConstBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	timeConstBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	timeConstBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	timeConstBufDesc.MiscFlags = 0;
	timeConstBufDesc.StructureByteStride = 0;

	TimeConstBuf timeConstBufData;
	timeConstBufData.DeltaTime = 0;
	timeConstBufData.TotalTime = 0;

	D3D11_SUBRESOURCE_DATA timeSubData;
	timeSubData.pSysMem = &timeConstBufData;

	HR(device->CreateBuffer(&timeConstBufDesc, &timeSubData, &mTimeConstBuf));


	mComputeShader = new ComputeShader(L"./Shader/cTractrixSplineSimulation.hlsl", "Simulation", true);
	mComputeShader->prepare(device);
}


TractrixSplineSimulation::~TractrixSplineSimulation()
{

}

void TractrixSplineSimulation::Simulate(const float deltaTime, ID3D11DeviceContext *context)
{
	ElapsedTimeInSimulation += deltaTime;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HR(context->Map(mTimeConstBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	TimeConstBuf *simConstBuf = (TimeConstBuf*)mappedResource.pData;
	simConstBuf->DeltaTime = deltaTime;
	simConstBuf->TotalTime = ElapsedTimeInSimulation;

	context->Unmap(mTimeConstBuf, 0);

	mComputeShader->activate(context);

	context->CSSetUnorderedAccessViews(0, 1, &mUAV, NULL);
	context->CSSetConstantBuffers(ConstBufSlots::TIME_CONST_BUF, 1, &mTimeConstBuf);

	context->Dispatch(16, 1, 1);

	ResetUtils::ResetShaders(context);
	ResetUtils::ResetComputeUavBuffer(context);
	ResetUtils::ResetAllConstantBuffers(context);
}
