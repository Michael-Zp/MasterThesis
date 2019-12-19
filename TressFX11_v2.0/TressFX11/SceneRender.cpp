//--------------------------------------------------------------------------------------
// File: SceneRender.cpp
//
// Main scene rendering code
// 
// Copyright © AMD Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "..\\DXUT\\Core\\DXUT.h"
#include "SceneRender.h"
#include "TressFXRender.h"
#include "..\\AMD_SDK\\AMD_SDK.h"

extern CTressFXRender g_TressFXRender;

struct CB_PER_MATERIAL
{
    D3DXVECTOR4		m_MatAmbient	;
    D3DXVECTOR4		m_MatDiffuse	; // alpha value: whether texture is used (yes: 1; no: -1)
    D3DXVECTOR4		m_MatSpecular	; // alpha value: Specular Exponent
    D3DXVECTOR4     m_ScalpColor    ;
};

struct CB_BLUR
{
	float		width;
	float		height;
	float		invWidth;
	float		invHeight;
};

extern D3DXVECTOR3 g_HairColor;

//--------------------------------------------------------------------------------------
//
// CreateShaderAndLayout
//
// Creates the shaders and layouts for scene rendering
//
//--------------------------------------------------------------------------------------
void CSceneRender::CreateShaderAndLayout( ID3D11Device* pd3dDevice, AMD::ShaderCache *pShaderCache )
{

    // Ensure all shaders (and input layouts) are released
    SAFE_RELEASE( m_pLayoutScene );
    SAFE_RELEASE( m_pLayoutScene_1 );
    SAFE_RELEASE( m_pLayoutScene_2 );

    // the vertex layout for the mesh structure
    const D3D11_INPUT_ELEMENT_DESC layout_mesh[] =
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
		pShaderCache->AddShader( (ID3D11DeviceChild**)&m_pVSRenderScene, AMD::ShaderCache::SHADER_TYPE_VERTEX, 
			L"vs_5_0", L"VS_RenderScene",L"shaders\\Source\\SceneRender.hlsl", 0, NULL, &m_pLayoutScene, 
			(D3D11_INPUT_ELEMENT_DESC*)layout_mesh, ARRAYSIZE( layout_mesh ) );
		pShaderCache->AddShader( (ID3D11DeviceChild**)&m_pVSGenerateSceneSM, AMD::ShaderCache::SHADER_TYPE_VERTEX, 
			L"vs_5_0", L"VS_GenerateSceneSM",L"shaders\\Source\\SceneRender.hlsl", 0, NULL, &m_pLayoutScene_1, 
			(D3D11_INPUT_ELEMENT_DESC*)layout_mesh, ARRAYSIZE( layout_mesh ) );
		pShaderCache->AddShader( (ID3D11DeviceChild**)&m_pVSScreenQuad, AMD::ShaderCache::SHADER_TYPE_VERTEX, 
			L"vs_5_0", L"VS_ScreenQuad",L"shaders\\Source\\TressFXInclude.hlsl", 0, NULL, &m_pLayoutScene_2, 
			(D3D11_INPUT_ELEMENT_DESC*)layout_mesh, ARRAYSIZE( layout_mesh ) );

		// pixel shaders
		pShaderCache->AddShader( (ID3D11DeviceChild**)&m_pPSRenderScene, AMD::ShaderCache::SHADER_TYPE_PIXEL, L"ps_5_0", L"PS_RenderScene",
			L"shaders\\Source\\SceneRender.hlsl", 0, NULL, NULL, NULL, 0 );
		pShaderCache->AddShader( (ID3D11DeviceChild**)&m_pPSRenderSceneShadow, AMD::ShaderCache::SHADER_TYPE_PIXEL, L"ps_5_0", L"PS_RenderSceneShadow",
			L"shaders\\Source\\SceneRender.hlsl", 0, NULL, NULL, NULL, 0 );
		
		AMD::ShaderCache::Macro Macros[4];
		wcscpy_s( Macros[0].m_wsName, AMD::ShaderCache::m_uMACRO_MAX_LENGTH, L"GAUSSIAN_FILTER" );
        Macros[0].m_iValue = 1;
        wcscpy_s( Macros[1].m_wsName, AMD::ShaderCache::m_uMACRO_MAX_LENGTH, L"KERNEL_RADIUS" );
        Macros[1].m_iValue = SHADOW_BLUR_RADIUS;
        wcscpy_s( Macros[2].m_wsName, AMD::ShaderCache::m_uMACRO_MAX_LENGTH, L"USE_APPROXIMATE_FILTER" );
        Macros[2].m_iValue = SHADOW_BLUR_PRECISION;
        wcscpy_s( Macros[3].m_wsName, AMD::ShaderCache::m_uMACRO_MAX_LENGTH, L"HORIZ" );
        Macros[3].m_iValue = 1;
		pShaderCache->AddShader( (ID3D11DeviceChild**)&m_pHorizontalBlurPS, AMD::ShaderCache::SHADER_TYPE_PIXEL, L"ps_5_0", L"PSFilterX",
			L"shaders\\Source\\GaussianFilter.hlsl", 4, Macros, NULL, NULL, 0 );
		wcscpy_s( Macros[3].m_wsName, AMD::ShaderCache::m_uMACRO_MAX_LENGTH, L"VERT" );
		Macros[3].m_iValue = 1;
		pShaderCache->AddShader( (ID3D11DeviceChild**)&m_pVerticalBlurPS, AMD::ShaderCache::SHADER_TYPE_PIXEL, L"ps_5_0", L"PSFilterY",
			L"shaders\\Source\\GaussianFilter.hlsl", 4, Macros, NULL, NULL, 0 );

	    bFirstPass = false;
	}
}

