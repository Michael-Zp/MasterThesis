//--------------------------------------------------------------------------------------
// File: TressFXMesh.cpp
//
// Hair mesh code
// 
// Copyright © AMD Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "..\\DXUT\\Core\\dxut.h"
#include "TressFXMesh.h"

#include <fstream>
using namespace std;


//--------------------------------------------------------------------------------------
//
// Constructor
//
// Used for initializing member variables to default values.
//--------------------------------------------------------------------------------------
CTressFXMesh::CTressFXMesh(void) :
   m_pVertexBuffer(NULL)
  , m_pIndexBuffer(NULL)
  , m_pTriangleIndexBuffer(NULL)
  , m_pThicknessCoeffsBuffer(NULL)
  , m_pTriangleIndexBufferSRV(NULL)
  , m_pThicknessCoeffsSRV(NULL)
{
    m_HairVertexPositionsUAB = NULL;
    m_HairVertexPositionsPrevUAB = NULL;
    
    m_HairVertexPositionsSRV = NULL;	
    
    m_HairVerticesOffsetsBuffer = NULL;
    m_HairVerticesOffsetsSRV = NULL;

    m_HairStrandTypeBuffer = NULL;
    m_HairStrandTypeSRV = NULL;

    m_HairVertexPositionsUAV = NULL;
    m_HairVertexPositionsPrevUAV = NULL;

    m_HairLengthBuffer = NULL;
    m_HairRestLengthSRV = NULL;
	
    m_InitialHairPositionsBuffer = NULL;
	m_InitialHairPositionsSRV = NULL;

    m_HairVertexTangentsUAB = NULL;
    m_HairVertexTangentsSRV = NULL;
    m_HairVertexTangentsUAV = NULL;

    m_HairRefVectorsBuffer = NULL;
    m_HairRefVecsInLocalFrameSRV = NULL;
    
    m_GlobalRotationsUAB = NULL;
    m_GlobalRotationsUAV = NULL;
	
    m_LocalRotationsUAB = NULL;
    m_LocalRotationsUAV = NULL;
	    
	m_MaxNumOfVerticesInStrand = 32;
}

//--------------------------------------------------------------------------------------
//
// Destructor
//
//--------------------------------------------------------------------------------------
CTressFXMesh::~CTressFXMesh(void)
{
}


