//--------------------------------------------------------------------------------------
// File: TressFXSimulation.cpp
//
// Hair physics simulation using DirectCompute
// 
// Copyright © AMD Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include <algorithm>
#include "TressFXSimulation.h"
#include "..\\DXUT\\Core\\dxut.h"
#include "util.h"
#include "..\\DXUT\\Optional\\SDKmisc.h"
#include "..\\DXUT\\Optional\\DXUTcamera.h"
#include "TressFXMesh.h"

struct ConstBufferCS_Per_Frame
{
    D3DXMATRIX ModelTransformForHead;
	D3DXMATRIX ModelPrevInvTransformForHead; // at previous frame

    float4 ModelRotateForHead;

	float4 Wind;
	float4 Wind1;
	float4 Wind2;
	float4 Wind3;

    int NumLengthConstraintIterations;
    int bCollision;

    float GravityMagnitude;
    float timeStep;
    
    float Damping0;
    float StiffnessForLocalShapeMatching0;
    float StiffnessForGlobalShapeMatching0;
    float GlobalShapeMatchingEffectiveRange0;
    
    float Damping1;
    float StiffnessForLocalShapeMatching1;
    float StiffnessForGlobalShapeMatching1;
    float GlobalShapeMatchingEffectiveRange1;

    float Damping2;
    float StiffnessForLocalShapeMatching2;
    float StiffnessForGlobalShapeMatching2;
    float GlobalShapeMatchingEffectiveRange2;
    
    float Damping3;
    float StiffnessForLocalShapeMatching3;
    float StiffnessForGlobalShapeMatching3;
    float GlobalShapeMatchingEffectiveRange3;
};


//--------------------------------------------------------------------------------------
//
// Constructor
//
//--------------------------------------------------------------------------------------
CTressFXSimulation::CTressFXSimulation(void)
{
    m_CSSkipSimulateHair = NULL;
	m_CSIntegrationAndGlobalShapeConstraints = NULL;
	m_CSLocalShapeConstraints = NULL;
	m_CSLengthConstriantsAndWind = NULL;
	m_CSCollisionAndTangents = NULL;
    m_pCBCSPerFrame = NULL;
	m_pCBCSCollisionCapsule = NULL;


	D3DXMatrixIdentity(&m_modelPrevInvTransformForHead);

	m_numLengthConstraintIterations = 2;
	m_numLocalShapeMatchingIterations = 1;
	m_localShapeConstraintMethod = 4; // must be 1, 2, 3 or 4
	m_gravityMagnitude = 9.82f;

	m_bCollision = true;
	m_hairSimulationFixedFrameRate = 60;

	m_shapeParams[0].damping = 0.25f;
	m_shapeParams[0].stiffnessForLocalShapeMatching = 1.0f;
	m_shapeParams[0].stiffnessForGlobalShapeMatching = 0.2f;
	m_shapeParams[0].globalShapeMatchingEffectiveRange = 0.3f;

	m_shapeParams[2].damping = 0.02f;
	m_shapeParams[2].stiffnessForLocalShapeMatching = 0.7f;
	m_shapeParams[2].stiffnessForGlobalShapeMatching = 0.0f;
	m_shapeParams[2].globalShapeMatchingEffectiveRange = 0.0f;

	m_shapeParams[3].damping = 0.1f;
	m_shapeParams[3].stiffnessForLocalShapeMatching = 1.0f;
	m_shapeParams[3].stiffnessForGlobalShapeMatching = 0.2f;
	m_shapeParams[3].globalShapeMatchingEffectiveRange = 0.3f;

	m_elapsedTimeSinceLastSim = 0;
}

//--------------------------------------------------------------------------------------
//
// Destructor
//
//--------------------------------------------------------------------------------------
CTressFXSimulation::~CTressFXSimulation(void)
{
}

