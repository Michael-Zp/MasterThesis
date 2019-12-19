#pragma once

#include <d3d11.h>
#include "GeometryGenerator.h"
#include "VertexShader.h"
#include "GeometryShader.h"
#include "PixelShader.h"

class IRenderableItem
{
public:
	virtual void Draw(ID3D11DeviceContext *context) = 0;
	virtual void UpdateCamera(ID3D11DeviceContext *context, XMMATRIX view, XMMATRIX proj) = 0;

protected:
	ID3D11Buffer *mVertexBuffer;
	ID3D11Buffer *mIndexBuffer;
	ID3D11Buffer *mConstantBuffer;

	VertexShader *mVertexShader;
	GeometryShader *mGeometryShader;
	PixelShader *mPixelShader;
};