//--------------------------------------------------------------------------------------
//
// CreateTextureAndViews
//
// Creates the textures and corrisponding views for scene rendering
//
//--------------------------------------------------------------------------------------
void CSceneRender::CreateTextureAndViews( ID3D11Device* pd3dDevice )
{
    HRESULT hr;

    // Create SM DSVs for scene	
    D3D11_TEXTURE2D_DESC tex2D_desc;
    tex2D_desc.Width =  SM_SCENE_WIDTH;
    tex2D_desc.Height = SM_SCENE_HEIGHT;
    tex2D_desc.ArraySize = 1;
    tex2D_desc.Format = DXGI_FORMAT_R32_TYPELESS;
    tex2D_desc.Usage = D3D11_USAGE_DEFAULT;//
    tex2D_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL|D3D11_BIND_SHADER_RESOURCE  ;
    tex2D_desc.CPUAccessFlags = 0;
    tex2D_desc.MipLevels = 0;
    tex2D_desc.MiscFlags = 0;
    tex2D_desc.SampleDesc.Count = 1;
    tex2D_desc.SampleDesc.Quality = 0;
	
    hr = pd3dDevice->CreateTexture2D(&tex2D_desc, 0, &m_pSMSceneTx);
    if(FAILED(hr))
        ::MessageBoxW(0, L"fail to create m_pSMSceneTx", L"d3d Error", 0);
    
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    dsvDesc.Flags = 0  ;
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D ;
    dsvDesc.Texture2D.MipSlice = 0;
    if(FAILED(pd3dDevice->CreateDepthStencilView(m_pSMSceneTx, &dsvDesc, &m_pSMSceneDSV)))
        ::MessageBoxW(0, L"fail to create m_pSMSceneDSV", L"d3d Error", 0);
    
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    if(FAILED(pd3dDevice->CreateShaderResourceView(m_pSMSceneTx, &srvDesc, &m_pSMSceneSRV)))
        ::MessageBoxW(0, L"fail to create m_pMSceneSRV", L"d3d Error", 0);
    
	// Shadow texture for scene rendering
    tex2D_desc.Width =  SM_SCENE_WIDTH;
    tex2D_desc.Height = SM_SCENE_HEIGHT;
    tex2D_desc.MipLevels = 0;
    tex2D_desc.ArraySize = 1;
    tex2D_desc.Format = DXGI_FORMAT_R8_UNORM;
    tex2D_desc.SampleDesc.Count = 1;
    tex2D_desc.SampleDesc.Quality = 0;
    tex2D_desc.Usage = D3D11_USAGE_DEFAULT;
    tex2D_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    tex2D_desc.CPUAccessFlags = 0;
    tex2D_desc.MiscFlags = 0;
	
    hr = pd3dDevice->CreateTexture2D(&tex2D_desc, 0, &m_pSceneShadowTexture);
    if(FAILED(hr))
        ::MessageBoxW(0, L"fail to create m_pSceneShadowTexture", L"d3d Error", 0);

    hr = pd3dDevice->CreateTexture2D(&tex2D_desc, 0, &m_pSceneShadowTexture_temp);
    if(FAILED(hr))
        ::MessageBoxW(0, L"fail to create m_pSceneShadowTexture_temp", L"d3d Error", 0);

    D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
	srDesc.Format = DXGI_FORMAT_R8_UNORM;
    srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srDesc.Texture1D.MostDetailedMip = 0;
    srDesc.Texture1D.MipLevels = 0xffffffff;
    pd3dDevice->CreateShaderResourceView(m_pSceneShadowTexture, &srDesc, &m_pSceneShadowSRV);
    pd3dDevice->CreateShaderResourceView(m_pSceneShadowTexture_temp, &srDesc, &m_pSceneShadowSRV_temp);

	D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
	rtDesc.Format = DXGI_FORMAT_R8_UNORM;
	rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtDesc.Texture2D.MipSlice = 0;
	pd3dDevice->CreateRenderTargetView(m_pSceneShadowTexture, &rtDesc, &m_pSceneShadowRTV);
	pd3dDevice->CreateRenderTargetView(m_pSceneShadowTexture_temp, &rtDesc, &m_pSceneShadowRTV_temp);
}

