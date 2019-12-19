#include "GeometryShader.h"

void GeometryShader::prepare(ID3D11Device *device)
{
	HR(device->CreateGeometryShader(mShaderBlob->GetBufferPointer(), mShaderBlob->GetBufferSize(), NULL, &mGeometryShader));
	mShaderBlob->Release();

	prepared = true;
}

void GeometryShader::activate(ID3D11DeviceContext * context)
{
	Shader::activate(context);

	context->GSSetShader(mGeometryShader, NULL, 0);
}
