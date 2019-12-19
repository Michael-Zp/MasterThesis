//--------------------------------------------------------------------------------------
// File: main.cpp
//
// Starting point for the TressFX sample. 
// 
// Copyright © AMD Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "..\\DXUT\\Core\\DXUT.h"
#include "..\\DXUT\\Optional\\DXUTcamera.h"
#include "..\\DXUT\\Optional\\DXUTgui.h"
#include "..\\DXUT\\Optional\\DXUTsettingsDlg.h"
#include "..\\DXUT\\Optional\\SDKmisc.h"
#include "..\\DXUT\\Optional\\SDKMesh.h"

#include "resource.h"

#include "TressFXSimulation.h"
#include "TressFXRender.h"
#include "SceneRender.h"
#include "..\\AMD_SDK\\AMD_SDK.h"
#include "..\\AMD_SDK\\ShaderCacheSampleHelper.h"

//prevent warnings from system headers about deprecated functions
#pragma warning(disable: 4995)
#include "strsafe.h"

const float MATH_PI = 3.14159265359f;

#define NEAR_PLANE    (10.f)
#define FAR_PLANE     (5000.f)
#define FIELD_OF_VIEW (MATH_PI / 4.f)

// Hair LOD ranges
#define MIN_LOD_DISTANCE 400
#define MAX_LOD_DISTANCE 1500
#define MIN_HAIR_DENSITY 0.3f
#define MAX_HAIR_DENSITY 1
#define MIN_HAIR_RADIUS 0.14f
#define MAX_HAIR_RADIUS 1.0f
#define MAX_FRAGMENTS 768
#define MIN_FRAGMENTS 8
#define MAX_ALPHA_SM 0.03f

// maximum strand copies
#define MAX_STRAND_COPIES 3

#define AVERAGE_FRAME_COUNT 200


//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
// DXUT resources
CDXUTDialogResourceManager          g_DialogResourceManager;    // Manager for shared resources of dialogs
CModelViewerCamera                  g_Camera;
CModelViewerCamera					g_CameraLight0;
CD3DSettingsDlg                     g_D3DSettingsDlg;           // Device settings dialog
CDXUTTextHelper*                    g_pTxtHelper = NULL;

int g_ScreenWidth  = 1024;
int g_ScreenHeight = 768; 

CTressFXRender		g_TressFXRender;		// Hair rendering class
CTressFXSimulation	g_TressFXSimulation;	// Hair simulation class
CTressFXMesh		g_TressFXMesh;			// Hair asset class
CSceneRender		g_SceneRender;			// Scene rendering class

HAIR_PARAMS			g_HairParams;
D3DXVECTOR3			g_HairColor;

// Manipulate model
D3DXVECTOR3 g_ModelCenter(0, 50, 0);
D3DXMATRIX g_ModelTransform;
D3DXMATRIX g_ModelTransformForHead;

bool g_bModelTransformUpdated = false;

// toggle UI
bool g_bShowUI    = true;

// toggle on-screen instructions
bool g_bShowControls = false;

// Adaptive LOD
bool g_bEnableLOD = false;
float g_savedDensity;
float g_savedRadius;
int g_savedMaxFragments;
float g_savedAlphaSM;
float g_SavedTransparency;

static COLORREF g_vCustHairColors[16] =
{
	RGB( 112,  84,  48 ),	//1 brown
	RGB( 100,  68,  55 ),	//2 dark purple brown
    RGB(  59,  48,  36 ),	//3 darkest brown
    RGB(  78,  67,  63 ),	//4 Med dark brown
    RGB(  80,  68,  68 ),	//5 chestnut brown
    RGB( 106,  78,  66 ),	//6 lt chestnut brown
    RGB(  85,  72,  56 ),	//7 dark golden brown
    RGB( 167, 133, 106 ),	//8 light golden brown 
    RGB( 184, 151, 120 ),	//9 dark honey blonde
    RGB( 220, 208, 186 ),	//10 bleached blonde
    RGB( 222, 188, 153 ),	//11 light ash blonde 
    RGB( 151, 121,  97 ),	//12 med ash brown 
    RGB( 230, 206, 168 ),	//13 lightest blonde 
    RGB( 229, 200, 168 ),	//14 pale golden blonde
    RGB( 165, 107,  70 ),	//15 strawberry blonde
    RGB( 145,  85,  61 ),	//16 light aubum
};

const D3DXVECTOR3			g_vTranslation = D3DXVECTOR3(0.f, 0.f, 0.f);
const D3DXVECTOR3			g_vRotation = D3DXVECTOR3(0.f, 0.f, 0.f);
const float					g_fScale = 1.f;

static const D3DXVECTOR3	g_vHairKa = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
static const D3DXVECTOR3	g_vHairKd = D3DXVECTOR3(0.4f, 0.0f, 2.0f);
static const D3DXVECTOR3	g_vHairKs = D3DXVECTOR3(0.14f, 0.0f, 2.0f);
static const D3DXVECTOR3	g_vHairEx = D3DXVECTOR3(80.f, 1.f, 100.f);
static const D3DXVECTOR3	g_vHairKs2 = D3DXVECTOR3(0.5f, 0.0f, 2.0f);
static const D3DXVECTOR3	g_vHairEx2 = D3DXVECTOR3(8.f, 1.f, 100.f);
static const D3DXVECTOR3	g_vHairDensity = D3DXVECTOR3(1.f, 0.f, 1.f);
static const D3DXVECTOR3	g_vHairAlpha = D3DXVECTOR3(0.33f, 0.f, 1.0f);
static const D3DXVECTOR3	g_vHairAlphaSM = D3DXVECTOR3(0.004f, 0.f, 0.03f);
static const D3DXVECTOR3	g_vFiberRadius = D3DXVECTOR3(0.14f, 0.02f, 0.2f); // in mm
static const D3DXVECTOR3	g_vFiberSpacing = D3DXVECTOR3(0.3f, 0.1f, 2.f); // 1 fiber every 15 x fiber-radius distance

const D3DXVECTOR3			g_vCameraPosition = D3DXVECTOR3(190.0f, 70.0f, 250.0f);
const D3DXVECTOR3			g_vCameraAt = D3DXVECTOR3(0, 40, 0);
const D3DXVECTOR3			g_vLightPosition = D3DXVECTOR3(421.25043f, 306.7890949f, 343.22232f);
const D3DXVECTOR3			g_vLightAt = D3DXVECTOR3(0.f, 0.f, 0.f);

typedef struct 
{
	float effect;
	float simulation;
	float generateShadow;
	float renderScene;
	float renderHair;
	float aBuffer;
	float kBuffer;
} timerData;

timerData g_timerResults[AVERAGE_FRAME_COUNT];

unsigned g_frameCount;

//--------------------------------------------------------------------------------------
// AMD helper classes defined here
//--------------------------------------------------------------------------------------
static AMD::ShaderCache     g_ShaderCache;
static AMD::MagnifyTool     g_MagnifyTool;
static AMD::HUD             g_SimulationHUD;
static AMD::HUD             g_RenderHUD;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
enum 
{
	IDC_TOGGLEFULLSCREEN = 1,					
	IDC_TOGGLEREF,								
	IDC_CHANGEDEVICE,						
	IDC_CHECKBOX_SIMULATE,						
	IDC_CHECKBOX_COLLISION,						
	IDC_STATIC_HAIR_DAMPING,				
	IDC_SLIDER_HAIR_DAMPING,				
	IDC_STATIC_HAIR_LOCAL_STIFFNESS,		
	IDC_SLIDER_HAIR_LOCAL_STIFFNESS,		
	IDC_STATIC_HAIR_GLOBAL_STIFFNESS,		
	IDC_SLIDER_HAIR_GLOBAL_STIFFNESS,		
	IDC_STATIC_GLOBAL_SHAPE_MATCHING_EFFECT_RANGE,
	IDC_SLIDER_GLOBAL_SHAPE_MATCHING_EFFECT_RANGE,
	IDC_STATIC_NUM_LENGTH_CONSTRAINT_ITERATIONS,	
	IDC_SLIDER_NUM_LENGTH_CONSTRAINT_ITERATIONS,	
	IDC_STATIC_NUM_LOCAL_SHAPE_MATCHING_ITERATIONS, 
	IDC_SLIDER_NUM_LOCAL_SHAPE_MATCHING_ITERATIONS,
	IDC_OPACITY,		        
	IDC_OPACITY_STATIC,   
	IDC_CHECKBOX_TRANSPARENCY,
	IDC_OPACITYSM,		        
	IDC_OPACITYSM_STATIC,      
	IDC_FIBERRADIUS,		    
	IDC_FIBERRADIUS_STATIC,  
	IDC_DENSITY,				
	IDC_DENSITY_STATIC,		
	IDC_CHECKBOX_SM,			
	IDC_CHECKBOX_THINTIP,
	IDC_CHECKBOX_LOD,
	IDC_WIND_MAGNITUDE,       
	IDC_WIND_MAGNITUDE_STATIC,
	IDC_CHECKBOX_USE_COVERAGE,
	IDC_STATIC_STRAND_COPIES,
	IDC_BUTTON_STRAND_COPIES_PLUS,
	IDC_BUTTON_STRAND_COPIES_MINUS,
	IDC_STATIC_STRAND_COPIES_VALUE,
	IDC_COMBOBOX_HAIR_SECTION,
	IDC_RENDER_MENU,
	IDC_SIMULATION_MENU,
	IDC_BUTTON_HAIR_COLOR,
	IDC_NUM_CONTROL_IDS // THIS ONE SHOULD ALWAYS BE LAST!!!!!
};