//--------------------------------------------------------------------------------------
//
// CreateVertexBuffers
//
// Creates the vertex buffers for scene rendering
//
//--------------------------------------------------------------------------------------
void CSceneRender::CreateVertexBuffers(ID3D11Device* pd3dDevice)
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
// Creates the constant buffers for scene rendering
//
//--------------------------------------------------------------------------------------
void CSceneRender::CreateConstantBuffer( ID3D11Device* pd3dDevice )
{
    HRESULT hr;
    D3D11_BUFFER_DESC cbDesc;
    ZeroMemory( &cbDesc, sizeof(cbDesc) );
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    cbDesc.ByteWidth = sizeof( CB_PER_MATERIAL );
    hr = pd3dDevice->CreateBuffer( &cbDesc, NULL, &m_pcbPerMaterial ) ;
    if (FAILED(hr))
        ::MessageBoxA(0, "Fail to create constant buffer for material", "d3d error", 0);

	cbDesc.ByteWidth = sizeof( CB_BLUR );
	cbDesc.Usage = D3D11_USAGE_IMMUTABLE;
	cbDesc.CPUAccessFlags = 0;
	CB_BLUR initData;
	initData.width = (float)SM_SCENE_WIDTH;
	initData.height = (float)SM_SCENE_HEIGHT;
	initData.invWidth = 1.0f / (float)SM_SCENE_WIDTH;
	initData.invHeight = 1.0f / (float)SM_SCENE_HEIGHT;

	D3D11_SUBRESOURCE_DATA subResourceData;
	subResourceData.pSysMem = (void *)&initData;
    hr = pd3dDevice->CreateBuffer( &cbDesc, &subResourceData, &m_pcbBlur ) ;
    if (FAILED(hr))
        ::MessageBoxA(0, "Fail to create constant buffer for shadow blur", "d3d error", 0);
}

