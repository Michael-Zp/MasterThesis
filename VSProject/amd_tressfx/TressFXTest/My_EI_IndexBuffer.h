#pragma once
#include "AMD_TressFX.h"
#include <d3d11.h>

class EI_IndexBuffer {
public:
	EI_IndexBuffer(EI_StringHash name) : name(name) {};
	EI_StringHash name;
	ID3D11Buffer *indexBuffer;
};
