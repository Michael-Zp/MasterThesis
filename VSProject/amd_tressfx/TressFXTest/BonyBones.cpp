#include "BonyBones.h"
#include <stdint.h>
#include <stdio.h>
#include <fstream>
#include "d3d11.h"


template <typename T>
void ReadSome(std::ifstream &in, T &t) 
{
	static_assert(std::is_same<ReadBufUint32, T>::value || std::is_same<ReadBufFloat, T>::value, "Hey static assert failed. Fuck.");
	in.read(t.buf, sizeof(t.value));
}

void AllocAndRead(std::ifstream &in, char** dest, uint32_t size) 
{
	*dest = new char[size];
	in.read(*dest, size);
}

BonyBones::BonyBones(std::ifstream &in)
{	
	size_t fileSize = in.tellg();
	in.seekg(0, std::ifstream::beg);


	ReadSome(in, mMetaData.numberOfBones);

	

	for(size_t i = 0; i < mMetaData.numberOfBones.value; i++)
	{
		BoneEntry *entry = new BoneEntry();
		ReadSome(in, entry->index);
		ReadSome(in, entry->size);
		AllocAndRead(in, &entry->name, entry->size.value);
		mBones.push_back(entry);
	}

	ReadSome(in, mMetaData.numberOfStrands);


	for(size_t i = 0; i < mMetaData.numberOfStrands.value; i++)
	{
		Strand *strand = new Strand();
		ReadSome(in, strand->index);
		ReadSome(in, strand->joints[0].index);
		ReadSome(in, strand->joints[0].weight);
		ReadSome(in, strand->joints[1].index);
		ReadSome(in, strand->joints[1].weight);
		ReadSome(in, strand->joints[2].index);
		ReadSome(in, strand->joints[2].weight);
		ReadSome(in, strand->joints[3].index);
		ReadSome(in, strand->joints[3].weight);
		mStrands.push_back(strand);
	}
}

BonyBones::~BonyBones()
{
}

unsigned int BonyBones::GetBoneIndexByName(const char * pBoneName) const
{
	for (int i = 0; i < mBones.size(); i++)
	{
		if (strcmp(pBoneName, mBones[i]->name) == 0)
		{
			return i;
		}
	}
	return -1;
}

const char * BonyBones::GetBoneNameByIndex(unsigned int index) const
{
	if (index < mBones.size()) 
	{
		return mBones[index]->name;
	}
	return nullptr;
}

unsigned int BonyBones::GetNumberOfBones() const
{
	return mBones.size();
}