//--------------------------------------------------------------------------------------
//
// CreateRenderStateObjects
//
// Creates the constant buffers for scene rendering
//
//--------------------------------------------------------------------------------------
void CSceneRender::CreateRenderStateObjects( ID3D11Device* pd3dDevice )
{
    HRESULT hr;
    
	// Create depth stencil states
    D3D11_DEPTH_STENCIL_DESC DSDesc;
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
    DSDesc.DepthEnable                  = TRUE;
    DSDesc.StencilEnable                = FALSE;
    DSDesc.DepthFunc                    = D3D11_COMPARISON_LESS_EQUAL;
    DSDesc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ALL;
    hr = pd3dDevice->CreateDepthStencilState(&DSDesc, &m_pDepthTestEnabledDSS);

    DSDesc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ZERO;
    hr = pd3dDevice->CreateDepthStencilState(&DSDesc, &m_pDepthTestEnabledNoWriteDSS);

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

	ZeroMemory( &samDesc, sizeof(samDesc) );
    samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samDesc.AddressU = samDesc.AddressV = samDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samDesc.MaxAnisotropy = 1;
    samDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samDesc.MaxLOD = D3D11_FLOAT32_MAX;
    if(FAILED( pd3dDevice->CreateSamplerState( &samDesc, &m_pLinearClampSampler ) ) )
		::MessageBoxA(0, "Fail to create linear clamp sampler state", "D3D error", 0);
    // Point sampler
	samDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    if(FAILED( pd3dDevice->CreateSamplerState( &samDesc, &m_pPointSampler ) ) )
		::MessageBoxA(0, "Fail to create linear point sampler state", "D3D error", 0);

    // Create rasterizer state for shadow map rendering
    D3D11_RASTERIZER_DESC drd = {
        D3D11_FILL_SOLID,	//D3D11_FILL_MODE FillMode;
        D3D11_CULL_NONE,	//D3D11_CULL_MODE CullMode;
        FALSE,				//BOOL FrontCounterClockwise;
        256,				//INT DepthBias;
        0.0f,				//FLOAT DepthBiasClamp;
        2.f,				//FLOAT SlopeScaledDepthBias;
        FALSE,				//BOOL DepthClipEnable;
        FALSE,				//BOOL ScissorEnable;
        FALSE,				//BOOL MultisampleEnable;
        FALSE				//BOOL AntialiasedLineEnable;        
    };
    hr = pd3dDevice->CreateRasterizerState(&drd, &m_pRasterizerStateShadowmap);
	if(FAILED(hr))
      ::MessageBoxA(0, "Fail to create rasterizer state: sceneShadowmap", "D3D Error", 0);
}

//--------------------------------------------------------------------------------------
//
// OnCreateDevice
//
// Called when the device is created. Allocates resources used by the scene rendering.
//
//--------------------------------------------------------------------------------------
void CSceneRender::OnCreateDevice( ID3D11Device* pd3dDevice, const WCHAR* headFile,
									float fScale, D3DXVECTOR3 vRotation, 
									D3DXVECTOR3 vTranslation, AMD::ShaderCache *pShaderCache)
{
    m_MeshHead.CreateMeshFromOBJFile(pd3dDevice, headFile, 
        D3DXVECTOR3(fScale,fScale,fScale), vRotation, vTranslation);

    CreateShaderAndLayout(pd3dDevice, pShaderCache);
    CreateTextureAndViews(pd3dDevice);
	CreateVertexBuffers(pd3dDevice);
	CreateConstantBuffer(pd3dDevice);
	CreateRenderStateObjects(pd3dDevice);
}

//--------------------------------------------------------------------------------------
//
// RenderScreenQuad
//
// Renders a full screen quad
// 
//--------------------------------------------------------------------------------------
void CSceneRender::RenderScreenQuad(ID3D11DeviceContext* pd3dContext,
    ID3D11VertexShader* pVS, ID3D11PixelShader* pPS)
{
    // set shader
    pd3dContext->VSSetShader( pVS, NULL, 0 );
    pd3dContext->PSSetShader( pPS, NULL, 0 );

    UINT strides = sizeof( StandardVertex );
    UINT offsets = 0;
    pd3dContext->IASetInputLayout( m_pLayoutScene );
    pd3dContext->IASetVertexBuffers( 0, 1, &m_pScreenQuadVB, &strides, &offsets );
    pd3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Draw full screen quad
    pd3dContext->Draw( 6, 0);
}

