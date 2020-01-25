#include "Hair.h"



Hair::Hair(ID3D11Device *device)
{
	GeometryGenerator::MeshData meshData;
	GeometryGenerator generator;

	std::vector<XMFLOAT3> points;
	points.push_back(XMFLOAT3(0, 1.25, 0));
	points.push_back(XMFLOAT3(0, 0, 0));

	generator.CreateLineStrip(points, meshData);

	std::vector<HairVertexData> vertexData;

	for (int i = 0; i < meshData.Vertices.size(); i++)
	{
		vertexData.push_back({
			meshData.Vertices[i].Position,
			(XMFLOAT4)Colors::Magenta
			});
	}

	mVertexCount = vertexData.size();

	D3D11_BUFFER_DESC vertexDesc;
	vertexDesc.ByteWidth = sizeof(HairVertexData) * mVertexCount;
	vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.CPUAccessFlags = 0;
	vertexDesc.MiscFlags = 0;
	vertexDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexSubData;
	vertexSubData.pSysMem = vertexData.data();

	HR(device->CreateBuffer(&vertexDesc, &vertexSubData, &mVertexBuffer));


	D3D11_BUFFER_DESC constantBuffer;
	constantBuffer.ByteWidth = sizeof(HairConstantBuffer);
	constantBuffer.Usage = D3D11_USAGE_DYNAMIC;
	constantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBuffer.MiscFlags = 0;
	constantBuffer.StructureByteStride = 0;

	HR(device->CreateBuffer(&constantBuffer, NULL, &mConstantBuffer));


	mVertexShader = new VertexShader(L"./Shader/vgpHair.hlsl", "HairVS", true);
	mVertexShader->addVertexDesc("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, 0);
	mVertexShader->addVertexDesc("COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, 0);
	mVertexShader->prepare(device);

	mGeometryShader = new GeometryShader(L"./Shader/vgpHair.hlsl", "HairGS", true);
	mGeometryShader->prepare(device);

	mPixelShader = new PixelShader(L"./Shader/vgpHair.hlsl", "HairPS", true);
	mPixelShader->prepare(device);
}


Hair::~Hair()
{

}

void Hair::Draw(float deltaTime, ID3D11DeviceContext *context)
{
	if (!mIsUpdated)
	{
		DebugBreak();
	}

	mVertexShader->activateInputLayout(context);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);


	mVertexShader->activate(context);
	mGeometryShader->activate(context);
	mPixelShader->activate(context);

	UINT stride = sizeof(HairVertexData);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
	context->VSSetConstantBuffers(0, 1, &mConstantBuffer);


	context->Draw(mVertexCount, 0);

	ResetUtils::ResetShaders(context);
	ResetUtils::ResetVertexBuffer(context);
	ResetUtils::ResetAllConstantBuffers(context);


	mIsUpdated = false;
}

void Hair::UpdateCamera(ID3D11DeviceContext *context, XMMATRIX view, XMMATRIX proj)
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