const int AMD::g_MaxApplicationControlID = IDC_NUM_CONTROL_IDS;


#define FRONT_HAIR_INDEX			0x00000000
#define TOP_SIDE_BACK_HAIR_INDEX	0x00000001
#define PONYTAIL_HAIR_INDEX			0x00000002

#define NONE_SHADOW_INDEX			0x00000000
#define SDSM_SHADOW_INDEX			0x00000002

// for finding D3D memory leaks
//#define MEM_DEBUG

#ifdef MEM_DEBUG
ID3D11Device*						g_pMemDebugDevice;
#endif


//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext );
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo, 
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext );
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, 
                                      void* pUserContext );
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain, 
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext );
void CALLBACK OnD3D11DestroyDevice( void* pUserContext );
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, 
                                  double fTime, float fElapsedTime, void* pUserContext );

void InitApp();
void RenderText();

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // Set DXUT callbacks
    DXUTSetCallbackMsgProc( MsgProc );
	DXUTSetCallbackKeyboard( OnKeyboard );
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

    DXUTSetCallbackD3D11DeviceAcceptable( IsD3D11DeviceAcceptable );
    DXUTSetCallbackD3D11DeviceCreated( OnD3D11CreateDevice );
    DXUTSetCallbackD3D11SwapChainResized( OnD3D11ResizedSwapChain );
    DXUTSetCallbackD3D11SwapChainReleasing( OnD3D11ReleasingSwapChain );
    DXUTSetCallbackD3D11DeviceDestroyed( OnD3D11DestroyDevice );
    DXUTSetCallbackD3D11FrameRender( OnD3D11FrameRender );

    InitApp();
    DXUTInit( true, true );
    DXUTSetCursorSettings( true, true );
    DXUTCreateWindow( L"TressFX11 v2.0" );

	// Require D3D_FEATURE_LEVEL_11_0
    DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, true, g_ScreenWidth, g_ScreenHeight);

    DXUTMainLoop(); // Enter into the DXUT render loop

	// Ensure the ShaderCache aborts if in a lengthy generation process
	g_ShaderCache.Abort();

	return DXUTGetExitCode();
}

