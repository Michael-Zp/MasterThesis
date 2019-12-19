//--------------------------------------------------------------------------------------
// File: ShaderCacheSampleHelper.h
//
// Helpers to implement the DXUT related ShaderCache interface in samples.
//
// Author: Layla Mah
// 
// Copyright © AMD Corporation. All rights reserved.
//--------------------------------------------------------------------------------------


#pragma once

class CDXUTControl;
class CDXUTTextHelper;
typedef long HRESULT;
typedef unsigned int UINT;

namespace AMD
{

class HUD;
class ShaderCache;

typedef HRESULT (*AddShadersApplicationCallback)(void);
extern AddShadersApplicationCallback g_ApplicationAddShadersCallback;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
extern const int	g_MaxApplicationControlID;
	
enum 
{
												AMD_IDC_START = 0,
	AMD_IDC_BUTTON_SHOW_SHADERCACHE_UI		 =	AMD_IDC_START,
	AMD_IDC_BUTTON_RECOMPILESHADERS_CHANGED,
	AMD_IDC_BUTTON_RECOMPILESHADERS_GLOBAL,
	AMD_IDC_BUTTON_RECREATE_SHADERS,
	AMD_IDC_CHECKBOX_AUTORECOMPILE_SHADERS,
	AMD_IDC_CHECKBOX_SHOW_SHADER_ERRORS,
	AMD_IDC_CHECKBOX_SHOW_ISA_GPR_PRESSURE,
	AMD_IDC_STATIC_TARGET_ISA,
	AMD_IDC_STATIC_TARGET_ISA_INFO,
	AMD_IDC_COMBOBOX_TARGET_ISA,
												AMD_IDC_END
};

template< typename T >
T GetEnum( T i_AMDEnum )
{
	return static_cast< T > ( g_MaxApplicationControlID + i_AMDEnum );
}

void InitApp( ShaderCache& r_ShaderCache, HUD& r_HUD, int& iY, const bool i_bAdvancedShaderCacheGUI_VisibleByDefault = false );
void ProcessUIChanges();
void RenderHUDUpdates( CDXUTTextHelper* i_pTxtHelper );
void __stdcall OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );

}; // namespace AMD

//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------
