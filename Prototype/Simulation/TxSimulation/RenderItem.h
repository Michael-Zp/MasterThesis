#pragma once

#include <d3d11.h>
#include "GeometryGenerator.h"
#include "VertexShader.h"
#include "GeometryShader.h"
#include "PixelShader.h"
#include "IDrawable.h"

class RenderItem
{
public:
	ID3D11Buffer *mVertexBuffer;
	ID3D11Buffer *mIndexBuffer;
	ID3D11Buffer *mConstantBuffer;

	VertexShader *mVertexShader;
	GeometryShader *mGeometryShader;
	PixelShader *mPixelShader;
};

