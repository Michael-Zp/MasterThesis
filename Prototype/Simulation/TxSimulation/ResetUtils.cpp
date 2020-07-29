#pragma once

#include "ResetUtils.h"
#include <d3d11.h>

namespace ResetUtils
{
	//--------------
	//Shaders
	//--------------


	void ResetVertexShader(ID3D11DeviceContext *context)
	{
		context->VSSetShader(NULL, NULL, 0);
	}

	void ResetGeometryShader(ID3D11DeviceContext *context)
	{
		context->GSSetShader(NULL, NULL, 0);
	}

	void ResetPixelShader(ID3D11DeviceContext *context)
	{
		context->PSSetShader(NULL, NULL, 0);
	}

	void ResetComputeShader(ID3D11DeviceContext *context)
	{
		context->CSSetShader(NULL, NULL, 0);
	}

	void ResetShaders(ID3D11DeviceContext *context)
	{
		ResetVertexShader(context);
		ResetGeometryShader(context);
		ResetPixelShader(context);
		ResetComputeShader(context);
	}

	//-------------
	//Buffers
	//-------------

	void ResetVertexBuffer(ID3D11DeviceContext *context)
	{
		//context->IASetVertexBuffers(0, 1, NULL, NULL, NULL);
	}

	void ResetVertexShaderResources(ID3D11DeviceContext *context)
	{
		ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
		context->VSSetShaderResources(0, 1, nullSRV);
	}

	void ResetGeometryShaderResources(ID3D11DeviceContext *context)
	{
		ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
		context->GSSetShaderResources(0, 1, nullSRV);
	}

	void ResetIndexBuffer(ID3D11DeviceContext *context)
	{
		context->IASetIndexBuffer(NULL, DXGI_FORMAT_UNKNOWN, 0);
	}

	void ResetComputeUavBuffer(ID3D11DeviceContext *context)
	{
		ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
		context->CSSetUnorderedAccessViews(0, 1, nullUAV, NULL);
	}

	//-----------
	//Constant Buffers
	//-----------

	void ResetGeometryConstantBuffer(ID3D11DeviceContext *context)
	{
		context->GSSetConstantBuffers(0, 0, NULL);
	}

	void ResetPixelConstantBuffer(ID3D11DeviceContext *context)
	{
		context->PSSetConstantBuffers(0, 0, NULL);
	}

	void ResetVertexConstantBuffer(ID3D11DeviceContext *context)
	{
		context->VSSetConstantBuffers(0, 0, NULL);
	}

	void ResetComputeConstantBuffer(ID3D11DeviceContext *context)
	{
		context->CSSetConstantBuffers(0, 0, NULL);
	}

	void ResetAllConstantBuffers(ID3D11DeviceContext * context)
	{
		ResetVertexConstantBuffer(context);
		ResetGeometryConstantBuffer(context);
		ResetPixelConstantBuffer(context);
		ResetComputeConstantBuffer(context);
	}
}