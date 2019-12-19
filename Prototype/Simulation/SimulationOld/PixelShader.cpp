#include "PixelShader.h"


void PixelShader::prepare(ID3D11Device *device)
{
	HR(device->CreatePixelShader(mShaderBlob->GetBufferPointer(), mShaderBlob->GetBufferSize(), NULL, &mPixelShader));
	mShaderBlob->Release();

	prepared = true;
}

void PixelShader::activate(ID3D11DeviceContext * context)
{
	Shader::activate(context);

	context->PSSetShader(mPixelShader, NULL, 0);
}
