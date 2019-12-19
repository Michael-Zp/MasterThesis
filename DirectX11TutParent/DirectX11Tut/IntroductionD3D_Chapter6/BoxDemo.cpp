#include "BoxDemo.h"


#include <stdio.h>
#include <direct.h>
#include <iostream>


BoxDemo::BoxDemo(HINSTANCE hInstance) : D3DApp(hInstance)
{
	char path[FILENAME_MAX];
	_getcwd(path, sizeof(path));
	path[sizeof(path) - 1] = '\0';
	std::cout << path << std::endl;
}

BoxDemo::~BoxDemo()
{
}

struct MatrixBufferType
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX proj;
};


struct VertexData {
	XMFLOAT3 pos;
	XMFLOAT4 color;
};


bool BoxDemo::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}

	GeometryGenerator::MeshData mesh;
	GeometryGenerator generator;

	
	generator.CreateBox(1, 1, 1, mesh);
	
	bool simple = false;

	if (simple) {

		VertexData vertexDataSimple[]{
			{ XMFLOAT3(-1, -1, 1), (XMFLOAT4)Colors::White },
			{ XMFLOAT3(0,  1, 1), (XMFLOAT4)Colors::White },
			{ XMFLOAT3(1, -1, 1), (XMFLOAT4)Colors::White }
		};

		D3D11_BUFFER_DESC vertexDesc;
		vertexDesc.ByteWidth = sizeof(VertexData) * 3;
		vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexDesc.CPUAccessFlags = 0;
		vertexDesc.MiscFlags = 0;
		vertexDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexSubData;
		vertexSubData.pSysMem = vertexDataSimple;

		HR(md3dDevice->CreateBuffer(&vertexDesc, &vertexSubData, &mVertexBuffer));


		UINT indexSimple[]{
			0, 1, 2
		};


		D3D11_BUFFER_DESC indexDesc;
		indexDesc.ByteWidth = sizeof(UINT) * 3;
		indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexDesc.CPUAccessFlags = 0;
		indexDesc.MiscFlags = 0;
		indexDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexSubData;
		indexSubData.pSysMem = indexSimple;

		md3dDevice->CreateBuffer(&indexDesc, &indexSubData, &mIndexBuffer);
	}
	else
	{
		const UINT colorCount = 8;
		XMFLOAT4 colors[] = {
			(XMFLOAT4)Colors::White, 
			(XMFLOAT4)Colors::Black,
			(XMFLOAT4)Colors::Red,
			(XMFLOAT4)Colors::Green,
			(XMFLOAT4)Colors::Blue,
			(XMFLOAT4)Colors::Yellow,
			(XMFLOAT4)Colors::Cyan,
			(XMFLOAT4)Colors::Magenta,
		};

		UINT vertexCount = mesh.Vertices.size();
		VertexData *vertexData = new VertexData[vertexCount];

		for (int i = 0; i < vertexCount; i++)
		{
			vertexData[i] = { mesh.Vertices[i].Position, colors[i % colorCount] };
		}



		D3D11_BUFFER_DESC vertexDesc;
		vertexDesc.ByteWidth = sizeof(VertexData) * vertexCount;
		vertexDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexDesc.CPUAccessFlags = 0;
		vertexDesc.MiscFlags = 0;
		vertexDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA vertexSubData;
		vertexSubData.pSysMem = vertexData;

		HR(md3dDevice->CreateBuffer(&vertexDesc, &vertexSubData, &mVertexBuffer));
		
		mIndexCount = mesh.Indices.size();

		D3D11_BUFFER_DESC indexDesc;
		indexDesc.ByteWidth = sizeof(UINT) * mesh.Indices.size();
		indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
		indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexDesc.CPUAccessFlags = 0;
		indexDesc.MiscFlags = 0;
		indexDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexSubData;
		indexSubData.pSysMem = mesh.Indices.data();

		md3dDevice->CreateBuffer(&indexDesc, &indexSubData, &mIndexBuffer);
	}


	D3D11_BUFFER_DESC constantBuffer;
	constantBuffer.ByteWidth = sizeof(MatrixBufferType);
	constantBuffer.Usage = D3D11_USAGE_DYNAMIC;
	constantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBuffer.MiscFlags = 0;
	constantBuffer.StructureByteStride = 0;

	md3dDevice->CreateBuffer(&constantBuffer, NULL, &mConstBuffer);




	mVertexShader = new VertexShader(L"./Shader/box.hlsl", "BoxVertex", ShaderType::Vertex, true);
	mVertexShader->addVertexDesc("POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, 0);
	mVertexShader->addVertexDesc("COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, 0);
	mVertexShader->prepare(md3dDevice);

	mPixelShader = new PixelShader(L"./Shader/box.hlsl", "BoxPixel", ShaderType::Pixel, true);
	mPixelShader->prepare(md3dDevice);

	return true;
}

void BoxDemo::OnResize()
{
	D3DApp::OnResize();
}

void BoxDemo::UpdateScene(float dt)
{
}


void BoxDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void BoxDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void BoxDemo::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.005f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f*static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 3.0f, 15.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}


void BoxDemo::DrawScene()
{
	float x = mRadius * sinf(mPhi)*cosf(mTheta);
	float y = mRadius * cosf(mPhi);
	float z = mRadius * sinf(mPhi)*sinf(mTheta);

	XMMATRIX world = XMMatrixIdentity();
	XMMATRIX view = XMMatrixLookAtLH(XMVectorSet(x, y, z, 1.0f), XMVectorZero(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	XMMATRIX proj = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	//proj = DirectX::XMMatrixOrthographicLH((float)800, (float)600, 1.0f, 1000.0f);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HR(md3dImmediateContext->Map(mConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));

	MatrixBufferType *matrix = (MatrixBufferType*)mappedResource.pData;
	matrix->world = XMMatrixTranspose(world);
	matrix->view = XMMatrixTranspose(view);
	matrix->proj = XMMatrixTranspose(proj);

	md3dImmediateContext->Unmap(mConstBuffer, 0);


	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView,
		reinterpret_cast<const float*>(&Colors::Black));

	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	mVertexShader->activateInputLayout(md3dImmediateContext);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	mVertexShader->activate(md3dImmediateContext);
	mPixelShader->activate(md3dImmediateContext);

	UINT stride = sizeof(VertexData);
	UINT offset = 0;
	md3dImmediateContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
	md3dImmediateContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	md3dImmediateContext->VSSetConstantBuffers(0, 1, &mConstBuffer);


	md3dImmediateContext->DrawIndexed(mIndexCount, 0, 0);

	HR(mSwapChain->Present(0, 0));
}
