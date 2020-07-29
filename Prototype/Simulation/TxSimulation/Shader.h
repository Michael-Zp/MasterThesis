#pragma once

#include <string>
#include <vector>
#include "d3dUtil.h"
#include <d3d11.h>
#include <d3dcompiler.h>

enum ShaderType
{
	Compute,
	Domain,
	Geometry,
	Hull,
	Pixel, 
	Vertex
};

class Shader
{
public:
	Shader(LPCWSTR filePath, LPCSTR functionName, ShaderType targetType, bool debug);
	~Shader();

	virtual void prepare(ID3D11Device *device) = 0;
	void activate(ID3D11DeviceContext *context) {
		if (!prepared)
		{
			DebugBreak();
		}
	}

	void addVertexDesc(LPCSTR name, UINT index, DXGI_FORMAT format, UINT inputSlot, UINT offset, UINT instanceDataStepRate);

protected:
	std::vector<D3D11_INPUT_ELEMENT_DESC> mInputDesc;
	D3D11_INPUT_ELEMENT_DESC *mInputDescPreped;
	ID3D11InputLayout *mInputLayout;

	ID3D10Blob *mShaderBlob;

	bool prepared = false;

};

