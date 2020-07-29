#pragma once

#include "Shader.h"

class PixelShader : Shader
{
public:
	PixelShader(LPCWSTR filePath, LPCSTR functionName, bool debug) : Shader(filePath, functionName, ShaderType::Pixel, debug) {};
	void prepare(ID3D11Device *device);
	void activate(ID3D11DeviceContext *context);

private:
	ID3D11PixelShader *mPixelShader;
};

