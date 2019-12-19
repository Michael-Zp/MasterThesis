#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

namespace MeshGenerator
{
	void CreateBox(XMFLOAT3 size, XMFLOAT3 **positions, UINT **indices, UINT *indexCount) 
	{
		*positions = new XMFLOAT3[8];

		int zShananigans = 1;

		(*positions)[0] = XMFLOAT3(-size.x, -size.y, -size.z + zShananigans);
		(*positions)[1] = XMFLOAT3(-size.x, +size.y, -size.z + zShananigans);
		(*positions)[2] = XMFLOAT3(+size.x, +size.y, -size.z + zShananigans);
		(*positions)[3] = XMFLOAT3(+size.x, -size.y, -size.z + zShananigans);
		(*positions)[4] = XMFLOAT3(-size.x, -size.y, +size.z + zShananigans);
		(*positions)[5] = XMFLOAT3(-size.x, +size.y, +size.z + zShananigans);
		(*positions)[6] = XMFLOAT3(+size.x, +size.y, +size.z + zShananigans);
		(*positions)[7] = XMFLOAT3(+size.x, -size.y, +size.z + zShananigans);


		//6 sides * 2 triangles * 3 points
		*indexCount = 6 * 2 * 3;

		*indices = new UINT[*indexCount]{
			// front face
			0, 1, 2,
			0, 2, 3,
			// back face
			4, 6, 5,
			4, 7, 6,
			// left face
			4, 5, 1,
			4, 1, 0,
			// right face
			3, 2, 6,
			3, 6, 7,
			// top face
			1, 5, 6,
			1, 6, 2,
			// bottom face
			4, 0, 3,
			4, 3, 7
		};

	}
}