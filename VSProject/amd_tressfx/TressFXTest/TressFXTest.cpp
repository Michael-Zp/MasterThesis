#include <iostream>
#include <stdio.h>
#include "TressFXLayouts.h"
#include "AMD_TressFX.h"
#include "AMD_Types.h"
#include "TressFXAsset.h"
#include "TressFXHairObject.h"
#include "d3d11.h"
#include "d3d12.h"
#include <Windows.h>
#include <direct.h>
#include <exception>

TressFXLayouts* g_TressFXLayouts = 0;

class EI_Resource
{
public:
	ID3D11Buffer* textureBuffer;
	ID3D11ShaderResourceView* srv;
	ID3D11UnorderedAccessView* uav;
};

class EI_Device
{
private:
	ID3D11Device* device;
public:
	EI_Device(ID3D11Device* pDevice) : device(pDevice) {}


	HRESULT CreateBuffer(const D3D11_BUFFER_DESC *pDesc, const D3D11_SUBRESOURCE_DATA *pInitialData, ID3D11Buffer **ppBuffer) {
		return device->CreateBuffer(pDesc, pInitialData, ppBuffer);
	}

	HRESULT CreateSrv(ID3D11Resource *pResource, const D3D11_SHADER_RESOURCE_VIEW_DESC *pDesc, ID3D11ShaderResourceView **ppSRView) {
		return device->CreateShaderResourceView(pResource, pDesc, ppSRView);
		

		
	}
};

class EI_BindSet
{
public:
	//Identical to TressFXBindSet
	int     nSRVs;
	EI_SRV* srvs;
	int     nUAVs;
	EI_UAV* uavs;
	void*   values;
	int     nBytes;
};

void myError(EI_StringHash msg)
{
	std::cout << msg << std::endl;
}

void myRead(void* ptr, AMD::uint size, EI_StreamRef pFile)
{
	fread(ptr, size, 1, (FILE*)pFile);
}

void mySeek(EI_StreamRef pFile, AMD::uint offset)
{
	fseek((FILE*)pFile, offset, SEEK_SET);
}

void CreateShaderBuffer(EI_Resource* result, EI_Device* pDevice, AMD::uint32 sizeOfElement, AMD::uint32 numberOfElements, EI_StringHash myName, EI_StringHash name, bool isUAV) 
{
	D3D11_BUFFER_DESC bufDes;
	bufDes.ByteWidth = sizeOfElement * numberOfElements;
	bufDes.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	bufDes.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
	if (isUAV)
	{
		bufDes.BindFlags |= D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS;
	}
	bufDes.CPUAccessFlags = 0;
	bufDes.MiscFlags = 0;
	//MIGHT_NEED: bufDes.StructureByteStride = ?
	pDevice->CreateBuffer(&bufDes, NULL, &(result->textureBuffer));

	//TODO: Create SRV Desc
	/*D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = tex.Format;
	srvDesc.Texture2D.MipLevels = tex.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	m_device->CreateShaderResourceView(m_textures[i].Get(), &srvDesc, cbvSrvHandle);*/
	pDevice->CreateSrv(result->textureBuffer, NULL, &(result->srv));
	if (isUAV)
	{
		//TODO: Create UAVs
	}
}

EI_Resource* myCreateReadWriteSB(EI_Device* pDevice, AMD::uint32 structSize, AMD::uint32 structCount, EI_StringHash resourceName, EI_StringHash objectName)
{
	EI_Resource* result = new EI_Resource;
	CreateShaderBuffer(result, pDevice, structSize, structCount, resourceName, objectName, true);
	return result;
}

EI_Resource* myCreateReadOnlySB(EI_Device* pDevice, AMD::uint32 structSize, AMD::uint32 structCount, EI_StringHash resourceName, EI_StringHash objectName) 
{
	EI_Resource* result = new EI_Resource;
	CreateShaderBuffer(result, pDevice, structSize, structCount, resourceName, objectName, false);
	return result;
}

