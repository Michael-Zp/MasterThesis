//--------------------------------------------------------------------------------------
// File: TressFXSimulation.h
//
// Hair physics simulation header
// 
// Copyright © AMD Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include "..\\DXUT\\Core\\DXUT.h"
#include <d3dx11.h>
#include <string>
#include "..\\AMD_SDK\\AMD_SDK.h"
#include "TressFXMesh.h"
#include "Util.h"
#include <map>


typedef D3DXVECTOR4 float4;
typedef D3DXVECTOR3 float3;

// Dampening, and Local and global stiffness for the parts of the hair
typedef struct
{
	float damping;
	float stiffnessForLocalShapeMatching;
	float stiffnessForGlobalShapeMatching;
	float globalShapeMatchingEffectiveRange;	
} ShapeParameters;

struct ConstBufferCS_Collision_Capsule
{
	float3	cc0_center;
	float	cc0_radius;
	float3	cc1_center;
	float	cc1_radius;
	float3	cc2_center;
	float	cc2_radius;
	float	cc0_radiusSquared;
	float	cc1_radiusSquared;
	float	cc2_radiusSquared;
	float	pad;
};


class CTressFXSimulation
{
public:
    CTressFXSimulation(void);
    ~CTressFXSimulation(void);

    HRESULT OnCreateDevice(ID3D11Device* pd3dDevice, CTressFXMesh* pTressFXMesh, AMD::ShaderCache *pShaderCache, 
		ConstBufferCS_Collision_Capsule *pCollision);
    HRESULT Simulate(ID3D11DeviceContext* pd3dContext, float fElapsedTime, 
						const CVector3D &windDir, float windMag, D3DXMATRIX *pModelTransformForHead, float targetFrameRate = 1.0f/60.0f);
    void OnDestroy();
    CTressFXMesh*			m_pTressFXMesh;
	float m_elapsedTimeSinceLastSim;

    // Compute Shader
    //data for compute shaders
	ID3D11ComputeShader*    m_CSSkipSimulateHair;
	ID3D11ComputeShader*    m_CSIntegrationAndGlobalShapeConstraints;
	ID3D11ComputeShader*    m_CSLocalShapeConstraints;
	ID3D11ComputeShader*    m_CSLengthConstriantsAndWind;
	ID3D11ComputeShader*    m_CSCollisionAndTangents;

    // const buffers for CS
    ID3D11Buffer* m_pCBCSPerFrame;
    ID3D11Buffer* m_pCBCSCollisionCapsule;
    
    HRESULT CreateComputeShaderConstantBuffers(ID3D11Device* pd3dDevice, ConstBufferCS_Collision_Capsule *pCollision);	
	
	D3DXMATRIX m_modelPrevInvTransformForHead;

	// hair simulation parameters
	int				m_numLengthConstraintIterations;
	int				m_numLocalShapeMatchingIterations;
	int				m_localShapeConstraintMethod; 
	float			m_gravityMagnitude;
	int				m_hairSimulationFixedFrameRate;
	bool			m_bCollision;
	ShapeParameters	m_shapeParams[HAIR_MAX_SECTIONS];
};
