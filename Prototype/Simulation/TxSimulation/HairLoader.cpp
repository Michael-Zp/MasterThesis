#include "HairLoader.h"

#include <assert.h>



HairLoader::HairLoader(FILE * ioObject)
{
	TressFXTFXFileHeader header = {};

	// read the header
	fseek(ioObject, 0, SEEK_SET); // make sure the stream pos is at the beginning. 
	fread((void*)&header, sizeof(TressFXTFXFileHeader), 1, ioObject);

	unsigned int numStrandsInFile = header.numHairStrands;

	// We make the number of strands be multiple of TRESSFX_SIM_THREAD_GROUP_SIZE. 
	m_numGuideStrands = (numStrandsInFile - numStrandsInFile % TRESSFX_SIM_THREAD_GROUP_SIZE) + TRESSFX_SIM_THREAD_GROUP_SIZE;

	m_numVerticesPerStrand = header.numVerticesPerStrand;

	// Make sure number of vertices per strand is greater than two and less than or equal to
	// thread group size (64). Also thread group size should be a mulitple of number of
	// vertices per strand. So possible number is 4, 8, 16, 32 and 64.
	assert(m_numVerticesPerStrand > 2 && m_numVerticesPerStrand <= TRESSFX_SIM_THREAD_GROUP_SIZE && TRESSFX_SIM_THREAD_GROUP_SIZE % m_numVerticesPerStrand == 0);

	m_numFollowStrandsPerGuide = 0;
	m_numTotalStrands = m_numGuideStrands; // Until we call GenerateFollowHairs, the number of total strands is equal to the number of guide strands. 
	m_numGuideVertices = m_numGuideStrands * m_numVerticesPerStrand;
	m_numTotalVertices = m_numGuideVertices; // Again, the total number of vertices is equal to the number of guide vertices here. 

	assert(m_numTotalVertices % TRESSFX_SIM_THREAD_GROUP_SIZE == 0); // number of total vertices should be multiple of thread group size. 
																			// This assert is actually redundant because we already made m_numGuideStrands
																			// and m_numTotalStrands are multiple of thread group size. 
																			// Just demonstrating the requirement for number of vertices here in case 
																			// you are to make your own loader. 

	m_positions.resize(m_numTotalVertices); // size of m_positions = number of total vertices * sizeo of each position vector. 

	// Read position data from the io stream. 
	fseek(ioObject, header.offsetVertexPosition, SEEK_SET);
	fread((void*)m_positions.data(), sizeof(PositionData), numStrandsInFile * m_numVerticesPerStrand, ioObject); // note that the position data in io stream contains only guide hairs. If we call GenerateFollowHairs
																									// to generate follow hairs, m_positions will be re-allocated. 

																									// We need to make up some strands to fill up the buffer because the number of strands from stream is not necessarily multile of thread size. 
	int32_t numStrandsToMakeUp = m_numGuideStrands - numStrandsInFile;

	for (int32_t i = 0; i < numStrandsToMakeUp; ++i)
	{
		for (int32_t j = 0; j < m_numVerticesPerStrand; ++j)
		{
			int32_t indexLastVertex = (numStrandsInFile - 1) * m_numVerticesPerStrand + j;
			int32_t indexVertex = (numStrandsInFile + i) * m_numVerticesPerStrand + j;
			m_positions[indexVertex] = m_positions[indexLastVertex];
		}
	}
}