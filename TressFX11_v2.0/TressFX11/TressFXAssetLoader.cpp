//--------------------------------------------------------------------------------------
// File: TressFXAssetLoader.cpp
//
// File for loading the hair assets 
// 
// Copyright © AMD Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "TressFXAssetLoader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "StringTokenizer.h"

using namespace std;


//--------------------------------------------------------------------------------------
//
// Constructor
//
//--------------------------------------------------------------------------------------
CTressFXAsset::CTressFXAsset(void)
{
   
}

//--------------------------------------------------------------------------------------
//
// Destructor
//
//--------------------------------------------------------------------------------------
CTressFXAsset::~CTressFXAsset(void)
{
    Clear();
}

//--------------------------------------------------------------------------------------
//
// Clear
//
// Clears the vertex array
//
//--------------------------------------------------------------------------------------
void CTressFXAsset::Clear()
{
    m_VertexArray.clear();
}

//--------------------------------------------------------------------------------------
//
// ConstructAsset
//
// Creates an array of vertices for the hair strand
//
//--------------------------------------------------------------------------------------
void CTressFXAsset::ConstructAsset()
{   
    // reference vector
    for ( int i = 1; i < (int)m_VertexArray.size()-1; i++ )
    {
        CTressFXVertex& vert_i_minus_1 = m_VertexArray[i-1];
        CTressFXVertex& vert_i = m_VertexArray[i];
        CTressFXVertex& vert_i_plus_1 = m_VertexArray[i+1];

        // arbitrary vector
        CVector3D a(1.0f, 0, 0);

        CVector3D X_i = (vert_i.m_Pos - vert_i_minus_1.m_Pos);
        CVector3D X_i_plus_one = (vert_i_plus_1.m_Pos - vert_i.m_Pos);

        CVector3D X_i_norm = X_i.NormalizeOther();

        vert_i.m_XVec = X_i_norm;

        CVector3D Z_i = X_i.Cross(a);

        if ( Z_i.LengthSqr() < 0.0001 )
        {
            // another arbitrary vector
            a.Set(0, 1.0f, 0);
            Z_i = X_i_norm.Cross(a);
        }

        Z_i.Normalize();

        vert_i.m_ZVec = Z_i;

        CVector3D Y_i = Z_i.Cross(X_i_norm).Normalize();

        // reference vector
        vert_i.m_RefVecInLocalFrame = X_i_plus_one;

        CMatrix33 rot;

        rot(0, 0) = X_i_norm.m_X;	rot(0, 1) = Y_i.m_X;	rot(0, 2) = Z_i.m_X;
        rot(1, 0) = X_i_norm.m_Y;	rot(1, 1) = Y_i.m_Y;	rot(1, 2) = Z_i.m_Y;
        rot(2, 0) = X_i_norm.m_Z;	rot(2, 1) = Y_i.m_Z;	rot(2, 2) = Z_i.m_Z;

        rot.Transpose(); // inverse it. 

        vert_i.m_RefVecInLocalFrame = rot * vert_i.m_RefVecInLocalFrame;
    }

    InitGlobalLocalFrame();
}


