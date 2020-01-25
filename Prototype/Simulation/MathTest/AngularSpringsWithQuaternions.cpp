#include "AngularSpringsWithQuaternions.h"

#include <DirectXMath.h>
#include <iostream>
#include <string>
#include <algorithm>
#include "PrintHelper.h"

using namespace DirectX;


XMFLOAT3X3 quatToRotMatrix(XMFLOAT4 q)
{
	//See https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation#Quaternion-derived_rotation_matrix

	XMVECTOR quat = XMLoadFloat4(&q);

	float s = 0;
	XMStoreFloat(&s, XMVector4Length(quat));

	float m00 = 1 - 2 * s * (q.z * q.z + q.w * q.w);
	float m01 = 2 * s * (q.y * q.z - q.w * q.x);
	float m02 = 2 * s * (q.y * q.w + q.z * q.x);
	float m10 = 2 * s * (q.y * q.z + q.w * q.x);
	float m11 = 1 - 2 * s * (q.y * q.y + q.w * q.w);
	float m12 = 2 * s * (q.z * q.w - q.y * q.x);
	float m20 = 2 * s * (q.y * q.w - q.z * q.x);
	float m21 = 2 * s * (q.z * q.w + q.y * q.x);
	float m22 = 1 - 2 * s * (q.y * q.y + q.z * q.z);
	XMFLOAT3X3 ret = XMFLOAT3X3(
		m00, m01, m02,
		m10, m11, m12,
		m20, m21, m22
	);
	return ret;
}


// See https://en.wikipedia.org/wiki/Quaternion#Hamilton_product
XMFLOAT4 hamiltonProduct(XMFLOAT4 p1, XMFLOAT4 p2)
{
	return XMFLOAT4(p1.x * p2.x - p1.y * p2.y - p1.z * p2.z - p1.w * p2.w,
		p1.x * p2.y + p1.y * p2.x + p1.z * p2.w - p1.w * p2.z,
		p1.x * p2.z - p1.y * p2.w + p1.z * p2.x + p1.w * p2.y,
		p1.x * p2.w + p1.y * p2.z - p1.z * p2.y + p1.w * p2.x);
}


// See https://math.stackexchange.com/questions/40164/how-do-you-rotate-a-vector-by-a-unit-quaternion
XMFLOAT4 quaternionRotation(XMFLOAT3 point, XMFLOAT4 quat)
{
	XMFLOAT4 p = XMFLOAT4(0, point.x, point.y, point.z);
	XMFLOAT4 q0 = quat;
	XMFLOAT4 q1 = XMFLOAT4(quat.x, -quat.y, -quat.z, -quat.w);

	return hamiltonProduct(hamiltonProduct(q0, p), q1);
}

XMFLOAT4 quaternionLerp(XMFLOAT4 q0, XMFLOAT4 q1, float f)
{
	XMVECTOR v0 = XMLoadFloat4(&q0);
	XMVECTOR v1 = XMLoadFloat4(&q1);

	f = std::clamp(f, 0.0f, 1.0f);

	XMFLOAT4 ret;
	XMStoreFloat4(&ret, v0 * f + v1 * (1.0f - f));

	return ret;
}