//--------------------------------------------------------------------------------------
//
// OnCreateDevice
//
// Called when the D3D device is being created. Creates the compute shaders and the D3D
// resources used for the hair simulation.
//
//--------------------------------------------------------------------------------------
HRESULT CTressFXSimulation::OnCreateDevice(ID3D11Device* pd3dDevice, CTressFXMesh *pTressFXMesh,
							AMD::ShaderCache *pShaderCache, ConstBufferCS_Collision_Capsule *pCollision)
{
    HRESULT hr;

	m_pTressFXMesh = pTressFXMesh;

    //--------------
    // Add compute shaders to cache (do this only once)
    //--------------
	static bool bFirstPass = true;
    if( bFirstPass )
    {
		pShaderCache->AddShader( (ID3D11DeviceChild**)&m_CSSkipSimulateHair, AMD::ShaderCache::SHADER_TYPE_COMPUTE, 
			L"cs_5_0", L"SkipSimulateHair",
			L"shaders\\Source\\TressFXSimulation.hlsl", 0, NULL, NULL, NULL, 0 );
		pShaderCache->AddShader( (ID3D11DeviceChild**)&m_CSIntegrationAndGlobalShapeConstraints, AMD::ShaderCache::SHADER_TYPE_COMPUTE, 
			L"cs_5_0", L"IntegrationAndGlobalShapeConstraints",
			L"shaders\\Source\\TressFXSimulation.hlsl", 0, NULL, NULL, NULL, 0 );
		pShaderCache->AddShader( (ID3D11DeviceChild**)&m_CSLocalShapeConstraints, AMD::ShaderCache::SHADER_TYPE_COMPUTE, 
			L"cs_5_0", L"LocalShapeConstraints",
			L"shaders\\Source\\TressFXSimulation.hlsl", 0, NULL, NULL, NULL, 0 );
		pShaderCache->AddShader( (ID3D11DeviceChild**)&m_CSLengthConstriantsAndWind, AMD::ShaderCache::SHADER_TYPE_COMPUTE, 
			L"cs_5_0", L"LengthConstriantsAndWind",
			L"shaders\\Source\\TressFXSimulation.hlsl", 0, NULL, NULL, NULL, 0 );
		pShaderCache->AddShader( (ID3D11DeviceChild**)&m_CSCollisionAndTangents, AMD::ShaderCache::SHADER_TYPE_COMPUTE, 
			L"cs_5_0", L"CollisionAndTangents",
			L"shaders\\Source\\TressFXSimulation.hlsl", 0, NULL, NULL, NULL, 0 );

		bFirstPass = false;
	}

    //-------------------------
    // Create constant buffers
    //-------------------------    
    hr = CreateComputeShaderConstantBuffers(pd3dDevice, pCollision);
    
    if (FAILED(hr))
    {
        ::MessageBoxA(0, "Fail to create compute shader constant buffers", "d3d error", 0);
        return hr;
    }

	// reset m_elapsedTimeSinceLastSim to zero
	m_elapsedTimeSinceLastSim = 0;

    return S_OK;
}


const float MATH_PI2 = 3.14159265359f;
#define DEG_TO_RAD2(d) (d * MATH_PI2 / 180)

