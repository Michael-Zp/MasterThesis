#pragma once

#include "My_EI_CommandContext.h"
#include "TressFXGPUInterface.h"

void* myMap(EI_CommandContextRef context, EI_StructuredBuffer& sb)
{
	return context.Map(sb);
}

bool myUnmap(EI_CommandContextRef context, EI_StructuredBuffer& sb)
{
	return context.Unmap(sb);
}