//--------------------------------------------------------------------------------------
//
// InitGlobalLocalFrame
//
// Initializes the local and global transformations for the hair strand
//
//--------------------------------------------------------------------------------------
void CTressFXAsset::InitGlobalLocalFrame()
{
    // vertex 0
    {
        CTressFXVertex& vert_i = m_VertexArray[0];
        CTressFXVertex& vert_i_plus_1 = m_VertexArray[1];

        const CVector3D vec = vert_i_plus_1.m_Pos - vert_i.m_Pos;
        CVector3D vecX = vec.NormalizeOther();

        CVector3D vecZ = vecX.Cross(CVector3D(1.0, 0, 0));

        if ( vecZ.LengthSqr() < 0.0001 )
        {
            vecZ = vecX.Cross(CVector3D(0, 1.0f, 0));
        }

        vecZ.Normalize();
        CVector3D vecY = vecZ.Cross(vecX).Normalize();

        CMatrix33 rotL2W;

        rotL2W(0, 0) = vecX.m_X;	rotL2W(0, 1) = vecY.m_X;		rotL2W(0, 2) = vecZ.m_X;
        rotL2W(1, 0) = vecX.m_Y;	rotL2W(1, 1) = vecY.m_Y;		rotL2W(1, 2) = vecZ.m_Y;
        rotL2W(2, 0) = vecX.m_Z;	rotL2W(2, 1) = vecY.m_Z;		rotL2W(2, 2) = vecZ.m_Z;

        vert_i.m_LocalTransform.GetRotation() = rotL2W;
        vert_i.m_LocalTransform.GetTranslation() = vert_i.m_Pos;
        vert_i.m_GlobalTransform = vert_i.m_LocalTransform; // For vertex 0, local and global transforms are the same. 
    }

    // vertex 1 through n-1
    for (int i = 1 ; i < (int)m_VertexArray.size(); i++ )
    {
        CTressFXVertex& vert_i_minus_1 = m_VertexArray[i-1];
        CTressFXVertex& vert_i = m_VertexArray[i];
        
        CVector3D vec = vert_i.m_Pos - vert_i_minus_1.m_Pos;
        vec = vert_i_minus_1.m_GlobalTransform.GetRotation().InverseOther() * vec;

        CVector3D vecX = vec.NormalizeOther();
    
        CVector3D X = CVector3D(1.0f, 0, 0);
        CVector3D rotAxis = X.Cross(vecX);
        float angle = acos(X.Dot(vecX));

        if ( abs(angle) < 0.001 || rotAxis.LengthSqr() < 0.001 )
        {
            vert_i.m_LocalTransform.GetRotation().SetIdentity();
        }
        else
        {
            rotAxis.Normalize();
            CQuaternion rot = CQuaternion(rotAxis, angle);
            vert_i.m_LocalTransform.GetRotation() = rot;
        }

        vert_i.m_LocalTransform.GetTranslation() = vec;
        vert_i.m_GlobalTransform = vert_i_minus_1.m_GlobalTransform * vert_i.m_LocalTransform;
        vert_i.m_OrgPosInLocalFrame_i_minus_1 = vert_i.m_LocalTransform.GetTranslation();
    }

    for (int i = 0 ; i < (int)m_VertexArray.size(); i++ )
    {
        CTressFXVertex& vert = m_VertexArray[i];

        CVector3D x(1.0f, 0.0f, 0.0f);
        CVector3D y(0.0f, 1.0f, 0.0f);
        CVector3D z(0.0f, 0.0f, 1.0f);
        
        x = vert.m_GlobalTransform * x;
        y = vert.m_GlobalTransform * y;
        z = vert.m_GlobalTransform * z;

        vert.m_XVec = x - vert.m_Pos;
        vert.m_ZVec = z - vert.m_Pos;
    }
}

//--------------------------------------------------------------------------------------
//
// Constructor
//
//--------------------------------------------------------------------------------------
CTressFXAssetLoader::CTressFXAssetLoader(void) 
{
	m_pHairVerticesOffsets = NULL;
	m_pHairStrandType = NULL;
	m_pRefVectors = NULL;
	m_pTriangleVertices = NULL;
	m_pGlobalRotations = NULL;
	m_pLocalRotations = NULL;
	m_pVertices = NULL;
	m_pTangents = NULL;
	m_pThicknessCoeffs = NULL;
	m_pRestLengths = NULL;
}

//--------------------------------------------------------------------------------------
//
// Destructor
//
//--------------------------------------------------------------------------------------
CTressFXAssetLoader::~CTressFXAssetLoader(void)
{
    if ( m_pVertices )
    {
        delete [] m_pVertices;
        m_pVertices = NULL;
    }

    if ( m_pTangents )
    {
        delete [] m_pTangents;
        m_pTangents = NULL;
    }

    if ( m_pHairVerticesOffsets )
    {
        delete [] m_pHairVerticesOffsets;
        m_pHairVerticesOffsets = NULL;
    }

    if ( m_pHairStrandType )
    {
        delete [] m_pHairStrandType;
        m_pHairStrandType = NULL;
    }

	if ( m_pLocalRotations )
	{
		delete [] m_pLocalRotations;
		m_pLocalRotations = NULL;
	}

	if ( m_pGlobalRotations )
	{
		delete [] m_pGlobalRotations;
		m_pGlobalRotations = NULL;
	}

	if ( m_pRefVectors )
	{
		delete [] m_pRefVectors;
		m_pRefVectors = NULL;
	}

	if ( m_pTriangleVertices )
	{
		delete [] m_pTriangleVertices;
		m_pTriangleVertices = NULL;
	}

	if ( m_pThicknessCoeffs )
	{
		delete [] m_pThicknessCoeffs;
		m_pThicknessCoeffs = NULL;
	}
	
	if ( m_pRestLengths )
	{
		delete [] m_pRestLengths;
		m_pRestLengths = NULL;
	}

    Clear();
}