//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{    
    D3DCOLOR DlgColor = 0x88888888; // Semi-transparent background for the dialog
    WCHAR szMessage[100];   
	szMessage[0] = 0;

    // Initialize dialogs
    g_D3DSettingsDlg.Init( &g_DialogResourceManager );

    g_RenderHUD.m_GUI.Init( &g_DialogResourceManager );
    g_RenderHUD.m_GUI.SetBackgroundColors( DlgColor );
    g_RenderHUD.m_GUI.SetCallback( OnGUIEvent );
	g_RenderHUD.m_GUI.SetVisible(true);

	g_SimulationHUD.m_GUI.Init( &g_DialogResourceManager );
    g_SimulationHUD.m_GUI.SetBackgroundColors( DlgColor );
    g_SimulationHUD.m_GUI.SetCallback( OnGUIEvent );
	g_SimulationHUD.m_GUI.SetVisible(false);

	int iX = AMD::HUD::iElementOffset / 2;
    int iY = AMD::HUD::iElementDelta / 2;

    g_RenderHUD.m_GUI.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", AMD::HUD::iElementOffset, 
		iY, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );
    g_RenderHUD.m_GUI.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", AMD::HUD::iElementOffset, 
		iY += AMD::HUD::iElementDelta,AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, VK_F3 );
    g_RenderHUD.m_GUI.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", AMD::HUD::iElementOffset, 
		iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, VK_F2 );
		
	AMD::InitApp( g_ShaderCache, g_RenderHUD, iY );

	iY += AMD::HUD::iElementDelta/2;

	//------------
    // Hair Rendering Parameters
    //------------	
    g_HairParams.Ka = g_vHairKa.x;
    g_HairParams.Kd = g_vHairKd.x;
    g_HairParams.Ks1 = g_vHairKs.x;
    g_HairParams.Ex1 = g_vHairEx.x;
    g_HairParams.Ks2 = g_vHairKs2.x;
    g_HairParams.Ex2 = g_vHairEx2.x;

    g_HairParams.alpha = g_vHairAlpha.x;
    g_HairParams.alpha_sm = g_vHairAlphaSM.x;
	g_HairParams.bTransparency = true;

    g_HairParams.radius = g_vFiberRadius.x;
    g_HairParams.density = g_vHairDensity.x;
    g_HairParams.fiber_spacing = g_vFiberSpacing.x;

	g_HairParams.color.x = ((g_vCustHairColors[14]>> 0) & 0xFF)/255.f;
	g_HairParams.color.y = ((g_vCustHairColors[14]>> 8) & 0xFF)/255.f;
	g_HairParams.color.z = ((g_vCustHairColors[14]>>16) & 0xFF)/255.f;
	g_HairColor = g_HairParams.color;

    g_HairParams.tech_shadow = SDSM_SHADOW_INDEX;
    g_HairParams.bSimulation = true;
    g_HairParams.bThinTip = true;
	g_HairParams.bUseCoverage = true;
	
	g_HairParams.iStrandCopies = 1;

	g_HairParams.iMaxFragments = MAX_FRAGMENTS;

	g_RenderHUD.m_GUI.AddCheckBox(IDC_CHECKBOX_USE_COVERAGE, L"Strand Antialiasing", iX, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, g_HairParams.bUseCoverage);

	g_RenderHUD.m_GUI.AddCheckBox(IDC_CHECKBOX_TRANSPARENCY, NULL, iX, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, g_HairParams.bTransparency);
	swprintf(szMessage, L" Transparency:%.4f", g_HairParams.alpha); szMessage[99] = 0;
    g_RenderHUD.m_GUI.AddStatic( IDC_OPACITY_STATIC, szMessage, iX + AMD::HUD::iElementDelta, iY, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );
    g_RenderHUD.m_GUI.AddSlider( IDC_OPACITY, iX, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, 0, 100, (int)(100 * (g_HairParams.alpha - g_vHairAlpha.y) / (g_vHairAlpha.z - g_vHairAlpha.y)) );
 
	g_RenderHUD.m_GUI.AddCheckBox(IDC_CHECKBOX_SM, NULL, iX, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, g_HairParams.tech_shadow != NONE_SHADOW_INDEX);
	swprintf(szMessage, L" Self-shadowing:%.4f", g_HairParams.alpha_sm); szMessage[99] = 0;
    g_RenderHUD.m_GUI.AddStatic( IDC_OPACITYSM_STATIC, szMessage, iX + AMD::HUD::iElementDelta, iY, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );
    g_RenderHUD.m_GUI.AddSlider( IDC_OPACITYSM, iX, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, 0, 100, (int)(100 * (g_HairParams.alpha_sm - g_vHairAlphaSM.y) / (g_vHairAlphaSM.z - g_vHairAlphaSM.y)) );

	iY += AMD::HUD::iElementDelta/2;

	swprintf(szMessage, L"Density:%.4f", g_HairParams.density); szMessage[99] = 0;
    g_RenderHUD.m_GUI.AddStatic( IDC_DENSITY_STATIC, szMessage, iX, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );
    g_RenderHUD.m_GUI.AddSlider( IDC_DENSITY, iX, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, 0, 100, (int)(100 * (g_HairParams.density - g_vHairDensity.y) / (g_vHairDensity.z - g_vHairDensity.y)) );

    swprintf(szMessage, L"Hair thickness:%.4f", g_HairParams.radius); szMessage[99] = 0;
    g_RenderHUD.m_GUI.AddStatic( IDC_FIBERRADIUS_STATIC, szMessage, iX, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );
    g_RenderHUD.m_GUI.AddSlider( IDC_FIBERRADIUS, iX, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, 0, 200, (int)(100 * (g_HairParams.radius - g_vFiberRadius.y) / (g_vFiberRadius.z - g_vFiberRadius.y)) );

  	g_RenderHUD.m_GUI.AddCheckBox(IDC_CHECKBOX_LOD, L"Distance adaptive LOD", iX, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, g_bEnableLOD);

	iY += AMD::HUD::iElementDelta/2;

	int iSavedX = iX;
	swprintf(szMessage, L"Strand Copies: ");
	g_RenderHUD.m_GUI.AddStatic( IDC_STATIC_STRAND_COPIES, szMessage, iX, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight);	
	g_RenderHUD.m_GUI.AddButton( IDC_BUTTON_STRAND_COPIES_MINUS, L"-", iX += 120, iY, AMD::HUD::iElementDelta, AMD::HUD::iElementHeight);
	swprintf(szMessage, L" %d", g_HairParams.iStrandCopies);
	g_RenderHUD.m_GUI.AddStatic( IDC_STATIC_STRAND_COPIES_VALUE, szMessage, iX += AMD::HUD::iElementDelta, iY, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight);	
	g_RenderHUD.m_GUI.AddButton( IDC_BUTTON_STRAND_COPIES_PLUS, L"+", iX += AMD::HUD::iElementDelta, iY, AMD::HUD::iElementDelta, AMD::HUD::iElementHeight);
	iX = iSavedX;

	g_RenderHUD.m_GUI.AddCheckBox(IDC_CHECKBOX_THINTIP, L"Thin Tip", iX, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, g_HairParams.bThinTip);

	iY += AMD::HUD::iElementDelta/2;

	g_RenderHUD.m_GUI.AddButton( IDC_BUTTON_HAIR_COLOR, L"Change Hair Color", iX + 20, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );

	iY += AMD::HUD::iElementDelta/2;

	// Add the magnify tool UI to our HUD
    g_MagnifyTool.InitApp( &g_RenderHUD.m_GUI, iY += AMD::HUD::iElementDelta );

	iY += AMD::HUD::iElementDelta / 2;

	g_RenderHUD.m_GUI.AddButton( IDC_SIMULATION_MENU, L"Simulation Menu", iX + 20, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );
	unsigned int iSimButtonY = iY;

   //------------
    // Hair Simulation Parameters
    //------------	
	iX = AMD::HUD::iElementOffset / 2;
    iY = AMD::HUD::iElementDelta;

	g_HairParams.wind_magnitude = 0.f;
    g_HairParams.wind_direction.m_X = 1;
    g_HairParams.wind_direction.m_Y = 0;
    g_HairParams.wind_direction.m_Z = 0;
	g_HairParams.targetFrameRateForSimulation = 1.f/60.f;

    g_SimulationHUD.m_GUI.AddCheckBox( IDC_CHECKBOX_SIMULATE, L"(S)imulate", iX, iY, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, g_HairParams.bSimulation, 'S');
	g_SimulationHUD.m_GUI.AddCheckBox( IDC_CHECKBOX_COLLISION, L"(C)ollision with head", iX, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, g_TressFXSimulation.m_bCollision, 'C');
    
    iY += AMD::HUD::iGroupDelta;

	CDXUTComboBox* pComboHairSection;
    g_SimulationHUD.m_GUI.AddComboBox( IDC_COMBOBOX_HAIR_SECTION, iX, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth+55, 
		AMD::HUD::iElementHeight, L'L', false, &pComboHairSection );

    if( pComboHairSection )
    {   
        pComboHairSection->SetDropHeight( 65 );
        pComboHairSection->SetScrollBarWidth(20);
        pComboHairSection->AddItem( L"Front hair",	( LPVOID )FRONT_HAIR_INDEX );
        pComboHairSection->AddItem( L"Top, side & back hair",	( LPVOID )TOP_SIDE_BACK_HAIR_INDEX );
        pComboHairSection->AddItem( L"Ponytail",	( LPVOID )PONYTAIL_HAIR_INDEX );
        pComboHairSection->SetSelectedByData( ( LPVOID )FRONT_HAIR_INDEX );
    }

	g_SimulationHUD.m_GUI.AddStatic( IDC_STATIC_HAIR_DAMPING, L"Damping", iX, iY += AMD::HUD::iElementDelta, 
		AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );
    g_SimulationHUD.m_GUI.AddSlider( IDC_SLIDER_HAIR_DAMPING, iX, iY += AMD::HUD::iElementDelta, 
		AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, 0, (int)(100.0f), (int)(100.0f*g_TressFXSimulation.m_shapeParams[3].damping), true );

    g_SimulationHUD.m_GUI.AddStatic( IDC_STATIC_HAIR_LOCAL_STIFFNESS, L"Local stiffness", iX, iY += AMD::HUD::iElementDelta, 
		AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );
    g_SimulationHUD.m_GUI.AddSlider( IDC_SLIDER_HAIR_LOCAL_STIFFNESS, iX, iY += AMD::HUD::iElementDelta, 
		AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, 0, (int)(100.0f), (int)(100.0f*g_TressFXSimulation.m_shapeParams[3].stiffnessForLocalShapeMatching), true );

    g_SimulationHUD.m_GUI.AddStatic( IDC_STATIC_HAIR_GLOBAL_STIFFNESS, L"Global shape stiffness", iX, iY += AMD::HUD::iElementDelta, 
		AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );
    g_SimulationHUD.m_GUI.AddSlider( IDC_SLIDER_HAIR_GLOBAL_STIFFNESS, iX, iY += AMD::HUD::iElementDelta, 
		AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, 0, (int)(100.0f), (int)(100.0f * g_TressFXSimulation.m_shapeParams[3].stiffnessForGlobalShapeMatching), true );

    g_SimulationHUD.m_GUI.AddStatic( IDC_STATIC_GLOBAL_SHAPE_MATCHING_EFFECT_RANGE, L"Global shape stiffness range", iX, iY += AMD::HUD::iElementDelta, 
		AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );
    g_SimulationHUD.m_GUI.AddSlider( IDC_SLIDER_GLOBAL_SHAPE_MATCHING_EFFECT_RANGE, iX, iY += AMD::HUD::iElementDelta, 
		AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, 0, (int)(100.0f), (int)(100.0f*g_TressFXSimulation.m_shapeParams[3].globalShapeMatchingEffectiveRange), true );

    iY += AMD::HUD::iGroupDelta;

    swprintf(szMessage, L"Length constraint\n iterations:%d", g_TressFXSimulation.m_numLengthConstraintIterations); 
    g_SimulationHUD.m_GUI.AddStatic( IDC_STATIC_NUM_LENGTH_CONSTRAINT_ITERATIONS, szMessage, iX, iY += AMD::HUD::iElementDelta, 
		AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );
    g_SimulationHUD.m_GUI.AddSlider( IDC_SLIDER_NUM_LENGTH_CONSTRAINT_ITERATIONS, iX, iY += AMD::HUD::iElementDelta * 2, 
		AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, 0, (int)(10.0f), g_TressFXSimulation.m_numLengthConstraintIterations, true );

	swprintf(szMessage, L"Local shape constraint\n iterations:%d", g_TressFXSimulation.m_numLocalShapeMatchingIterations); 
    g_SimulationHUD.m_GUI.AddStatic( IDC_STATIC_NUM_LOCAL_SHAPE_MATCHING_ITERATIONS, szMessage, iX, iY += AMD::HUD::iElementDelta, 
		AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );
    g_SimulationHUD.m_GUI.AddSlider( IDC_SLIDER_NUM_LOCAL_SHAPE_MATCHING_ITERATIONS, iX, iY += AMD::HUD::iElementDelta * 2, 
		AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, 0, (int)(20.0f), g_TressFXSimulation.m_numLocalShapeMatchingIterations, true );

    swprintf(szMessage, L"Wind Magnitude: %.2f", g_HairParams.wind_magnitude); 
    g_SimulationHUD.m_GUI.AddStatic( IDC_WIND_MAGNITUDE_STATIC, szMessage, iX, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight);
	g_SimulationHUD.m_GUI.AddSlider( IDC_WIND_MAGNITUDE, iX, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, -50, 50, 0 );

    iY += AMD::HUD::iGroupDelta;

	g_SimulationHUD.m_GUI.AddButton( IDC_RENDER_MENU, L"Render Menu", iX+20, iSimButtonY, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );


	// Setup the camera's view parameters
    g_Camera.SetEnablePositionMovement( true );
    D3DXVECTOR3 eyePos = g_vCameraPosition; 
    D3DXVECTOR3 at = g_vCameraAt;
    g_Camera.SetViewParams( &eyePos, &at );
    g_Camera.SetScalers(0.001f,20.f);

    D3DXVECTOR3 vecEye = g_vLightPosition;
    D3DXVECTOR3 vecAt = g_vLightAt;
    g_CameraLight0.SetViewParams(&vecEye, &vecAt);

	for (int i = 0; i < AVERAGE_FRAME_COUNT; i++)
	{
		g_timerResults[i].effect = 0;
		g_timerResults[i].simulation = 0;
		g_timerResults[i].generateShadow = 0;
		g_timerResults[i].renderScene = 0;
		g_timerResults[i].renderHair = 0;
		g_timerResults[i].aBuffer = 0;
		g_timerResults[i].kBuffer = 0;
		g_frameCount = 0;
	}
    
}


//--------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D10 device, allowing the app to modify the device settings as needed
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( ( DXUT_D3D11_DEVICE == pDeviceSettings->ver &&
              pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE ) )
        {
            DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
        }

        // Start with vsync disabled
        pDeviceSettings->d3d11.SyncInterval = 0;
    }

    // Disable MSAA
    pDeviceSettings->d3d11.sd.SampleDesc.Count = 1;
    pDeviceSettings->d3d11.sd.SampleDesc.Quality = 0;

	return true;
}


//--------------------------------------------------------------------------------------
// Handle updates to the scene
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    // Update the camera's position based on user input 
    g_Camera.FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text
