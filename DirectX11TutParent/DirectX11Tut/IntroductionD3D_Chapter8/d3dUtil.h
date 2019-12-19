//***************************************************************************************
// d3dUtil.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#pragma once

#ifndef D3DUTIL_H
#define D3DUTIL_H

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
 
#include "d3dx11Effect.h"
#include <cassert>
#include <ctime>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include "MathHelper.h"
#include "LightHelper.h"
#include <directxmath.h>
#include <DirectXPackedVector.h>
#include "DDSTextureLoader.h"
#include <iostream>

using namespace DirectX;
using namespace PackedVector;

//---------------------------------------------------------------------------------------
// Simple d3d error checker for book demos.
//---------------------------------------------------------------------------------------


#if defined(DEBUG) | defined(_DEBUG)


	#ifndef HR
	#define HR(x)                                              \
	{														   \
		HRESULT hr = (x);									   \
		if(FAILED(hr))										   \
		{													   \
			std::string text(std::string("Error in file ") + std::string(__FILE__) + std::string(" at line ") + std::to_string(__LINE__));  \
			std::string s = std::system_category().message(hr); \
			text += std::string("\n\n") + s;\
			int len;														\
			int slength = (int)text.length() + 1;								\
			len = MultiByteToWideChar(CP_ACP, 0, text.c_str(), slength, 0, 0);	\
			wchar_t* buf = new wchar_t[len];								\
			MultiByteToWideChar(CP_ACP, 0, text.c_str(), slength, buf, len);	\
			std::wstring r(buf);											\
			delete[] buf;													\
			MessageBoxW(GetForegroundWindow(), r.c_str() , L"Unexpected Error", MB_YESNO | MB_ICONERROR); \
			DebugBreak();										\
		}													   \
		/*									   \
		const size_t size = strlen(__FILE__)+1;				   \
		size_t convertedChars;								   \
		WCHAR *wc = new WCHAR[size];						   \
		mbstowcs_s(&convertedChars, wc, size, __FILE__, size); \
		if(FAILED(hr))                                         \
		{                                                      \
			DXTraceW(wc, (DWORD)__LINE__, hr, L#x, true);	   \
		}                                                      \
*/															   \
	}
	#endif

#else
	#ifndef HR
	#define HR(x) (x)
	#endif
#endif 


//---------------------------------------------------------------------------------------
// Convenience macro for releasing COM objects.
//---------------------------------------------------------------------------------------

#define ReleaseCOM(x) { if(x){ x->Release(); x = 0; } }

//---------------------------------------------------------------------------------------
// Convenience macro for deleting objects.
//---------------------------------------------------------------------------------------

#define SafeDelete(x) { delete x; x = 0; }

//---------------------------------------------------------------------------------------
// Utility classes.
//---------------------------------------------------------------------------------------

class d3dHelper
{
public:
	///<summary>
	/// 
	/// Does not work with compressed formats.
	///</summary>
	static ID3D11ShaderResourceView* CreateTexture2DArraySRV(
		ID3D11Device* device, ID3D11DeviceContext* context,
		std::vector<std::wstring>& filenames,
		DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN,
		UINT filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR, 
		UINT mipFilter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR);

	static ID3D11ShaderResourceView* CreateRandomTexture1DSRV(ID3D11Device* device);
};

class TextHelper
{
public:

	template<typename T>
	static std::wstring ToString(const T& s)
	{
		std::wostringstream oss;
		oss << s;

		return oss.str();
	}

	template<typename T>
	static T FromString(const std::wstring& s)
	{
		T x;
		std::wistringstream iss(s);
		iss >> x;

		return x;
	}
};

// Order: left, right, bottom, top, near, far.
void ExtractFrustumPlanes(XMFLOAT4 planes[6], CXMMATRIX M);


// #define XMGLOBALCONST extern CONST __declspec(selectany)
//   1. extern so there is only one copy of the variable, and not a separate
//      private copy in each .obj.
//   2. __declspec(selectany) so that the compiler does not complain about
//      multiple definitions in a .cpp file (it can pick anyone and discard 
//      the rest because they are constant--all the same).

namespace Colors
{
	XMGLOBALCONST XMVECTORF32 White     = {1.0f, 1.0f, 1.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Black     = {0.0f, 0.0f, 0.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Red       = {1.0f, 0.0f, 0.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Green     = {0.0f, 1.0f, 0.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Blue      = {0.0f, 0.0f, 1.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Yellow    = {1.0f, 1.0f, 0.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Cyan      = {0.0f, 1.0f, 1.0f, 1.0f};
	XMGLOBALCONST XMVECTORF32 Magenta   = {1.0f, 0.0f, 1.0f, 1.0f};

	XMGLOBALCONST XMVECTORF32 Silver    = {0.75f, 0.75f, 0.75f, 1.0f};
	XMGLOBALCONST XMVECTORF32 LightSteelBlue = {0.69f, 0.77f, 0.87f, 1.0f};
}

///<summary>
/// Utility class for converting between types and formats.
///</summary>
class Convert
{
public:
	///<summary>
	/// Converts XMVECTOR to XMCOLOR, where XMVECTOR represents a color.
	///</summary>
	static XMCOLOR ToXmColor(FXMVECTOR v)
	{
		XMCOLOR dest;
		XMStoreColor(&dest, v);
		return dest;
	}

	///<summary>
	/// Converts XMVECTOR to XMFLOAT4, where XMVECTOR represents a color.
	///</summary>
	static XMFLOAT4 ToXmFloat4(FXMVECTOR v)
	{
		XMFLOAT4 dest;
		XMStoreFloat4(&dest, v);
		return dest;
	}

	static UINT ArgbToAbgr(UINT argb)
	{
		BYTE A = (argb >> 24) & 0xff;
		BYTE R = (argb >> 16) & 0xff;
		BYTE G = (argb >>  8) & 0xff;
		BYTE B = (argb >>  0) & 0xff;

		return (A << 24) | (B << 16) | (G << 8) | (R << 0);
	}

};


#endif // D3DUTIL_H