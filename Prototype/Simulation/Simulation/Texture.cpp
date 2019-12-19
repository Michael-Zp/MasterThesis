#include "Texture.h"

Texture::Texture(std::wstring filePath, ID3D11Device *device)
{
	DirectX::CreateDDSTextureFromFile(device, filePath.c_str(), (ID3D11Resource**)&Tex, &SRV);
}

void Texture::Activate(ID3D11DeviceContext * context)
{
	context->PSSetShaderResources(0, 1, &SRV);
}
