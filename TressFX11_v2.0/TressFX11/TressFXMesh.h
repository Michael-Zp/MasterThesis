//--------------------------------------------------------------------------------------
// File: TressFXMesh.h
//
// Hair mesh code
// 
// Copyright © AMD Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once
#include "..\\DXUT\\Core\\DXUT.h"
#include "..\\DXUT\\Optional\\DXUTcamera.h"
#include "util.h"
#include "TressFXAssetLoader.h"

// If you change the value below, you must change it in TresFXSimulation.hlsl as well. 
#define THREAD_GROUP_SIZE 64
#define HAIR_MAX_SECTIONS 4

class CTressFXMesh
{
private:
	CTressFXAssetLoader	m_HairAssetLoader;

	// private member function
	HRESULT CreateBufferAndViews(ID3D11Device* pd3dDevice);

public:
    ID3D11Buffer*				m_pVertexBuffer;
	ID3D11Buffer*				m_pIndexBuffer;
    ID3D11Buffer*				m_pTriangleIndexBuffer;
    ID3D11ShaderResourceView*	m_pTriangleIndexBufferSRV;
	ID3D11Buffer*				m_pThicknessCoeffsBuffer;
	ID3D11ShaderResourceView*	m_pThicknessCoeffsSRV;
    ID3D11InputLayout*			m_pLayout;
	int							m_MaxNumOfVerticesInStrand;
	int							m_NumHairStrands;
	int							m_TotalIndexCount;
    int							m_TotalTriangleIndexCount;
	BSphere						m_bSphere;

    // vertex positions
    ID3D11Buffer* m_HairVertexPositionsUAB;
    ID3D11UnorderedAccessView* m_HairVertexPositionsUAV;
	ID3D11ShaderResourceView* m_HairVertexPositionsSRV;
    
    ID3D11Buffer* m_HairVertexPositionsPrevUAB;
    ID3D11UnorderedAccessView* m_HairVertexPositionsPrevUAV;
	    
    ID3D11Buffer* m_InitialHairPositionsBuffer;
    ID3D11ShaderResourceView* m_InitialHairPositionsSRV;
    ID3D11UnorderedAccessView* m_InitialHairPositionsUAV;

    // vertex offsets
    ID3D11Buffer* m_HairVerticesOffsetsBuffer;
    ID3D11ShaderResourceView* m_HairVerticesOffsetsSRV;

    // strand type
    ID3D11Buffer* m_HairStrandTypeBuffer;
    ID3D11ShaderResourceView* m_HairStrandTypeSRV;
    
    // rest length
    ID3D11Buffer* m_HairLengthBuffer;
    ID3D11ShaderResourceView* m_HairRestLengthSRV;

    // reference vectors
    ID3D11Buffer* m_HairRefVectorsBuffer;
    ID3D11ShaderResourceView* m_HairRefVecsInLocalFrameSRV;

    // global and local transforms for each vertex
    ID3D11Buffer* m_GlobalRotationsUAB;
    ID3D11UnorderedAccessView* m_GlobalRotationsUAV;

    ID3D11Buffer* m_LocalRotationsUAB;
    ID3D11UnorderedAccessView* m_LocalRotationsUAV;

    // vertex tangents
    ID3D11Buffer* m_HairVertexTangentsUAB;
    ID3D11UnorderedAccessView* m_HairVertexTangentsUAV;
    ID3D11ShaderResourceView* m_HairVertexTangentsSRV;

	// public member functions
	CTressFXMesh(void);
	~CTressFXMesh(void);

	HRESULT OnCreateDevice(ID3D11Device* pd3dDevice, char **filenameArray, bool* bMakeBothEndsImmovable, int numFiles,
				float fScale, D3DXVECTOR3 vRotation, D3DXVECTOR3 vTranslation);
	void OnDestroy();
};