//--------------------------------------------------------------------------------------
void RenderText()
{

    g_pTxtHelper->Begin();
    g_pTxtHelper->SetInsertionPos( 5, 5 );
    g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    g_pTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
    g_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );

	g_timerResults[g_frameCount].effect = (float)TIMER_GetTime( Gpu, L"Effect"          ) * 1000.0f;
	g_timerResults[g_frameCount].simulation = (float)TIMER_GetTime( Gpu, L"Effect|Simulation"      ) * 1000.0f;
	g_timerResults[g_frameCount].generateShadow = (float)TIMER_GetTime( Gpu, L"Effect|GenerateShadows" ) * 1000.0f;
	g_timerResults[g_frameCount].renderScene = (float)TIMER_GetTime( Gpu, L"Effect|RenderScene"     ) * 1000.0f;
	g_timerResults[g_frameCount].renderHair = (float)TIMER_GetTime( Gpu, L"Effect|RenderHair"      ) * 1000.0f;
	g_timerResults[g_frameCount].aBuffer = (float)TIMER_GetTime( Gpu, L"Effect|RenderHair|ABufferFill"       ) * 1000.0f;
	g_timerResults[g_frameCount].kBuffer = (float)TIMER_GetTime( Gpu, L"Effect|RenderHair|KBufferSortAndDraw") * 1000.0f;

	timerData movingAverage = {0};
	unsigned simZeroCount = 0;
	int simCount;

	for (int i=0; i < AVERAGE_FRAME_COUNT; i++)
	{
		movingAverage.effect += g_timerResults[i].effect; 
		// only count the non-zero simulation times, since the simulation may skip frames
		if (g_timerResults[i].simulation == 0.0f)
			simZeroCount++;
		movingAverage.simulation += g_timerResults[i].simulation;
		movingAverage.generateShadow += g_timerResults[i].generateShadow; 
		movingAverage.renderScene += g_timerResults[i].renderScene; 
		movingAverage.renderHair += g_timerResults[i].renderHair; 
		movingAverage.aBuffer += g_timerResults[i].aBuffer; 
		movingAverage.kBuffer += g_timerResults[i].kBuffer; 
	}
	movingAverage.effect /= AVERAGE_FRAME_COUNT;
	simCount = AVERAGE_FRAME_COUNT - simZeroCount;
	if (simCount <= 0)
		simCount = AVERAGE_FRAME_COUNT;
	movingAverage.simulation /= simCount;
	movingAverage.generateShadow /= AVERAGE_FRAME_COUNT;
	movingAverage.renderScene /= AVERAGE_FRAME_COUNT;
	movingAverage.renderHair /= AVERAGE_FRAME_COUNT;
	movingAverage.aBuffer /= AVERAGE_FRAME_COUNT;
	movingAverage.kBuffer /= AVERAGE_FRAME_COUNT;

	WCHAR wcbuf[256];
	swprintf_s( wcbuf, 256, L"Effect cost in milliseconds( Total = %.3f )", movingAverage.effect );
	g_pTxtHelper->DrawTextLine( wcbuf );
	swprintf_s( wcbuf, 256, L"  Simulation      (ms): %.3f", movingAverage.simulation );
	g_pTxtHelper->DrawTextLine( wcbuf );
	swprintf_s( wcbuf, 256, L"  GenerateShadows (ms): %.3f", movingAverage.generateShadow );
	g_pTxtHelper->DrawTextLine( wcbuf );
	swprintf_s( wcbuf, 256, L"  RenderScene     (ms): %.3f", movingAverage.renderScene );
	g_pTxtHelper->DrawTextLine( wcbuf );
	swprintf_s( wcbuf, 256, L"  RenderHair      (ms): %.3f", movingAverage.renderHair );
	g_pTxtHelper->DrawTextLine( wcbuf );
	swprintf_s( wcbuf, 256, L"    A-Buffer Fill      (ms): %.3f", movingAverage.aBuffer );
	g_pTxtHelper->DrawTextLine( wcbuf );
	swprintf_s( wcbuf, 256, L"    K-Buffer Sort&Draw (ms): %.3f", movingAverage.kBuffer );
	g_pTxtHelper->DrawTextLine( wcbuf );

	if (++g_frameCount >= AVERAGE_FRAME_COUNT)
	{
		g_frameCount = 0;
	}

	if (g_bShowControls)
	{
		g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 0.0f, 1.0f, 0.0f, 1.0f ) );
		g_pTxtHelper->SetInsertionPos( 5, DXUTGetDXGIBackBufferSurfaceDesc()->Height/2 );
		g_pTxtHelper->DrawTextLine( L"Rotate Camera   : Left Mouse Button" );
		g_pTxtHelper->DrawTextLine( L"Translate Model : Middle Mouse Button" );
		g_pTxtHelper->DrawTextLine( L"Rotate Model    : Ctrl + Middle Mouse Button" );
		g_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
	}

    g_pTxtHelper->SetInsertionPos( 5, DXUTGetDXGIBackBufferSurfaceDesc()->Height - (AMD::HUD::iElementDelta * 2) );
	g_pTxtHelper->DrawTextLine( L"Toggle GUI    : F1" );

	g_pTxtHelper->SetInsertionPos( 5, DXUTGetDXGIBackBufferSurfaceDesc()->Height - AMD::HUD::iElementDelta );
	if (g_bShowControls)
		g_pTxtHelper->DrawTextLine( L"Hide Controls : F4" );
	else
		g_pTxtHelper->DrawTextLine( L"Show Controls : F4" );

    g_pTxtHelper->End();
}

//--------------------------------------------------------------------------------------
// Calculate the screen coordinates from 3D coordinates
//--------------------------------------------------------------------------------------
void CalcScreenCoordinates(const D3DXVECTOR3& position, float& x, float& y)
{
    D3DXVECTOR3 screenCoord;
    D3DXMATRIX projection;
    D3DXMATRIX view;
    projection = *g_Camera.GetProjMatrix();
    view = *g_Camera.GetViewMatrix();
    D3DXMATRIX viewProjection;
    viewProjection = view * projection;

    D3DXVec3TransformCoord(&screenCoord, &position, &viewProjection);
    x = 0.5f * ( screenCoord.x + 1) * g_ScreenWidth;
    y = 0.5f * (-screenCoord.y + 1) * g_ScreenHeight;
}

//--------------------------------------------------------------------------------------
// Scale and normalize the coordinates
//--------------------------------------------------------------------------------------
D3DXVECTOR3 ScaleAndNormalize(float x, float y)
{
    float scale = 4;
    x = - x / (scale * g_ScreenWidth / 2);
    y =   y / (scale * g_ScreenHeight / 2);
    float z = 0;
    float mag = x * x + y * y;

    if ( mag > 1 ) 
    {
        float scale = 1.0f / sqrtf(mag);
        x *= scale;
        y *= scale;
    }
    else
        z = sqrtf(1.0f - mag);

    return D3DXVECTOR3(x, y, z);
}

//--------------------------------------------------------------------------------------
// Move Model
//--------------------------------------------------------------------------------------
void MoveModel(int msg, WPARAM wParam, LPARAM lParam)
{
    g_bModelTransformUpdated = false;

    static LPARAM g_LastMouse;
    static D3DXVECTOR3 g_LastPoint;
    static bool g_ModelIsMoved;
    static D3DXMATRIX mtxTfm, mtxInvTfm;
    static float g_ScreenCenterX, g_ScreenCenterY;
    
    int mouseX = (short)LOWORD(lParam);
    int mouseY = (short)HIWORD(lParam);
    switch (msg) {
		case WM_MBUTTONDOWN:
			{
				D3DXMATRIX mtxTranslation, mtxRotation;
				D3DXMatrixTranslation(&mtxTranslation, -g_ModelCenter.x, -g_ModelCenter.y, -g_ModelCenter.z);
				mtxRotation = *g_Camera.GetViewMatrix();
				for (int i = 0; i<3; i++)
				{
					mtxRotation.m[3][i] = 0;
				}
				mtxTfm = mtxTranslation * mtxRotation;
				D3DXMatrixInverse(&mtxInvTfm, 0, &mtxTfm);
			}

			CalcScreenCoordinates(g_ModelCenter, g_ScreenCenterX, g_ScreenCenterY);
			g_LastPoint = ScaleAndNormalize(mouseX - g_ScreenCenterX, mouseY - g_ScreenCenterY);
			g_LastMouse = lParam;
			g_ModelIsMoved = true;
			D3DXMatrixIdentity(&g_ModelTransform);
			break;

		case WM_MOUSEMOVE:
			if (g_ModelIsMoved) 
			{
				D3DXMATRIX transform;
				D3DXMatrixIdentity(&transform);
                
				if (wParam & MK_MBUTTON) // Translate
				{
					D3DXMatrixTranslation(&transform, (float)-(42 * ((short)LOWORD(g_LastMouse) - mouseX)) / g_ScreenWidth,
						(float)(42 * ((short)HIWORD(g_LastMouse) - mouseY)) / g_ScreenHeight, 0);
					g_LastMouse = lParam;
				}
                
				g_ModelTransform = mtxTfm * transform;
				g_ModelTransform = g_ModelTransform * mtxInvTfm;	

				bool bSimulate = g_SimulationHUD.m_GUI.GetCheckBox(IDC_CHECKBOX_SIMULATE)->GetChecked();

				if ( bSimulate )
					g_ModelTransformForHead = g_ModelTransformForHead * g_ModelTransform;

				g_bModelTransformUpdated = true;
			}
			break;
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
			{
				g_ModelIsMoved = false;
				D3DXMatrixIdentity(&g_ModelTransform);
				break;
			}
		default:
    break;
    }
}

