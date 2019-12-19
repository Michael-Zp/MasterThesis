#pragma once

#include "My_EI_Device.h"
#include "My_EI_BindLayout.h"

//
//
//struct TressFXLayoutDescription
//{
//	const int nSRVs;
//	const EI_StringHash* srvNames;
//
//	const int nUAVs;
//	const EI_StringHash* uavNames;
//
//	TressFXConstantBufferDesc constants;
//	EI_ShaderStage stage;
//};

//
//struct TressFXConstantBufferDesc
//{
//	const EI_StringHash     constantBufferName;
//	const int               bytes;
//	const int               nConstants;
//	const EI_StringHash*    parameterNames;
//};

EI_BindLayout* myCreateLayout(EI_Device* pDevice, EI_LayoutManagerRef layoutManager, const AMD::TressFXLayoutDescription& description)
{
	EI_BindLayout *layout = new EI_BindLayout();

	UINT temp;
	for (int i = 0; i < description.nSRVs; i++)
	{
		layout->SrvBindSlots.push_back(layoutManager.GetBindSlotByName(description.srvNames[i]));
	}

	for (int i = 0; i < description.nUAVs; i++)
	{
		layout->UavBindSlots.push_back(layoutManager.GetBindSlotByName(description.uavNames[i]));
	}

	if (description.constants.nConstants > 0)
	{
		layout->ConstantBindSlot = layoutManager.GetBindSlotByName(description.constants.constantBufferName);

		for (int i = 0; i < description.constants.nConstants; i++)
		{
			ConstBindSlot cbs = layoutManager.GetConstVarByName(description.constants.constantBufferName, description.constants.parameterNames[i]);
			if (description.constants.bytes > 0 && cbs.Valid)
			{
				layout->ConstVarOffsets.push_back(cbs);
			}
		}

		D3D11_BUFFER_DESC constantBuffer;
		constantBuffer.ByteWidth = layoutManager.GetConstBufferSizeByName(description.constants.constantBufferName);
		constantBuffer.Usage = D3D11_USAGE_DYNAMIC;
		constantBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constantBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		constantBuffer.MiscFlags = 0;
		constantBuffer.StructureByteStride = 0;

		layout->ConstantBuffer = new EI_Resource();

		pDevice->CreateBuffer(&constantBuffer, NULL, (ID3D11Buffer**)(&layout->ConstantBuffer->resource));

	}
	else
	{
		layout->ConstantBindSlot = { (UINT)-1, false };
		layout->ConstantBuffer = nullptr;
	}



	layout->VertexShader = layoutManager.GetVertexShader();
	layout->PixelShader = layoutManager.GetPixelShader();
	layout->ComputeShader = layoutManager.GetComputeShader();

	layout->Stage = description.stage;

	return layout;
}