//--------------------------------------------------------------------------------------
//
// OnCreateDevice
//
//	Called during D3D device createion. This function calls the HairAssetLoader class
//	to load hair vertices from the file. This function also creates the D3D resources
//  and stores the hair data into these resources.
//
//--------------------------------------------------------------------------------------
HRESULT CTressFXMesh::OnCreateDevice(ID3D11Device* pd3dDevice, char** filenameArray, bool* bMakeBothEndsImmovable, int numFiles,
								float fScale, D3DXVECTOR3 vRotation, D3DXVECTOR3 vTranslation)
{
	HRESULT hr;

	static bool bHairLoaded = false;

	if (numFiles > HAIR_MAX_SECTIONS)
		numFiles = HAIR_MAX_SECTIONS;
  
	if (!bHairLoaded)
	{
		for (int i = 0; i < numFiles; i++)
		{
			if ( !m_HairAssetLoader.LoadAppend(filenameArray[i], i, bMakeBothEndsImmovable[i], m_MaxNumOfVerticesInStrand, THREAD_GROUP_SIZE))
			{
				char sMsg[200];
				sprintf(sMsg, "Failed to load hair file");
        
				::MessageBoxA(0, sMsg, "file loading error", 0);
				return S_FALSE;
			}
		}
		m_HairAssetLoader.ProcessVertices(fScale, vRotation, vTranslation);
		m_bSphere = m_HairAssetLoader.m_bSphere;
		m_NumHairStrands = m_HairAssetLoader.m_NumHairStrands;

		bHairLoaded = true;
	}


	 //---------------
    // Vertex buffer
    //---------------

    D3D11_BUFFER_DESC bd;
    bd.Usage               = D3D11_USAGE_DEFAULT;
    bd.ByteWidth           = sizeof( StrandVertex ) * m_HairAssetLoader.m_NumHairVertices;
    bd.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags      = 0;
    bd.MiscFlags           = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = m_HairAssetLoader.m_pTriangleVertices;
    V_RETURN( pd3dDevice->CreateBuffer( &bd, &InitData, &m_pVertexBuffer ) );
	
    //thickness coeff buffer
    {
        D3D11_BUFFER_DESC bd;
        ZeroMemory(&bd, sizeof(bd));

        bd.Usage     = D3D11_USAGE_IMMUTABLE;
        bd.ByteWidth = sizeof(float) * m_HairAssetLoader.m_NumHairVertices;
        bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        bd.StructureByteStride = sizeof(float);

        D3D11_SUBRESOURCE_DATA srData;
        ZeroMemory(&srData, sizeof(srData));
        srData.pSysMem = m_HairAssetLoader.m_pThicknessCoeffs;

        V_RETURN( pd3dDevice->CreateBuffer( &bd, &srData, &m_pThicknessCoeffsBuffer) );
    }

    //thickness coeff buffer srv
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
        ZeroMemory(&SRVDesc, sizeof(SRVDesc));

        SRVDesc.Format                = DXGI_FORMAT_R32_FLOAT;
        SRVDesc.ViewDimension         = D3D11_SRV_DIMENSION_BUFFER;
        SRVDesc.Buffer.FirstElement  = 0;
        SRVDesc.Buffer.NumElements   = m_HairAssetLoader.m_NumHairVertices;
        
        V_RETURN( pd3dDevice->CreateShaderResourceView(m_pThicknessCoeffsBuffer, &SRVDesc, &m_pThicknessCoeffsSRV) );
    }


    //-----------------------------------
    // Index buffer (lines and triangles)
    //-----------------------------------

    // Line index buffer
    m_TotalIndexCount = m_HairAssetLoader.m_LineIndices.size();

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( unsigned int ) * m_HairAssetLoader.m_LineIndices.size();
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER | D3D11_BIND_SHADER_RESOURCE;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;

    InitData.pSysMem = &(m_HairAssetLoader.m_LineIndices[0]);
    hr = pd3dDevice->CreateBuffer( &bd, &InitData, &m_pIndexBuffer );

    if( FAILED(hr) )
        return hr;

    // Triangle index buffer
    m_TotalTriangleIndexCount = m_HairAssetLoader.m_Triangleindices.size();
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof( unsigned int ) * m_HairAssetLoader.m_Triangleindices.size();
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER | D3D11_BIND_SHADER_RESOURCE;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;

    InitData.pSysMem = &(m_HairAssetLoader.m_Triangleindices[0]);
    hr = pd3dDevice->CreateBuffer( &bd, &InitData, &m_pTriangleIndexBuffer );

    if( FAILED(hr) )
        return hr;
    
    // triangle index buffer srv
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC sbSRVDesc;
        sbSRVDesc.Buffer.FirstElement           = 0;
        sbSRVDesc.Buffer.NumElements            = (UINT)m_HairAssetLoader.m_Triangleindices.size();
        sbSRVDesc.Format                        = DXGI_FORMAT_R32_UINT;
        sbSRVDesc.ViewDimension                 = D3D11_SRV_DIMENSION_BUFFER;
        SAFE_RELEASE(m_pTriangleIndexBufferSRV);
        V_RETURN( pd3dDevice->CreateShaderResourceView(m_pTriangleIndexBuffer, &sbSRVDesc, &m_pTriangleIndexBufferSRV) );
    }

	hr = CreateBufferAndViews(pd3dDevice);

    return hr;
}