//--------------------------------------------------------------------------------------
//
// GenerateShadowMap
//
// Renders the scene geometry from the point of view of the light into a shadow map.
//
//--------------------------------------------------------------------------------------
void CSceneRender::GenerateShadowMap(ID3D11DeviceContext* pd3dContext)
{
    // Get original render target and depth stencil view
    ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
    ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();
	ID3D11RasterizerState* pRSTemp;
	pd3dContext->RSGetState( &pRSTemp );

    // Set viewport value
    D3D11_VIEWPORT viewportSMScene = {0, 0, SM_SCENE_WIDTH, SM_SCENE_HEIGHT, 0.0f, 1.0f};
    pd3dContext->RSSetViewports( 1, &viewportSMScene );
    // clear depth for early z
    pd3dContext->ClearDepthStencilView(m_pSMSceneDSV, D3D11_CLEAR_DEPTH|D3D10_CLEAR_STENCIL, 1.0, 0);
	// make suure the scene shadow map texture is unbound
	ID3D11ShaderResourceView *pNull = NULL;
	pd3dContext->PSSetShaderResources(IDSRV_SCENESM, 1, &pNull);
    // set render target to shadow map texture
    pd3dContext->OMSetRenderTargets(0, 0, m_pSMSceneDSV);
	pd3dContext->RSSetState( m_pRasterizerStateShadowmap );

    RenderSceneGeometry(pd3dContext, m_pVSGenerateSceneSM, NULL);

    // reset view port
    D3D11_VIEWPORT viewportWin = {0, 0, (float)g_ScreenWidth, (float)g_ScreenHeight, 0.0f, 1.0f};
    pd3dContext->RSSetViewports(1, &viewportWin);

    // reset render targets
    pd3dContext->OMSetRenderTargets(1, &pRTV, pDSV);
	pd3dContext->RSSetState( pRSTemp );
	pRSTemp->Release();
}


//--------------------------------------------------------------------------------------
//
// RenderSceneGeometry
//
// Renders the scene geometry which is a head model, stored in a mesh.
//
//--------------------------------------------------------------------------------------
void CSceneRender::RenderSceneGeometry( ID3D11DeviceContext* pd3dContext, ID3D11VertexShader* pVS, ID3D11PixelShader* pPS)
{
    HRESULT hr;

    // set shaders
    pd3dContext->VSSetShader( pVS, NULL, 0 );
    pd3dContext->PSSetShader( pPS, NULL, 0 );

    UINT strides = sizeof( StandardVertex );
    UINT offsets = 0;

    pd3dContext->IASetInputLayout( m_pLayoutScene );
    pd3dContext->IASetVertexBuffers( 0, 1, &m_MeshHead.m_pVertexBuffer, &strides, &offsets );
    pd3dContext->IASetIndexBuffer( m_MeshHead.m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );
    pd3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    for(UINT i=0; i<m_MeshHead.m_Subsets.size(); i++)
    {
        // Set material value for each subset
        D3D11_MAPPED_SUBRESOURCE MappedResource;
        hr = pd3dContext->Map( m_pcbPerMaterial, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) ;
        if (FAILED(hr))
            ::MessageBoxA(0, "Fail to map constant buffer material", "d3d error", 0);

        CB_PER_MATERIAL* pcbPerMaterial = ( CB_PER_MATERIAL* )MappedResource.pData;

        const Material* pMat = &m_MeshHead.m_Materials[m_MeshHead.m_Subsets[i].materialID];
        pcbPerMaterial->m_MatAmbient = D3DXVECTOR4(pMat->vAmbient, 1);
        pcbPerMaterial->m_MatDiffuse = D3DXVECTOR4(pMat->vDiffuse, -1);
        pcbPerMaterial->m_MatSpecular = D3DXVECTOR4(pMat->vSpecular, (float)pMat->nShininess);
        pcbPerMaterial->m_ScalpColor = D3DXVECTOR4(g_HairColor.x, g_HairColor.y, g_HairColor.z, 1.0);

        // Set shader resource view
        if(	pMat->pTextureRV11)
        {
            pcbPerMaterial->m_MatDiffuse.w = 1;
            pd3dContext->PSSetShaderResources(IDSRV_SCENE, 1, &pMat->pTextureRV11);
        }

        pd3dContext->Unmap( m_pcbPerMaterial, 0 );

        pd3dContext->VSSetConstantBuffers( 2, 1, &m_pcbPerMaterial );
        pd3dContext->PSSetConstantBuffers( 2, 1, &m_pcbPerMaterial );

        int indexCount = m_MeshHead.m_Subsets[i].triangleCount*3;
        int startLocation = m_MeshHead.m_Subsets[i].startIndex;
        pd3dContext->DrawIndexed( indexCount, startLocation, 0 );
    }
}