//--------------------------------------------------------------------------------------
// Rotate Model
//--------------------------------------------------------------------------------------
void RotateModel(int msg, WPARAM wParam, LPARAM lParam)
{
    g_bModelTransformUpdated = false;

    static LPARAM g_LastMouse;
    static D3DXVECTOR3 g_LastPoint;
    static bool g_ModelIsMoved;
	static D3DXMATRIX mtxTfm, mtxInvTfm;
    static float g_ScreenCenterX, g_ScreenCenterY;
    
    int mouseX = (short)LOWORD(lParam);
    int mouseY = (short)HIWORD(lParam);
    switch (msg) {
		case WM_MBUTTONDOWN:
			{
				D3DXMATRIX mtxTranslation, mtxRotation;
				D3DXMatrixTranslation(&mtxTranslation, -g_ModelCenter.x, -g_ModelCenter.y, -g_ModelCenter.z);
				mtxRotation = *g_Camera.GetViewMatrix();
				for (int i = 0; i<3; i++)
				{
					mtxRotation.m[3][i] = 0;
				}
				mtxTfm = mtxTranslation * mtxRotation;
				D3DXMatrixInverse(&mtxInvTfm, 0, &mtxTfm);
			}

			CalcScreenCoordinates(g_ModelCenter, g_ScreenCenterX, g_ScreenCenterY);
			g_LastPoint = ScaleAndNormalize(mouseX - g_ScreenCenterX, mouseY - g_ScreenCenterY);
			g_LastMouse = lParam;
			g_ModelIsMoved = true;
			D3DXMatrixIdentity(&g_ModelTransform);
			break;

		case WM_MOUSEMOVE:
			if (g_ModelIsMoved) 
			{
				D3DXMATRIX transformX;
				D3DXMATRIX transformY;
				D3DXMatrixIdentity(&transformX);
				D3DXMatrixIdentity(&transformY);
                
				if (wParam & MK_MBUTTON) // Translate
				{
					D3DXMatrixRotationY( &transformX, (float)(42 * ((short)LOWORD(g_LastMouse) - mouseX)) / (g_ScreenWidth * 10) );
					D3DXMatrixRotationX( &transformY, (float)(42 * ((short)HIWORD(g_LastMouse) - mouseY)) / (g_ScreenHeight* 10) );
					g_LastMouse = lParam;
				}
        
				g_ModelTransform = mtxTfm * transformX * transformY;
				g_ModelTransform = g_ModelTransform * mtxInvTfm;

				bool bSimulate = g_SimulationHUD.m_GUI.GetCheckBox(IDC_CHECKBOX_SIMULATE)->GetChecked();

				if ( bSimulate )
					g_ModelTransformForHead = g_ModelTransformForHead * g_ModelTransform;

				g_bModelTransformUpdated = true;
			}
			break;
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
			{
				g_ModelIsMoved = false;
				D3DXMatrixIdentity(&g_ModelTransform);
				break;
			}
		default:
    break;
    }
}
//--------------------------------------------------------------------------------------
// Open a DialogBox to select the hair color
//--------------------------------------------------------------------------------------
void ChooseHairColor()
{
	CHOOSECOLOR cc;
	DWORD rgbCurrent = RGB((int)(g_HairColor.x*255), (int)(g_HairColor.y*255), (int)(g_HairColor.z*255) );

	// the color picker only works in windowed mode
	DXUTDeviceSettings deviceSettings = DXUTGetDeviceSettings();
	bool fullscreen = !deviceSettings.d3d11.sd.Windowed;
	if (fullscreen)
		DXUTToggleFullScreen();	

	ZeroMemory(&cc, sizeof(cc));
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = DXUTGetHWND();
	cc.lpCustColors = (LPDWORD) g_vCustHairColors;
	cc.rgbResult = rgbCurrent;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;
	if (ChooseColor(&cc)==TRUE) 
	{
		g_HairParams.color.x = ((cc.rgbResult>> 0) & 0xFF)/255.f;
		g_HairParams.color.y = ((cc.rgbResult>> 8) & 0xFF)/255.f;
		g_HairParams.color.z = ((cc.rgbResult>>16) & 0xFF)/255.f;
		g_HairColor = g_HairParams.color;
	}

	// restore full screen mode
	if (fullscreen)
		DXUTToggleFullScreen();	

}

//--------------------------------------------------------------------------------------
// Handle messages to the application
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    // Pass messages to dialog resource manager calls so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass messages to settings dialog if its active
    if( g_D3DSettingsDlg.IsActive() )
    {
        g_D3DSettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_RenderHUD.m_GUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    *pbNoFurtherProcessing = g_SimulationHUD.m_GUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
    
	if( DXUTIsKeyDown( VK_CONTROL ) )
		RotateModel(uMsg, wParam, lParam);
	else
		MoveModel(uMsg, wParam, lParam);

    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}


