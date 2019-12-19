#include "ComputeShader.h"


void ComputeShader::prepare(ID3D11Device *device)
{
	HR(device->CreateComputeShader(mShaderBlob->GetBufferPointer(), mShaderBlob->GetBufferSize(), NULL, &mComputeShader));
	mShaderBlob->Release();

	prepared = true;
}

void ComputeShader::activate(ID3D11DeviceContext * context)
{
	Shader::activate(context);

	context->CSSetShader(mComputeShader, NULL, 0);
}