//--------------------------------------------------------------------------------------
//
// RenderScene
//
// Renders the scene and generates the hair shadow for the scene model.
//
//--------------------------------------------------------------------------------------
void CSceneRender::RenderScene(ID3D11DeviceContext* pd3dContext, bool shadow)
{
  	// render the scene shadow
	D3D11_VIEWPORT shadowViewport = {0, 0, (float)SM_SCENE_WIDTH, (float)SM_SCENE_HEIGHT, 0.0f, 1.0f};
    pd3dContext->RSSetViewports(1, &shadowViewport);
	ID3D11ShaderResourceView* pNull = 0;
    pd3dContext->PSSetShaderResources(IDSVR_SCENE_SHADOW_TEXTURE, 1, &pNull);
	ID3D11RenderTargetView* pRTV = DXUTGetD3D11RenderTargetView();
	ID3D11DepthStencilView* pDSV = DXUTGetD3D11DepthStencilView();

	float clearColor[4] = { 1, 1, 1, 1 };
	pd3dContext->ClearRenderTargetView(m_pSceneShadowRTV, clearColor);

	// Create a hair shadow by rendering the hair shadow map into a buffer which will get blurred and projected on the scene
	if (shadow)
	{
		pd3dContext->PSSetShaderResources(IDSRV_SCENESM, 1, &pNull);
		pd3dContext->OMSetRenderTargets(1, &m_pSceneShadowRTV, m_pSMSceneDSV);
		ID3D11ShaderResourceView *pSMHairSRV = g_TressFXRender.GetShadowMapSRV();
		pd3dContext->PSSetShaderResources(IDSVR_SCENE_SHADOW_TEXTURE, 1, &pSMHairSRV);
		pd3dContext->OMSetDepthStencilState(m_pDepthTestEnabledNoWriteDSS, 0x00);
		// render the hair shadow map into the shadow texture
		RenderScreenQuad(pd3dContext, m_pVSScreenQuad, m_pPSRenderSceneShadow);
		pd3dContext->OMSetDepthStencilState(m_pDepthTestEnabledDSS, 0x00);

		// blur the shadow texture
		pd3dContext->PSSetSamplers( 0, 1, &m_pPointSampler );
		pd3dContext->PSSetSamplers( 1, 1, &m_pLinearClampSampler );
		pd3dContext->PSSetConstantBuffers( 0, 1, &m_pcbBlur );      
		// PS Horizontal filter pass
		pd3dContext->OMSetRenderTargets( 1, &m_pSceneShadowRTV_temp, NULL );
		pd3dContext->PSSetShaderResources(0, 1, &m_pSceneShadowSRV);
		RenderScreenQuad(pd3dContext, m_pVSScreenQuad, m_pHorizontalBlurPS);
		// PS Vertical filter pass
		ID3D11ShaderResourceView *pNullResource = NULL;
		pd3dContext->PSSetShaderResources(0, 1, &pNullResource);
		pd3dContext->OMSetRenderTargets( 1, &m_pSceneShadowRTV, NULL );
		pd3dContext->PSSetShaderResources(0, 1, &m_pSceneShadowSRV_temp);
		RenderScreenQuad(pd3dContext, m_pVSScreenQuad, m_pVerticalBlurPS);
		// restore the render target
		pd3dContext->OMSetRenderTargets(1, &pRTV, pDSV);	
		pd3dContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0 );
		// restore the samplers
		pd3dContext->PSSetSamplers( 0, 1, &m_pSamplerStateLinearWrap );
		pd3dContext->PSSetSamplers( 1, 1, &m_pSamplerStatePointClamp );
		pd3dContext->PSSetShaderResources(IDSRV_SCENESM, 1, &m_pSMSceneSRV);
	}

    // render scene
	D3D11_VIEWPORT sceneViewport = {0, 0, (float)g_ScreenWidth, (float)g_ScreenHeight, 0.0f, 1.0f};
    pd3dContext->RSSetViewports(1, &sceneViewport);
    pd3dContext->PSSetShaderResources(IDSVR_SCENE_SHADOW_TEXTURE, 1, &m_pSceneShadowSRV);
    RenderSceneGeometry(pd3dContext, m_pVSRenderScene, m_pPSRenderScene);
}