//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {     
            case VK_F1:     
                g_bShowUI = !g_bShowUI;
                break;
             case VK_F4:     
                g_bShowControls = !g_bShowControls;
                break;
       }
    }
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
    float value = 0;
    WCHAR szMessage[100];   
    szMessage[0] = 0;

    switch( nControlID )
    {
        // Standard DXUT controls
        case IDC_TOGGLEFULLSCREEN:  
            DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:         
            DXUTToggleREF();        break;
        case IDC_CHANGEDEVICE:      
            g_D3DSettingsDlg.SetActive( !g_D3DSettingsDlg.IsActive() ); break;

        case IDC_RENDER_MENU:
			g_SimulationHUD.m_GUI.SetVisible(false);
			g_RenderHUD.m_GUI.SetVisible(true);
			break;

        case IDC_SIMULATION_MENU:
			g_RenderHUD.m_GUI.SetVisible(false);
			g_SimulationHUD.m_GUI.SetVisible(true);
			break;

		case IDC_WIND_MAGNITUDE:
            value = (float)(((CDXUTSlider*)pControl)->GetValue());
            g_HairParams.wind_magnitude = value;
            swprintf(szMessage, L"Wind Mag: %.2f", g_HairParams.wind_magnitude); szMessage[99] = 0;
            g_SimulationHUD.m_GUI.GetStatic( IDC_WIND_MAGNITUDE_STATIC )->SetText( szMessage );
            break;

        case IDC_OPACITY:
            value = ((CDXUTSlider*)pControl)->GetValue()/100.0f;
            g_HairParams.alpha = g_vHairAlpha.y * (1-value) + g_vHairAlpha.z*value;
            swprintf(szMessage, L" Transparency:%.4f", g_HairParams.alpha); szMessage[99] = 0;
            g_RenderHUD.m_GUI.GetStatic( IDC_OPACITY_STATIC )->SetText( szMessage );
            break;

        case IDC_CHECKBOX_TRANSPARENCY:
			g_HairParams.bTransparency = (((CDXUTCheckBox*)pControl)->GetChecked());
			if (g_HairParams.bTransparency == false)
			{
				g_SavedTransparency =  g_HairParams.alpha;
				g_HairParams.alpha = 1.0;
				g_RenderHUD.m_GUI.GetSlider( IDC_OPACITY )->SetEnabled(false);
			}
			else
			{
				g_HairParams.alpha = g_SavedTransparency;
				g_RenderHUD.m_GUI.GetSlider( IDC_OPACITY )->SetEnabled(true);
			}
 			swprintf(szMessage, L" Transparency:%.4f", g_HairParams.alpha);
			g_RenderHUD.m_GUI.GetStatic( IDC_OPACITY_STATIC )->SetText( szMessage );
			g_RenderHUD.m_GUI.GetSlider( IDC_OPACITY )->SetValue((int)(100 * (g_HairParams.alpha - g_vHairAlpha.y) / (g_vHairAlpha.z - g_vHairAlpha.y)));
           break;

		case IDC_FIBERRADIUS:
            value = ((CDXUTSlider*)pControl)->GetValue()/100.0f;
            g_HairParams.radius = g_vFiberRadius.y * (1-value) + g_vFiberRadius.z*value;
            swprintf(szMessage, L"Hair thickness:%.4f", g_HairParams.radius); szMessage[99] = 0;
            g_RenderHUD.m_GUI.GetStatic( IDC_FIBERRADIUS_STATIC )->SetText( szMessage );
            break;

        case IDC_DENSITY:
            value = ((CDXUTSlider*)pControl)->GetValue()/100.0f;
            g_HairParams.density = g_vHairDensity.y * (1-value) + g_vHairDensity.z*value;
            swprintf(szMessage, L"Density:%.4f", g_HairParams.density); szMessage[99] = 0;
            g_RenderHUD.m_GUI.GetStatic( IDC_DENSITY_STATIC )->SetText( szMessage );
            break;

         case IDC_OPACITYSM:
            value = ((CDXUTSlider*)pControl)->GetValue()/100.0f;
            g_HairParams.alpha_sm = g_vHairAlphaSM.y * (1-value) + g_vHairAlphaSM.z*value;
            swprintf(szMessage, L" Self-shadowing:%.4f", g_HairParams.alpha_sm); szMessage[99] = 0;
            g_RenderHUD.m_GUI.GetStatic( IDC_OPACITYSM_STATIC )->SetText( szMessage );
            break;

        case IDC_SLIDER_NUM_LENGTH_CONSTRAINT_ITERATIONS:
            g_TressFXSimulation.m_numLengthConstraintIterations = ((CDXUTSlider*)pControl)->GetValue();
            swprintf(szMessage, L"Length constraint\n iterations:%d", g_TressFXSimulation.m_numLengthConstraintIterations); szMessage[99] = 0;
            g_SimulationHUD.m_GUI.GetStatic( IDC_STATIC_NUM_LENGTH_CONSTRAINT_ITERATIONS )->SetText( szMessage );
            break;

        case IDC_SLIDER_NUM_LOCAL_SHAPE_MATCHING_ITERATIONS:
            g_TressFXSimulation.m_numLocalShapeMatchingIterations = ((CDXUTSlider*)pControl)->GetValue();
            swprintf(szMessage, L"Local shape matching\n iterations:%d", g_TressFXSimulation.m_numLocalShapeMatchingIterations); szMessage[99] = 0;
            g_SimulationHUD.m_GUI.GetStatic( IDC_STATIC_NUM_LOCAL_SHAPE_MATCHING_ITERATIONS )->SetText( szMessage );
			break;

		case IDC_BUTTON_STRAND_COPIES_PLUS:
			if (g_HairParams.iStrandCopies < MAX_STRAND_COPIES)
				g_HairParams.iStrandCopies++;
			swprintf(szMessage, L" %d", g_HairParams.iStrandCopies); szMessage[99] = 0;
			g_RenderHUD.m_GUI.GetStatic( IDC_STATIC_STRAND_COPIES_VALUE )->SetText( szMessage );
            break;

		case IDC_BUTTON_STRAND_COPIES_MINUS:
			if (g_HairParams.iStrandCopies > 1)
				g_HairParams.iStrandCopies--;
			swprintf(szMessage, L" %d", g_HairParams.iStrandCopies); szMessage[99] = 0;
			g_RenderHUD.m_GUI.GetStatic( IDC_STATIC_STRAND_COPIES_VALUE )->SetText( szMessage );
            break;

		case IDC_SLIDER_HAIR_DAMPING:
			value = (float)(((CDXUTSlider*)pControl)->GetValue()) / 100.0f;
			switch (g_SimulationHUD.m_GUI.GetComboBox(IDC_COMBOBOX_HAIR_SECTION)->GetSelectedIndex())
			{
			case TOP_SIDE_BACK_HAIR_INDEX:
				g_TressFXSimulation.m_shapeParams[0].damping = value;
				break;					
			case PONYTAIL_HAIR_INDEX:
				g_TressFXSimulation.m_shapeParams[2].damping = value;
				break;
			case FRONT_HAIR_INDEX:
				g_TressFXSimulation.m_shapeParams[3].damping = value;
				break;
			}
            break;

 		case IDC_SLIDER_HAIR_LOCAL_STIFFNESS:
			value = (float)(((CDXUTSlider*)pControl)->GetValue()) / 100.0f;
			switch (g_SimulationHUD.m_GUI.GetComboBox(IDC_COMBOBOX_HAIR_SECTION)->GetSelectedIndex())
			{
			case TOP_SIDE_BACK_HAIR_INDEX:
				g_TressFXSimulation.m_shapeParams[0].stiffnessForLocalShapeMatching = value;
				break;					
			case PONYTAIL_HAIR_INDEX:
				g_TressFXSimulation.m_shapeParams[2].stiffnessForLocalShapeMatching = value;
				break;
			case FRONT_HAIR_INDEX:
				g_TressFXSimulation.m_shapeParams[3].stiffnessForLocalShapeMatching = value;
				break;
			}
            break;

 		case IDC_SLIDER_HAIR_GLOBAL_STIFFNESS:
			value = (float)(((CDXUTSlider*)pControl)->GetValue()) / 100.0f;
			switch (g_SimulationHUD.m_GUI.GetComboBox(IDC_COMBOBOX_HAIR_SECTION)->GetSelectedIndex())
			{
			case TOP_SIDE_BACK_HAIR_INDEX:
				g_TressFXSimulation.m_shapeParams[0].stiffnessForGlobalShapeMatching = value;
				break;					
			case PONYTAIL_HAIR_INDEX:
				g_TressFXSimulation.m_shapeParams[2].stiffnessForGlobalShapeMatching = value;
				break;
			case FRONT_HAIR_INDEX:
				g_TressFXSimulation.m_shapeParams[3].stiffnessForGlobalShapeMatching = value;
				break;
			}
            break;

 		case IDC_SLIDER_GLOBAL_SHAPE_MATCHING_EFFECT_RANGE:
			value = (float)(((CDXUTSlider*)pControl)->GetValue()) / 100.0f;
			switch (g_SimulationHUD.m_GUI.GetComboBox(IDC_COMBOBOX_HAIR_SECTION)->GetSelectedIndex())
			{
			case TOP_SIDE_BACK_HAIR_INDEX:
				g_TressFXSimulation.m_shapeParams[0].globalShapeMatchingEffectiveRange = value;
				break;					
			case PONYTAIL_HAIR_INDEX:
				g_TressFXSimulation.m_shapeParams[2].globalShapeMatchingEffectiveRange = value;
				break;
			case FRONT_HAIR_INDEX:
				g_TressFXSimulation.m_shapeParams[3].globalShapeMatchingEffectiveRange = value;
				break;
			}
            break;

		case IDC_CHECKBOX_SM:
			{
				bool bSelfShadow = (((CDXUTCheckBox*)pControl)->GetChecked());
				if (bSelfShadow)
				{
					g_HairParams.tech_shadow = SDSM_SHADOW_INDEX;
					g_RenderHUD.m_GUI.GetSlider( IDC_OPACITYSM )->SetEnabled(true);
				}
				else
				{
					g_HairParams.tech_shadow = NONE_SHADOW_INDEX;
					g_RenderHUD.m_GUI.GetSlider( IDC_OPACITYSM )->SetEnabled(false);
				}
			}
			break;

		case IDC_CHECKBOX_THINTIP:
			g_HairParams.bThinTip = (((CDXUTCheckBox*)pControl)->GetChecked());
			break;

		case IDC_CHECKBOX_LOD:
			g_bEnableLOD = (((CDXUTCheckBox*)pControl)->GetChecked());
			if (g_bEnableLOD)
			{
				g_savedDensity = g_HairParams.density;
				g_savedRadius = g_HairParams.radius;
				g_savedMaxFragments = g_HairParams.iMaxFragments;
				g_savedAlphaSM = g_HairParams.alpha_sm;
				g_RenderHUD.m_GUI.GetSlider( IDC_DENSITY )->SetEnabled(false);
				g_RenderHUD.m_GUI.GetSlider( IDC_FIBERRADIUS )->SetEnabled(false);
				g_RenderHUD.m_GUI.GetSlider( IDC_OPACITYSM )->SetEnabled(false);
			}
			else
			{
				g_HairParams.alpha_sm = g_savedAlphaSM;
				g_HairParams.iMaxFragments = g_savedMaxFragments;
				g_HairParams.density = g_savedDensity;
				g_HairParams.radius = g_savedRadius;
				g_RenderHUD.m_GUI.GetSlider( IDC_DENSITY )->SetEnabled(true);
				g_RenderHUD.m_GUI.GetSlider( IDC_FIBERRADIUS )->SetEnabled(true);
				g_RenderHUD.m_GUI.GetSlider( IDC_OPACITYSM )->SetEnabled(true);

				swprintf(szMessage, L"Density:%.4f", g_HairParams.density);
				g_RenderHUD.m_GUI.GetStatic( IDC_DENSITY_STATIC )->SetText( szMessage );
				g_RenderHUD.m_GUI.GetSlider( IDC_DENSITY )->SetValue((int)(100 * (g_HairParams.density - g_vHairDensity.y) / (g_vHairDensity.z - g_vHairDensity.y)));
				swprintf(szMessage, L"Hair thickness:%.4f", g_HairParams.radius);
				g_RenderHUD.m_GUI.GetStatic( IDC_FIBERRADIUS_STATIC )->SetText( szMessage );
				g_RenderHUD.m_GUI.GetSlider( IDC_FIBERRADIUS )->SetValue((int)(100 * (g_HairParams.radius - g_vFiberRadius.y) / (g_vFiberRadius.z - g_vFiberRadius.y)));
				swprintf(szMessage, L" Self-shadowing:%.4f", g_HairParams.alpha_sm);
				g_RenderHUD.m_GUI.GetStatic( IDC_OPACITYSM_STATIC )->SetText( szMessage );
				g_RenderHUD.m_GUI.GetSlider( IDC_OPACITYSM )->SetValue((int)(100 * (g_HairParams.alpha_sm - g_vHairAlphaSM.y) / (g_vHairAlphaSM.z - g_vHairAlphaSM.y)));
			}
			break;

		case IDC_CHECKBOX_USE_COVERAGE:
			g_HairParams.bUseCoverage = (((CDXUTCheckBox*)pControl)->GetChecked());
			break;

		case IDC_CHECKBOX_SIMULATE:
			g_HairParams.bSimulation = (((CDXUTCheckBox*)pControl)->GetChecked());
			break;

		case IDC_CHECKBOX_COLLISION:
			g_TressFXSimulation.m_bCollision = (((CDXUTCheckBox*)pControl)->GetChecked());
			break;

		case IDC_BUTTON_HAIR_COLOR:
			ChooseHairColor();
			break;

		case IDC_COMBOBOX_HAIR_SECTION:
			switch (((CDXUTComboBox*)pControl)->GetSelectedIndex())
			{
			case TOP_SIDE_BACK_HAIR_INDEX:
				g_SimulationHUD.m_GUI.GetSlider(IDC_SLIDER_HAIR_DAMPING)->SetValue((int)(g_TressFXSimulation.m_shapeParams[0].damping * 100));
				g_SimulationHUD.m_GUI.GetSlider(IDC_SLIDER_HAIR_LOCAL_STIFFNESS)->SetValue((int)(g_TressFXSimulation.m_shapeParams[0].stiffnessForLocalShapeMatching * 100));
				g_SimulationHUD.m_GUI.GetSlider(IDC_SLIDER_HAIR_GLOBAL_STIFFNESS)->SetValue((int)(g_TressFXSimulation.m_shapeParams[0].stiffnessForGlobalShapeMatching * 100));
				g_SimulationHUD.m_GUI.GetSlider(IDC_SLIDER_GLOBAL_SHAPE_MATCHING_EFFECT_RANGE)->SetValue((int)(g_TressFXSimulation.m_shapeParams[0].globalShapeMatchingEffectiveRange * 100));
				break;					
			case PONYTAIL_HAIR_INDEX:
				g_SimulationHUD.m_GUI.GetSlider(IDC_SLIDER_HAIR_DAMPING)->SetValue((int)(g_TressFXSimulation.m_shapeParams[2].damping * 100));
				g_SimulationHUD.m_GUI.GetSlider(IDC_SLIDER_HAIR_LOCAL_STIFFNESS)->SetValue((int)(g_TressFXSimulation.m_shapeParams[2].stiffnessForLocalShapeMatching * 100));
				g_SimulationHUD.m_GUI.GetSlider(IDC_SLIDER_HAIR_GLOBAL_STIFFNESS)->SetValue((int)(g_TressFXSimulation.m_shapeParams[2].stiffnessForGlobalShapeMatching * 100));
				g_SimulationHUD.m_GUI.GetSlider(IDC_SLIDER_GLOBAL_SHAPE_MATCHING_EFFECT_RANGE)->SetValue((int)(g_TressFXSimulation.m_shapeParams[2].globalShapeMatchingEffectiveRange * 100));
				break;
			case FRONT_HAIR_INDEX:
				g_SimulationHUD.m_GUI.GetSlider(IDC_SLIDER_HAIR_DAMPING)->SetValue((int)(g_TressFXSimulation.m_shapeParams[3].damping * 100));
				g_SimulationHUD.m_GUI.GetSlider(IDC_SLIDER_HAIR_LOCAL_STIFFNESS)->SetValue((int)(g_TressFXSimulation.m_shapeParams[3].stiffnessForLocalShapeMatching * 100));
				g_SimulationHUD.m_GUI.GetSlider(IDC_SLIDER_HAIR_GLOBAL_STIFFNESS)->SetValue((int)(g_TressFXSimulation.m_shapeParams[3].stiffnessForGlobalShapeMatching * 100));
				g_SimulationHUD.m_GUI.GetSlider(IDC_SLIDER_GLOBAL_SHAPE_MATCHING_EFFECT_RANGE)->SetValue((int)(g_TressFXSimulation.m_shapeParams[3].globalShapeMatchingEffectiveRange * 100));
				break;
			}
			break;
 		default:
			AMD::OnGUIEvent( nEvent, nControlID, pControl, pUserContext );
			break;
  }

    // Call the MagnifyTool gui event handler
    g_MagnifyTool.OnGUIEvent( nEvent, nControlID, pControl, pUserContext );

}

