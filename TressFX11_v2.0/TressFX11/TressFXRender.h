//--------------------------------------------------------------------------------------
// File: TressFXRender.h
//
// Header file for the main hair rendering code 
// 
// Copyright © AMD Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once
#include "..\\..\\DXUT\\Optional\\DXUTcamera.h"
#include "..\\AMD_SDK\\AMD_SDK.h"
#include "DX11Mesh.h"
#include "TressFXMesh.h"

// shader resource view slots
#define IDSRV_PPLL							0
#define IDSRV_HEAD_PPLL						1
#define IDSRV_SCENESM						2
#define IDSRV_HAIRSM						3
#define IDSRV_SCENE							4
#define IDSRV_NOISEMAP						5
#define IDSRV_HAIR_THICKNESSES				6
#define IDSRV_HAIR_VERTEX_POSITIONS	7
#define IDSRV_HAIR_TANGENTS			8
#define IDSVR_SCENE_SHADOW_TEXTURE			9


#define SM_HAIR_WIDTH	640
#define SM_HAIR_HEIGHT	640
#define ALPHA_THRESHOLD 0.99f/255.0f	// minimum alpha value required to render a hair fragment

struct HAIR_PARAMS 
{
    float Ka; // ambient 
    float Kd; // diffuse 
    float Ks1;
    float Ex1;

    float Ks2;
    float Ex2;

    float radius;
    float density; // between [0, 1]

    float alpha;
    float alpha_sm;
	bool  bTransparency;

    float fiber_spacing;
    bool bThinTip;

    D3DXVECTOR3 color;

    int tech_shadow;

    bool bSimulation;
	float targetFrameRateForSimulation; // i.e 1/60

    float wind_magnitude;
    CVector3D wind_direction;
	
	bool bUseCoverage;

	int iStrandCopies;
	int iMaxFragments;
};

class CTressFXRender
{
private:
    CTressFXMesh*				m_pTressFXMesh;
    
    ID3D11Buffer*               m_pScreenQuadVB;

    // shaders

    // VS for rendering the hair
    ID3D11VertexShader*			m_pVSRenderHair;
	ID3D11VertexShader*			m_pVSRenderHairAA;
	
	// VS for rendering the hair with copies of the strands
	ID3D11VertexShader*			m_pVSRenderHairStrandCopies;
	ID3D11VertexShader*			m_pVSRenderHairAAStrandCopies;

    // VS for generating the hair shadow map
    ID3D11VertexShader*			m_pVSGenerateHairSM;

    // PS for rendering the hair to the A buffer
    ID3D11PixelShader*			m_pPSABuffer_Hair;

    // PS for final rendering using a k-buffer for sorting the k nearest fragments
    ID3D11PixelShader*			m_pPSKBuffer_Hair;
	
    ID3D11VertexShader*			m_pVSScreenQuad;

    // constant buffer
	ID3D11Buffer*				m_pcbPerFrame;

    // input layouts for hair
    ID3D11InputLayout*			m_pLayoutHair_0;
    ID3D11InputLayout*			m_pLayoutHair_1;
    ID3D11InputLayout*			m_pLayoutHair_2;
    ID3D11InputLayout*			m_pLayoutHair_3;
    ID3D11InputLayout*			m_pLayoutHair_4;
    ID3D11InputLayout*			m_pLayoutHair_5;


    // Buffers for the head of the per-pixel linked lists (PPLL)
    ID3D11Texture2D*            m_pHeadPPLL_Buffer;
    ID3D11ShaderResourceView*   m_pHeadPPLL_SRV;
	ID3D11UnorderedAccessView*  m_pHeadPPLL_UAV;

    // Buffers for the per-pixel linked list (PPLL) data
    ID3D11Buffer*               m_pPPLL_Buffer;
    ID3D11UnorderedAccessView*  m_pPPLL_UAV;
    ID3D11ShaderResourceView*   m_pPPLL_SRV;

    // textures and views
    ID3D11Texture2D*			m_pNoiseTexture2D;
    ID3D11ShaderResourceView*	m_pNoiseSRV;

    // Hair shadow map depth stencil buffer
    ID3D11Texture2D*			m_pSMHairTx;
    ID3D11DepthStencilView*		m_pSMHairDSV;
    ID3D11ShaderResourceView*	m_pSMHairSRV;

	// Render States
	ID3D11DepthStencilState		*m_pDepthTestEnabledDSS;
	ID3D11DepthStencilState		*m_pDepthTestEnabledNoDepthWritesStencilWriteIncrementDSS;
	ID3D11DepthStencilState		*m_pDepthTestDisabledStencilTestLessDSS;
	ID3D11SamplerState			*m_pSamplerStateLinearWrap;
	ID3D11SamplerState			*m_pSamplerStatePointClamp;
	ID3D11SamplerState			*m_pSamplerStateCmpLess;
	ID3D11BlendState			*m_pBlendStateBlendToBg;
	ID3D11BlendState			*m_pColorWritesOff;


private:
    void CreateShaderAndLayout( ID3D11Device* pd3dDevice, AMD::ShaderCache *pShaderCache);
    void CreateTextureAndViews(ID3D11Device* pd3dDevice);
    void CreateConstantBuffer(ID3D11Device* pd3dDevice);
    void CreateVertexBuffers(ID3D11Device* pd3dDevice);
	void CreateRenderStateObjects( ID3D11Device* pd3dDevice );

    void SetConstantBuffer(ID3D11DeviceContext* pd3dContext, 	
        CModelViewerCamera* pCamera, CModelViewerCamera* pCameraLight, // view camera and light camera
        HAIR_PARAMS& hair_params, 
		D3DXMATRIX *pModelTransformForHead,
		float screenWidth, float screenHeight);

    void SetSamplerStates(ID3D11DeviceContext* pd3dContext);

    void RenderHairGeometry(ID3D11DeviceContext* pd3dContext, 
                        ID3D11VertexShader* pVS, 
                        ID3D11PixelShader* pPS, 
                        float density,
                        bool useLinePrimitives = true,
						int	iStrandCopies = 1);

    void RenderScreenQuad(ID3D11DeviceContext* pd3dContext,
                    ID3D11VertexShader* pVS, ID3D11PixelShader* pPS);
public:
	CTressFXRender(void) {};
	~CTressFXRender(void) {};

	ID3D11ShaderResourceView* GetShadowMapSRV() { return m_pSMHairSRV;};
    void OnCreateDevice(ID3D11Device* pd3dDevice, CTressFXMesh* pTressFXMesh, AMD::ShaderCache *pShaderCache);
	void OnResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* 
							pSwapChain, const DXGI_SURFACE_DESC* 
							pBackBufferSurfaceDesc, void* pUserContext );
    void BeginHairFrame(ID3D11DeviceContext* pd3dContext, CModelViewerCamera* pCamera, 
							CModelViewerCamera* pCameraLight, HAIR_PARAMS& hair_params,
							D3DXMATRIX *pModelTrasnsformForHead,
							float screenWidth, float screenHeight);
    void GenerateShadowMap(ID3D11DeviceContext* pd3dContext, float density, float screenWidth, float screenHeight);
    void RenderHair(ID3D11DeviceContext* pd3dContext, HAIR_PARAMS& hair_params);
    void EndHairFrame(ID3D11DeviceContext* pd3dContext, HAIR_PARAMS& hair_params);
    void OnDestroy();
};

