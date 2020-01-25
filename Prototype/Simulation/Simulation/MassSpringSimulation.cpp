#include "MassSpringSimulation.h"

#include "ResetUtils.h"


MassSpringSimulation::MassSpringSimulation(std::vector<std::vector<XMFLOAT3>*> positions, ID3D11Device *device, ID3D11DeviceContext *context)
{
	D3D11_BUFFER_DESC structuredBufferDesc;
	structuredBufferDesc.ByteWidth = sizeof(Strand) * positions.size();
	structuredBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	structuredBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	structuredBufferDesc.CPUAccessFlags = 0;
	structuredBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	structuredBufferDesc.StructureByteStride = sizeof(Strand);

	std::vector<Strand> strands;
	strands.resize(positions.size());
	for (int i = 0; i < strands.size(); i++)
	{
		strands[i].NumberOfParticles = positions[i]->size();
		strands[i].StrandIdx = i;

		for (int k = 0; k < positions[i]->size(); k++)
		{
			float size = 0;
			//XMFLOAT4 rotation = XMFLOAT4(0, 0, 0, 0);
			XMFLOAT3 desiredRelativPos = XMFLOAT3(0, 0, 0);
			float mass = 10000;
			XMFLOAT3 velocity = XMFLOAT3(0, 0, 0);
			if (k + 1 < strands[i].NumberOfParticles)
			{
				XMVECTOR root = XMLoadFloat3(&positions[i]->at(k));
				XMVECTOR leaf = XMLoadFloat3(&positions[i]->at(k + 1));
				XMVECTOR diff = root - leaf;
				XMStoreFloat(&size, XMVector3Length(diff));

				mass = (MAXIMUM_NUMBER_OF_PARTICLES - k) / 10;

				if (k > 0)
				{
					//See http://lolengine.net/blog/2013/09/18/beautiful-maths-quaternion-from-vectors for explanation
					/*XMVECTOR beforeRoot = XMLoadFloat3(&positions[i]->at(k - 1));

					XMVECTOR segment0 = XMVector3Normalize(root - beforeRoot);
					XMVECTOR segment1 = XMVector3Normalize(leaf - root);

					XMVECTOR cross = XMVector3Cross(segment0, segment1);
					XMFLOAT3 w;
					XMStoreFloat3(&w, cross);
					float dotProduct;
					XMStoreFloat(&dotProduct, XMVector3Dot(segment0, segment1));
					XMVECTOR quaternion = XMLoadFloat4(&(XMFLOAT4(1.0f + dotProduct, w.x, w.y, w.z)));
					quaternion = XMVector4Normalize(quaternion);

					XMStoreFloat4(&rotation, quaternion);*/

					XMVECTOR beforeRoot = XMLoadFloat3(&positions[i]->at(k - 1));

					XMStoreFloat3(&desiredRelativPos, root - beforeRoot);
				}
			}
			strands[i].Particles[k] = { size, desiredRelativPos, positions[i]->at(k), mass, velocity, k };
		}
	}

	D3D11_SUBRESOURCE_DATA subData;
	subData.pSysMem = strands.data();

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
	srvDesc.Buffer.ElementWidth = sizeof(Strand);
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


	mComputeShader = new ComputeShader(L"./Shader/cMassSpringSimulation.hlsl", "Simulation", true);
	mComputeShader->prepare(device);
}


MassSpringSimulation::~MassSpringSimulation()
{

}

void MassSpringSimulation::Simulate(const float deltaTime, ID3D11DeviceContext *context)
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
