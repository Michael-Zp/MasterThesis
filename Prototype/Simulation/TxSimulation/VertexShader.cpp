#include "VertexShader.h"

void VertexShader::prepare(ID3D11Device *device)
{
	UINT numVertexDescs = mInputDesc.size();

	if (numVertexDescs > 0)
	{
		mInputDescPreped = new D3D11_INPUT_ELEMENT_DESC[numVertexDescs];
		for (int i = 0; i < numVertexDescs; i++)
		{
			mInputDescPreped[i] = mInputDesc[i];
		}

		HR(device->CreateInputLayout(mInputDescPreped, numVertexDescs, mShaderBlob->GetBufferPointer(), mShaderBlob->GetBufferSize(), &mInputLayout));
	}
	else
	{
		mInputDescPreped = new D3D11_INPUT_ELEMENT_DESC;
		HR(device->CreateInputLayout(mInputDescPreped, 0, mShaderBlob->GetBufferPointer(), mShaderBlob->GetBufferSize(), &mInputLayout));
	}

	HR(device->CreateVertexShader(mShaderBlob->GetBufferPointer(), mShaderBlob->GetBufferSize(), NULL, &mVertexShader));


	prepared = true;
}

void VertexShader::activateInputLayout(ID3D11DeviceContext *context)
{
	context->IASetInputLayout(mInputLayout);
}

void VertexShader::activate(ID3D11DeviceContext * context)
{
	Shader::activate(context);

	context->VSSetShader(mVertexShader, NULL, 0);
}