//--------------------------------------------------------------------------------------
//
// Simulate
//
// Runs the hair simulation wich will animate the hair vertices. This function calls
// Dispatch to execute compute shaders that implement the simulation.
//
//--------------------------------------------------------------------------------------
HRESULT CTressFXSimulation::Simulate(ID3D11DeviceContext* pd3dContext, float fElapsedTime, 
	const CVector3D & windDir, float windMag, D3DXMATRIX *pModelTransformForHead, float targetFrameRate/* = 1.0f/60.0f*/)
{
	m_elapsedTimeSinceLastSim += fElapsedTime;
	bool bSkipSimulation = false;

	// Simulation is sensitive to frame rate. So, we set the target frame rate (defaulted to 1/60) and skip the simulation if the current elapsed time 
	// since the last simulation is too early. 
	if ( m_elapsedTimeSinceLastSim < targetFrameRate )
		bSkipSimulation = true;
	else
	{
		bSkipSimulation = false;
		m_elapsedTimeSinceLastSim = 0;
		TIMER_Begin( 0, L"Simulation" )
	}

    HRESULT hr = S_OK;
    UINT initCounts = 0;    

    //--------------------
    // CB_CS_TRANSFORMS
    //--------------------
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    
    //--------------------
    // ConstBufferCS_Per_Frame
    //--------------------
    V( pd3dContext->Map(m_pCBCSPerFrame, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
        ConstBufferCS_Per_Frame* pCSPerFrame = ( ConstBufferCS_Per_Frame* )MappedResource.pData;
        pCSPerFrame->ModelTransformForHead = *pModelTransformForHead;

        D3DXMATRIX ModelInvTransformForHead;
        D3DXMatrixInverse(&ModelInvTransformForHead, NULL, pModelTransformForHead);

		pCSPerFrame->ModelPrevInvTransformForHead = m_modelPrevInvTransformForHead;
		m_modelPrevInvTransformForHead = ModelInvTransformForHead;

        D3DXQUATERNION quat;
        D3DXQuaternionRotationMatrix(&quat, pModelTransformForHead);
        pCSPerFrame->ModelRotateForHead = float4(quat.x, quat.y, quat.z, quat.w);

        D3DXQuaternionRotationMatrix(&quat, &ModelInvTransformForHead);
       
        pCSPerFrame->NumLengthConstraintIterations = m_numLengthConstraintIterations;
	    pCSPerFrame->bCollision = (m_bCollision == true) ? 1 : 0;

        pCSPerFrame->GravityMagnitude = m_gravityMagnitude;
        
		pCSPerFrame->timeStep = targetFrameRate;

        static int frame = 0;

		float wM = windMag * (pow( sin(frame*0.05f), 2.0f ) + 0.5f);

		CVector3D windDirN(windDir);
		windDirN.Normalize();

		CVector3D XAxis(1.0f, 0, 0);
		CVector3D xCrossW = XAxis.Cross(windDirN);

		CQuaternion rotFromXAxisToWindDir;
		rotFromXAxisToWindDir.SetIdentity();

		float angle = asin(xCrossW.Length());

		if ( angle > 0.001 )
		{
			rotFromXAxisToWindDir.SetRotation(xCrossW.Normalize(), angle);
		}

		float angleToWideWindCone = DEG_TO_RAD2(40.f);

		{
			CVector3D rotAxis(0, 1.0f, 0);
			
			CQuaternion rot(rotAxis, angleToWideWindCone);
			CVector3D newWindDir = rotFromXAxisToWindDir * rot * XAxis; 
			pCSPerFrame->Wind = float4(newWindDir.m_X * wM, newWindDir.m_Y * wM, newWindDir.m_Z * wM, (float)frame);
		}

		{
			CVector3D rotAxis(0, -1.0f, 0);
			CQuaternion rot(rotAxis, angleToWideWindCone);
			CVector3D newWindDir = rotFromXAxisToWindDir * rot * XAxis;
			pCSPerFrame->Wind1 = float4(newWindDir.m_X * wM, newWindDir.m_Y * wM, newWindDir.m_Z * wM, (float)frame);
		}

		{
			CVector3D rotAxis(0, 0, 1.0f);
			CQuaternion rot(rotAxis, angleToWideWindCone);
			CVector3D newWindDir = rotFromXAxisToWindDir * rot * XAxis;
			pCSPerFrame->Wind2 = float4(newWindDir.m_X * wM, newWindDir.m_Y * wM, newWindDir.m_Z * wM, (float)frame);
		}

		{
			CVector3D rotAxis(0, 0, -1.0f);
			CQuaternion rot(rotAxis, angleToWideWindCone);
			CVector3D newWindDir = rotFromXAxisToWindDir * rot * XAxis;
			pCSPerFrame->Wind3 = float4(newWindDir.m_X * wM, newWindDir.m_Y * wM, newWindDir.m_Z * wM, (float)frame);
		}

        frame++;

        // hair section 0
        pCSPerFrame->Damping0 = m_shapeParams[0].damping;
        pCSPerFrame->StiffnessForLocalShapeMatching0 = m_shapeParams[0].stiffnessForLocalShapeMatching;
        pCSPerFrame->StiffnessForGlobalShapeMatching0 = m_shapeParams[0].stiffnessForGlobalShapeMatching;
        pCSPerFrame->GlobalShapeMatchingEffectiveRange0 = m_shapeParams[0].globalShapeMatchingEffectiveRange;

        // hair section 1
        pCSPerFrame->Damping1 = m_shapeParams[0].damping;
        pCSPerFrame->StiffnessForLocalShapeMatching1 = m_shapeParams[0].stiffnessForLocalShapeMatching;
        pCSPerFrame->StiffnessForGlobalShapeMatching1 = m_shapeParams[0].stiffnessForGlobalShapeMatching;
        pCSPerFrame->GlobalShapeMatchingEffectiveRange1 = m_shapeParams[0].globalShapeMatchingEffectiveRange;

        // hair section 2
        pCSPerFrame->Damping2 = m_shapeParams[2].damping;
        pCSPerFrame->StiffnessForLocalShapeMatching2 = m_shapeParams[2].stiffnessForLocalShapeMatching;
        pCSPerFrame->StiffnessForGlobalShapeMatching2 = m_shapeParams[2].stiffnessForGlobalShapeMatching;
        pCSPerFrame->GlobalShapeMatchingEffectiveRange2 = m_shapeParams[2].globalShapeMatchingEffectiveRange;
    
        // hair section 3
        pCSPerFrame->Damping3 = m_shapeParams[3].damping;
        pCSPerFrame->StiffnessForLocalShapeMatching3 = m_shapeParams[3].stiffnessForLocalShapeMatching;
        pCSPerFrame->StiffnessForGlobalShapeMatching3 = m_shapeParams[3].stiffnessForGlobalShapeMatching;
        pCSPerFrame->GlobalShapeMatchingEffectiveRange3 = m_shapeParams[3].globalShapeMatchingEffectiveRange;
    pd3dContext->Unmap(m_pCBCSPerFrame, 0);

    pd3dContext->CSSetConstantBuffers(0, 1, &m_pCBCSPerFrame);
    pd3dContext->CSSetConstantBuffers(3, 1, &m_pCBCSCollisionCapsule);

    //------------------------
    //Set the shader resources
    //------------------------
    ID3D11ShaderResourceView* ppSRV[4] = { m_pTressFXMesh->m_HairVerticesOffsetsSRV, 
                                           m_pTressFXMesh->m_HairRestLengthSRV,
                                           m_pTressFXMesh->m_HairStrandTypeSRV,
                                           m_pTressFXMesh->m_HairRefVecsInLocalFrameSRV
                                           };

    pd3dContext->CSSetShaderResources( 0, 4, ppSRV);

    //---------------------------
    //Bind unordered access views
    //---------------------------
    ID3D11UnorderedAccessView* ppUAV[8] = { m_pTressFXMesh->m_HairVertexPositionsUAV,
                                            m_pTressFXMesh->m_HairVertexPositionsPrevUAV,
                                            m_pTressFXMesh->m_HairVertexTangentsUAV,
                                            m_pTressFXMesh->m_InitialHairPositionsUAV, 
                                            m_pTressFXMesh->m_GlobalRotationsUAV,
                                            m_pTressFXMesh->m_LocalRotationsUAV };

    pd3dContext->CSSetUnorderedAccessViews( 0, 8, ppUAV, &initCounts );

    //-----------------------
    // Run the compute shader
    //-----------------------
	if ( bSkipSimulation )
	{
		// In this case, we don't simulate hair. Instead, we use the previous vertex positions and 
		// transform them using the head transform as a rigid motion.
		pd3dContext->CSSetShader( m_CSSkipSimulateHair, NULL, 0 );
	}
	else
	{
		int numOfGroupsForCS = 0;

		int numOfStrandsPerThreadGroup = THREAD_GROUP_SIZE/m_pTressFXMesh->m_MaxNumOfVerticesInStrand;

		// Integrate and global shape constraints
		// One thread computes one vertex
		numOfGroupsForCS = m_pTressFXMesh->m_NumHairStrands / numOfStrandsPerThreadGroup;
		pd3dContext->CSSetShader( m_CSIntegrationAndGlobalShapeConstraints, NULL, 0 );
		pd3dContext->Dispatch(numOfGroupsForCS, 1, 1);

		// Local shape constraints. If the hair is very curly, increase the iteration so that
		// hair style can be preserved well. 
		// One thread computes one strand
		for ( int iteration = 0; iteration < m_numLocalShapeMatchingIterations; iteration++)
		{
			int numOfGroupsForCS_B = (int)((float)m_pTressFXMesh->m_NumHairStrands/(float)THREAD_GROUP_SIZE);
			pd3dContext->CSSetShader( m_CSLocalShapeConstraints, NULL, 0 );
			pd3dContext->Dispatch(numOfGroupsForCS_B, 1, 1);
		}

		// Wind and edge length constraints
		// One thread computes one vertex
		pd3dContext->CSSetShader( m_CSLengthConstriantsAndWind, NULL, 0 );
		pd3dContext->Dispatch(numOfGroupsForCS, 1, 1);

		// Collision and tangents
		// One thread computes one vertex
		pd3dContext->CSSetShader( m_CSCollisionAndTangents, NULL, 0 );
		pd3dContext->Dispatch(numOfGroupsForCS, 1, 1);
	}

    //--------------------------
    // Unbind resources for CS
    //--------------------------
    ID3D11UnorderedAccessView* ppUAViewNULL[8] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    pd3dContext->CSSetUnorderedAccessViews( 0, 8, ppUAViewNULL, &initCounts );
    
    ID3D11ShaderResourceView* ppSRVNULL[4] = { NULL, NULL, NULL, NULL};
    pd3dContext->CSSetShaderResources( 0, 4, ppSRVNULL );

	if (!bSkipSimulation)
		TIMER_End(); // Simulation

    return hr;
}

//--------------------------------------------------------------------------------------
//
// CreateComputeShaderConstantBuffers
//
// Called by OnCreateDevice to allocate a constant buffer used by the compute shader code
//
//--------------------------------------------------------------------------------------
HRESULT CTressFXSimulation::CreateComputeShaderConstantBuffers(ID3D11Device* pd3dDevice, ConstBufferCS_Collision_Capsule *pCollision)
{
    HRESULT hr = S_OK;

    D3D11_BUFFER_DESC Desc;
    Desc.Usage = D3D11_USAGE_DYNAMIC;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    Desc.MiscFlags = 0;
    Desc.ByteWidth = sizeof( ConstBufferCS_Per_Frame );
    SAFE_RELEASE(m_pCBCSPerFrame);
    V_RETURN( pd3dDevice->CreateBuffer(&Desc, NULL, &m_pCBCSPerFrame) );

	Desc.Usage = D3D11_USAGE_IMMUTABLE;
	Desc.CPUAccessFlags = 0;
	Desc.ByteWidth = sizeof( ConstBufferCS_Collision_Capsule );
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = (void *)pCollision;
    SAFE_RELEASE(m_pCBCSCollisionCapsule);
    V_RETURN( pd3dDevice->CreateBuffer(&Desc, &data, &m_pCBCSCollisionCapsule) );

    return S_OK;
}



//--------------------------------------------------------------------------------------
//
// OnDestroy
//
// Called when the D3D device is being destroyed
//
//--------------------------------------------------------------------------------------
void CTressFXSimulation::OnDestroy()
{
    SAFE_RELEASE(m_CSSkipSimulateHair);
    SAFE_RELEASE(m_CSIntegrationAndGlobalShapeConstraints);
    SAFE_RELEASE(m_CSLocalShapeConstraints);
    SAFE_RELEASE(m_CSLengthConstriantsAndWind);
	SAFE_RELEASE(m_CSCollisionAndTangents);
    SAFE_RELEASE(m_pCBCSPerFrame);
    SAFE_RELEASE(m_pCBCSCollisionCapsule);
}
