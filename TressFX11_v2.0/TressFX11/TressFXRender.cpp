//--------------------------------------------------------------------------------------
// File: TressFXRender.cpp
//
// Main hair rendering code
// 
// Copyright © AMD Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "..\\DXUT\\Core\\DXUT.h"
#include "TressFXRender.h"

struct CB_PER_FRAME
{
	D3DXMATRIX	m_mWorld;
    D3DXMATRIX	m_mViewProj;
	D3DXMATRIX  m_mInvViewProj;
    D3DXMATRIX  m_mViewProjLight;
    
    D3DXVECTOR3 m_vEye;
	float		m_fvFOV;

	D3DXVECTOR4 m_AmbientLightColor;
    D3DXVECTOR4 m_PointLightColor;
    D3DXVECTOR4 m_PointLightPos;
    D3DXVECTOR4 m_MatBaseColor;
    D3DXVECTOR4 m_MatKValue;

    float		m_FiberAlpha;
    float		m_HairSMAlpha;
    float		m_bExpandPixels;
	float		m_FiberRadius;

    D3DXVECTOR4 m_WinSize;

    float		m_FiberSpacing;
    float		m_bThinTip;
    float		m_fNearLight;
    float		m_fFarLight;
	       
    int			m_iTechSM;
    int			m_bUseCoverage;
	int			m_iStrandCopies;
	int			m_iMaxFragments;

	float		m_alphaThreshold;
    float		m_fHairKs2;
    float		m_fHairEx2;
    float		m_unused;

    D3DXMATRIX  m_mInvViewProjViewport;

};

struct PER_PIXEL_LINKED_LIST_STRUCT
{
    DWORD			TangentAndCoverage;
	DWORD			depth;
    DWORD			dwNext;
};

const static UINT			g_HairTotalLayers = 8; 
static const D3DXVECTOR3	g_vPointLight = D3DXVECTOR3(1.f, 1.f, 1.f);
static const D3DXVECTOR3	g_vAmbientLight = D3DXVECTOR3(0.15f, 0.15f, 0.15f);

