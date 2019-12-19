#pragma once

#include "Shader.h"

class VertexShader : public Shader
{
public:
	VertexShader(LPCWSTR filePath, LPCSTR functionName, ShaderType targetType, bool debug) : Shader(filePath, functionName, targetType, debug) {};
	void prepare(ID3D11Device *device);
	void activate(ID3D11DeviceContext *context);
	void activateInputLayout(ID3D11DeviceContext *context);

private:
	ID3D11VertexShader *mVertexShader;
};