//--------------------------------------------------------------------------------------
//
// LoadAppend
//
// Reads in a file of hair data and appends it to the list of hair strands
//
//--------------------------------------------------------------------------------------
bool CTressFXAssetLoader::LoadAppend(const char* filename, int groupId, bool bMakeBothEndsImmovable, 
								int maxNumOfVerticesInStrand, int threadGoupSize)
{
    ifstream inFile(filename);
    string sLine;
    vector<string> sTokens;

    if ( !inFile.is_open() )
        return false;

    getline(inFile, sLine);
    sTokens.clear(); 
    int numFound = StringTokenizer(sLine, string(" "), sTokens, false);

    if ( numFound == 0 )
        return false;

    int numStrands = 0;

    vector <string>::iterator iter;
    string sToken; 

    iter = sTokens.begin();
    sToken = *(iter);

    assert(sToken == "numStrands");

    ++iter;
    sToken = *(iter);

    numStrands = atoi(sToken.c_str());

    getline(inFile, sLine); // is sorted 1

    for ( int strand = 0; strand < numStrands; strand++ )
    {
        CTressFXAsset* pHair = new CTressFXAsset();

        // numVerts
        sTokens.clear(); 
		sTokens.reserve(10);
        getline(inFile, sLine); // strand 0 numVerts 25 texcoord 0.000000 0.522833
        StringTokenizer(sLine, string(" "), sTokens, false);
        iter = sTokens.begin() + 3;
        sToken = *(iter);
        int numVerts = atoi(sToken.c_str());

		// Limit the maximum number of vertices per strand.
		if ( numVerts > maxNumOfVerticesInStrand )
			numVerts = maxNumOfVerticesInStrand;

        pHair->GetVertexArray().reserve(numVerts);

        int indexVert = 0;

        for ( int vertex = 0; vertex < numVerts; vertex++ )
        {
            getline(inFile, sLine);
            sTokens.clear(); 
			sTokens.reserve(10);
            int numFound = StringTokenizer(sLine, string(" "), sTokens, false);

            if ( numFound == 0 )
                continue;

            iter = sTokens.begin();

            if ( (*iter) == "-1.#INF" )
                continue;

            CVector3D pnt;
            
            // x
            sToken = (*iter);			
            pnt.m_X = (float)atof(sToken.c_str());

            // y
            ++iter;
            sToken = (*iter);			
            pnt.m_Y = (float)atof(sToken.c_str());

            // z
            ++iter;
            sToken = (*iter);			
            pnt.m_Z = (float)atof(sToken.c_str());
            
            CTressFXVertex vert;
            vert.m_Pos.Set(pnt.m_X, pnt.m_Y, pnt.m_Z);

			// In some cases, two end vertices in both ends of strand are needed to be immovable. 
            if ( bMakeBothEndsImmovable )
            {
                if ( vertex == 0 || vertex == 1 || vertex == numVerts -1 || vertex == numVerts - 2 )
                    vert.m_InvMass = 0;
                else
                    vert.m_InvMass = 1.0f;
            }
            else
            {
                if ( vertex == 0 || vertex == 1 )
                    vert.m_InvMass = 0;
                else
                    vert.m_InvMass = 1.0f;
            }

            bool bPushback = true;

            if ( vertex > 0 )
            {
                const CTressFXVertex& vert_minus_1 = pHair->GetVertexArray()[indexVert-1];

                float length = (vert.m_Pos - vert_minus_1.m_Pos).Length();

                if ( length < 0.05f )
                    bPushback = false;
            }

            if ( bPushback )
            {
                pHair->GetVertexArray().push_back(vert);
                indexVert++;
            }
        }

		pHair->m_GroupID = groupId;
		
        // add the new hair into m_Hairs
        if ( pHair->GetVertexArray().size() > 2 )
            m_Hairs.push_back(pHair);
        else
            delete pHair;
    }

    inFile.close();

	// Make the number of strands a multiple of thread group size.
	// This is an easy way to avoid branching in compute shader.
	unsigned int loadedNumStrands = m_Hairs.size();
	unsigned int numOfDelete = loadedNumStrands % threadGoupSize;

	for ( unsigned int i = 0; i < numOfDelete; i++ )
	{
		std::vector<CTressFXAsset*>::iterator iter = --m_Hairs.end();

		if ( (*iter) )
			delete (*iter);

		m_Hairs.pop_back();
	}
	
    return true;	
}

