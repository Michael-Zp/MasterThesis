#pragma once
#include "d3d11.h"

class EI_Device
{
public:
	EI_Device() {
		device = nullptr;
	}

	ID3D11Device* device;
	EI_Device(ID3D11Device* pDevice) : device(pDevice) {}

	ID3D11Device& GetDevice() { return *device; }

	HRESULT CreateBuffer(const D3D11_BUFFER_DESC *pDesc, const D3D11_SUBRESOURCE_DATA *pInitialData, ID3D11Buffer **ppBuffer) {
		return device->CreateBuffer(pDesc, pInitialData, ppBuffer);
	}

	HRESULT CreateSrv(ID3D11Resource *pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC *pDesc, ID3D11ShaderResourceView **ppSRView) {
		return device->CreateShaderResourceView(pResource, pDesc, ppSRView);
	}

	HRESULT CreateUav(ID3D11Resource *pResource, const D3D11_UNORDERED_ACCESS_VIEW_DESC *pDesc, ID3D11UnorderedAccessView **ppUAView)
	{
		return device->CreateUnorderedAccessView(pResource, pDesc, ppUAView);
	}
};