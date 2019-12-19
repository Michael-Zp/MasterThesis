#pragma once
#ifndef _MODELCLASS_H_
#define _MODELCLASS_H_

#include <d3d11.h>
#include <directxmath.h>

#include "Texture.h"

class Model
{
public:
	Model();
	Model(const Model&);
	~Model();

	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

	struct VertexTypeTexture
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texture;
	};

	bool InitializeWithColor(ID3D11Device*);
	bool InitializeWithTexture(ID3D11Device*, ID3D11DeviceContext*, const char*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();

	ID3D11ShaderResourceView* GetTexture();

private:
	bool InitializeBuffersWithColor(ID3D11Device*);
	bool InitializeBuffersWithTexture(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTexture(ID3D11Device*, ID3D11DeviceContext*, const char*);
	void ReleaseTexture();

	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;
	Texture* m_Texture;
	int m_bufferStride;
};

#endif