//--------------------------------------------------------------------------------------
//
// Clear
//
// Clear the array of hair strands
//
//--------------------------------------------------------------------------------------
void CTressFXAssetLoader::Clear()
{
    for ( int i = 0; i < (int)m_Hairs.size(); i++ )
    {
        if ( m_Hairs[i] )
            delete m_Hairs[i];
    }

    m_Hairs.clear();
}

//--------------------------------------------------------------------------------------
//
// ComputeStrandTangent
//
// Calculates the tangent value for each vertices of the strand
//
//--------------------------------------------------------------------------------------
void CTressFXAssetLoader::ComputeStrandTangent(vector<StrandVertex>& strand)
{
    int numVertices = strand.size();

    // Calculate the tangent value for the first vertex of the strand 
    D3DXVECTOR3 tangent = strand[1].position - strand[0].position;
    D3DXVec3Normalize(&tangent, &tangent);
    strand[0].tangent = tangent;
    
    for (int vertex = 1; vertex < numVertices-1; vertex++) 
    {
        D3DXVECTOR3 tangent_pre = strand[vertex].position - strand[vertex-1].position;
        D3DXVec3Normalize(&tangent_pre, &tangent_pre);

        D3DXVECTOR3 tangent_next = strand[vertex+1].position - strand[vertex].position;
        D3DXVec3Normalize(&tangent_next, &tangent_next);

        tangent = tangent_pre + tangent_next;
        D3DXVec3Normalize(&tangent, &tangent);

        strand[vertex].tangent = tangent;
    }

    // Calculate the tangent value for the last vertex of the strand 
    tangent = strand[numVertices-1].position - strand[numVertices-2].position;
    D3DXVec3Normalize(&tangent, &tangent);
    strand[numVertices-1].tangent = tangent;
}

//--------------------------------------------------------------------------------------
//
// ComputeDistanceToRoot
//
// Calculates the parametric distance to the root for each vertex in the strand
//
// z value: [0, 1] root:0, tip:1
//
//--------------------------------------------------------------------------------------
float CTressFXAssetLoader::ComputeDistanceToRoot(vector<StrandVertex>& strand)
{
    int numVertices = strand.size();
    float strandLength = 0;
    strand[0].texcoord.z = 0;
    for (int i=1; i<numVertices; i++)
    {
        D3DXVECTOR3 vec = strand[i].position - strand[i-1].position;
        float disSeg = D3DXVec3Length(&vec);
        strand[i].texcoord.z = strand[i-1].texcoord.z + disSeg;
        strandLength += disSeg;
    }
    for (int i=0; i<numVertices; i++)
    {
        strand[i].texcoord.z /= strandLength;
    }

    return strandLength;
}

//--------------------------------------------------------------------------------------
//
// ScaleRotateTranslate
//
// Affine transforms on the mesh
//
//--------------------------------------------------------------------------------------
void CTressFXAssetLoader::ScaleRotateTranslate(float scale, 
                                            D3DXVECTOR3 rotation, 
                                            D3DXVECTOR3 translation)
{
    // Scale, rotate, translate and calculate bounding box and sphere
    BBox bBox;

    // Translation
    for(int i=0; i < int(GetHairs().size()); i++)
    {
        std::vector<CTressFXVertex>& vertices = GetHairs().at(i)->GetVertexArray();

        for ( int j = 0; j < (int)vertices.size(); j++ )
        {

            StrandVertex* pVertex = &(m_HairStrands[i][j]);

            // scale
            pVertex->position *= scale;

            // rotation
            D3DXMATRIX rotateMat;
            D3DXMatrixRotationYawPitchRoll(&rotateMat, rotation.y, rotation.x, rotation.z);

            D3DXVECTOR3 temp;
            D3DXVec3TransformCoord(&temp, &(pVertex->position), &rotateMat);
            pVertex->position = temp;
            
            D3DXVec3TransformNormal(&temp, &(pVertex->tangent), &rotateMat);
            pVertex->tangent = temp;

            // translation
            pVertex->position += translation;
            bBox = Union(bBox, Float3(pVertex->position));
        }
    }
        
    Float3 c; float radius;
    bBox.BoundingSphere(&c, &radius);
    m_bSphere.center = D3DXVECTOR3(c.x, c.y, c.z);
    m_bSphere.radius = radius;
}

