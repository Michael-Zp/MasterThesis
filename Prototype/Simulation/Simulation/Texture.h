#pragma once

#include <string>
#include <d3d11.h>
#include "DDSTextureLoader.h"

struct Texture
{
public:
	Texture(std::wstring filePath, ID3D11Device *device);
	void Activate(ID3D11DeviceContext *context);

	ID3D11Texture2D *Tex;
	ID3D11ShaderResourceView *SRV;
};