//--------------------------------------------------------------------------------------
//
// OnDestroy
//
// Called when device is destroyed.
// 
//--------------------------------------------------------------------------------------
void CSceneRender::OnDestroy()
{
    // Mesh body
    m_MeshHead.OnDestroy();
    
    SAFE_RELEASE(m_pScreenQuadVB);

    // vs for rendering scene  
    SAFE_RELEASE(m_pVSRenderScene);
    // vs for generate hair SM from the light camera.
    SAFE_RELEASE(m_pVSGenerateSceneSM);
    SAFE_RELEASE(m_pPSRenderScene);

    SAFE_RELEASE(m_pVSScreenQuad);

	// PS for generating scene shadow
	SAFE_RELEASE(m_pPSRenderSceneShadow);
	SAFE_RELEASE(m_pHorizontalBlurPS);
	SAFE_RELEASE(m_pVerticalBlurPS);

    // constant buffers
    SAFE_RELEASE(m_pcbBlur);
    SAFE_RELEASE(m_pcbPerMaterial);

    // vertex layout for scene
    SAFE_RELEASE(m_pLayoutScene);
    SAFE_RELEASE( m_pLayoutScene_1 );
    SAFE_RELEASE( m_pLayoutScene_2 );

    // Scene shadow map depth stencil buffer
    SAFE_RELEASE(m_pSMSceneTx);
    SAFE_RELEASE(m_pSMSceneDSV);
    SAFE_RELEASE(m_pSMSceneSRV);

	// scene shadow
    SAFE_RELEASE(m_pSceneShadowTexture);
    SAFE_RELEASE(m_pSceneShadowRTV);
    SAFE_RELEASE(m_pSceneShadowSRV);
    SAFE_RELEASE(m_pSceneShadowTexture_temp);
    SAFE_RELEASE(m_pSceneShadowRTV_temp);
    SAFE_RELEASE(m_pSceneShadowSRV_temp);

	// render states
	SAFE_RELEASE(m_pRasterizerStateShadowmap);
    SAFE_RELEASE( m_pDepthTestEnabledDSS );
    SAFE_RELEASE( m_pDepthTestEnabledNoWriteDSS );
    SAFE_RELEASE(m_pSamplerStateLinearWrap );
    SAFE_RELEASE(m_pSamplerStatePointClamp );
    SAFE_RELEASE(m_pLinearClampSampler);
    SAFE_RELEASE(m_pPointSampler);

}
