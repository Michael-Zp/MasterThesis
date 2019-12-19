#pragma once
#include "d3d11.h"

class EI_Resource
{
public:
	ID3D11Resource* resource;
	ID3D11ShaderResourceView* srv;
	ID3D11UnorderedAccessView* uav;
};