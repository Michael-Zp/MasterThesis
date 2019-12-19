#include "Simulation.h"

#include "ResetUtils.h"


Simulation::Simulation(std::vector<XMFLOAT3> positions, ID3D11Device *device, ID3D11DeviceContext *context)
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
		particles.push_back({ positions[i], 0 });
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


	mComputeShader = new ComputeShader(L"./Shader/simulation.hlsl", "Simulation", true);
	mComputeShader->prepare(device);
}


Simulation::~Simulation()
{

}

void Simulation::Simulate(ID3D11DeviceContext *context)
{
	mComputeShader->activate(context);

	context->CSSetUnorderedAccessViews(0, 1, &mUAV, NULL);

	context->Dispatch(2, 1, 1);

	ResetUtils::ResetShaders(context);
	ResetUtils::ResetComputeUavBuffer(context);
	ResetUtils::ResetAllConstantBuffers(context);
}
