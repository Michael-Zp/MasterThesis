#pragma once

#include "d3d11.h"
#include "Utils.h"
#include "TressFXAsset.h"
#include "My_EI_CommandContext.h"
#include "My_EI_Resource.h"


class EI_CommandContext
{
public:
	EI_CommandContext() {
		context = nullptr;
	}
	ID3D11DeviceContext* context;
	EI_CommandContext(ID3D11DeviceContext* pContext) : context(pContext) {}

	void* Map(EI_Resource& pResource) {

		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		HR(context->Map((ID3D11Buffer*)pResource.resource, 0, D3D11_MAP::D3D11_MAP_WRITE, 0, &mappedSubresource));
		return mappedSubresource.pData;
	}

	bool Unmap(EI_Resource& pResource) {
		context->Unmap(pResource.resource, 0);
		return true;
	}
};