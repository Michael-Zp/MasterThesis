#pragma once

#include "My_EI_Device.h"
#include "AMD_TressFX.h"
#include "My_EI_BindSet.h"
#include "My_EI_BindLayout.h"
#include "TressFXGPUInterface.h"
#include <iostream>

EI_BindSet* myCreateBindSet(EI_Device* device, AMD::TressFXBindSet& pBindSet)
{
	EI_BindSet* bindSet = new EI_BindSet;

	bindSet->nSRVs = pBindSet.nSRVs;
	bindSet->srvs = nullptr;
	bindSet->nUAVs = pBindSet.nUAVs;
	bindSet->uavs = nullptr;
	bindSet->values = pBindSet.values;
	bindSet->nBytes = pBindSet.nBytes;

	if (pBindSet.nSRVs > 0)
	{
		bindSet->srvs = new EI_SRV[pBindSet.nSRVs];
		memcpy(bindSet->srvs, pBindSet.srvs, sizeof(pBindSet.srvs[0]) * pBindSet.nSRVs);
	}


	if (pBindSet.nUAVs > 0)
	{
		bindSet->uavs = new EI_UAV[pBindSet.nUAVs];
		memcpy(bindSet->uavs, pBindSet.uavs, sizeof(pBindSet.uavs[0]) * pBindSet.nUAVs);
	}

	return bindSet;
}

void myBind(EI_CommandContextRef commandContext, EI_BindLayout* pLayout, EI_BindSet& bindSet)
{
	for (int i = 0; i < pLayout->SrvBindSlots.size() && i < bindSet.nSRVs; i++)
	{
		//Early exit loop
		if (!pLayout->SrvBindSlots[i].Valid)
			continue;

		switch (pLayout->Stage)
		{
			case AMD::EI_ShaderStage::EI_VS:
				commandContext.context->VSSetShaderResources(pLayout->SrvBindSlots[i].Value, 1, &bindSet.srvs[i]->srv);
				break;

			case AMD::EI_ShaderStage::EI_PS:
				commandContext.context->PSSetShaderResources(pLayout->SrvBindSlots[i].Value, 1, &bindSet.srvs[i]->srv);
				break;

			case AMD::EI_ShaderStage::EI_CS:
				commandContext.context->CSSetShaderResources(pLayout->SrvBindSlots[i].Value, 1, &bindSet.srvs[i]->srv);
				break;

			case AMD::EI_ShaderStage::EI_UNINITIALIZED:
				std::cout << "Shader stage was marked unitialized" << std::endl;
				break;
		}
	}


	for (int i = 0; i < pLayout->UavBindSlots.size() && i < bindSet.nUAVs; i++)
	{
		//Early exit loop
		if (!pLayout->UavBindSlots[i].Valid)
			continue;

		switch (pLayout->Stage)
		{
			case AMD::EI_ShaderStage::EI_PS:
				//Todo: Is 0 correct for the 4th parameter?
				commandContext.context->CSSetUnorderedAccessViews(pLayout->UavBindSlots[i].Value, 1, &bindSet.uavs[i]->uav, 0);
				break;

			case AMD::EI_ShaderStage::EI_UNINITIALIZED:
				std::cout << "Shader stage was marked unitialized" << std::endl;
				break;
		}
	}

	if (pLayout->ConstantBuffer != nullptr && pLayout->ConstantBindSlot.Valid)
	{
		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		HR(commandContext.context->Map(pLayout->ConstantBuffer->resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource));

		for (int i = 0, offset = 0; i < pLayout->ConstVarOffsets.size(); i++)
		{
			//
			//
			uint8_t *source = (uint8_t*)mappedSubresource.pData;
			uint8_t *dest = (uint8_t*)bindSet.values;

			memcpy(&source[pLayout->ConstVarOffsets[i].Offset], &dest[offset], pLayout->ConstVarOffsets[i].Size);
			offset += pLayout->ConstVarOffsets[i].Offset;
		}

		commandContext.context->Unmap(pLayout->ConstantBuffer->resource, 0);

		switch (pLayout->Stage)
		{
			case AMD::EI_ShaderStage::EI_VS:
				commandContext.context->VSSetConstantBuffers(pLayout->ConstantBindSlot.Value, 1, (ID3D11Buffer**)&pLayout->ConstantBuffer->resource);
				break;

			case AMD::EI_ShaderStage::EI_PS:
				commandContext.context->PSSetConstantBuffers(pLayout->ConstantBindSlot.Value, 1, (ID3D11Buffer**)&pLayout->ConstantBuffer->resource);
				break;

			case AMD::EI_ShaderStage::EI_CS:
				commandContext.context->CSSetConstantBuffers(pLayout->ConstantBindSlot.Value, 1, (ID3D11Buffer**)&pLayout->ConstantBuffer->resource);
				break;

			case AMD::EI_ShaderStage::EI_UNINITIALIZED:
				std::cout << "Shader stage was marked unitialized" << std::endl;
				break;
		}
	}


	switch (pLayout->Stage)
	{
		case AMD::EI_ShaderStage::EI_VS:
			commandContext.context->VSSetShader(pLayout->VertexShader, NULL, 0);
			break;

		case AMD::EI_ShaderStage::EI_PS:
			commandContext.context->PSSetShader(pLayout->PixelShader, NULL, 0);
			break;

		case AMD::EI_ShaderStage::EI_CS:
			commandContext.context->CSSetShader(pLayout->ComputeShader, NULL, 0);
			break;

		case AMD::EI_ShaderStage::EI_UNINITIALIZED:
			std::cout << "Shader stage was marked unitialized" << std::endl;
			break;
	}
}