void AngularSpringsWithQuaternions::Start()
{
	XMFLOAT3 left = XMFLOAT3(1, 0, 0);
	XMFLOAT3 up = XMFLOAT3(0, 1, 0);
	XMFLOAT3 upLeft = XMFLOAT3(0.71, 0.71, 0);

	XMFLOAT4 rotation = XMFLOAT4(0, 0, 0, 0);


	printXMFLOAT3("source", left);
	printXMFLOAT3("target", up);

	XMVECTOR segment0 = XMVector3Normalize(XMLoadFloat3(&left));
	XMVECTOR segment1 = XMVector3Normalize(XMLoadFloat3(&up));


	// See http://lolengine.net/blog/2013/09/18/beautiful-maths-quaternion-from-vectors

	XMVECTOR cross = XMVector3Cross(segment0, segment1);
	XMFLOAT3 w;
	XMStoreFloat3(&w, cross);
	float dotProduct;
	XMStoreFloat(&dotProduct, XMVector3Dot(segment0, segment1));
	XMVECTOR quaternion = XMLoadFloat4(&(XMFLOAT4(1 + dotProduct, w.x, w.y, w.z)));
	//XMVECTOR quaternion = XMLoadFloat4(&(XMFLOAT4(dotProduct, w.x, w.y, w.z)));
	//float length;
	//XMStoreFloat(&length, XMVector4Length(quaternion));

	//quaternion += XMLoadFloat4(&(XMFLOAT4(0, 0, 0, length)));
	quaternion = XMVector4Normalize(quaternion);

	XMStoreFloat4(&rotation, quaternion);

	printXMFLOAT4("rotation", rotation);

	XMFLOAT3X3 rotMatrix = quatToRotMatrix(rotation);

	printXMFLOAT3X3("rotMatrix", rotMatrix);

	XMMATRIX rotationMatrix = XMLoadFloat3x3(&rotMatrix);

	XMVECTOR s0Rotated = XMVector3Transform(segment0, XMMatrixTranspose(rotationMatrix));

	XMFLOAT3 rotated;
	XMStoreFloat3(&rotated, s0Rotated);

	printXMFLOAT3("rotated", rotated);

	XMFLOAT4 storedQuat;
	XMStoreFloat4(&storedQuat, quaternion);
	XMFLOAT4 quatRotated = quaternionRotation(left, storedQuat);
	XMFLOAT3 quatRotatedShrink = XMFLOAT3(quatRotated.y, quatRotated.z, quatRotated.w);

	printXMFLOAT3("quatRotated", quatRotatedShrink);


	XMVECTOR halfQuat = XMVector4Normalize(XMLoadFloat4(&storedQuat) / 4);
	XMFLOAT4 storedHalfQuat;
	XMStoreFloat4(&storedHalfQuat, halfQuat);
	XMFLOAT4 quatRotatedHalf = quaternionRotation(left, storedHalfQuat);
	XMFLOAT3 quatRotatedHalfShrink = XMFLOAT3(quatRotatedHalf.y, quatRotatedHalf.z, quatRotatedHalf.w);

	printXMFLOAT3("quatHalfRot", quatRotatedHalfShrink);



	//Test correction of vector direction
	//If there are positions p0, p1 and p2 like this 
	// p0-----p1-----p2
	//the desired position for p1 is p1 - p0 + desiredRelativPosFromP1ToP0 (desired pos is set up in initialization)
	//Now the direction could be just desiredRelativePosFromP1ToP0 - (p1 - p0)
	//But this ignores the fact, that angular springs should rotate something and not accelerate linearly
	//	  desRelPos
	//       /   \
	//      /     \linearDir
	//     /       \ 
	//  (0,0)-------p1
	//
	//Rather it should be
	//	  desRelPos
	//       /       |
	//      /        |angularDir
	//     /         | 
	//  (0,0)-------p1
	//This acceleration is stil linear, but much more realistic, because it actually rotates. The next frame will be slightly more in the direction
	//of the desRelPos, so it should work over time.
	//Now instead of trying to get the dir vector somehow perpendicular to segment0 (p0-p1) just add segment0 with some factor onto the linearDir
	//This factor should be connected to the dot product.
	//So use: 
	//s0 = p1 - p0
	//linearDir = desiredRelativePosFromP1ToP0 - s0
	//angularDir = linearDir + (1 - dot(desRelPos, segment0)) * s0
	//angularDir = normalize(angularDir)
	//Now if dot(angularDir, segment0) == 0 we are golden.

	XMFLOAT3 p0 = XMFLOAT3(0, 0, 0);
	XMFLOAT3 p1 = XMFLOAT3(1, 0, 0);
	XMFLOAT3 desRelPos = XMFLOAT3(-1, 0, 0);

	float one = 1;
	XMVECTOR s0 = XMLoadFloat3(&p1) - XMLoadFloat3(&p0);
	printXMFLOAT3("s0    ", s0);
	XMVECTOR normalizedDesRelPos = XMVector3Normalize(XMLoadFloat3(&desRelPos));
	printXMFLOAT3("normDeRe", normalizedDesRelPos);
	XMVECTOR notNormDesRelPos = XMLoadFloat3(&desRelPos);
	XMVECTOR linearDir = notNormDesRelPos - s0;
	printXMFLOAT3("linearDir", linearDir);
	XMVECTOR dotProdLinDir = XMVector3Dot(notNormDesRelPos, s0);
	printFloat("dotPrLiDir", dotProdLinDir);
	XMVECTOR angularDir = linearDir + (XMLoadFloat(&one) - dotProdLinDir) * s0;
	angularDir = XMVector3Normalize(angularDir);

	XMFLOAT3 storedAngularDir;
	XMStoreFloat3(&storedAngularDir, angularDir);

	float dotProd;
	XMStoreFloat(&dotProd, XMVector3Dot(angularDir, s0));

	printXMFLOAT3("angularDir", storedAngularDir);
	printFloat("dotProd", dotProd);


}

