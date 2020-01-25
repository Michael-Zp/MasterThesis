#pragma once

#ifndef _RESET_UTILS
#define _RESET_UTILS




#include <d3d11.h>

namespace ResetUtils
{
	//--------------
	//Shaders
	//--------------


	void ResetVertexShader(ID3D11DeviceContext *context);
	void ResetGeometryShader(ID3D11DeviceContext *context);
	void ResetPixelShader(ID3D11DeviceContext *context);
	void ResetComputeShader(ID3D11DeviceContext *context);
	void ResetShaders(ID3D11DeviceContext *context);

	//-------------
	//Buffers
	//-------------

	void ResetVertexBuffer(ID3D11DeviceContext *context);
	void ResetVertexShaderResources(ID3D11DeviceContext *context);
	void ResetGeometryShaderResources(ID3D11DeviceContext *context);
	void ResetIndexBuffer(ID3D11DeviceContext *context);
	void ResetComputeUavBuffer(ID3D11DeviceContext *context);

	//-----------
	//Constant Buffers
	//-----------

	void ResetGeometryConstantBuffer(ID3D11DeviceContext *context);
	void ResetPixelConstantBuffer(ID3D11DeviceContext *context);
	void ResetVertexConstantBuffer(ID3D11DeviceContext *context);
	void ResetComputeConstantBuffer(ID3D11DeviceContext *context);
	void ResetAllConstantBuffers(ID3D11DeviceContext * context);
}

#endif // !_RESET_UTILS