EI_BindSet* myCreateBindSet(EI_Device* device, AMD::TressFXBindSet& pBindSet)
{
	EI_BindSet* bindSet = new EI_BindSet;

	bindSet->nSRVs = pBindSet.nSRVs;
	bindSet->srvs = nullptr;
	bindSet->nUAVs = pBindSet.nUAVs;
	bindSet->srvs = nullptr;
	bindSet->values = pBindSet.values;
	bindSet->nBytes = pBindSet.nBytes;

	if (pBindSet.nSRVs > 0)
	{
		bindSet->srvs = new EI_SRV[pBindSet.nSRVs];
		memcpy(bindSet->srvs, pBindSet.srvs, sizeof(pBindSet.srvs[0]) * pBindSet.nSRVs);
	}


	if (pBindSet.nUAVs > 0)
	{
		bindSet->uavs = new EI_SRV[pBindSet.nUAVs];
		memcpy(bindSet->uavs, pBindSet.uavs, sizeof(pBindSet.uavs[0]) * pBindSet.nUAVs);
	}

	return bindSet;
}

void mySubmitBarriers(EI_CommandContextRef commands, int numBarriers, AMD::EI_Barrier* barriers)
{
	for (int i = 0; i < numBarriers; i++)
	{
		if (barriers->from == barriers->to)
		{
			if (barriers->from == AMD::EI_STATE_UAV)
			{
				D3D12_RESOURCE_UAV_BARRIER *bar = new D3D12_RESOURCE_UAV_BARRIER();
				bar->pResource = barriers->pResource->uav;
				barriers->pResource.
			}
		}
	}
}


int main()
{
	D3D_FEATURE_LEVEL levels[] = {
	D3D_FEATURE_LEVEL_9_1,
	D3D_FEATURE_LEVEL_9_2,
	D3D_FEATURE_LEVEL_9_3,
	D3D_FEATURE_LEVEL_10_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_11_1
	};

	// This flag adds support for surfaces with a color-channel ordering different
	// from the API default. It is required for compatibility with Direct2D.
	UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(DEBUG) || defined(_DEBUG)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Create the Direct3D 11 API device object and a corresponding context.
	ID3D11Device *device;
	ID3D11DeviceContext *context;

	HRESULT hr = D3D11CreateDevice(
		nullptr,                    // Specify nullptr to use the default adapter.
		D3D_DRIVER_TYPE_HARDWARE,   // Create a device using the hardware graphics driver.
		0,                          // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
		deviceFlags,                // Set debug and Direct2D compatibility flags.
		0,							// List of feature levels this app can support.
		0,							// Size of the list above.
		D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
		&device,                    // Returns the Direct3D device created.
		levels,						// Returns feature level of device created.
		&context                    // Returns the device immediate context.
	);

	if (FAILED(hr))
	{
		// Handle device interface creation failure if it occurs.
		// For example, reduce the feature level requirement, or fail over 
		// to WARP rendering.
		throw new std::invalid_argument("Faild to get device.");
	}

	// Store pointers to the Direct3D 11.1 API device and immediate context.
	/*device.As(&m_pd3dDevice);
	context.As(&m_pd3dDeviceContext);*/

	EI_Device pDevice = EI_Device(device);
	//DestroyAllLayouts(pDevice);

	AMD::g_Callbacks.pfMalloc = malloc;
	AMD::g_Callbacks.pfFree = free;

	AMD::g_Callbacks.pfError = myError;

	AMD::g_Callbacks.pfRead = myRead;
	AMD::g_Callbacks.pfSeek = mySeek;

	AMD::g_Callbacks.pfCreateReadOnlySB = myCreateReadOnlySB;
	AMD::g_Callbacks.pfCreateReadWriteSB = myCreateReadWriteSB;

	AMD::g_Callbacks.pfCreateBindSet = myCreateBindSet;

	char _buf[1024];
	_getcwd(_buf, sizeof(_buf));
	std::cout << _buf << std::endl;

	FILE* ratboy = fopen(".\\Assets\\Ratboy\\Ratboy_mohawk.tfx", "rb");
	
	AMD::TressFXAsset asset;
	asset.LoadHairData(ratboy);
	fclose(ratboy);
	asset.ProcessAsset();

	TressFXHairObject hair;
	hair.Create(&asset, &pDevice, (EI_CommandContextRef)context, "myHair", nullptr);
}

