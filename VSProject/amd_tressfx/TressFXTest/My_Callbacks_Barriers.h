#pragma once

#include "My_EI_CommandContext.h"

void mySubmitBarriers(EI_CommandContextRef commands, int numBarriers, AMD::EI_Barrier* barriers)
{
	//Only relevant for Vulkan / DX12
	//I use DX11 so it is blank for now
}