#pragma once

#include "My_EI_Resource.h"
#include "My_EI_Device.h"
#include "AMD_TressFX.h"
#include "Utils.h"


void CreateStructuredBuffer(EI_Resource* result, EI_Device* pDevice, AMD::uint32 sizeOfElement, AMD::uint32 numberOfElements, EI_StringHash myName, EI_StringHash name, bool isUAV, bool hasCounter, bool readWrite)
{
	D3D11_BUFFER_DESC bufDes;
	ZeroMemory(&bufDes, sizeof(bufDes));
	bufDes.ByteWidth = sizeOfElement * numberOfElements;
	//bufDes.Usage = D3D11_USAGE_DYNAMIC;
	//bufDes.Usage = readWrite ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	bufDes.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (isUAV)
	{
		bufDes.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
	}
	bufDes.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufDes.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufDes.StructureByteStride = sizeOfElement;

	HR(pDevice->CreateBuffer(&bufDes, nullptr, (ID3D11Buffer**)(&result->resource)));


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.ElementOffset = 0;
	srvDesc.Buffer.ElementWidth = sizeOfElement;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = numberOfElements;
	pDevice->CreateSrv(result->resource, &srvDesc, &result->srv);


	if (isUAV)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.Flags = hasCounter ? D3D11_BUFFER_UAV_FLAG_COUNTER : 0;
		uavDesc.Buffer.NumElements = numberOfElements;
		pDevice->CreateUav(result->resource, &uavDesc, &result->uav);
	}
}

EI_Resource* myCreateReadWriteSB(EI_Device* pDevice, AMD::uint32 structSize, AMD::uint32 structCount, EI_StringHash resourceName, EI_StringHash objectName)
{
	EI_Resource* result = new EI_Resource;
	CreateStructuredBuffer(result, pDevice, structSize, structCount, resourceName, objectName, true, true, true);
	return result;
}

EI_Resource* myCreateReadOnlySB(EI_Device* pDevice, AMD::uint32 structSize, AMD::uint32 structCount, EI_StringHash resourceName, EI_StringHash objectName)
{
	EI_Resource* result = new EI_Resource;
	CreateStructuredBuffer(result, pDevice, structSize, structCount, resourceName, objectName, false, false, false);
	return result;
}