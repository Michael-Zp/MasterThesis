#pragma once

#include "My_EI_Device.h"
#include "AMD_TressFX.h"
#include "My_EI_IndexBuffer.h"
#include <DirectXMath.h>

using namespace DirectX;

EI_IndexBuffer* myCreateIndexBuffer(EI_Device* pDevice, AMD::uint32 size, void* pInitialData, EI_StringHash objectName)
{
	EI_IndexBuffer *result = new EI_IndexBuffer(objectName);

	D3D11_BUFFER_DESC bufDes;
	ZeroMemory(&bufDes, sizeof(bufDes));
	bufDes.ByteWidth = size * TRESSFX_INDEX_SIZE; //Size should be 32 bits -> 4 byte
	bufDes.Usage = D3D11_USAGE_IMMUTABLE;
	bufDes.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufDes.CPUAccessFlags = 0;
	bufDes.MiscFlags = 0;
	bufDes.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA subData;
	subData.pSysMem = pInitialData;

	HR(pDevice->CreateBuffer(&bufDes, &subData, &result->indexBuffer));

	return result;
}

namespace Colors
{
	XMGLOBALCONST XMVECTORF32 White = { 1.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Black = { 0.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Green = { 0.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
	XMGLOBALCONST XMVECTORF32 Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };

	XMGLOBALCONST XMVECTORF32 Silver = { 0.75f, 0.75f, 0.75f, 1.0f };
	XMGLOBALCONST XMVECTORF32 LightSteelBlue = { 0.69f, 0.77f, 0.87f, 1.0f };
}

void myDraw(EI_CommandContextRef commandContext, EI_PSO& pso, AMD::EI_IndexedDrawParams& drawParams)
{
	commandContext.context->ClearRenderTargetView(pso.mRenderTargetView, reinterpret_cast<const float*>(&Colors::Black));

	commandContext.context->ClearDepthStencilView(pso.mDepthStencilView, D3D11_CLEAR_DEPTH , 1.0f, 0);

	commandContext.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	commandContext.context->IASetIndexBuffer(drawParams.pIndexBuffer->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	if (pso.mVertexConstantBindSlot.Valid)
	{
		commandContext.context->VSSetConstantBuffers(pso.mVertexConstantBindSlot.Value, 1, &pso.mVertexConstantBuffer);
	}

	if (pso.mPixelConstantBindSlot.Valid)
	{
		commandContext.context->PSSetConstantBuffers(pso.mPixelConstantBindSlot.Value, 1, &pso.mPixelConstantBuffer);
	}

	commandContext.context->DrawIndexedInstanced(drawParams.numIndices, drawParams.numInstances, 0, 0, 0);

	HR(pso.mSwapChain->Present(0, 0));
}