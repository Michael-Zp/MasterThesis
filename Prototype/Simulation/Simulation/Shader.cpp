#include "Shader.h"




Shader::Shader(LPCWSTR filePath, LPCSTR functionName, ShaderType targetType, bool debug)
{
	UINT flags1 = debug ? (D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION) : 0;
	ID3D10Blob *errorMsgs;

	LPCSTR target;

	switch (targetType)
	{
		case ShaderType::Compute:
			target = "cs_5_0";
			break;
		case ShaderType::Domain:
			target = "ds_5_0";
			break;
		case ShaderType::Geometry:
			target = "gs_5_0";
			break;
		case ShaderType::Hull:
			target = "hs_5_0";
			break;
		case ShaderType::Pixel:
			target = "ps_5_0";
			break;
		case ShaderType::Vertex:
			target = "vs_5_0";
			break;
		default:
			target = "";
			DebugBreak();
	}


	HR(D3DCompileFromFile(filePath, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, functionName, target, flags1, 0, &mShaderBlob, &errorMsgs));


	if (errorMsgs != 0)
	{
		MessageBoxA(0, (char*)errorMsgs->GetBufferPointer(), 0, 0);
		ReleaseCOM(errorMsgs);
	}
}

Shader::~Shader()
{
}

void Shader::addVertexDesc(LPCSTR name, UINT index, DXGI_FORMAT format, UINT inputSlot, UINT offset, UINT instanceDataStepRate)
{
	mInputDesc.push_back(
		{name, index, format, inputSlot, offset, D3D11_INPUT_PER_VERTEX_DATA, instanceDataStepRate}
	);
}
