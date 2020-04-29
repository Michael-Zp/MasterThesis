#include "TractrixSplineSimulation.h"

#include "ResetUtils.h"
#include "GeometryGenerator.h"


TractrixSplineSimulation::TractrixSplineSimulation(ID3D11Device *device, ID3D11DeviceContext *context, PropertiesConstBuf props, XMFLOAT4 strandColor, Configuration config)
{
	std::vector<GeometryGenerator::MeshData> meshData;
	GeometryGenerator generator;

	std::vector<std::vector<XMFLOAT3>> strandPoints;
	strandPoints.resize(mStrandsCount);
	//strandPoints[0].push_back(XMFLOAT3(0, 1.25, 0));
	//strandPoints[0].push_back(XMFLOAT3(1, 0, 0));
	//strandPoints[0].push_back(XMFLOAT3(0, -1.25, 0));
	//strandPoints[0].push_back(XMFLOAT3(-1, -1.5, 0));
	//strandPoints[0].push_back(XMFLOAT3(1, -2.75, 0));
	//strandPoints[0].push_back(XMFLOAT3(-1, -4.5, 0));
	//strandPoints[0].push_back(XMFLOAT3(0, -6.5, 0));

	//Overall test
	//std::vector<XMFLOAT3> myDirections = {
	//	XMFLOAT3(0, -1, 0),
	//	XMFLOAT3(1, -1, 0),
	//	XMFLOAT3(-0.5, -1, 0),
	//	XMFLOAT3(1, -1, 0),
	//	XMFLOAT3(-2, -1, 0),
	//	XMFLOAT3(3, -1, 0)
	//};
	std::vector<XMFLOAT3> myDirections;
	switch (config)
	{
		case TractrixSplineSimulation::Configuration::Z4Points:
		case TractrixSplineSimulation::Configuration::Z4PointsStretch:
			myDirections = {
				XMFLOAT3(0, -1, 0),
				XMFLOAT3(1, 0, 0),
				XMFLOAT3(0, -1, 0)
			};
			break;
		case TractrixSplineSimulation::Configuration::ZReverse4Points:
			myDirections = {
				XMFLOAT3(0, -1, 0),
				XMFLOAT3(-1, 0, 0),
				XMFLOAT3(0, -1, 0)
			};
			break;
		case TractrixSplineSimulation::Configuration::I4Points:
			myDirections = {
				XMFLOAT3(0, -1, 0),
				XMFLOAT3(0, -1, 0),
				XMFLOAT3(0, -1, 0)
			};
			break;		

		case TractrixSplineSimulation::Configuration::Z5Points:
		case TractrixSplineSimulation::Configuration::Z5PointsStretch:
			myDirections = {
				XMFLOAT3(0, -1, 0),
				XMFLOAT3(1, 0, 0),
				XMFLOAT3(0, -1, 0),
				XMFLOAT3(-1, -1, 0)
			};
			break;
		case TractrixSplineSimulation::Configuration::ZReverse5Points:
			myDirections = {
				XMFLOAT3(0, -1, 0),
				XMFLOAT3(1, 0, 0),
				XMFLOAT3(0, -1, 0),
				XMFLOAT3(-1, -1, 0)
			};
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
	strandPoints[0].push_back(basePoint);
	for (int i = 0; i < myDirections.size(); i++)
	{
		XMVECTOR currDir = XMLoadFloat3(&myDirections[i]);
		currentPoint += XMVector3Normalize(currDir);
		XMFLOAT3 tempPoint;
		XMStoreFloat3(&tempPoint, currentPoint);
		strandPoints[0].push_back(tempPoint);
	}

	std::vector<TractrixSplineSimulation::Strand> strands;


	strands.resize(strandPoints.size());
	meshData.resize(strandPoints.size());
	for (int i = 0; i < strandPoints.size(); i++)
	{
		generator.CreateLineStrip(strandPoints[i], meshData[i]);

		strands[i].ParticlesCount = strandPoints[i].size();
		strands[i].StrandIdx = i;

		switch (config)
		{
			case TractrixSplineSimulation::Configuration::Z4Points:
			case TractrixSplineSimulation::Configuration::ZReverse4Points:
				strands[i].DesiredHeadMovement = XMFLOAT3(-1, 0.7, 0);
				break;
			case TractrixSplineSimulation::Configuration::Z4PointsStretch:
				strands[i].DesiredHeadMovement = XMFLOAT3(1, -0.7, 0);
				break;
			case TractrixSplineSimulation::Configuration::I4Points:
				strands[i].DesiredHeadMovement = XMFLOAT3(0, -1, 0);
				break;
			case TractrixSplineSimulation::Configuration::Z5Points:
				strands[i].DesiredHeadMovement = XMFLOAT3(0.7, 2.5, 0);
				break;
			case TractrixSplineSimulation::Configuration::ZReverse5Points:
				strands[i].DesiredHeadMovement = XMFLOAT3(-1, 0.7, 0);
				break;
			case TractrixSplineSimulation::Configuration::Z5PointsStretch:
				strands[i].DesiredHeadMovement = XMFLOAT3(1, -1.7, 0);
				break;
			default:
				strands[i].DesiredHeadMovement = XMFLOAT3(-1, 0.7, 0);
				break;
		}

		strands[i].HeadVelocity = XMFLOAT3(0, 0, 0);
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


		for (int k = 0; k < strands[i].ParticlesCount; k++)
		{
			strands[i].Particles[k] = {
				meshData[i].Vertices[k].Position,
				strandColor
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
	ID3D11Buffer* buf[2];
	buf[0] = mTimeConstBuf;
	buf[1] = mPropertiesConstBuf;
	context->CSSetConstantBuffers(0, 2, buf);
	//context->CSSetConstantBuffers(ConstBufSlots::TIME_CONST_BUF, 1, &mTimeConstBuf);
	//context->CSSetConstantBuffers(ConstBufSlots::PROPERTEIS_CONST_BUF, 1, &mPropertiesConstBuf);

	context->Dispatch(16, 1, 1);

	ResetUtils::ResetShaders(context);
	ResetUtils::ResetComputeUavBuffer(context);
	ResetUtils::ResetAllConstantBuffers(context);
}
