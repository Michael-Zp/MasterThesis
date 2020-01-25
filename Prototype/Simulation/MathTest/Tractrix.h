#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class Tractrix
{
public:
	static void SimpleTractrixStep(XMFLOAT3&, XMFLOAT3&, XMFLOAT3, bool printEverything);
	static void SimpleTractrix();
	static void LerpTractrix();
};