//--------------------------------------------------------------------------------------
//
// CreateShaderAndLayout
//
// Creates the shaders and layouts for hair rendering
//
//--------------------------------------------------------------------------------------
void CTressFXRender::CreateShaderAndLayout( ID3D11Device* pd3dDevice, AMD::ShaderCache *pShaderCache  )
{

    // Ensure all shaders (and input layouts) are released
    SAFE_RELEASE( m_pVSRenderHair );
    SAFE_RELEASE( m_pLayoutHair_0 );
    SAFE_RELEASE( m_pLayoutHair_1 );
    SAFE_RELEASE( m_pLayoutHair_2 );
    SAFE_RELEASE( m_pLayoutHair_3 );
    SAFE_RELEASE( m_pLayoutHair_4 );

	// Hair vertex layout
    const D3D11_INPUT_ELEMENT_DESC layout_hair[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // Full screen quad layout structure
    const D3D11_INPUT_ELEMENT_DESC layout_quad[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };


    // Add shaders to the cache (do this only once)
	static bool bFirstPass = true;
    if( bFirstPass )
    {
		// vertex shaders
		pShaderCache->AddShader( (ID3D11DeviceChild**)&m_pVSRenderHair, AMD::ShaderCache::SHADER_TYPE_VERTEX, 
			L"vs_5_0", L"VS_RenderHair",L"shaders\\Source\\TressFXRender.hlsl", 0, NULL, &m_pLayoutHair_0, 
			(D3D11_INPUT_ELEMENT_DESC*)layout_hair, ARRAYSIZE( layout_hair ) );
		pShaderCache->AddShader( (ID3D11DeviceChild**)&m_pVSRenderHairAA, AMD::ShaderCache::SHADER_TYPE_VERTEX, 
			L"vs_5_0", L"VS_RenderHair_AA",L"shaders\\Source\\TressFXRender.hlsl", 0, NULL, &m_pLayoutHair_1, 
			(D3D11_INPUT_ELEMENT_DESC*)layout_hair, ARRAYSIZE( layout_hair ) );
		pShaderCache->AddShader( (ID3D11DeviceChild**)&m_pVSGenerateHairSM, AMD::ShaderCache::SHADER_TYPE_VERTEX, 
			L"vs_5_0", L"VS_GenerateHairSM",L"shaders\\Source\\TressFXRender.hlsl", 0, NULL, &m_pLayoutHair_2, 
			(D3D11_INPUT_ELEMENT_DESC*)layout_hair, ARRAYSIZE( layout_hair ) );
		pShaderCache->AddShader( (ID3D11DeviceChild**)&m_pVSRenderHairStrandCopies, AMD::ShaderCache::SHADER_TYPE_VERTEX, 
			L"vs_5_0", L"VS_RenderHair_StrandCopies",L"shaders\\Source\\TressFXRender.hlsl", 0, NULL, &m_pLayoutHair_3, 
			(D3D11_INPUT_ELEMENT_DESC*)layout_hair, ARRAYSIZE( layout_hair ) );
		pShaderCache->AddShader( (ID3D11DeviceChild**)&m_pVSRenderHairAAStrandCopies, AMD::ShaderCache::SHADER_TYPE_VERTEX, 
			L"vs_5_0", L"VS_RenderHair_AA_StrandCopies",L"shaders\\Source\\TressFXRender.hlsl", 0, NULL, &m_pLayoutHair_4, 
			(D3D11_INPUT_ELEMENT_DESC*)layout_hair, ARRAYSIZE( layout_hair ) );
		pShaderCache->AddShader( (ID3D11DeviceChild**)&m_pVSScreenQuad, AMD::ShaderCache::SHADER_TYPE_VERTEX, 
			L"vs_5_0", L"VS_ScreenQuad",L"shaders\\Source\\TressFXInclude.hlsl", 0, NULL, &m_pLayoutHair_5, 
			(D3D11_INPUT_ELEMENT_DESC*)layout_quad, ARRAYSIZE( layout_quad ) );
	
		// pixel shaders
		pShaderCache->AddShader( (ID3D11DeviceChild**)&m_pPSABuffer_Hair, AMD::ShaderCache::SHADER_TYPE_PIXEL, L"ps_5_0", L"PS_ABuffer_Hair",
			L"shaders\\Source\\TressFXRender.hlsl", 0, NULL, NULL, NULL, 0 );
		pShaderCache->AddShader( (ID3D11DeviceChild**)&m_pPSKBuffer_Hair, AMD::ShaderCache::SHADER_TYPE_PIXEL, L"ps_5_0", L"PS_KBuffer_Hair",
			L"shaders\\Source\\TressFXRender.hlsl", 0, NULL, NULL, NULL, 0 );

	    bFirstPass = false;
	}
}

//--------------------------------------------------------------------------------------
//
// CreateTextureAndViews
//
// Creates the textures and corrisponding views for hair rendering
//
//--------------------------------------------------------------------------------------
void CTressFXRender::CreateTextureAndViews( ID3D11Device* pd3dDevice )
{
    HRESULT hr;

    // Create SM DSVs for hair	
    D3D11_TEXTURE2D_DESC tex2D_desc;
    tex2D_desc.Width =  SM_HAIR_WIDTH;
    tex2D_desc.Height = SM_HAIR_WIDTH;
    tex2D_desc.ArraySize = 1;
    tex2D_desc.Format = DXGI_FORMAT_R32_TYPELESS;
    tex2D_desc.Usage = D3D11_USAGE_DEFAULT;
    tex2D_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL|D3D11_BIND_SHADER_RESOURCE;
    tex2D_desc.CPUAccessFlags = 0;
    tex2D_desc.MipLevels = 0;
    tex2D_desc.MiscFlags = 0;
    tex2D_desc.SampleDesc.Count = 1;
    tex2D_desc.SampleDesc.Quality = 0;
	
    hr = pd3dDevice->CreateTexture2D(&tex2D_desc, 0, &m_pSMHairTx);
    if(FAILED(hr))
        ::MessageBoxW(0, L"fail to create m_pSMHairTx", L"d3d Error", 0);

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = 0  ;
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D ;
    dsvDesc.Texture2D.MipSlice = 0;
    if(FAILED(pd3dDevice->CreateDepthStencilView(m_pSMHairTx, &dsvDesc, &m_pSMHairDSV)))
        ::MessageBoxW(0, L"fail to create m_pSMHairDSV", L"d3d Error", 0);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    if(FAILED(pd3dDevice->CreateShaderResourceView(m_pSMHairTx, &srvDesc, &m_pSMHairSRV)))
        ::MessageBoxW(0, L"fail to create m_pSMSRV", L"d3d Error", 0);
	
    // Noise texture
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = 512;
    texDesc.Height = 512;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE ;
    texDesc.MiscFlags = 0;
    texDesc.CPUAccessFlags = 0;
    texDesc.SampleDesc.Count=1;
    texDesc.SampleDesc.Quality=0;

    D3DXVECTOR4* noiseArray = new D3DXVECTOR4[texDesc.Width*texDesc.Height];
    for(UINT i=0; i<texDesc.Width*texDesc.Height ; i++)
    {
        noiseArray[i].x = rand()/(float)RAND_MAX; 
        noiseArray[i].y = rand()/(float)RAND_MAX;
        noiseArray[i].z = rand()/(float)RAND_MAX;
        noiseArray[i].w = rand()/(float)RAND_MAX;
    }

    D3D11_SUBRESOURCE_DATA initData0;
    initData0.pSysMem = noiseArray;
    initData0.SysMemPitch = texDesc.Width*sizeof(D3DXVECTOR4);
    initData0.SysMemSlicePitch = 0;

    hr = pd3dDevice->CreateTexture2D(&texDesc, &initData0, &m_pNoiseTexture2D);
    if(FAILED(hr))
        ::MessageBoxW(0, L"fail to create random texture", L"Error", 0);
	
    D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
    srDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srDesc.Texture1D.MipLevels = 0xffffffff;
    srDesc.Texture1D.MostDetailedMip = 0;

    pd3dDevice->CreateShaderResourceView(m_pNoiseTexture2D, &srDesc, &m_pNoiseSRV);
    SAFE_DELETE_ARRAY(noiseArray);
}


//--------------------------------------------------------------------------------------
//
// CreateVertexBuffers
//
// Creates the vertex buffers for hair rendering
//
//--------------------------------------------------------------------------------------
void CTressFXRender::CreateVertexBuffers(ID3D11Device* pd3dDevice)
{
    HRESULT hr;
    // Create the screen quad vertex buffer(use StandardVertex for simplicity)
    const StandardVertex screenQuad[6] = 
    {
        { D3DXVECTOR3(-1.0f, -1.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR2(0.0f, 1.0f) },//0
        { D3DXVECTOR3(-1.0f,  1.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR2(0.0f, 0.0f) },//1
        { D3DXVECTOR3( 1.0f, -1.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR2(1.0f, 1.0f) },//2
        { D3DXVECTOR3( 1.0f, -1.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR2(1.0f, 1.0f) },//2
        { D3DXVECTOR3(-1.0f,  1.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR2(0.0f, 0.0f) },//1
        { D3DXVECTOR3( 1.0f,  1.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR2(1.0f, 0.0f) } //3
    };

    D3D11_BUFFER_DESC bd;
    bd.Usage =          D3D11_USAGE_DEFAULT;
    bd.ByteWidth =      sizeof( StandardVertex ) * 6; 
    bd.BindFlags =      D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags =      0;

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem =  screenQuad;
    initData.SysMemPitch = 0;
    initData.SysMemSlicePitch = 0;
    hr = pd3dDevice->CreateBuffer( &bd, &initData, &m_pScreenQuadVB );
    if( FAILED( hr ) )
        ::MessageBoxA(0, "Fail to create screen quad vertex buffer", "d3d error", 0);
}

//--------------------------------------------------------------------------------------
//
// CreateConstantBuffers
//
// Creates the constant buffers for hair rendering
//
//--------------------------------------------------------------------------------------
void CTressFXRender::CreateConstantBuffer( ID3D11Device* pd3dDevice )
{
    HRESULT hr;
    D3D11_BUFFER_DESC cbDesc;
    ZeroMemory( &cbDesc, sizeof(cbDesc) );
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    cbDesc.ByteWidth = sizeof( CB_PER_FRAME );
    hr = pd3dDevice->CreateBuffer( &cbDesc, NULL, &m_pcbPerFrame ) ;
    if (FAILED(hr))
        ::MessageBoxA(0, "Fail to create constant buffer for frame", "d3d error", 0);
}

//--------------------------------------------------------------------------------------
//
// CreateRenderStateObjects
//
// Creates the render state objects for hair rendering
//
//--------------------------------------------------------------------------------------
void CTressFXRender::CreateRenderStateObjects( ID3D11Device* pd3dDevice )
{
    HRESULT hr;
    
	// Create depth stencil states
    D3D11_DEPTH_STENCIL_DESC DSDesc;
    DSDesc.DepthEnable                  = TRUE;
    DSDesc.DepthFunc                    = D3D11_COMPARISON_LESS_EQUAL;
    DSDesc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ALL;
    DSDesc.StencilEnable                = FALSE;
    DSDesc.StencilReadMask              = 0xff;
    DSDesc.StencilWriteMask             = 0xff;
    DSDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    DSDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    DSDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
    DSDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
    DSDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
    DSDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
    DSDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_KEEP;
    DSDesc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;
    hr = pd3dDevice->CreateDepthStencilState(&DSDesc, &m_pDepthTestEnabledDSS);

    DSDesc.DepthEnable                  = TRUE;
    DSDesc.DepthFunc                    = D3D11_COMPARISON_LESS_EQUAL;
    DSDesc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ZERO;
    DSDesc.StencilEnable                = TRUE;
    DSDesc.StencilReadMask              = 0xFF;
    DSDesc.StencilWriteMask             = 0xFF;
    DSDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    DSDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    DSDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_INCR_SAT;
    DSDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
    DSDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
    DSDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
    DSDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_INCR_SAT;
    DSDesc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;
    hr = pd3dDevice->CreateDepthStencilState(&DSDesc, &m_pDepthTestEnabledNoDepthWritesStencilWriteIncrementDSS);

    DSDesc.DepthEnable                  = FALSE;
    DSDesc.DepthFunc                    = D3D11_COMPARISON_LESS_EQUAL;
    DSDesc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ZERO;
    DSDesc.StencilEnable                = TRUE;
    DSDesc.StencilReadMask              = 0xFF;
    DSDesc.StencilWriteMask             = 0x00;
    DSDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
    DSDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    DSDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
    DSDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_LESS;
    DSDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
    DSDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_KEEP;
    DSDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_KEEP;
    DSDesc.BackFace.StencilFunc         = D3D11_COMPARISON_LESS;
    hr = pd3dDevice->CreateDepthStencilState(&DSDesc, &m_pDepthTestDisabledStencilTestLessDSS);

    // Create sampler state objects
    D3D11_SAMPLER_DESC samDesc;
    ZeroMemory( &samDesc, sizeof(samDesc) );
    samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samDesc.AddressU = samDesc.AddressV = samDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samDesc.MaxAnisotropy = 16;
    samDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samDesc.MinLOD = 0;
    samDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = pd3dDevice->CreateSamplerState( &samDesc, &m_pSamplerStateLinearWrap ) ;
    if(FAILED(hr))
        ::MessageBoxA(0, "Fail to create linear wrap sampler state", "D3D Error", 0);

    samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    hr = pd3dDevice->CreateSamplerState( &samDesc, &m_pSamplerStatePointClamp ) ;
    if(FAILED(hr))
        ::MessageBoxA(0, "Fail to create point sampler state", "D3D Error", 0);

    samDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    samDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
    samDesc.BorderColor[0] = samDesc.BorderColor[1] = 
        samDesc.BorderColor[2] = samDesc.BorderColor[3] = 1;
    samDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    samDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    samDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    if(FAILED(pd3dDevice->CreateSamplerState( &samDesc, &m_pSamplerStateCmpLess )))
        ::MessageBoxA(0, "Fail to create compare sampler state", "D3D error", 0);

    // Create blend state objects
    D3D11_BLEND_DESC blendDesc;
    ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
    blendDesc.IndependentBlendEnable = false;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE ;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;//D3D11_BLEND_SRC_ALPHA  ;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO ;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD ;
    hr = pd3dDevice->CreateBlendState(&blendDesc, &m_pBlendStateBlendToBg);
    if(FAILED(hr))
        ::MessageBoxA(0, "Fail to create blend state: blend to background", "D3D Error", 0);

    // Create a blend state to disable color writes
    blendDesc.RenderTarget[0].SrcBlend  = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = 0;
    hr = pd3dDevice->CreateBlendState(&blendDesc, &m_pColorWritesOff);

}

//--------------------------------------------------------------------------------------
//
// OnCreateDevice
//
// Called when the device is created to create resources for hair rendering
//
//--------------------------------------------------------------------------------------
void CTressFXRender::OnCreateDevice( ID3D11Device* pd3dDevice, CTressFXMesh* pTressFXMesh, 
						AMD::ShaderCache *pShaderCache )
{
	m_pTressFXMesh = pTressFXMesh;
    CreateShaderAndLayout(pd3dDevice, pShaderCache);
    CreateTextureAndViews(pd3dDevice);
    CreateConstantBuffer(pd3dDevice);
    CreateVertexBuffers(pd3dDevice);
	CreateRenderStateObjects(pd3dDevice);
}

//--------------------------------------------------------------------------------------
//
// OnResizedSwapChain
//
// Called when the swap chain is being resized. Allocate resources that need to be
// created at this time because they are dependent on the size of the frame buffer.
//
//--------------------------------------------------------------------------------------
void CTressFXRender::OnResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr;

	// Release any previously allocated buffers
    SAFE_RELEASE(m_pHeadPPLL_Buffer);
    SAFE_RELEASE(m_pHeadPPLL_SRV);
    SAFE_RELEASE(m_pHeadPPLL_UAV);
    SAFE_RELEASE(m_pPPLL_Buffer);
    SAFE_RELEASE(m_pPPLL_UAV);
    SAFE_RELEASE(m_pPPLL_SRV) ;
	
	int winWidth, winHeight;

	winWidth = pBackBufferSurfaceDesc->Width;
	winHeight = pBackBufferSurfaceDesc->Height;

	// linked list head texture
    D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width = winWidth;
    texDesc.Height = winHeight;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R32_UINT;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    texDesc.MiscFlags = 0;
    texDesc.CPUAccessFlags = 0;
    texDesc.SampleDesc.Count=1;
    texDesc.SampleDesc.Quality=0;
	
    hr = pd3dDevice->CreateTexture2D(&texDesc, NULL, &m_pHeadPPLL_Buffer);
    if(FAILED(hr))
        ::MessageBoxW(0, L"Failed to create heads of linked lists texture", L"Error", 0);

	// SRV for linked list head
    D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
    srDesc.Format = DXGI_FORMAT_R32_UINT;
    srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srDesc.Texture2D.MipLevels = 1;
    srDesc.Texture2D.MostDetailedMip = 0;

    pd3dDevice->CreateShaderResourceView(m_pHeadPPLL_Buffer, &srDesc, &m_pHeadPPLL_SRV);
    if(FAILED(hr))
        ::MessageBoxW(0, L"Failed to create SRV for the heads of linked lists texture", L"d3d error", 0);

    // Create UAV view of the buffer containing the heads of the per-pixel linked lists
    D3D11_UNORDERED_ACCESS_VIEW_DESC UAVHeadPPLLBufferDesc;
    UAVHeadPPLLBufferDesc.Format			  = DXGI_FORMAT_R32_UINT;
    UAVHeadPPLLBufferDesc.ViewDimension       = D3D11_UAV_DIMENSION_TEXTURE2D;
	UAVHeadPPLLBufferDesc.Texture2D.MipSlice  = 0;
    hr = pd3dDevice->CreateUnorderedAccessView(m_pHeadPPLL_Buffer, &UAVHeadPPLLBufferDesc, &m_pHeadPPLL_UAV);
    if (FAILED(hr))
        ::MessageBoxA(0, "Failed to create UAV for the heads of linked lists texture ", "d3d error", 0);

	// Per-pixel Linked List (PPLL) buffer
    D3D11_BUFFER_DESC BufferDesc;
    BufferDesc.BindFlags               = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    BufferDesc.ByteWidth               = (DWORD)(g_HairTotalLayers * winWidth * winHeight * sizeof(PER_PIXEL_LINKED_LIST_STRUCT) );
    BufferDesc.CPUAccessFlags          = 0;
    BufferDesc.MiscFlags               = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    BufferDesc.Usage                   = D3D11_USAGE_DEFAULT;
    BufferDesc.StructureByteStride     = sizeof(PER_PIXEL_LINKED_LIST_STRUCT);
    hr = ( pd3dDevice->CreateBuffer(&BufferDesc, NULL, &m_pPPLL_Buffer) );
    if (FAILED(hr))
        ::MessageBoxA(0, "Failed to create per-pixel linked list buffer ", "d3d error", 0);
	
    // Create UAV view of Fragment and Link Buffer
    D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
    UAVDesc.Format                = DXGI_FORMAT_UNKNOWN;
    UAVDesc.ViewDimension         = D3D11_UAV_DIMENSION_BUFFER;
    UAVDesc.Buffer.FirstElement   = 0;
    UAVDesc.Buffer.NumElements    = (DWORD)(g_HairTotalLayers * winWidth * winHeight);
    UAVDesc.Buffer.Flags          = D3D11_BUFFER_UAV_FLAG_COUNTER;
    hr = pd3dDevice->CreateUnorderedAccessView(m_pPPLL_Buffer, &UAVDesc, &m_pPPLL_UAV);
    if (FAILED(hr))
        ::MessageBoxA(0, "Failed to create UAV for per-pixel linked list buffer ", "d3d error", 0);

    // Create SRV view of Fragment and Link Buffer
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
    SRVDesc.Format                = DXGI_FORMAT_UNKNOWN;
    SRVDesc.ViewDimension         = D3D11_SRV_DIMENSION_BUFFER;
    SRVDesc.Buffer.ElementOffset  = 0;
    SRVDesc.Buffer.ElementWidth   = (DWORD)(g_HairTotalLayers * winWidth * winHeight);
    hr = pd3dDevice->CreateShaderResourceView(m_pPPLL_Buffer, &SRVDesc, &m_pPPLL_SRV);
    if (FAILED(hr))
        ::MessageBoxA(0, "Failed to create SRV for per-pixel linked list buffer ", "d3d error", 0);
}


//--------------------------------------------------------------------------------------
//
// SetConstantBuffer
//
// Sets the GPU constant buffer values
// 
//--------------------------------------------------------------------------------------
void CTressFXRender::SetConstantBuffer( ID3D11DeviceContext* pd3dContext, 
                                        CModelViewerCamera* pCamera, CModelViewerCamera* pCameraLight, /* view camera and light camera */ 
                                        HAIR_PARAMS& hair_params, D3DXMATRIX *pModelTransformForHead,
										float screenWidth, float screenHeight)
{
    HRESULT hr;

    // Set up camera parameters for when the camera is at the position of the light for rendering the shadow map
    D3DXMATRIX mViewLight, mProjLight, mViewProjLight;
    D3DXVECTOR3 vLightPosition = *(pCameraLight->GetEyePt());
    D3DXVECTOR3 vLightAt = m_pTressFXMesh->m_bSphere.center;
    D3DXVECTOR3 vUp = D3DXVECTOR3(0,1,0);
    D3DXMatrixLookAtLH(&mViewLight, &vLightPosition, &vLightAt, &vUp);

    D3DXVECTOR3 vLightToObject = vLightPosition - m_pTressFXMesh->m_bSphere.center;
    float dis = D3DXVec3Length(&vLightToObject);

    float min_dis = max(0.001f, dis - m_pTressFXMesh->m_bSphere.radius);
    float max_dis = dis + m_pTressFXMesh->m_bSphere.radius;

    float halfAngle = 1.5f*asin(m_pTressFXMesh->m_bSphere.radius/dis);
    float vfov = 2*halfAngle;

    float ratio = 1;

    D3DXMatrixPerspectiveFovLH(&mProjLight, vfov, ratio, min_dis, max_dis );
    mViewProjLight = mViewLight * mProjLight;

    // Map the per-frame constant buffer
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    hr = pd3dContext->Map( m_pcbPerFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) ;
    if (FAILED(hr))
        ::MessageBoxA(0, "Fail to map constant buffer", "d3d error", 0);

    CB_PER_FRAME* pcbPerFrame = ( CB_PER_FRAME* )MappedResource.pData;

    // camera parameters
    D3DXMATRIX mWorld = *(pCamera->GetWorldMatrix());
    D3DXMATRIX mView = *(pCamera->GetViewMatrix());
    D3DXMATRIX mProj = *(pCamera->GetProjMatrix());
    D3DXMATRIX mViewProj = mView * mProj;

    D3DXMATRIX mInvViewProj;
    D3DXMatrixInverse(&mInvViewProj, 0, &mViewProj);

    float fRenderWidth =  (float)DXUTGetDXGIBackBufferSurfaceDesc()->Width;
    float fRenderHeight = (float)DXUTGetDXGIBackBufferSurfaceDesc()->Height;

    // Inverse of viewprojection matrix with viewport mapping
    D3DXMATRIX mViewport  ( 2.0f/fRenderWidth,  0.0f,                   0.0f, 0.0f,
                            0.0f,               -2.0f/fRenderHeight,    0.0f, 0.0f,
                            0.0f,               0.0f,                   1.0f, 0.0f,
                            -1.0f,              1.0f,                   0.0f, 1.0f );

    D3DXMATRIX mInvViewProjViewport = mViewport * mInvViewProj;

    D3DXMatrixTranspose( &pcbPerFrame->m_mViewProj, &mViewProj);
    D3DXMatrixTranspose( &pcbPerFrame->m_mInvViewProj, &mInvViewProj);

    D3DXMatrixTranspose( &pcbPerFrame->m_mInvViewProjViewport, &mInvViewProjViewport);

    D3DXMatrixTranspose( &pcbPerFrame->m_mWorld, pModelTransformForHead);

    pcbPerFrame->m_vEye = *(pCamera->GetEyePt());

    pcbPerFrame->m_fvFOV = D3DX_PI/4;

    // Light camera parameters
    D3DXMATRIX mInvViewProjLight;
    D3DXMatrixInverse(&mInvViewProjLight, 0, &mViewProjLight);

    D3DXMatrixTranspose( &pcbPerFrame->m_mViewProjLight, &mViewProjLight);
    pcbPerFrame->m_fNearLight = min_dis;
    pcbPerFrame->m_fFarLight = max_dis;

    pcbPerFrame->m_PointLightPos = D3DXVECTOR4(vLightPosition, 1);

    // scene light color
    pcbPerFrame->m_AmbientLightColor = D3DXVECTOR4(g_vAmbientLight, 1.0);
    pcbPerFrame->m_PointLightColor = D3DXVECTOR4( g_vPointLight, 1.f );

    // hair material
    pcbPerFrame->m_MatBaseColor =  D3DXVECTOR4(hair_params.color, 1);
    pcbPerFrame->m_MatKValue = D3DXVECTOR4(hair_params.Ka, hair_params.Kd, hair_params.Ks1, hair_params.Ex1);
    pcbPerFrame->m_fHairKs2 = hair_params.Ks2;
    pcbPerFrame->m_fHairEx2 = hair_params.Ex2;
	
    pcbPerFrame->m_FiberAlpha = hair_params.alpha;
    pcbPerFrame->m_HairSMAlpha = hair_params.alpha_sm;
	
    pcbPerFrame->m_FiberRadius = hair_params.radius;

    pcbPerFrame->m_FiberSpacing = hair_params.fiber_spacing;

    pcbPerFrame->m_bThinTip = (hair_params.bThinTip ? 1.f : -1.f);
    pcbPerFrame->m_bExpandPixels = 1; 

    pcbPerFrame->m_WinSize = D3DXVECTOR4((float)screenWidth, (float)screenHeight, 1.0f/(float)screenWidth, 1.0f/(float)screenHeight);

	pcbPerFrame->m_iMaxFragments = hair_params.iMaxFragments;

	pcbPerFrame->m_alphaThreshold = ALPHA_THRESHOLD / (hair_params.density * hair_params.density * hair_params.density);
  
    pcbPerFrame->m_iTechSM = hair_params.tech_shadow;
	pcbPerFrame->m_bUseCoverage = hair_params.bUseCoverage ? 1 : 0;
	pcbPerFrame->m_iStrandCopies = hair_params.iStrandCopies;

    pd3dContext->Unmap( m_pcbPerFrame, 0 );

    // Set constant buffer for vertex and pixel shader
    pd3dContext->VSSetConstantBuffers( 1, 1, &m_pcbPerFrame );
    pd3dContext->PSSetConstantBuffers( 1, 1, &m_pcbPerFrame );
}

//--------------------------------------------------------------------------------------
//
// SetSamplerStates
//
// Sets the sampler states for hair rendering
// 
//--------------------------------------------------------------------------------------
void CTressFXRender::SetSamplerStates( ID3D11DeviceContext* pd3dContext )
{
    // Set texture sampler states
    ID3D11SamplerState* pSS[3];
    pSS[0] = m_pSamplerStateLinearWrap;
    pSS[1] = m_pSamplerStatePointClamp;
    pSS[2] = m_pSamplerStateCmpLess;
    pd3dContext->PSSetSamplers(0, 3, pSS);
}

//--------------------------------------------------------------------------------------
//
// RenderHairGeometry
//
// Renders the hair geometry as individual strands of hair
// 
//--------------------------------------------------------------------------------------
void CTressFXRender::RenderHairGeometry( ID3D11DeviceContext* pd3dContext,
                                ID3D11VertexShader* pVS, 
                                ID3D11PixelShader* pPS,
                                float density,
                                bool useLinePrimitives /*= true*/,
								int	iStrandCopies /*= 1*/) 
{
    
	pd3dContext->VSSetShader( pVS, NULL, 0 );
    pd3dContext->PSSetShader( pPS, NULL, 0 );

    pd3dContext->PSSetShaderResources(IDSRV_NOISEMAP, 1, &m_pNoiseSRV);
	pd3dContext->VSSetShaderResources(IDSRV_NOISEMAP, 1, &m_pNoiseSRV);
	pd3dContext->VSSetShaderResources( IDSRV_HAIR_VERTEX_POSITIONS, 1, &m_pTressFXMesh->m_HairVertexPositionsSRV );
    pd3dContext->VSSetShaderResources( IDSRV_HAIR_TANGENTS, 1, &m_pTressFXMesh->m_HairVertexTangentsSRV );

    if (useLinePrimitives)
	{
		// render line primitives for the shadow map
        pd3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_LINELIST );

        UINT strides = sizeof( StrandVertex );
        UINT offsets = 0;
        pd3dContext->IASetInputLayout( m_pLayoutHair_2 );
        pd3dContext->IASetVertexBuffers( 0, 1, &m_pTressFXMesh->m_pVertexBuffer, &strides, &offsets );
        pd3dContext->IASetIndexBuffer( m_pTressFXMesh->m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );

        pd3dContext->DrawIndexed(UINT( density * m_pTressFXMesh->m_TotalIndexCount), 0, 0 );
    }
    else
	{
		// triangle rendering
        pd3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

        UINT stride = 0;
        UINT offset = 0;
        pd3dContext->IASetInputLayout( 0 );
        ID3D11Buffer* buf[]={0};
        pd3dContext->IASetVertexBuffers( 0, 1, buf, &stride, &offset );
        pd3dContext->IASetIndexBuffer( m_pTressFXMesh->m_pTriangleIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );

    	pd3dContext->VSSetShaderResources( IDSRV_HAIR_THICKNESSES, 1, &m_pTressFXMesh->m_pThicknessCoeffsSRV);

        pd3dContext->DrawIndexedInstanced(UINT( density * m_pTressFXMesh->m_TotalTriangleIndexCount), iStrandCopies, 0, 0, 0);  // If iStrandCopies>1 a different VS is set
    }

    ID3D11ShaderResourceView* nullViews[] = { NULL };
    pd3dContext->VSSetShaderResources( IDSRV_HAIR_VERTEX_POSITIONS, 1, nullViews );
    pd3dContext->VSSetShaderResources( IDSRV_HAIR_TANGENTS, 1, nullViews );
}

//--------------------------------------------------------------------------------------
//
// RenderScreenQuad
//
// Renders a full screen quad
// 
//--------------------------------------------------------------------------------------
void CTressFXRender::RenderScreenQuad(ID3D11DeviceContext* pd3dContext,
    ID3D11VertexShader* pVS, ID3D11PixelShader* pPS)
{
    // set shader
    pd3dContext->VSSetShader( pVS, NULL, 0 );
    pd3dContext->PSSetShader( pPS, NULL, 0 );

    UINT strides = sizeof( StandardVertex );
    UINT offsets = 0;
    pd3dContext->IASetInputLayout( m_pLayoutHair_5 );
    pd3dContext->IASetVertexBuffers( 0, 1, &m_pScreenQuadVB, &strides, &offsets );
    pd3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Draw full screen quad
    pd3dContext->Draw( 6, 0);
}


//--------------------------------------------------------------------------------------
//
// GenerateShadowMap
//
// Renders the hair from the point of view of the light into a shadow map
// 
//--------------------------------------------------------------------------------------
void CTressFXRender::GenerateShadowMap(ID3D11DeviceContext* pd3dContext, float density,
						float screenWidth, float screenHeight)
{
    // Get original render target and depth stencil view
    ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
    ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();

    D3D11_VIEWPORT viewportSMHair = {0, 0, SM_HAIR_WIDTH, SM_HAIR_HEIGHT, 0.0f, 1.0f};
    pd3dContext->RSSetViewports( 1, &viewportSMHair );
    // clear depth for early z
    pd3dContext->ClearDepthStencilView(m_pSMHairDSV, D3D11_CLEAR_DEPTH|D3D10_CLEAR_STENCIL, 1.0, 0);
    // set render target to shadow map texture
    pd3dContext->OMSetRenderTargets(0, 0, m_pSMHairDSV);
    RenderHairGeometry(pd3dContext, m_pVSGenerateHairSM, NULL, density);
    
    // reset view port
    D3D11_VIEWPORT viewportWin = {0, 0, (float)screenWidth, (float)screenHeight, 0.0f, 1.0f};
    pd3dContext->RSSetViewports(1, &viewportWin);

    // reset render targets
    pd3dContext->OMSetRenderTargets(1, &pRTV, pDSV);

	// set the hair shadow map shader resource view
	pd3dContext->PSSetShaderResources(IDSRV_HAIRSM, 1, &m_pSMHairSRV);
}


//--------------------------------------------------------------------------------------
//
// RenderHair
//
// Renders the hair in two passes. The first pass fills an A-buffer by rendering the
// hair geometry into a per-pixel linked list which keeps all of the overlapping fragments.
// The second pass renders a full screen quad (using a stencil mask set in the first pass 
// to avoid unecssary pixels) which reads fragments from the per-pixel linked list
// and blends the nearest k fragments (K-buffer) in back to front order.
// 
//--------------------------------------------------------------------------------------
void CTressFXRender::RenderHair(ID3D11DeviceContext* pd3dContext, HAIR_PARAMS &hairParams)
{
    // Get original render target and depth stencil view
	TIMER_Begin( 0, L"ABufferFill" );
    ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
    ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();

    // render hair

    const UINT dwClearDataMinusOne[1] = {0xFFFFFFFF};
	pd3dContext->ClearUnorderedAccessViewUint(m_pHeadPPLL_UAV, dwClearDataMinusOne);

    // Clear stencil buffer to mask the rendering area
    // Keep depth buffer for correct depth and early z
    pd3dContext->ClearDepthStencilView(pDSV, D3D10_CLEAR_STENCIL, 1.0, 0); 

	ID3D11UnorderedAccessView*  pUAV[] = {m_pHeadPPLL_UAV, m_pPPLL_UAV, NULL, NULL, NULL, NULL, NULL};
    UINT    pUAVCounters[] = { 0, 0, 0, 0, 0, 0, 0 };
    pd3dContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &pRTV, pDSV, 1, 7, pUAV, pUAVCounters);

    // disable color write if there is no need for fragments counting
    pd3dContext->OMSetBlendState(m_pColorWritesOff, 0, 0xffffffff);

    // Enable depth test to use early z, disable depth write to make sure required layers won't be clipped out in early z
    pd3dContext->OMSetDepthStencilState(m_pDepthTestEnabledNoDepthWritesStencilWriteIncrementDSS, 0x00);

	// Pass 1: A-Buffer pass
	if(hairParams.bUseCoverage)
	{
		if(hairParams.iStrandCopies > 1)
			RenderHairGeometry(pd3dContext, m_pVSRenderHairAAStrandCopies, m_pPSABuffer_Hair, hairParams.density, false, hairParams.iStrandCopies);
		else
			RenderHairGeometry(pd3dContext, m_pVSRenderHairAA, m_pPSABuffer_Hair, hairParams.density, false, 1);
	}
	else
	{
		if(hairParams.iStrandCopies > 1)
			RenderHairGeometry(pd3dContext, m_pVSRenderHairStrandCopies, m_pPSABuffer_Hair, hairParams.density, false, hairParams.iStrandCopies);
		else
			RenderHairGeometry(pd3dContext, m_pVSRenderHair, m_pPSABuffer_Hair, hairParams.density, false, 1);
	}
	TIMER_End();

	// Pass 2: K-Buffer pass
	TIMER_Begin( 0, L"KBufferSortAndDraw" );
    pd3dContext->OMSetBlendState(m_pBlendStateBlendToBg, 0, 0xffffffff);
    pd3dContext->OMSetDepthStencilState(m_pDepthTestDisabledStencilTestLessDSS, 0x00);

	pUAV[0] = pUAV[1] = pUAV[2] = pUAV[3] = pUAV[4] = pUAV[5] = pUAV[6] = 0; 
    pd3dContext->OMSetRenderTargetsAndUnorderedAccessViews(1, &pRTV, pDSV, 1, 7, pUAV, pUAVCounters);

    pd3dContext->PSSetShaderResources(IDSRV_HEAD_PPLL, 1, &m_pHeadPPLL_SRV);
    pd3dContext->PSSetShaderResources(IDSRV_PPLL, 1, &m_pPPLL_SRV);

	RenderScreenQuad(pd3dContext, m_pVSScreenQuad, m_pPSKBuffer_Hair);
	TIMER_End();

    ID3D11ShaderResourceView* pNULL = NULL;
    pd3dContext->PSSetShaderResources(IDSRV_HEAD_PPLL, 1, &pNULL);
    pd3dContext->PSSetShaderResources(IDSRV_PPLL, 1, &pNULL);

    pd3dContext->OMSetDepthStencilState(m_pDepthTestEnabledDSS, 0x00);
    pd3dContext->OMSetRenderTargets(1, &pRTV, pDSV);
    pd3dContext->OMSetBlendState(NULL, 0, 0xffffffff);
}


//--------------------------------------------------------------------------------------
//
// BeginHairFrame
//
// Start of hair rendering.
// 
//--------------------------------------------------------------------------------------
void CTressFXRender::BeginHairFrame(ID3D11DeviceContext* pd3dContext, CModelViewerCamera* pCamera, 
	CModelViewerCamera* pCameraLight, HAIR_PARAMS& hair_params, D3DXMATRIX *pModelTrasnsformForHead,
	float screenWidth, float screenHeight)
{
    SetSamplerStates(pd3dContext);
    SetConstantBuffer(pd3dContext, pCamera, pCameraLight, hair_params, pModelTrasnsformForHead, 
						screenWidth, screenHeight);
}

//--------------------------------------------------------------------------------------
//
// EndHairFrame
//
// Start of hair rendering.
// 
//--------------------------------------------------------------------------------------
void CTressFXRender::EndHairFrame(ID3D11DeviceContext* pd3dContext, HAIR_PARAMS& hair_params)
{
    ID3D11ShaderResourceView* pNull = 0;
    pd3dContext->PSSetShaderResources(IDSRV_HAIRSM, 1, &pNull);
}

//--------------------------------------------------------------------------------------
//
// OnDestroy
//
// Called when device is destroyed.
// 
//--------------------------------------------------------------------------------------
void CTressFXRender::OnDestroy()
{	

    SAFE_RELEASE(m_pScreenQuadVB);

    // vs for rendering hair from the eye camera.
    SAFE_RELEASE(m_pVSRenderHair);
	SAFE_RELEASE(m_pVSRenderHairAA);

	SAFE_RELEASE(m_pVSRenderHairStrandCopies);
	SAFE_RELEASE(m_pVSRenderHairAAStrandCopies);

    // vs to generate hair SM from the light camera.
    SAFE_RELEASE(m_pVSGenerateHairSM);

    // A-buffer
    SAFE_RELEASE(m_pPSABuffer_Hair);
    // PS composite nearest k hair fragments
    SAFE_RELEASE(m_pPSKBuffer_Hair)	;

    SAFE_RELEASE(m_pVSScreenQuad);

    // constant buffer
    SAFE_RELEASE(m_pcbPerFrame);

    // vertex layouts for hair
    SAFE_RELEASE( m_pLayoutHair_0 );
    SAFE_RELEASE( m_pLayoutHair_1 );
    SAFE_RELEASE( m_pLayoutHair_2 );
    SAFE_RELEASE( m_pLayoutHair_3 );
    SAFE_RELEASE( m_pLayoutHair_4 );
    SAFE_RELEASE( m_pLayoutHair_5 );

    // Head of PPLL buffer (roots of the linked list)
    SAFE_RELEASE(m_pHeadPPLL_Buffer);
    SAFE_RELEASE(m_pHeadPPLL_SRV);
    SAFE_RELEASE(m_pHeadPPLL_UAV);

    // PPLL buffer(all linked list data)
    SAFE_RELEASE(m_pPPLL_Buffer);
    SAFE_RELEASE(m_pPPLL_UAV);
    SAFE_RELEASE(m_pPPLL_SRV) ;

    // textures and views
    SAFE_RELEASE(m_pNoiseTexture2D);
    SAFE_RELEASE(m_pNoiseSRV);

    // Hair shadow map depth stencil buffer
    SAFE_RELEASE(m_pSMHairTx);
    SAFE_RELEASE(m_pSMHairDSV);
    SAFE_RELEASE(m_pSMHairSRV);

	// render states
    SAFE_RELEASE(m_pBlendStateBlendToBg);
    SAFE_RELEASE(m_pColorWritesOff );
    SAFE_RELEASE( m_pDepthTestEnabledDSS );
    SAFE_RELEASE( m_pDepthTestEnabledNoDepthWritesStencilWriteIncrementDSS );
    SAFE_RELEASE( m_pDepthTestDisabledStencilTestLessDSS );
    SAFE_RELEASE(m_pSamplerStateLinearWrap );
    SAFE_RELEASE(m_pSamplerStatePointClamp );
    SAFE_RELEASE(m_pSamplerStateCmpLess);
}
