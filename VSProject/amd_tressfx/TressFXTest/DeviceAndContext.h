#pragma once

#include "d3d11.h"
#include "Utils.h"
#include "TressFXAsset.h"
#include "My_EI_CommandContext.h"
#include "My_EI_Device.h"

class DeviceAndContext
{
public:
	EI_Device *device;
	EI_CommandContext *context;

	EI_Device& GetDevice() { return *device; }
	EI_CommandContext GetContext() { return *context; }

	DeviceAndContext()
	{
		device = new EI_Device();
		context = new EI_CommandContext();

		D3D_FEATURE_LEVEL level;

		// This flag adds support for surfaces with a color-channel ordering different
		// from the API default. It is required for compatibility with Direct2D.
		UINT deviceFlags = 0;

		//#define DEBUG
#if defined(DEBUG) || defined(_DEBUG)
		deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT hr = D3D11CreateDevice(
			nullptr,                    // Specify nullptr to use the default adapter.
			D3D_DRIVER_TYPE_HARDWARE,   // Create a device using the hardware graphics driver.
			0,                          // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
			deviceFlags,                // Set debug and Direct2D compatibility flags.
			0,							// List of feature levels this app can support.
			0,							// Size of the list above.
			D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
			&(device->device),                    // Returns the Direct3D device created.
			&level,						// Returns feature level of device created.
			&(context->context)                    // Returns the device immediate context.
		);

		if (FAILED(hr))
		{
			// Handle device interface creation failure if it occurs.
			// For example, reduce the feature level requirement, or fail over 
			// to WARP rendering.
			throw new std::invalid_argument("Failed to get device.");
		}
	}
};