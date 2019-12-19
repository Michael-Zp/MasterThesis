#pragma once

#include <iostream>
#include <stdio.h>
#include "TressFXBoneSkeletonInterface.h"
#include <vector>

template <typename T>
union ReadBuf {
	char buf[sizeof(T)];
	T value;
};

typedef ReadBuf<uint32_t> ReadBufUint32;
typedef ReadBuf<float> ReadBufFloat;

struct MetaBoneData {
	ReadBufUint32 numberOfBones;
	ReadBufUint32 numberOfStrands;
};

struct BoneEntry {
	ReadBufUint32 index;
	ReadBufUint32 size;
	char* name;
};

struct Joint {
	ReadBufUint32 index;
	ReadBufFloat weight;
};

struct Strand {
	ReadBufUint32 index;
	Joint joints[4];
};

class BonyBones : public TressFXSkeletonInterface
{
public:
	BonyBones(std::ifstream &in);
	~BonyBones();

	// Inherited via TressFXSkeletonInterface
	virtual unsigned int GetBoneIndexByName(const char * pBoneName) const override;
	virtual const char * GetBoneNameByIndex(unsigned int index) const override;
	virtual unsigned int GetNumberOfBones() const override;

private:
	MetaBoneData mMetaData;
	std::vector<BoneEntry*> mBones;
	std::vector<Strand*> mStrands;

};