//--------------------------------------------------------------------------------------
//
// CreateBufferAndViews
//
// Called by the OnCreateDevice function to create the D3D resources
//
//--------------------------------------------------------------------------------------
HRESULT CTressFXMesh::CreateBufferAndViews(ID3D11Device* pd3dDevice)
{
    HRESULT hr;

    //--------------------------------
    // m_HairVerticesOffsetsSRV
    //--------------------------------
    {
        D3D11_SUBRESOURCE_DATA initialData;
        initialData.pSysMem = m_HairAssetLoader.m_pHairVerticesOffsets;

        HRESULT hr;
        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.ByteWidth = m_HairAssetLoader.m_NumHairStrands * sizeof(int);
        bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    
        SAFE_RELEASE(m_HairVerticesOffsetsBuffer);
        V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, &initialData, &m_HairVerticesOffsetsBuffer));
    
        D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
        SRVDesc.Format = DXGI_FORMAT_R32_SINT;
        SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        SRVDesc.Buffer.FirstElement = 0;
        SRVDesc.Buffer.NumElements = m_HairAssetLoader.m_NumHairStrands;
        SAFE_RELEASE(m_HairVerticesOffsetsSRV);
        V_RETURN(pd3dDevice->CreateShaderResourceView(m_HairVerticesOffsetsBuffer, &SRVDesc, &m_HairVerticesOffsetsSRV));
    }

    //--------------------------------
    // m_HairStrandTypeSRV
    //--------------------------------
    {
        D3D11_SUBRESOURCE_DATA initialData;
        initialData.pSysMem = m_HairAssetLoader.m_pHairStrandType;

        HRESULT hr;
        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;
        bufferDesc.ByteWidth = m_HairAssetLoader.m_NumHairStrands * sizeof(int);
        bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    
        SAFE_RELEASE(m_HairStrandTypeBuffer);
        V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, &initialData, &m_HairStrandTypeBuffer));
    
        D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
        SRVDesc.Format = DXGI_FORMAT_R32_SINT;
        SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        SRVDesc.Buffer.FirstElement = 0;
        SRVDesc.Buffer.NumElements = m_HairAssetLoader.m_NumHairStrands;
        SAFE_RELEASE(m_HairStrandTypeSRV);
        V_RETURN(pd3dDevice->CreateShaderResourceView(m_HairStrandTypeBuffer, &SRVDesc, &m_HairStrandTypeSRV));
    }

    //---------------------------------
    // m_InitialHairPositionsSRV
    // m_InitialHairPositionsUAV
    //---------------------------------
    {
        D3D11_SUBRESOURCE_DATA initialData;
        initialData.pSysMem = m_HairAssetLoader.m_pVertices;

        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.ByteWidth = m_HairAssetLoader.m_NumHairVertices * sizeof(D3DXVECTOR4);
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDesc.Usage = D3D11_USAGE_DEFAULT; //D3D11_USAGE_IMMUTABLE;
        bufferDesc.StructureByteStride  = sizeof(D3DXVECTOR4);
        bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;

        SAFE_RELEASE(m_InitialHairPositionsBuffer);
        V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, &initialData, &m_InitialHairPositionsBuffer));

        D3D11_SHADER_RESOURCE_VIEW_DESC desc;
        desc.Buffer.FirstElement = 0;
        desc.Buffer.NumElements = m_HairAssetLoader.m_NumHairVertices;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        SAFE_RELEASE(m_InitialHairPositionsSRV);
        V_RETURN(pd3dDevice->CreateShaderResourceView(m_InitialHairPositionsBuffer, &desc, &m_InitialHairPositionsSRV));
        
        D3D11_UNORDERED_ACCESS_VIEW_DESC sbUAVDesc;
        sbUAVDesc.Buffer.FirstElement       = 0;
        sbUAVDesc.Buffer.Flags              = 0;
        sbUAVDesc.Buffer.NumElements        = m_HairAssetLoader.m_NumHairVertices;
        sbUAVDesc.Format                    = DXGI_FORMAT_UNKNOWN; 
        sbUAVDesc.ViewDimension             = D3D11_UAV_DIMENSION_BUFFER;
        SAFE_RELEASE(m_InitialHairPositionsUAV);
        V_RETURN( pd3dDevice->CreateUnorderedAccessView(m_InitialHairPositionsBuffer, &sbUAVDesc, &m_InitialHairPositionsUAV) );
    }
    
    // ------------------------
    // m_HairVertexPositionsUAB
    //-------------------------
    {
        D3D11_SUBRESOURCE_DATA initialData;
        initialData.pSysMem = m_HairAssetLoader.m_pVertices;
        
        D3D11_BUFFER_DESC bufferDescUA;
        bufferDescUA.ByteWidth = m_HairAssetLoader.m_NumHairVertices * sizeof(D3DXVECTOR4);
        bufferDescUA.Usage = D3D11_USAGE_DEFAULT;
        bufferDescUA.BindFlags =  D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
        bufferDescUA.CPUAccessFlags = 0;
        bufferDescUA.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDescUA.StructureByteStride  = sizeof(D3DXVECTOR4);
        SAFE_RELEASE(m_HairVertexPositionsUAB);
        SAFE_RELEASE(m_HairVertexPositionsPrevUAB);
        V_RETURN(pd3dDevice->CreateBuffer(&bufferDescUA, &initialData, &m_HairVertexPositionsUAB));
        V_RETURN(pd3dDevice->CreateBuffer(&bufferDescUA, &initialData, &m_HairVertexPositionsPrevUAB));
    }

    // -----------------------------
    // m_HairVertexTangentsUAB
    //------------------------------
    {		
        D3D11_SUBRESOURCE_DATA initialData;
        initialData.pSysMem = m_HairAssetLoader.m_pTangents;

        D3D11_BUFFER_DESC bufferDescUA;
        bufferDescUA.ByteWidth = m_HairAssetLoader.m_NumHairVertices * sizeof(D3DXVECTOR4);
        bufferDescUA.Usage = D3D11_USAGE_DEFAULT;
        bufferDescUA.BindFlags =  D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
        bufferDescUA.CPUAccessFlags = 0;
        bufferDescUA.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDescUA.StructureByteStride  = sizeof(D3DXVECTOR4);
        SAFE_RELEASE(m_HairVertexTangentsUAB);
        V_RETURN(pd3dDevice->CreateBuffer(&bufferDescUA, &initialData, &m_HairVertexTangentsUAB));	
    }

    //-----------------------
    // m_HairVertexPositionsSRV
    //-----------------------
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC sbSRVDesc;
        sbSRVDesc.Buffer.FirstElement           = 0;
        sbSRVDesc.Buffer.NumElements            = m_HairAssetLoader.m_NumHairVertices;
        sbSRVDesc.Format                        = DXGI_FORMAT_UNKNOWN; //DXGI_FORMAT_R32G32B32A32_FLOAT
        sbSRVDesc.ViewDimension                 = D3D11_SRV_DIMENSION_BUFFER;
        SAFE_RELEASE(m_HairVertexPositionsSRV);
        V_RETURN( pd3dDevice->CreateShaderResourceView(m_HairVertexPositionsUAB, &sbSRVDesc, &m_HairVertexPositionsSRV) );
    }

    //-----------------------
    // m_HairVertexTangentsSRV
    //-----------------------
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC sbSRVDesc;
        sbSRVDesc.Buffer.FirstElement           = 0;
        sbSRVDesc.Buffer.NumElements            = m_HairAssetLoader.m_NumHairVertices;
        sbSRVDesc.Format                        = DXGI_FORMAT_UNKNOWN; //DXGI_FORMAT_R32G32B32A32_FLOAT
        sbSRVDesc.ViewDimension                 = D3D11_SRV_DIMENSION_BUFFER;
        SAFE_RELEASE(m_HairVertexTangentsSRV);
        V_RETURN( pd3dDevice->CreateShaderResourceView(m_HairVertexTangentsUAB, &sbSRVDesc, &m_HairVertexTangentsSRV) );
    }

    //-----------------------
    // m_HairVertexPositionsUAV
    //-----------------------
    {
        D3D11_UNORDERED_ACCESS_VIEW_DESC sbUAVDesc;
        sbUAVDesc.Buffer.FirstElement       = 0;
        sbUAVDesc.Buffer.Flags              = 0;
        sbUAVDesc.Buffer.NumElements        = m_HairAssetLoader.m_NumHairVertices;
        sbUAVDesc.Format                    = DXGI_FORMAT_UNKNOWN; //DXGI_FORMAT_R32G32B32A32_FLOAT
        sbUAVDesc.ViewDimension             = D3D11_UAV_DIMENSION_BUFFER;
        SAFE_RELEASE(m_HairVertexPositionsUAV);
        SAFE_RELEASE(m_HairVertexPositionsPrevUAV);
        V_RETURN( pd3dDevice->CreateUnorderedAccessView(m_HairVertexPositionsUAB, &sbUAVDesc, &m_HairVertexPositionsUAV) );
        V_RETURN( pd3dDevice->CreateUnorderedAccessView(m_HairVertexPositionsPrevUAB, &sbUAVDesc, &m_HairVertexPositionsPrevUAV) );
    }	

    //------------------------------
    // m_HairVertexTangentsUAV
    //------------------------------
    {
        D3D11_UNORDERED_ACCESS_VIEW_DESC sbUAVDesc;
        sbUAVDesc.Buffer.FirstElement       = 0;
        sbUAVDesc.Buffer.Flags              = 0;
        sbUAVDesc.Buffer.NumElements        = m_HairAssetLoader.m_NumHairVertices;
        sbUAVDesc.Format                    = DXGI_FORMAT_UNKNOWN; 
        sbUAVDesc.ViewDimension             = D3D11_UAV_DIMENSION_BUFFER;
        SAFE_RELEASE(m_HairVertexTangentsUAV);
        V_RETURN( pd3dDevice->CreateUnorderedAccessView(m_HairVertexTangentsUAB, &sbUAVDesc, &m_HairVertexTangentsUAV) );
    }	

    //---------------------------
    // m_HairRestLengthSRV
    //---------------------------
	{	
        HRESULT hr;
        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.ByteWidth = m_HairAssetLoader.m_NumHairVertices * sizeof(float);
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.BindFlags =  D3D11_BIND_SHADER_RESOURCE;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initialData;
        initialData.pSysMem = m_HairAssetLoader.m_pRestLengths;
        
        SAFE_RELEASE(m_HairLengthBuffer);
        V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, &initialData, &m_HairLengthBuffer));

        D3D11_SHADER_RESOURCE_VIEW_DESC desc;
        desc.Format = DXGI_FORMAT_R32_FLOAT;
        desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement = 0;
        desc.Buffer.NumElements = m_HairAssetLoader.m_NumHairVertices;
        SAFE_RELEASE(m_HairRestLengthSRV);
        V_RETURN(pd3dDevice->CreateShaderResourceView(m_HairLengthBuffer, &desc, &m_HairRestLengthSRV));
    }
	  
    //-----------------------------------
    // m_HairRefVecsInLocalFrameSRV
    //-----------------------------------
    {

        HRESULT hr;
        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.ByteWidth = m_HairAssetLoader.m_NumHairVertices * sizeof(D3DXVECTOR3);
        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.BindFlags =  D3D11_BIND_SHADER_RESOURCE;
        bufferDesc.CPUAccessFlags = 0;
        bufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initialData;
        initialData.pSysMem = m_HairAssetLoader.m_pRefVectors;
        
        SAFE_RELEASE(m_HairRefVectorsBuffer);
        V_RETURN(pd3dDevice->CreateBuffer(&bufferDesc, &initialData, &m_HairRefVectorsBuffer));

        D3D11_SHADER_RESOURCE_VIEW_DESC desc;
        desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
        desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement = 0;
        desc.Buffer.NumElements = m_HairAssetLoader.m_NumHairVertices;
        SAFE_RELEASE(m_HairRefVecsInLocalFrameSRV);
        V_RETURN(pd3dDevice->CreateShaderResourceView(m_HairRefVectorsBuffer, &desc, &m_HairRefVecsInLocalFrameSRV));
    }

    // --------------------------------------------
    // m_GlobalRotationsUAB & m_GlobalRotationsUAV
    //---------------------------------------------
    {

        D3D11_SUBRESOURCE_DATA initialData;
        initialData.pSysMem = m_HairAssetLoader.m_pGlobalRotations;

        D3D11_BUFFER_DESC bufferDescUA;
        bufferDescUA.ByteWidth = m_HairAssetLoader.m_NumHairVertices * sizeof(D3DXVECTOR4);
        bufferDescUA.Usage = D3D11_USAGE_DEFAULT;
        bufferDescUA.BindFlags =  D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
        bufferDescUA.CPUAccessFlags = 0;
        bufferDescUA.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDescUA.StructureByteStride  = sizeof(D3DXVECTOR4);
        SAFE_RELEASE(m_GlobalRotationsUAB);
        V_RETURN(pd3dDevice->CreateBuffer(&bufferDescUA, &initialData, &m_GlobalRotationsUAB));

        D3D11_UNORDERED_ACCESS_VIEW_DESC sbUAVDesc;
        sbUAVDesc.Buffer.FirstElement       = 0;
        sbUAVDesc.Buffer.Flags              = 0;
        sbUAVDesc.Buffer.NumElements        = m_HairAssetLoader.m_NumHairVertices;
        sbUAVDesc.Format                    = DXGI_FORMAT_UNKNOWN; 
        sbUAVDesc.ViewDimension             = D3D11_UAV_DIMENSION_BUFFER;
        SAFE_RELEASE(m_GlobalRotationsUAV);
        V_RETURN( pd3dDevice->CreateUnorderedAccessView(m_GlobalRotationsUAB, &sbUAVDesc, &m_GlobalRotationsUAV) );

    }

    // --------------------------------------------
    // m_LocalRotationsUAB & m_LocalRotationsUAV
    //---------------------------------------------
    {
        D3D11_SUBRESOURCE_DATA initialData;
        initialData.pSysMem = m_HairAssetLoader.m_pLocalRotations;

        D3D11_BUFFER_DESC bufferDescUA;
        bufferDescUA.ByteWidth = m_HairAssetLoader.m_NumHairVertices * sizeof(D3DXVECTOR4);
        bufferDescUA.Usage = D3D11_USAGE_DEFAULT;
        bufferDescUA.BindFlags =  D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
        bufferDescUA.CPUAccessFlags = 0;
        bufferDescUA.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufferDescUA.StructureByteStride  = sizeof(D3DXVECTOR4);
        SAFE_RELEASE(m_LocalRotationsUAB);
        V_RETURN(pd3dDevice->CreateBuffer(&bufferDescUA, &initialData, &m_LocalRotationsUAB));

        D3D11_UNORDERED_ACCESS_VIEW_DESC sbUAVDesc;
        sbUAVDesc.Buffer.FirstElement       = 0;
        sbUAVDesc.Buffer.Flags              = 0;
        sbUAVDesc.Buffer.NumElements        = m_HairAssetLoader.m_NumHairVertices;
        sbUAVDesc.Format                    = DXGI_FORMAT_UNKNOWN; 
        sbUAVDesc.ViewDimension             = D3D11_UAV_DIMENSION_BUFFER;
        SAFE_RELEASE(m_LocalRotationsUAV);
        V_RETURN( pd3dDevice->CreateUnorderedAccessView(m_LocalRotationsUAB, &sbUAVDesc, &m_LocalRotationsUAV) );
    }

    return S_OK;
}


