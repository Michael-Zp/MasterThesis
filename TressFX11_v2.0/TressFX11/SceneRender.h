//--------------------------------------------------------------------------------------
// File: SceneRender.h
//
// Header file for the scene rendering code 
// 
// Copyright © AMD Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once
#include "..\\..\\DXUT\\Optional\\DXUTcamera.h"
#include "DX11Mesh.h"
#include "TressFXRender.h"

extern int g_ScreenWidth;
extern int g_ScreenHeight;

#define SM_SCENE_WIDTH	640
#define SM_SCENE_HEIGHT	640
#define SHADOW_BLUR_RADIUS		6
#define SHADOW_BLUR_PRECISION	1 


class CSceneRender
{
private:
    // Mesh body
    CDX11Mesh					m_MeshHead;

    // VS for rendering the scene 
    ID3D11VertexShader*			m_pVSRenderScene;
    // VS for generating the scene shadow map
    ID3D11VertexShader*			m_pVSGenerateSceneSM;
 	ID3D11VertexShader*			m_pVSScreenQuad;
	// PS for rendering the scene
	ID3D11PixelShader*			m_pPSRenderScene;

	// pixel shaders for generating the scene shadow
	ID3D11PixelShader*			m_pPSRenderSceneShadow;
	ID3D11PixelShader*			m_pHorizontalBlurPS;
	ID3D11PixelShader*			m_pVerticalBlurPS;

	// constant buffers
	ID3D11Buffer*				m_pcbBlur;
    ID3D11Buffer*				m_pcbPerMaterial;

	// vertex buffer
	ID3D11Buffer*               m_pScreenQuadVB;

	// input layouts
    ID3D11InputLayout*			m_pLayoutScene;
    ID3D11InputLayout*			m_pLayoutScene_1;
    ID3D11InputLayout*			m_pLayoutScene_2;

    // Scene shadow map depth stencil buffer
    ID3D11Texture2D*			m_pSMSceneTx;
    ID3D11DepthStencilView*		m_pSMSceneDSV;
    ID3D11ShaderResourceView*	m_pSMSceneSRV;

	// Scene Shadow Textures
	ID3D11Texture2D*			m_pSceneShadowTexture;
	ID3D11ShaderResourceView*	m_pSceneShadowSRV;
	ID3D11RenderTargetView*		m_pSceneShadowRTV;
	ID3D11Texture2D*			m_pSceneShadowTexture_temp;
	ID3D11ShaderResourceView*	m_pSceneShadowSRV_temp;
	ID3D11RenderTargetView*		m_pSceneShadowRTV_temp;

	// Render States
	ID3D11DepthStencilState		*m_pDepthTestEnabledDSS;
	ID3D11DepthStencilState		*m_pDepthTestEnabledNoWriteDSS;
	ID3D11SamplerState			*m_pSamplerStateLinearWrap;
	ID3D11SamplerState			*m_pSamplerStatePointClamp;
	ID3D11SamplerState			*m_pLinearClampSampler;
	ID3D11SamplerState			*m_pPointSampler;
	ID3D11RasterizerState		*m_pRasterizerStateShadowmap;

private:
    void CreateShaderAndLayout( ID3D11Device* pd3dDevice, AMD::ShaderCache *pShaderCache);
    void CreateTextureAndViews(ID3D11Device* pd3dDevice);
    void CreateConstantBuffer(ID3D11Device* pd3dDevice);
    void CreateVertexBuffers(ID3D11Device* pd3dDevice);
	void CreateRenderStateObjects( ID3D11Device* pd3dDevice );

    void SetSamplerStates(ID3D11DeviceContext* pd3dContext);

    void RenderSceneGeometry(ID3D11DeviceContext* pd3dContext,
                    ID3D11VertexShader* pVS, ID3D11PixelShader* pPS);

	void RenderScreenQuad(ID3D11DeviceContext* pd3dContext,
                    ID3D11VertexShader* pVS, ID3D11PixelShader* pPS);

public:
	CSceneRender(void) {};
	~CSceneRender(void) {};

    void OnCreateDevice(ID3D11Device* pd3dDevice, const WCHAR* headFile, float fScale,
					D3DXVECTOR3 vRotation, D3DXVECTOR3 vTranslation, AMD::ShaderCache *pShaderCache);
    void GenerateShadowMap(ID3D11DeviceContext* pd3dContext);
    void RenderScene(ID3D11DeviceContext* pd3dContext, bool shadow);	
	void OnDestroy();
};

