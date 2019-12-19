#pragma once

#include "Shader.h"

class ComputeShader : Shader
{
public:
	ComputeShader(LPCWSTR filePath, LPCSTR functionName, bool debug) : Shader(filePath, functionName, ShaderType::Compute, debug) {};
	void prepare(ID3D11Device *device);
	void activate(ID3D11DeviceContext *context);

private:
	ID3D11ComputeShader *mComputeShader;
};