//--------------------------------------------------------------------------------------
//
// vec3Length
//
// Retuns the length of a 3D vector
//
//--------------------------------------------------------------------------------------
static float vec3Length(D3DXVECTOR3 vec)
{
    return sqrtf(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

//--------------------------------------------------------------------------------------
//
// ProcessVertices
//
// After all of the vertices have been loaded ProcessVertices is called to create the
// associated data with the hair vertices, which includes attributes like tangents,strand
// length, and transformations. Also the hair type is stored with each vertex which 
// allows different simulation parameters for different sections of the hair.
//
//--------------------------------------------------------------------------------------
void CTressFXAssetLoader::ProcessVertices(float fScale, D3DXVECTOR3 vRotation, D3DXVECTOR3 vTranslation)
{
	// count vertices
	for ( int i = 0; i < (int)m_Hairs.size(); i++ )
		m_NumTotalVertices += (int)m_Hairs[i]->GetVertexArray().size();

	// construct reference vectors and reference frames
    for ( int i = 0; i < (int)m_Hairs.size(); i++ )
    {
        m_Hairs[i]->ConstructAsset();
    }

	// randomize the hair strands so that hair density can be changed by simply adjusting the size of the array
	random_shuffle(GetHairs().begin(), GetHairs().end());
	
	m_NumHairStrands = GetHairs().size();
	m_NumHairVertices = GetNumTotalVertices();
    m_pVertices = new D3DXVECTOR4[m_NumHairVertices];
    m_pHairVerticesOffsets = new int[m_NumHairStrands];
    m_pHairStrandType = new int[m_NumHairStrands];  
    m_HairStrands.clear();
    m_HairStrands.resize(m_NumHairStrands);
    m_pTangents = new D3DXVECTOR4[m_NumHairVertices];
    int indexTang = 0;
    
	// Initialize the hair strands and compute tangents
    for ( int i=0; i < m_NumHairStrands; i++ )
    {
        int numVerts = int(GetHairs().at(i)->GetVertexArray().size());
        m_HairStrands[i].resize(numVerts);

		for( int v=0; v < numVerts; v++ )
        {
            D3DXVECTOR3 pos;
            pos.x = GetHairs().at(i)->GetVertexArray().at(v).m_Pos.m_X;
            pos.y = GetHairs().at(i)->GetVertexArray().at(v).m_Pos.m_Y;
            pos.z = GetHairs().at(i)->GetVertexArray().at(v).m_Pos.m_Z;

            m_HairStrands[i][v].position = pos;
        }

        ComputeStrandTangent(m_HairStrands[i]);
		ComputeDistanceToRoot(m_HairStrands[i]);

        for( int v=0; v < numVerts; v++ )
        {
            m_pTangents[indexTang].x = m_HairStrands[i][v].tangent.x;
            m_pTangents[indexTang].y = m_HairStrands[i][v].tangent.y;
            m_pTangents[indexTang].z = m_HairStrands[i][v].tangent.z;

            indexTang++;
        }
    }

    m_pRestLengths = new float[m_NumHairVertices];
	int index = 0;

	// Calculate rest lengths
	for ( int i = 0; i < m_NumHairStrands; i++ )
	{
		for ( int j = 0; j < (int)m_HairStrands[i].size()-1; j++ )
		{
			m_pRestLengths[index++] = vec3Length(m_HairStrands[i][j].position - m_HairStrands[i][j+1].position);
		}

		// Since number of edges are one less than number of vertices in hair strand, below line acts as a placeholder. 
		m_pRestLengths[index++] = 0;
	}

	assert(index == m_NumHairVertices);

    m_pRefVectors = new D3DXVECTOR3[m_NumHairVertices];
    m_pGlobalRotations = new D3DXVECTOR4[m_NumHairVertices];
	m_pLocalRotations = new D3DXVECTOR4[m_NumHairVertices];
    m_pTriangleVertices = new StrandVertex[m_NumHairVertices];
    m_pThicknessCoeffs = new float[m_NumHairVertices];
    m_LineIndices.reserve(m_NumHairVertices * 2);
    m_Triangleindices.reserve(m_NumHairVertices * 6);
    int id=0;
    index = 0;

	// initialize the remainder of the hair data
    for ( int i = 0; i < m_NumHairStrands; i++ )
    {
		int vertCount = GetHairs().at(i)->GetVertexArray().size();

        for ( int j = 0; j < vertCount - 1; j++ )
        {
			// line indices
            m_LineIndices.push_back(id);
            m_LineIndices.push_back(id+1);

 			// triangle indices
            m_Triangleindices.push_back(2*id);
            m_Triangleindices.push_back(2*id+1);
            m_Triangleindices.push_back(2*id+2);
            m_Triangleindices.push_back(2*id+2);
            m_Triangleindices.push_back(2*id+1);
            m_Triangleindices.push_back(2*id+3);
            id++;
		}

		id++;

        for ( int j = 0; j < vertCount; j++ )
        {
			// triangle vertices
            m_pTriangleVertices[index] = m_HairStrands[i][j];
            float tVal = m_pTriangleVertices[index].texcoord.z;
            m_pThicknessCoeffs[index] = sqrt(1.f - tVal * tVal);

            D3DXVECTOR4 v;

			// temp vertices
            v.x = GetHairs().at(i)->GetVertexArray().at(j).m_Pos.m_X;
            v.y = GetHairs().at(i)->GetVertexArray().at(j).m_Pos.m_Y;
            v.z = GetHairs().at(i)->GetVertexArray().at(j).m_Pos.m_Z;
            v.w = GetHairs().at(i)->GetVertexArray().at(j).m_InvMass;
            m_pVertices[index] = v;

			// global rotations
            v.x = GetHairs().at(i)->GetVertexArray().at(j).m_GlobalTransform.GetRotation().m_X;
            v.y = GetHairs().at(i)->GetVertexArray().at(j).m_GlobalTransform.GetRotation().m_Y;
            v.z = GetHairs().at(i)->GetVertexArray().at(j).m_GlobalTransform.GetRotation().m_Z;
            v.w = GetHairs().at(i)->GetVertexArray().at(j).m_GlobalTransform.GetRotation().m_W;    
            m_pGlobalRotations[index] = v;

			// local rotations
            v.x = GetHairs().at(i)->GetVertexArray().at(j).m_LocalTransform.GetRotation().m_X;
            v.y = GetHairs().at(i)->GetVertexArray().at(j).m_LocalTransform.GetRotation().m_Y;
            v.z = GetHairs().at(i)->GetVertexArray().at(j).m_LocalTransform.GetRotation().m_Z;
            v.w = GetHairs().at(i)->GetVertexArray().at(j).m_LocalTransform.GetRotation().m_W;
            m_pLocalRotations[index] = v;

				// ref vectors
			v.x = GetHairs().at(i)->GetVertexArray().at(j).m_OrgPosInLocalFrame_i_minus_1.m_X;
            v.y = GetHairs().at(i)->GetVertexArray().at(j).m_OrgPosInLocalFrame_i_minus_1.m_Y;
            v.z = GetHairs().at(i)->GetVertexArray().at(j).m_OrgPosInLocalFrame_i_minus_1.m_Z;
            m_pRefVectors[index].x = v.x;
            m_pRefVectors[index].y = v.y;
            m_pRefVectors[index].z = v.z;

			index++;
        }

		int groupId = GetHairs().at(i)->m_GroupID;
        m_pHairVerticesOffsets[i] = index;
        m_pHairStrandType[i] = groupId;
	}

	// transform the hair as needed
	ScaleRotateTranslate(fScale, vRotation, vTranslation);
}

