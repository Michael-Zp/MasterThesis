//--------------------------------------------------------------------------------------
// File: TressFXInclude.hlsl
//
// Include file for TressFX shaders. 
// 
// Copyright © AMD Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------

cbuffer cbPerFrame : register( b1 )
{
    matrix		g_mWorld			: packoffset( c0  );
    matrix		g_mViewProj			: packoffset( c4  );
	matrix		g_mInvViewProj		: packoffset( c8  );
    matrix		g_mViewProjLight	: packoffset( c12 );

    float3		g_vEye				: packoffset( c16 );
	float		g_fvFov				: packoffset( c16.w );

    float4      g_AmbientLightColor : packoffset( c17 );
    float4		g_PointLightColor	: packoffset( c18 );
    float4		g_PointLightPos		: packoffset( c19 );
    float4		g_MatBaseColor		: packoffset( c20 );
    float4		g_MatKValue			: packoffset( c21 ); // Ka, Kd, Ks, Ex
    
    float		g_FiberAlpha		: packoffset( c22.x );
    float		g_HairShadowAlpha	: packoffset( c22.y );
    float		g_bExpandPixels		: packoffset( c22.z );
    float		g_FiberRadius		: packoffset( c22.w );

    float4		g_WinSize			: packoffset( c23 ); // screen size
    
    float		g_FiberSpacing		: packoffset( c24.x ); // average spacing between fibers
    float		g_bThinTip			: packoffset( c24.y );
    float		g_fNearLight		: packoffset( c24.z );
    float		g_fFarLight			: packoffset( c24.w );
    
    int			g_iTechSM			: packoffset( c25.x );
    int			g_bUseCoverage		: packoffset( c25.y );
	int			g_iStrandCopies		: packoffset( c25.z ); // strand copies that the transparency shader will produce
	int			g_iMaxFragments		: packoffset( c25.w );

	float		g_alphaThreshold	: packoffset( c26.x ); 
    float		g_fHairKs2			: packoffset( c26.y ); // for second highlight
    float		g_fHairEx2			: packoffset( c26.z ); // for second highlight

    matrix      g_mInvViewProjViewport: packoffset( c27 );
};

//--------------------------------------------------------------------------------------
// Per-Pixel Linked List (PPLL) structure
//--------------------------------------------------------------------------------------
struct PPLL_STRUCT
{
    uint	TangentAndCoverage;	
    uint	depth;
    uint    uNext;
};

//--------------------------------------------------------------------------------------
// SRV buffers
//--------------------------------------------------------------------------------------
StructuredBuffer<PPLL_STRUCT>			LinkedListSRV				: register(t0);
Texture2D<uint>							LinkedListHeadSRV			: register(t1);

Texture2D								g_txSMScene					: register( t2 );
Texture2D								g_txSMHair					: register( t3 );
Texture2D								g_txObj						: register( t4 );
Texture2D								g_txNoise					: register( t5 );

Buffer<float>							g_HairThicknessCoeffs       : register( t6 );

StructuredBuffer<float4>				g_HairVertexPositions	: register( t7 );
StructuredBuffer<float4>				g_HairVertexTangents	: register( t8 );

Texture2D								g_txShadow					: register( t9 );

//--------------------------------------------------------------------------------------
// Unordered Access
//--------------------------------------------------------------------------------------
RWTexture2D<uint>						LinkedListHeadUAV			: register(u1);
RWStructuredBuffer<PPLL_STRUCT>			LinkedListUAV				: register(u2); // store fragment linked list

//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------
SamplerState							g_samLinearWrap				: register( s0 );
SamplerState							g_samPointClamp				: register( s1 );
SamplerComparisonState					g_samShadow					: register( s2 );


//--------------------------------------------------------------------------------------
// common shader input/output structure
//--------------------------------------------------------------------------------------

struct VS_OUTPUT_SM
{
    float4 Position     : SV_POSITION; // vertex position
    float3 WorldPos		: TEXCOORD0;
};


//--------------------------------------------------------------------------------------
// Screen quad
//--------------------------------------------------------------------------------------
struct VS_INPUT_SCREENQUAD
{
    float3 Position     : POSITION;		// vertex position 
    float3 Normal       : NORMAL;		// this normal comes in per-vertex
    float2 Texcoord	    : TEXCOORD0;	// vertex texture coords 
};

struct VS_OUTPUT_SCREENQUAD
{
    float4 vPosition : SV_POSITION;
    float2 vTex      : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Simple vertex shader for rendering a full screen quad
//--------------------------------------------------------------------------------------
VS_OUTPUT_SCREENQUAD VS_ScreenQuad(VS_INPUT_SCREENQUAD input)
{
    VS_OUTPUT_SCREENQUAD output;

    output.vPosition = float4(input.Position.xyz, 1.0);
    output.vTex = input.Texcoord.xy;

    return output;
}