//--------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
//--------------------------------------------------------------------------------------
bool CALLBACK IsD3D11DeviceAcceptable( const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
                                       DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    return true;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependent upon the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice( ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    TIMER_Init(pd3dDevice);

    HRESULT hr;

    // Get device context
    ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();

    // GUI creation
    V_RETURN( g_DialogResourceManager.OnD3D11CreateDevice( pd3dDevice, pd3dImmediateContext ) );
    V_RETURN( g_D3DSettingsDlg.OnD3D11CreateDevice( pd3dDevice ) );
    g_pTxtHelper = new CDXUTTextHelper( pd3dDevice, pd3dImmediateContext, &g_DialogResourceManager, 15 );
    
    const WCHAR* bodyFile = L".\\media\\objs\\model\\head_max4.obj";

	char *filenameArray[4];
	filenameArray[0] = ".\\media\\Hair\\hair_free.tfx";
	filenameArray[1] = ".\\media\\Hair\\hair_pulled.tfx";
	filenameArray[2] = ".\\media\\Hair\\hair_tail.tfx";
	filenameArray[3] = ".\\media\\Hair\\hair_front_long.tfx";

	// collision capsules for the scene model
	ConstBufferCS_Collision_Capsule headCollision;
	headCollision.cc0_center.x = -0.095f;
	headCollision.cc0_center.y = 92.000f;
	headCollision.cc0_center.z = -9.899f;
	headCollision.cc0_radius = 26.5f;
	headCollision.cc0_radiusSquared = headCollision.cc0_radius * headCollision.cc0_radius;
	headCollision.cc1_center.x = -0.405f;
	headCollision.cc1_center.y = 93.707f;
	headCollision.cc1_center.z = 5.111f;
	headCollision.cc1_radius = 24.113f;
	headCollision.cc1_radiusSquared = headCollision.cc1_radius * headCollision.cc1_radius;
	headCollision.cc2_center.x = -0.072f;
	headCollision.cc2_center.y = 68.548f;
	headCollision.cc2_center.z = 10.561f;
	headCollision.cc2_radius = 25.500f;
	headCollision.cc2_radiusSquared = headCollision.cc2_radius * headCollision.cc2_radius;

	bool bMakeBothEndsImmovable[4];
	bMakeBothEndsImmovable[0] = false;
	bMakeBothEndsImmovable[1] = true; 
	bMakeBothEndsImmovable[2] = false;
	bMakeBothEndsImmovable[3] = false;

	g_TressFXMesh.OnCreateDevice(pd3dDevice, filenameArray, bMakeBothEndsImmovable, 4, g_fScale, g_vRotation, g_vTranslation);

	g_TressFXSimulation.OnCreateDevice(pd3dDevice, &g_TressFXMesh, &g_ShaderCache, &headCollision);

	g_TressFXRender.OnCreateDevice(pd3dDevice, &g_TressFXMesh, &g_ShaderCache);

	g_SceneRender.OnCreateDevice(pd3dDevice, bodyFile, g_fScale, g_vRotation, g_vTranslation, &g_ShaderCache);

	// Generate shaders ( this is an async operation - call AMD::ShaderCache::ShadersReady() to find out if they are complete ) 
    static bool bFirstPass = true;
    if( bFirstPass )
    {

        #ifdef _DEBUG
            //g_ShaderCache.GenerateShaders( AMD::ShaderCache::CREATE_TYPE_FORCE_COMPILE );    // Clean the cache, and compile all
            g_ShaderCache.GenerateShaders( AMD::ShaderCache::CREATE_TYPE_COMPILE_CHANGES );    // Only compile shaders that have changed (development mode)
            //g_ShaderCache.GenerateShaders( AMD::ShaderCache::CREATE_TYPE_USE_CACHED );       // Use cached shaders (release mode)
        #else
            //g_ShaderCache.GenerateShaders( AMD::ShaderCache::CREATE_TYPE_FORCE_COMPILE );    // Clean the cache, and compile all
            g_ShaderCache.GenerateShaders( AMD::ShaderCache::CREATE_TYPE_COMPILE_CHANGES );    // Only compile shaders that have changed (development mode)
            //g_ShaderCache.GenerateShaders( AMD::ShaderCache::CREATE_TYPE_USE_CACHED );       // Use cached shaders (release mode)
        #endif

        bFirstPass = false;
    }

    // Create AMD_SDK resources here
    g_RenderHUD.OnCreateDevice( pd3dDevice );
    g_SimulationHUD.OnCreateDevice( pd3dDevice );
	g_MagnifyTool.OnCreateDevice( pd3dDevice );

    D3DXMatrixIdentity(&g_ModelTransform);
    D3DXMatrixIdentity(&g_ModelTransformForHead);

#ifdef MEM_DEBUG
	g_pMemDebugDevice = pd3dDevice;
#endif

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );
    V_RETURN( g_D3DSettingsDlg.OnD3D11ResizedSwapChain( pd3dDevice, pBackBufferSurfaceDesc ) );

    g_ScreenWidth = pBackBufferSurfaceDesc->Width;
    g_ScreenHeight = pBackBufferSurfaceDesc->Height;

    // Setup the camera's projection parameters
    float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
    g_Camera.SetProjParams(FIELD_OF_VIEW, fAspectRatio, NEAR_PLANE, FAR_PLANE);
    g_Camera.SetWindow(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);
    g_Camera.SetButtonMasks(NULL, MOUSE_WHEEL, MOUSE_LEFT_BUTTON );

    // Note: we will not use these parameters for light camera
    // only Eye and LookAt position will be used for light camera
    g_CameraLight0.SetProjParams( FIELD_OF_VIEW, fAspectRatio, NEAR_PLANE, FAR_PLANE); 
    g_CameraLight0.SetWindow(SM_SCENE_WIDTH, SM_SCENE_HEIGHT);

    // Set GUI size and locations
    g_RenderHUD.m_GUI.SetLocation( pBackBufferSurfaceDesc->Width - AMD::HUD::iDialogWidth, 0 );
    g_RenderHUD.m_GUI.SetSize( AMD::HUD::iDialogWidth, pBackBufferSurfaceDesc->Height  );
    g_RenderHUD.OnResizedSwapChain( pBackBufferSurfaceDesc );

    g_SimulationHUD.m_GUI.SetLocation( pBackBufferSurfaceDesc->Width - AMD::HUD::iDialogWidth, 0 );
    g_SimulationHUD.m_GUI.SetSize( AMD::HUD::iDialogWidth, pBackBufferSurfaceDesc->Height  );
    g_SimulationHUD.OnResizedSwapChain( pBackBufferSurfaceDesc );

	// Resize the buffers used for hair rendering
	g_TressFXRender.OnResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc, pUserContext);

    // Magnify tool will capture from the color buffer
    g_MagnifyTool.OnResizedSwapChain( pd3dDevice, pSwapChain, pBackBufferSurfaceDesc, pUserContext, 
        pBackBufferSurfaceDesc->Width - AMD::HUD::iDialogWidth, 0 );
    D3D11_RENDER_TARGET_VIEW_DESC RTDesc;
    ID3D11Resource* pTempRTResource;
    DXUTGetD3D11RenderTargetView()->GetResource( &pTempRTResource );
    DXUTGetD3D11RenderTargetView()->GetDesc( &RTDesc );
    g_MagnifyTool.SetSourceResources( pTempRTResource, RTDesc.Format, 
                DXUTGetDXGIBackBufferSurfaceDesc()->Width, DXUTGetDXGIBackBufferSurfaceDesc()->Height,
                DXUTGetDXGIBackBufferSurfaceDesc()->SampleDesc.Count );
    g_MagnifyTool.SetPixelRegion( 128 );
    g_MagnifyTool.SetScale( 5 );
    SAFE_RELEASE( pTempRTResource );

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, 
                                  double fTime, float fElapsedTime, void* pUserContext )
{
    TIMER_Reset();
    
    //---------------------------------
    // Settings dialog if it is active.
    //---------------------------------
    if( g_D3DSettingsDlg.IsActive() )
    {
        g_D3DSettingsDlg.OnRender( fElapsedTime );
        return;
    }

    D3DXVECTOR3 old_center = D3DXVECTOR3(0,0,0);
    D3DXVec3TransformCoord(&g_ModelCenter, &old_center, &g_ModelTransform);

    //-------------------------------------------	 
    // Clear the render target and depth stencil
    //-------------------------------------------
    DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"Clear primary render and depth targets");
    float ClearColor[4] = { 0.25f, 0.25f, 0.35f, 0.0f };
    ID3D11RenderTargetView* pPrimaryRTV = DXUTGetD3D11RenderTargetView();
    ID3D11DepthStencilView* pPrimaryDSV = DXUTGetD3D11DepthStencilView();
    pd3dImmediateContext->ClearRenderTargetView( pPrimaryRTV, ClearColor );
    pd3dImmediateContext->ClearDepthStencilView( pPrimaryDSV, D3D11_CLEAR_DEPTH, 1.0f, 0 );

    DXUT_EndPerfEvent();
    
	if( g_ShaderCache.ShadersReady() )
    {
        TIMER_Begin( 0, L"Effect" )
       
		if ( g_HairParams.bSimulation )
		{
			if ( !g_bModelTransformUpdated )
				D3DXMatrixIdentity(&g_ModelTransform);

			g_TressFXSimulation.Simulate(pd3dImmediateContext, fElapsedTime, 
				g_HairParams.wind_direction, g_HairParams.wind_magnitude, &g_ModelTransformForHead, g_HairParams.targetFrameRateForSimulation);
		}

		g_TressFXRender.BeginHairFrame(pd3dImmediateContext, &g_Camera, &g_CameraLight0, g_HairParams, 
			&g_ModelTransformForHead, (float)g_ScreenWidth, (float)g_ScreenHeight);
		
		TIMER_Begin( 0, L"GenerateShadows" )
		if (g_HairParams.tech_shadow != NONE_SHADOW_INDEX)
		{
			g_TressFXRender.GenerateShadowMap(pd3dImmediateContext, g_HairParams.density, (float)g_ScreenWidth, (float)g_ScreenHeight);
			g_SceneRender.GenerateShadowMap(pd3dImmediateContext);
		}
		TIMER_End(); // GenerateShadows

		TIMER_Begin( 0, L"RenderScene" )
		g_SceneRender.RenderScene(pd3dImmediateContext, g_HairParams.tech_shadow != NONE_SHADOW_INDEX);
		TIMER_End(); // RenderScene

		if (g_bEnableLOD)
		{
			// Adjust the density and the thickness based on distance from camera to hair
			D3DXVECTOR3 vCameraToHair = *(g_Camera.GetEyePt()) - g_ModelCenter;
			float distance = D3DXVec3Length(&vCameraToHair);
			if (distance > MAX_LOD_DISTANCE)
				distance = MAX_LOD_DISTANCE;
			if (distance <= MIN_LOD_DISTANCE)
				distance = 0;
			else
				distance -= MIN_LOD_DISTANCE;
			float scaledDistance = distance / (MAX_LOD_DISTANCE - MIN_LOD_DISTANCE);
			g_HairParams.density = (1 - scaledDistance) * MAX_HAIR_DENSITY + scaledDistance * MIN_HAIR_DENSITY;
			g_HairParams.radius = (1 - scaledDistance) * MIN_HAIR_RADIUS + scaledDistance * MAX_HAIR_RADIUS;
			// adjust the max overlapping fragment count
			g_HairParams.iMaxFragments = (int)((1 - scaledDistance) * MAX_FRAGMENTS + scaledDistance * MIN_FRAGMENTS);
			// adjust the shadow map alpha
			g_HairParams.alpha_sm = (1 - scaledDistance) * g_vHairAlphaSM.x + scaledDistance * MAX_ALPHA_SM;

			// update the UI
			WCHAR szMessage[100];   
			swprintf(szMessage, L"Density:%.4f", g_HairParams.density);
            g_RenderHUD.m_GUI.GetStatic( IDC_DENSITY_STATIC )->SetText( szMessage );
            g_RenderHUD.m_GUI.GetSlider( IDC_DENSITY )->SetValue((int)(100 * (g_HairParams.density - g_vHairDensity.y) / (g_vHairDensity.z - g_vHairDensity.y)));
			swprintf(szMessage, L"Hair thickness:%.4f", g_HairParams.radius);
            g_RenderHUD.m_GUI.GetStatic( IDC_FIBERRADIUS_STATIC )->SetText( szMessage );
            g_RenderHUD.m_GUI.GetSlider( IDC_FIBERRADIUS )->SetValue((int)(100 * (g_HairParams.radius - g_vFiberRadius.y) / (g_vFiberRadius.z - g_vFiberRadius.y)));
			swprintf(szMessage, L" Self-shadowing:%.4f", g_HairParams.alpha_sm);
			g_RenderHUD.m_GUI.GetStatic( IDC_OPACITYSM_STATIC )->SetText( szMessage );
			g_RenderHUD.m_GUI.GetSlider( IDC_OPACITYSM )->SetValue((int)(100 * (g_HairParams.alpha_sm - g_vHairAlphaSM.y) / (g_vHairAlphaSM.z - g_vHairAlphaSM.y)));
		}

		TIMER_Begin( 0, L"RenderHair" )
		g_TressFXRender.RenderHair(pd3dImmediateContext, g_HairParams);    
		TIMER_End(); // RenderHair

		g_TressFXRender.EndHairFrame(pd3dImmediateContext, g_HairParams);

		TIMER_End(); // Effect
	}

    DXUT_BeginPerfEvent( DXUT_PERFEVENTCOLOR, L"HUD / Stats" );

 	AMD::ProcessUIChanges();

    if( g_ShaderCache.ShadersReady() )
    {
		ID3D11BlendState *pBlendState;
		FLOAT blendFactor[4];
		UINT sampleMask;
		// save the current blend state
		pd3dImmediateContext->OMGetBlendState(&pBlendState, blendFactor, &sampleMask);

		if ( g_bShowUI )
		{
            g_MagnifyTool.Render();
			g_RenderHUD.OnRender( fElapsedTime );
			g_SimulationHUD.OnRender( fElapsedTime );
		}
		RenderText();

		AMD::RenderHUDUpdates( g_pTxtHelper );

		// restore the saved blend state
		pd3dImmediateContext->OMSetBlendState(pBlendState, blendFactor, sampleMask); 
	}
    else
    {
        // Render shader cache progress if still processing
        g_ShaderCache.RenderProgress( g_pTxtHelper, 15, D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    }

    DXUT_EndPerfEvent(/*HUD / Stats*/);
   
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}

//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice 
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D11DestroyDevice();
    g_D3DSettingsDlg.OnD3D11DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();
    SAFE_DELETE( g_pTxtHelper );
        
    g_TressFXRender.OnDestroy();
	g_TressFXSimulation.OnDestroy();
	g_TressFXMesh.OnDestroy();
	g_SceneRender.OnDestroy();

	// Destroy AMD_SDK resources here
	g_ShaderCache.OnDestroyDevice();
	g_RenderHUD.OnDestroyDevice();
	g_SimulationHUD.OnDestroyDevice();
    g_MagnifyTool.OnDestroyDevice();
    TIMER_Destroy();

#ifdef MEM_DEBUG
	ID3D11Debug *pd3dDebug;
	g_pMemDebugDevice->QueryInterface(__uuidof(ID3D11Debug) , (LPVOID *) &pd3dDebug);
	pd3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
#endif
}
