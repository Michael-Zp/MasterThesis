#include "TractrixSimulation.h"

#include "ResetUtils.h"
#include "GeometryGenerator.h"


TractrixSimulation::TractrixSimulation(ID3D11Device *device, ID3D11DeviceContext *context, PropertiesConstBuf props, XMFLOAT4 strandColor, Configuration config)
{
	std::vector<std::vector<XMFLOAT3>> strandPoints;
	strandPoints.resize(mStrandsCount);

	std::vector<TractrixSimulation::Strand> strands;

	srand(time(NULL));

	strands.resize(strandPoints.size());
	for (int i = 0; i < strandPoints.size(); i++)
	{
		std::vector<XMFLOAT3> myDirections;
		switch (config)
		{
		case TractrixSimulation::Configuration::Z4Points:
			myDirections = {
				XMFLOAT3(0, -1, 0),
				XMFLOAT3(1, 0, 0),
				XMFLOAT3(0, -1, 0)
			};
			break;
		case TractrixSimulation::Configuration::Random:
			myDirections.resize(mNumberOfSegments);
			for (int i = 0; i < mNumberOfSegments; i++)
			{
				float x = (float(rand()) / float((RAND_MAX)) - 0.5) * 2;
				float y = (float(rand()) / float((RAND_MAX)) - 0.5) * 2;
				y = min(0.5, abs(y)) * -1;
				float z = (float(rand()) / float((RAND_MAX)) - 0.5) * 2;
				XMVECTOR vec = XMLoadFloat3(&XMFLOAT3(x, y, z));
				vec = XMVector3Normalize(vec);
				XMStoreFloat3(&myDirections[i], vec);
			}
			break;
		default:
			myDirections = {
				XMFLOAT3(0, -1, 0),
				XMFLOAT3(1, 0, 0),
				XMFLOAT3(0, -1, 0)
			};
			break;
		}

		XMFLOAT3 basePoint(0, 1.25, 0);
		XMVECTOR currentPoint = XMLoadFloat3(&basePoint);
		strandPoints[i].push_back(basePoint);
		for (int k = 0; k < myDirections.size(); k++)
		{
			XMVECTOR currDir = XMLoadFloat3(&myDirections[k]);
			currentPoint += XMVector3Normalize(currDir);
			XMFLOAT3 tempPoint;
			XMStoreFloat3(&tempPoint, currentPoint);
			strandPoints[i].push_back(tempPoint);
		}

		strands[i].ParticlesCount = strandPoints[i].size();
		strands[i].StrandIdx = i;

		strands[i].HairRoot = strandPoints[i][0];
		strands[i].OriginalHeadPosition = strandPoints[i][strandPoints[i].size() - 1];
		strands[i].KnotHasChangedOnce = 0.0;

		int knotSize = strands[i].ParticlesCount + 4;
		strands[i].MaxKnotValue = strands[i].ParticlesCount - 3;
		for (int j = 0; j < 4; j++)
		{
			strands[i].Knot[j] = 0;
			strands[i].Knot[knotSize - j - 1] = strands[i].MaxKnotValue;
		}

		//This should change if the control polygons dont have the same length, but for now this works
		strands[i].KnotValues[0] = 0;
		strands[i].KnotValues[1] = 0.33;
		strands[i].KnotValues[2] = 0.67;
		strands[i].KnotValues[3] = 1;

		for (int j = 0; j < knotSize - 8; j++)
		{
			strands[i].Knot[j + 4] = j + 1;
			strands[i].KnotValues[j + 4] = j + 2;
		}

		/*
			Knot:			0 0    0    0 1 1 1 1
			KnotValues:		0 0.33 0.67 1

			Knot:			0 0	   0    0 1 2 2 2
			KnotValues:		0 0.33 0.67 1 2
		*/
		for (int k = 0; k < strands[i].ParticlesCount - 1; k++)
		{
			XMVECTOR tail = XMLoadFloat3(&strandPoints[i][k]);
			XMVECTOR head = XMLoadFloat3(&strandPoints[i][k + 1]);
			XMVECTOR direction = XMVector3Normalize(head - tail);
			XMStoreFloat3(&strands[i].DesiredSegmentDirections[k], direction);
		}

		for (int k = 0; k < strands[i].ParticlesCount; k++)
		{
			strands[i].Particles[k] = {
				strandPoints[i][k],
				strandColor,
				XMFLOAT3(0, 0, 0),
				1
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



	D3D11_BUFFER_DESC propertiesConstBufDesc;
	propertiesConstBufDesc.ByteWidth = sizeof(PropertiesConstBuf);
	propertiesConstBufDesc.Usage = D3D11_USAGE_IMMUTABLE;
	propertiesConstBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	propertiesConstBufDesc.CPUAccessFlags = 0;
	propertiesConstBufDesc.MiscFlags = 0;
	propertiesConstBufDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA propertiesSubData;
	propertiesSubData.pSysMem = &props;

	HR(device->CreateBuffer(&propertiesConstBufDesc, &propertiesSubData, &mPropertiesConstBuf));


	mComputeShader = new ComputeShader(L"./Shader/cTractrixSimulation.hlsl", "Simulation", true);
	mComputeShader->prepare(device);
}


TractrixSimulation::~TractrixSimulation()
{

}

void TractrixSimulation::Simulate(const float deltaTime, ID3D11DeviceContext *context)
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
	ID3D11Buffer* buf[2];
	buf[0] = mTimeConstBuf;
	buf[1] = mPropertiesConstBuf;
	context->CSSetConstantBuffers(0, 2, buf);

	context->Dispatch(16, 1, 1);

	ResetUtils::ResetShaders(context);
	ResetUtils::ResetComputeUavBuffer(context);
	ResetUtils::ResetAllConstantBuffers(context);
}
