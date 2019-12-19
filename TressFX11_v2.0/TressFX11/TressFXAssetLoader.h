//--------------------------------------------------------------------------------------
// File: TressFXAssetLoader.h
//
// Hair asset loading classes declared here.
// 
// Copyright © AMD Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include <vector>
#include <list>
#include <assert.h>

#include <iostream>
#include <fstream>

#include "Math\\Vector3D.h"
#include "Math\\Transform.h"
#include "..\\DXUT\\Core\\DXUT.h"
#include "Util.h"

struct CTressFXVertex
{
	CVector3D m_Pos;	
	float m_InvMass; // In case mass is infinite, m_InvMass is zero

	CTransform m_GlobalTransform;
	CTransform m_LocalTransform;

	CVector3D m_XVec;
	CVector3D m_ZVec; 
	
	CVector3D m_RefVecInLocalFrame;
	CVector3D m_OrgPosInLocalFrame_i_minus_1;
};

class CTressFXAsset
{
friend class CTressFXAssetLoader;

public:
	CTressFXAsset(void);
	~CTressFXAsset(void);

private:
	std::vector<CTressFXVertex> m_VertexArray;

public:
	int m_GroupID;

	std::vector<CTressFXVertex>& GetVertexArray() { return m_VertexArray; }
	const std::vector<CTressFXVertex>& GetVertexArray() const { return m_VertexArray; }
	
	void Clear();
	void ConstructAsset();
	void InitGlobalLocalFrame();
};

class CTressFXAssetLoader
{
public:
	CTressFXAssetLoader(void);
	~CTressFXAssetLoader(void);

	int				m_NumHairVertices;
    int				m_NumHairStrands;
    int*			m_pHairVerticesOffsets;
    int*			m_pHairStrandType;
	D3DXVECTOR3*	m_pRefVectors;
	D3DXVECTOR4*	m_pGlobalRotations;
	D3DXVECTOR4*	m_pLocalRotations;
    D3DXVECTOR4*	m_pVertices;
    D3DXVECTOR4*	m_pTangents;
	StrandVertex*	m_pTriangleVertices;
	float*			m_pThicknessCoeffs;
	vector<int>		m_LineIndices;
    vector<int>		m_Triangleindices;
	float*			m_pRestLengths;

	BSphere			m_bSphere;

private:
	int m_NumTotalVertices;
	std::vector<CTressFXAsset*> m_Hairs;
    vector< vector<StrandVertex> >	m_HairStrands;

	void ComputeStrandTangent(vector<StrandVertex>& strand);
	float ComputeDistanceToRoot(vector<StrandVertex>& strand);
	void ScaleRotateTranslate(	float scale = 1, 
								D3DXVECTOR3 rotation = D3DXVECTOR3(0, 0, 0), 
								D3DXVECTOR3 translation = D3DXVECTOR3(0, 0, 0));
	void Clear();

public:
	bool LoadAppend(const char* filename, int groupId, bool bMakeBothEndsImmovable, int maxNumOfVerticesInStrand, int threadGroupSize);
	void ProcessVertices(float scale, D3DXVECTOR3 rotation, D3DXVECTOR3 translation);
	int GetNumTotalVertices() const { return m_NumTotalVertices; }
	std::vector<CTressFXAsset*>& GetHairs() { return m_Hairs; }
	const std::vector<CTressFXAsset*>& GetHairs() const { return m_Hairs; }
};
