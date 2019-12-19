#pragma once
#include "TressFXAsset.h"

class EI_BindSet
{
public:
	//Identical to TressFXBindSet
	int     nSRVs;
	EI_SRV* srvs;
	int     nUAVs;
	EI_UAV* uavs;
	void*   values;
	int     nBytes;
};
