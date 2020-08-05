#pragma once

#include <iostream>
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

class HairLoader
{
public:
	int TRESSFX_SIM_THREAD_GROUP_SIZE = 64;

	HairLoader(FILE * ioObject);

	// TressFXTFXFileHeader Structure
	//
	// This structure defines the header of the file. The actual vertex data follows this as specified by the offsets.
	struct TressFXTFXFileHeader
	{
		float version;                      // Specifies TressFX version number
		unsigned int numHairStrands;        // Number of hair strands in this file. All strands in this file are guide strands.
											// Follow hair strands are generated procedurally.
		unsigned int numVerticesPerStrand;  // From 4 to 64 inclusive (POW2 only). This should be a fixed value within tfx value. 
											// The total vertices from the tfx file is numHairStrands * numVerticesPerStrand.

		// Offsets to array data starts here. Offset values are in bytes, aligned on 8 bytes boundaries,
		// and relative to beginning of the .tfx file
		unsigned int offsetVertexPosition;  // Array size: FLOAT4[numHairStrands]
		unsigned int offsetStrandUV;         // Array size: FLOAT2[numHairStrands], if 0 no texture coordinates
		unsigned int offsetVertexUV;         // Array size: FLOAT2[numHairStrands * numVerticesPerStrand], if 0, no per vertex texture coordinates
		unsigned int offsetStrandThickness;  // Array size: float[numHairStrands]
		unsigned int offsetVertexColor;      // Array size: FLOAT4[numHairStrands * numVerticesPerStrand], if 0, no vertex colors

		unsigned int reserved[32];           // Reserved for future versions
	};

	int m_numGuideStrands;
	int m_numVerticesPerStrand;
	int m_numFollowStrandsPerGuide;
	int m_numTotalStrands;
	int m_numGuideVertices;
	int m_numTotalVertices;

	struct PositionData
	{
		XMFLOAT3 Position;
		float IsMovable;
	};

	std::vector<PositionData> m_positions;
};

