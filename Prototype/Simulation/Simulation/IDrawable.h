#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

class IDrawable
{
public:
	virtual void Draw(const float deltaTime, ID3D11DeviceContext *context) = 0;
	virtual void UpdateCamera(ID3D11DeviceContext *context, XMMATRIX view, XMMATRIX proj) = 0;
};