//--------------------------------------------------------------------------------------
//
// OnDestroy
//
// Called when the D3D device is being destroyed. 
//
//--------------------------------------------------------------------------------------
void CTressFXMesh::OnDestroy()
{
    SAFE_RELEASE(m_pTriangleIndexBuffer);
    SAFE_RELEASE(m_pTriangleIndexBufferSRV);

    SAFE_RELEASE(m_pIndexBuffer);

    SAFE_RELEASE(m_pVertexBuffer);

    SAFE_RELEASE(m_pThicknessCoeffsBuffer);
    SAFE_RELEASE(m_pThicknessCoeffsSRV);

    //compute shader variables
    SAFE_RELEASE(m_HairVertexPositionsUAB);
    SAFE_RELEASE(m_HairVertexPositionsPrevUAB);
    SAFE_RELEASE(m_HairVertexTangentsUAB);
    SAFE_RELEASE(m_HairVertexPositionsSRV);
    SAFE_RELEASE(m_HairVertexTangentsSRV);
    SAFE_RELEASE(m_HairVertexPositionsUAV);
    SAFE_RELEASE(m_HairVertexPositionsPrevUAV);
    SAFE_RELEASE(m_HairVertexTangentsUAV);
    
    SAFE_RELEASE(m_HairVerticesOffsetsBuffer);
    SAFE_RELEASE(m_HairVerticesOffsetsSRV);

    SAFE_RELEASE(m_HairStrandTypeBuffer);
    SAFE_RELEASE(m_HairStrandTypeSRV);

    //vertex buffers
    SAFE_RELEASE(m_HairLengthBuffer);
    SAFE_RELEASE(m_HairRestLengthSRV);

    SAFE_RELEASE(m_InitialHairPositionsBuffer);
    SAFE_RELEASE(m_InitialHairPositionsSRV);
    SAFE_RELEASE(m_InitialHairPositionsUAV);

    SAFE_RELEASE(m_HairRefVectorsBuffer);
    SAFE_RELEASE(m_HairRefVecsInLocalFrameSRV);

    SAFE_RELEASE(m_GlobalRotationsUAB);
    SAFE_RELEASE(m_GlobalRotationsUAV);

    SAFE_RELEASE(m_LocalRotationsUAB);
    SAFE_RELEASE(m_LocalRotationsUAV);
}
