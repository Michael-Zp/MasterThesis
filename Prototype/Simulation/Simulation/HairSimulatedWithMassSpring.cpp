#include "HairSimulatedWithMassSpring.h"



HairSimulatedWithMassSpring::HairSimulatedWithMassSpring(ID3D11Device *device, ID3D11DeviceContext *context)
{
	std::vector<std::vector<XMFLOAT3>*> points;
	points.push_back(new std::vector<XMFLOAT3>);
	points[0]->push_back(XMFLOAT3(0, 1.25, 0));
	points[0]->push_back(XMFLOAT3(1, .5, 0));
	points[0]->push_back(XMFLOAT3(0.5, -.5, 0));
	points[0]->push_back(XMFLOAT3(-0.5, -1.5, 0));
	points[0]->push_back(XMFLOAT3(0.5, -2.5, 0));
	points[0]->push_back(XMFLOAT3(-0.5, -3.5, 0));

	mSimulation = new MassSpringSimulation(points, device, context);

	mVertexCount = points.size() * 16;

	D3D11_BUFFER_DESC constantBuffer;
	constantBuffer.ByteWidth = sizeof(HairSimulatedConstantBuffer);
	constantBuffer.Usage = D3D11_USAGE_DYNAMIC;
	constantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBuffer.MiscFlags = 0;
	constantBuffer.StructureByteStride = 0;

	HR(device->CreateBuffer(&constantBuffer, NULL, &mConstantBuffer));


	mVertexShader = new VertexShader(L"./Shader/vgpHairMassSpringSimulated.hlsl", "HairVS", true);
	mVertexShader->prepare(device);

	mGeometryShader = new GeometryShader(L"./Shader/vgpHairMassSpringSimulated.hlsl", "HairGS", true);
	mGeometryShader->prepare(device);

	mPixelShader = new PixelShader(L"./Shader/vgpHairMassSpringSimulated.hlsl", "HairPS", true);
	mPixelShader->prepare(device);
}


HairSimulatedWithMassSpring::~HairSimulatedWithMassSpring()
{
	free(mSimulation);
}

void HairSimulatedWithMassSpring::Draw(const float deltaTime, ID3D11DeviceContext *context)
{
	if (!mIsUpdated)
	{
		DebugBreak();
	}

	mSimulation->Simulate(deltaTime, context);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);


	mVertexShader->activate(context);
	mGeometryShader->activate(context);
	mPixelShader->activate(context);

	mVertexShader->activateInputLayout(context);
	context->VSSetShaderResources(0, 1, mSimulation->GetSRVPtr());
	context->GSSetShaderResources(0, 1, mSimulation->GetSRVPtr());
	context->VSSetConstantBuffers(0, 1, &mConstantBuffer);


	context->Draw(mVertexCount, 0);

	ResetUtils::ResetShaders(context);
	ResetUtils::ResetVertexShaderResources(context);
	ResetUtils::ResetGeometryShaderResources(context);
	ResetUtils::ResetAllConstantBuffers(context);


	mIsUpdated = false;
}

void HairSimulatedWithMassSpring::UpdateCamera(ID3D11DeviceContext *context, XMMATRIX view, XMMATRIX proj)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HR(context->Map(mConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	HairSimulatedConstantBuffer *matrix = (HairSimulatedConstantBuffer*)mappedResource.pData;
	matrix->view = XMMatrixTranspose(view);
	matrix->proj = XMMatrixTranspose(proj);
	matrix->world = XMMatrixTranspose(XMMatrixIdentity());

	context->Unmap(mConstantBuffer, 0);

	mIsUpdated = true;
}
