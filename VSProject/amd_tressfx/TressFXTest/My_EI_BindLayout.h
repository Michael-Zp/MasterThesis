#pragma once

#include "AMD_TressFX.h"
#include "My_EI_Resource.h"


struct BindSlot
{
	UINT Value;
	bool Valid;
};


struct ConstBindSlot
{
	UINT Value;
	UINT Offset;
	UINT Size;
	bool Valid;
};

struct EI_BindLayout 
{
	std::vector<BindSlot> SrvBindSlots;
	std::vector<BindSlot> UavBindSlots;

	BindSlot ConstantBindSlot;
	std::vector<ConstBindSlot> ConstVarOffsets;
	EI_Resource *ConstantBuffer;

	ID3D11VertexShader *VertexShader;
	ID3D11PixelShader *PixelShader;
	ID3D11ComputeShader *ComputeShader;

	AMD::EI_ShaderStage Stage;
};