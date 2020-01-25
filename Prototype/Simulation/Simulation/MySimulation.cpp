#include "MySimulation.h"

#include "ResetUtils.h"


MySimulation::MySimulation(std::vector<XMFLOAT3> positions, ID3D11Device *device, ID3D11DeviceContext *context)
{
	D3D11_BUFFER_DESC structuredBufferDesc;
	structuredBufferDesc.ByteWidth = sizeof(Particle) * positions.size();
	structuredBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	structuredBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	structuredBufferDesc.CPUAccessFlags = 0;
	structuredBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	structuredBufferDesc.StructureByteStride = sizeof(Particle);

	std::vector<Particle> particles;
	particles.resize(0);
	for (int i = 0; i < positions.size(); i++)
	{
		float size = 0;
		XMFLOAT3 desiredRelativePos = XMFLOAT3(0, 0, 0);
		if (i > 0)
		{
			XMVECTOR root = XMLoadFloat3(&positions[i - 1]);
			XMVECTOR strand = XMLoadFloat3(&positions[i]);
			XMVECTOR diff = root - strand;
			XMStoreFloat3(&desiredRelativePos, strand - root);
			XMStoreFloat(&size, XMVector3Length(diff));
		}
		particles.push_back({ positions[i], size, XMFLOAT3(0, 0, 0), i, desiredRelativePos });
	}

	D3D11_SUBRESOURCE_DATA subData;
	subData.pSysMem = particles.data();

	HR(device->CreateBuffer(&structuredBufferDesc, &subData, &mStructuredBuffer));

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.Flags = 0;
	uavDesc.Buffer.NumElements = positions.size();

	HR(device->CreateUnorderedAccessView(mStructuredBuffer, &uavDesc, &mUAV));


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.ElementOffset = 0;
	srvDesc.Buffer.ElementWidth = sizeof(Particle);
	srvDesc.Buffer.NumElements = positions.size();

	HR(device->CreateShaderResourceView(mStructuredBuffer, &srvDesc, &mSRV));



	D3D11_BUFFER_DESC timeConstBuf;
	timeConstBuf.ByteWidth = sizeof(TimeConstBuf);
	timeConstBuf.Usage = D3D11_USAGE_DYNAMIC;
	timeConstBuf.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	timeConstBuf.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	timeConstBuf.MiscFlags = 0;
	timeConstBuf.StructureByteStride = 0;

	HR(device->CreateBuffer(&timeConstBuf, NULL, &mTimeConstBuf));



	D3D11_BUFFER_DESC propertiesConstBuf;
	propertiesConstBuf.ByteWidth = sizeof(TimeConstBuf);
	propertiesConstBuf.Usage = D3D11_USAGE_DYNAMIC;
	propertiesConstBuf.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	propertiesConstBuf.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	propertiesConstBuf.MiscFlags = 0;
	propertiesConstBuf.StructureByteStride = 0;

	PropertiesConstBuf propertiesConstBufData;
	propertiesConstBufData.drag = 0.9992;
	propertiesConstBufData.stiffness = 10;

	D3D11_SUBRESOURCE_DATA propertiesSubData;
	propertiesSubData.pSysMem = &propertiesConstBufData;

	HR(device->CreateBuffer(&propertiesConstBuf, &propertiesSubData, &mPropertiesConstBuf));


	mComputeShader = new ComputeShader(L"./Shader/cSimulation.hlsl", "Simulation", true);
	mComputeShader->prepare(device);
}


MySimulation::~MySimulation()
{

}

void MySimulation::Simulate(const float deltaTime, ID3D11DeviceContext *context)
{

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HR(context->Map(mTimeConstBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	TimeConstBuf *simConstBuf = (TimeConstBuf*)mappedResource.pData;
	simConstBuf->DeltaTime = deltaTime;

	context->Unmap(mTimeConstBuf, 0);

	mComputeShader->activate(context);

	context->CSSetUnorderedAccessViews(0, 1, &mUAV, NULL);
	context->CSSetConstantBuffers(ConstBufSlots::TIME_CONST_BUF, 1, &mTimeConstBuf);
	context->CSSetConstantBuffers(ConstBufSlots::PROPERTIES_CONST_BUF, 1, &mPropertiesConstBuf);

	context->Dispatch(16, 1, 1);

	ResetUtils::ResetShaders(context);
	ResetUtils::ResetComputeUavBuffer(context);
	ResetUtils::ResetAllConstantBuffers(context);
}
