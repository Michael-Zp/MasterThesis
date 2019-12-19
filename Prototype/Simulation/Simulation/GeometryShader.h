#pragma once

#include "Shader.h"

class GeometryShader : public Shader
{
public:
	GeometryShader(LPCWSTR filePath, LPCSTR functionName, bool debug) : Shader(filePath, functionName, ShaderType::Geometry, debug) {};
	void prepare(ID3D11Device *device);
	void activate(ID3D11DeviceContext *context);
	ID3D11GeometryShader *mGeometryShader;

private:
};

