#pragma once

#include "Shader.h"

class VertexShader : public Shader
{
public:
	VertexShader(LPCWSTR filePath, LPCSTR functionName, bool debug) : Shader(filePath, functionName, ShaderType::Vertex, debug) {};
	void prepare(ID3D11Device *device);
	void activate(ID3D11DeviceContext *context);
	void activateInputLayout(ID3D11DeviceContext *context);
	ID3D11VertexShader *mVertexShader;

private:
};

