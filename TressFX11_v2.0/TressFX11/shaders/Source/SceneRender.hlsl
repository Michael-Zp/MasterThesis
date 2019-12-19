//--------------------------------------------------------------------------------------
// File: RenderScene.hlsl
//
// Scene rendering (non-hair) shaders
// 
// Copyright © AMD Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "TressFXInclude.hlsl"

cbuffer cbPerMaterial : register( b2 )
{
    float4		g_MatAmbient	: packoffset( c0 );
    float4		g_MatDiffuse	: packoffset( c1 ); 
    float4		g_MatSpecular	: packoffset( c2 );
    float4		g_ScalpColor	: packoffset( c3 );
};

struct VS_INPUT_SCENE
{
    float3 Position     : POSITION;		// vertex position 
    float3 Normal       : NORMAL;		// this normal comes in per-vertex
    float2 Texcoord	    : TEXCOORD0;	// vertex texture coords 
};

struct VS_OUTPUT_SCENE
{
    float4 Position     : SV_POSITION; // vertex position
    float2 Texcoord	    : TEXCOORD0;   // vertex texture coords 
    float3 Normal		: TEXCOORD1;
    float3 WorldPos	    : TEXCOORD2;
};

struct PS_SHADOW_OUTPUT
{
	float color : SV_TARGET;
	float depth	: SV_DEPTH;
};

//--------------------------------------------------------------------------------------
// VS_GenerateSceneSM
//
// Renders the scene geometry into a shadow map
//--------------------------------------------------------------------------------------
VS_OUTPUT_SM VS_GenerateSceneSM( VS_INPUT_SCENE input )
{
    VS_OUTPUT_SM Output;
    
    // Transform the position from object space to homogeneous projection space
    float3 pos = mul(float4(input.Position, 1), g_mWorld).xyz;
    Output.Position = mul( float4(pos, 1), g_mViewProjLight );
    
    // output position
    Output.WorldPos = input.Position;
    
    return Output;    
}

//--------------------------------------------------------------------------------------
// VS_GenerateSceneSM
//
// Scene rendering vertex shader
//--------------------------------------------------------------------------------------
VS_OUTPUT_SCENE VS_RenderScene( VS_INPUT_SCENE input )
{
    VS_OUTPUT_SCENE Output;
    
    // Transform the position from object space to homogeneous projection space
        
    float3 pos = mul(float4(input.Position, 1), g_mWorld).xyz;
    Output.Position = mul( float4(pos, 1), g_mViewProj );

    // output tangent and texture coordinate value
	matrix worldRotation = g_mWorld;

	// zero the translation components of the matrix to just change the orientation of the normal
	worldRotation[3].x = 0;
	worldRotation[3].y = 0;
	worldRotation[3].z = 0;
    Output.Normal = mul(float4(input.Normal, 1), worldRotation).xyz;

    Output.Texcoord = input.Texcoord;

    // output position
    Output.WorldPos = pos;
    
    return Output;    
}

//--------------------------------------------------------------------------------------
// ComputeSceneShading
//
// Computes Phong Illumination
//--------------------------------------------------------------------------------------
float4 ComputeSceneShading(float3 iPos, float3 iNormal, float2 tex, float amountLight)
{
    // define ambient, diffuse and specular term for material
    float4 matAmbient = g_MatAmbient;
    float4 matDiffuse = float4(g_MatDiffuse.xyz, 1);

    float4 matSpecular = float4(g_MatSpecular.xyz, 1);
    float exponent = g_MatSpecular.w;

    float3 lightPos = g_PointLightPos.xyz;
    float3 vLightDir = normalize(lightPos - iPos);
    float3 normal = normalize(iNormal);
    
    float3 vEyeDir = normalize(g_vEye.xyz - iPos.xyz);
    float3 vHalf = normalize(vEyeDir + vLightDir);
    float HDotN = dot(vHalf, normal);
    float vSpecular = max(0, HDotN);

    float LDotN = dot(normal, vLightDir);
    float vDiffuse = max(0, LDotN);

	if(LDotN <= 0)
		vSpecular = 0;

	// Check for scalp geometry, and use scalp color when rendering
    if(g_MatDiffuse.w > 0)
    {
        float4 texColor = g_txObj.Sample( g_samLinearWrap, tex );
        
        if ( texColor.x == 0 && texColor.y == 0 && texColor.z == 0 )
        {
            matDiffuse = 0.7 * g_ScalpColor;
            vSpecular = 0;
        }
    }

    float4 vColor = matAmbient * g_AmbientLightColor + amountLight * 
        g_PointLightColor *	( matDiffuse * vDiffuse ) + (matSpecular * pow(vSpecular, exponent));

    return vColor;
}


//--------------------------------------------------------------------------------------
// PS_RenderSceneShadow
//
// Renders the hair shadowmap into a color buffer to create a scene shadow (hair casting 
// shadows on the scene). Since the shadow map is rendered as a textured full screen quad
// it needs to output depth so that depth compares can be made with the current depth
// buffer. In this case, the current depth buffer is the scene model. Depth compares are
// necessary because the hair wraps around the head model, and you don't want shadows cast
// from hair behind the head.
//--------------------------------------------------------------------------------------
PS_SHADOW_OUTPUT PS_RenderSceneShadow(VS_OUTPUT_SCREENQUAD input)
{
	PS_SHADOW_OUTPUT Output;

	float shadowMapVal = g_txShadow.Sample(g_samPointClamp,input.vTex).x;
	// Areas in shadow will have a value < 1. Output 0.5 (the darkness of the shadow) if in shadow, otherwise output 1
    Output.color = saturate(step(1, shadowMapVal) + 0.5);
	Output.depth = shadowMapVal;
	return Output;
}


//--------------------------------------------------------------------------------------
// PS_RenderScene
//
// Renders the scene geometry (everything except the hair). Use the shadow texture created
// by PS_RenderSceneShadow to modulate the light intensity on the model.
//--------------------------------------------------------------------------------------
[earlydepthstencil]
float4 PS_RenderScene(VS_OUTPUT_SCENE input):SV_Target
{
	// Use light projected texture coordinates (same as shadowmap) since the shadow buffer was
	// generated from the hair shadow map.
	float4 projPosLight = mul(float4(input.WorldPos,1), g_mViewProjLight);
    float2 texSM = float2(projPosLight.x/projPosLight.w+1, -projPosLight.y/projPosLight.w+1)*0.5;
    float shadow = g_txShadow.SampleLevel(g_samLinearWrap, texSM, 0).x;
    return ComputeSceneShading(input.WorldPos, input.Normal, input.Texcoord, shadow);
}

