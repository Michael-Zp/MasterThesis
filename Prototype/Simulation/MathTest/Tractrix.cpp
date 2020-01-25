#include "Tractrix.h"

#include "PrintHelper.h"
#include <DirectXMath.h>
#include <iostream>

struct Particle
{
	XMFLOAT3 Position;
};

const static int MAXIMUM_NUMBER_OF_PARTICLES = 16;

struct Strand
{
	int NumberOfParticles;
	int StrandIdx;
	XMFLOAT3 DesiredHeadPosition;
	Particle Particles[MAXIMUM_NUMBER_OF_PARTICLES];
};


float sechInv(float z)
{
	//http://mathworld.wolfram.com/InverseHyperbolicSecant.html
	//return log(sqrt(1 / z - 1) * sqrt(1 / z + 1) + 1 / z);
//But this seems to be for a complex number
//thus
//https://en.wikipedia.org/wiki/Inverse_hyperbolic_functions
	return log((1 + sqrt(1 - (z * z))) / z);
}

float sech(float z)
{
	//http://mathworld.wolfram.com/HyperbolicSecant.html
	//this seems to be identical with https://en.wikipedia.org/wiki/Hyperbolic_function
	return 1 / cosh(z);
}


XMFLOAT3 NewTailPosByIndex(float L, float y, float lengthS, XMFLOAT3 S, XMFLOAT3 X, XMFLOAT3 Xh, XMFLOAT3 Xp, XMMATRIX R, bool printEverything)
{

	float p = L * sechInv(y / L);
	float p_p = p + lengthS;
	float p_n = p - lengthS;
	if (printEverything)
	{
		printFloat("p_p", p_p);
		printFloat("p_n", p_n);
	}

	float xr = -L * tanh(p_p / L);
	float xr_p = lengthS + xr;
	float xr_n = lengthS - xr;
	if (printEverything)
	{
		printFloat("xr_p", xr_p);
		printFloat("xr_n", xr_n);
	}

	float xr_np = -L * tanh(p_n / L);
	float xr_np_p = lengthS + xr_np;
	float xr_np_n = lengthS - xr_np;
	if (printEverything)
	{
		printFloat("xr_np_p", xr_np_p);
		printFloat("xr_np_n", xr_np_n);
	}


	float yr_p = L * sech(p_p / L);
	float yr_n = L * sech(p_n / L);
	if (printEverything)
	{
		printFloat("yr_p", yr_p);
		printFloat("yr_n", yr_n);
	}



	XMVECTOR newPos[8];
	newPos[0] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_p, yr_p, 0)), R);
	newPos[1] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_p, yr_n, 0)), R);
	newPos[2] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_n, yr_p, 0)), R);
	newPos[3] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_n, yr_n, 0)), R);
	newPos[4] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_np_p, yr_p, 0)), R);
	newPos[5] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_np_p, yr_n, 0)), R);
	newPos[6] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_np_n, yr_p, 0)), R);
	newPos[7] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_np_n, yr_n, 0)), R);

	XMFLOAT3 storedNewPos[8];
	for (int i = 0; i < 8; i++)
	{
		XMStoreFloat3(&(storedNewPos[i]), newPos[i]);
	}

	//float distance;
	int indexOfSmallest = 0;
	//XMStoreFloat(&distance, XMVector3LengthSq(newPos[indexOfSmallest] - X)); //X == TailPos in XMVECTOR
	//for (int i = 1; i < 8; i++)
	//{
	//	float tempDist;
	//	XMStoreFloat(&tempDist, XMVector3LengthSq(newPos[i] - X));
	//	if (tempDist < distance)
	//	{
	//		indexOfSmallest = i;
	//	}
	//}

	//XMStoreFloat3(&TailPos, newPos[indexOfSmallest]);

	XMFLOAT3 newTailPos;
	XMStoreFloat3(&newTailPos, newPos[0]);


	printXMFLOAT3("newTailPos" + std::to_string(indexOfSmallest), newTailPos);

	//printXMFLOAT3("newHeadPos", HeadPos);


	return newTailPos;
}

template <typename T> float sign(T val, float defaultValue) {
	if (T(0) < val)
	{
		return 1;
	}
	else if (val < T(0))
	{
		return -1;
	}
	return defaultValue;
}

float signbitSign(float val)
{
	return (signbit(val) ? -1 : 1);
}


XMFLOAT3 NewTailPosByMafsCleaned(float L, float y, float lengthS, XMFLOAT3 S, XMFLOAT3 X, XMFLOAT3 Xh, XMFLOAT3 Xp, XMMATRIX R, bool printEverything)
{
	float p_p = L * sechInv(y / L) + lengthS;
	float p_n = L * sechInv(y / L) - lengthS;

	float xr_p = +lengthS - L * tanh(p_p / L);
	float xr_n = -lengthS - L * tanh(p_p / L);

	float xr_np_p = +lengthS - L * tanh(p_n / L);
	float xr_np_n = -lengthS - L * tanh(p_n / L);

	float yr_p = L * sech(p_p / L);
	float yr_n = L * sech(p_n / L);

	//Approach based on signs of input

	XMFLOAT3 tempPos = XMFLOAT3(xr_p, yr_p, 0);


	XMVECTOR lengthXpX = XMVector3Length(XMLoadFloat3(&Xp) - XMLoadFloat3(&X));
	XMVECTOR lengthXhX = XMVector3Length(XMLoadFloat3(&Xh) - XMLoadFloat3(&X));
	float storedLengthXpX;
	float storedLengthXhX;
	XMStoreFloat(&storedLengthXpX, lengthXpX);
	XMStoreFloat(&storedLengthXhX, lengthXhX);




	if (storedLengthXpX > storedLengthXhX)
	{
		tempPos = XMFLOAT3(xr_p, yr_p, 0);
	}
	else
	{
		tempPos = XMFLOAT3(-xr_np_n, yr_n, 0);
	}

	if (printEverything)
	{
		printXMFLOAT3("tempPos", tempPos);
	}

	XMVECTOR calculatedPos = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&tempPos), R);

	XMFLOAT3 newTailPos;
	printXMFLOAT3("calcPos", calculatedPos);
	XMStoreFloat3(&newTailPos, calculatedPos);

	return newTailPos;
}

XMFLOAT3 NewTailPosByMafs(float L, float y, float lengthS, XMFLOAT3 S, XMFLOAT3 X, XMFLOAT3 Xh, XMFLOAT3 Xp, XMMATRIX R, bool printEverything)
{
	float p_p = L * sechInv(y / L) + lengthS;
	float p_n = L * sechInv(y / L) - lengthS;

	float xr_p = + lengthS - L * tanh(p_p / L);
	float xr_n = - lengthS - L * tanh(p_p / L);

	float xr_np_p = + lengthS - L * tanh(p_n / L);
	float xr_np_n = - lengthS - L * tanh(p_n / L);

	float yr_p = L * sech(p_p / L);
	float yr_n = L * sech(p_n / L);

	//Approach based on signs of input

	XMFLOAT3 tempPos = XMFLOAT3(xr_p, yr_p, 0);

	XMVECTOR det = XMMatrixDeterminant(R);
	XMVECTOR SInPlane = XMVector3Transform(XMLoadFloat3(&S), R);
	XMFLOAT3 storedSInPlane;
	XMStoreFloat3(&storedSInPlane, SInPlane);

	
	XMFLOAT3X3 storedR;
	XMStoreFloat3x3(&storedR, R);

	XMMATRIX absR = XMLoadFloat3x3(&XMFLOAT3X3(abs(storedR._11), abs(storedR._12), abs(storedR._13), abs(storedR._21), abs(storedR._22), abs(storedR._23), abs(storedR._31), abs(storedR._32), abs(storedR._33)));
	XMVECTOR absSInPlane = XMVector3Transform(XMLoadFloat3(&S), absR);
	XMFLOAT3 storedAbsSInPlane;
	XMStoreFloat3(&storedAbsSInPlane, absSInPlane);

	XMVECTOR lengthXpX = XMVector3Length(XMLoadFloat3(&Xp) - XMLoadFloat3(&X));
	XMVECTOR lengthXhX = XMVector3Length(XMLoadFloat3(&Xh) - XMLoadFloat3(&X));
	float storedLengthXpX;
	float storedLengthXhX;
	XMStoreFloat(&storedLengthXpX, lengthXpX);
	XMStoreFloat(&storedLengthXhX, lengthXhX);


	float signWorldX = sign(sign(storedR._11 + storedR._21 + storedR._31, 1) * storedSInPlane.x, 1);
	float signWorldY = sign(sign(storedR._12 + storedR._22 + storedR._32, 1) * storedSInPlane.y, 1);
	float signWorldZ = sign(sign(storedR._13 + storedR._23 + storedR._33, 1) * storedSInPlane.z, 1);

	float mWorld1 = (sign(storedR._12 + storedR._22 + storedR._32, 1) * storedSInPlane.y) / (sign(storedR._11 + storedR._21 + storedR._31, 1) * storedSInPlane.x);
	float mWorld2 = (sign(storedR._13 + storedR._23 + storedR._33, 1) * storedSInPlane.z) / (sign(storedR._11 + storedR._21 + storedR._31, 1) * storedSInPlane.x);
	mWorld1 = signWorldY / signWorldX;
	mWorld2 = signWorldZ / signWorldX;
	float m1 = storedSInPlane.y / storedSInPlane.x;
	float m2 = storedSInPlane.z / storedSInPlane.x;


	float signMWorld1 = signbitSign(mWorld1);
	float signMWorld2 = signbitSign(mWorld2);
	float signMWorld12 = signMWorld1 * signMWorld2;

	float signM1 = signbitSign(m1);
	float signM2 = signbitSign(m2);
	float signM12 = signM1 * signM2;

	if (printEverything)
	{
		printXMFLOAT3("SInPlane", SInPlane);
		printFloat("m1", m1);
		printFloat("m2", m2);
		printFloat("signWorldX", signWorldX);
		printFloat("signWorldY", signWorldY);
		printFloat("signWorldZ", signWorldZ);
		printFloat("signMWorld1", signMWorld1);
		printFloat("signMWorld2", signMWorld2);
		printFloat("signM1", signM1);
		printFloat("signM2", signM2);

		printFloat("absSInP", storedAbsSInPlane.x);
	}


	//float p = (L * sechInv(y / L) + (signM12 * signM12 * lengthS));
	//float p = (L * sechInv(y / L) + (signMWorld12 * lengthS));

	//if (printEverything)
	//{
	//	printFloat("pBefore", p);
	//	float invP = -(L * sechInv(y / L) - (signMWorld12 * lengthS));
	//	printFloat("pInv", invP);
	//}

	//if ((p <= 1 && signMWorld1 < 0 && signMWorld2 < 0) || (p > 1 && signMWorld1 > 0 && signMWorld2 > 0))
	//{
	//	//p = -(L * sechInv(y / L) - (signMWorld12 * lengthS));
	//}

	//if (usePInverse)
	//{
	//	p = -(L * sechInv(y / L) - (signMWorld12 * lengthS));
	//}
	//else
	//{
	//	p = (L * sechInv(y / L) + (signMWorld12 * lengthS));
	//}

	//if (storedAbsSInPlane.x > 0)
	//{
	//	tempPos = XMFLOAT3(xr_p, yr_p, 0);
	//}
	//else
	//{
	//	tempPos = XMFLOAT3(-xr_np_n, yr_n, 0);
	//}


	if (storedLengthXpX > storedLengthXhX)
	{
		tempPos = XMFLOAT3(xr_p, yr_p, 0);
	}
	else
	{
		tempPos = XMFLOAT3(-xr_np_n, yr_n, 0);
	}



	//tempPos.x = (signM12 * lengthS) - L * tanh(p / L);
	//tempPos.x = (signMWorld12 * lengthS) - L * tanh(p / L);
	//tempPos.x *= signM12;
	//tempPos.y = L * sech(p / L);


	//if (printEverything)
	//{
	//	printFloat("p", p);
	//}

	//tempPos.x = (signM2 * lengthS) - L * tanh(p / L);
	//tempPos.y = L * sech(p / L);

	if (printEverything)
	{
		printXMFLOAT3("tempPos", tempPos);
	}
	
	XMVECTOR newPos[10];
	newPos[0] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_p, yr_p, 0)), R);
	newPos[1] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_p, yr_n, 0)), R);
	newPos[2] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_n, yr_p, 0)), R);
	newPos[3] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_n, yr_n, 0)), R);
	newPos[4] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_np_p, yr_p, 0)), R);
	newPos[5] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_np_p, yr_n, 0)), R);
	newPos[6] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_np_n, yr_p, 0)), R);
	newPos[7] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_np_n, yr_n, 0)), R);
	newPos[8] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(-xr_np_n, yr_p, 0)), R);
	newPos[9] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(-xr_np_n, yr_n, 0)), R);

	//newPos[0] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_p, yr_p, 0)), XMMatrixInverse(&det, R));
	//newPos[1] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_p, yr_n, 0)), XMMatrixInverse(&det, R));
	//newPos[2] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_n, yr_p, 0)), XMMatrixInverse(&det, R));
	//newPos[3] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_n, yr_n, 0)), XMMatrixInverse(&det, R));
	//newPos[4] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_np_p, yr_p, 0)), XMMatrixInverse(&det, R));
	//newPos[5] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_np_p, yr_n, 0)), XMMatrixInverse(&det, R));
	//newPos[6] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_np_n, yr_p, 0)), XMMatrixInverse(&det, R));
	//newPos[7] = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&XMFLOAT3(xr_np_n, yr_n, 0)), XMMatrixInverse(&det, R));

	XMVECTOR calculatedPos = XMLoadFloat3(&Xh) + XMVector3Transform(XMLoadFloat3(&tempPos), R);

	XMFLOAT3 newTailPos;
	printXMFLOAT3("calcPos", calculatedPos);
	XMStoreFloat3(&newTailPos, calculatedPos);

	return newTailPos;
}

void Tractrix::SimpleTractrixStep(XMFLOAT3 &HeadPos, XMFLOAT3 &TailPos, XMFLOAT3 DesiredHeadPos, bool printEverything)
{
	XMVECTOR X = XMLoadFloat3(&TailPos);
	XMVECTOR Xh = XMLoadFloat3(&HeadPos);
	XMVECTOR Xp = XMLoadFloat3(&DesiredHeadPos);

	XMFLOAT3 storedX;
	XMFLOAT3 storedXh;
	XMFLOAT3 storedXp;
	XMStoreFloat3(&storedX, X);
	XMStoreFloat3(&storedXh, Xh);
	XMStoreFloat3(&storedXp, Xp);

	if (printEverything)
	{
		printXMFLOAT3("X", X);
		printXMFLOAT3("Xh", Xh);
		printXMFLOAT3("Xp", Xp);
	}



	XMVECTOR S = Xp - Xh;
	if (printEverything)
	{
		printXMFLOAT3("S", S);
	}
	XMVECTOR lengthS = XMVector3Length(S);
	float storedLengthS;
	XMStoreFloat(&storedLengthS, lengthS);
	XMFLOAT3 storedS;
	XMStoreFloat3(&storedS, S);



	if (printEverything)
	{
		printFloat("lengthS", lengthS);
	}

	if (storedLengthS < 0.01)
	{
		printXMFLOAT3("newP = oldP", TailPos);
		return;
	}

	XMVECTOR L = XMVector3Length(X - Xh);
	float storedL;
	XMStoreFloat(&storedL, L);

	if (printEverything)
	{
		printFloat("L", storedL);
	}



	XMVECTOR T = X - Xh;
	if (printEverything)
	{
		printXMFLOAT3("T", T);
	}

	XMVECTOR Xr = XMVector3Normalize(S);

	XMVECTOR Zr = XMVector3Normalize(XMVector3Cross(S, T));

	XMVECTOR Yr = XMVector3Cross(Zr, Xr);
	

	XMFLOAT3 storedXr;
	XMFLOAT3 storedYr;
	XMFLOAT3 storedZr;
	XMStoreFloat3(&storedXr, Xr);
	XMStoreFloat3(&storedYr, Yr);
	XMStoreFloat3(&storedZr, Zr);



	if (printEverything)
	{
		printXMFLOAT3("Xr", Xr);
		printXMFLOAT3("Yr", Yr);
		printXMFLOAT3("Zr", Zr);
	}


	storedZr.x *= -1;
	storedZr.y *= -1;
	storedZr.z *= -1;

	XMMATRIX R = XMLoadFloat3x3(&XMFLOAT3X3(storedXr.x, storedYr.x, storedZr.x, storedXr.y, storedYr.y, storedZr.y, storedXr.z, storedYr.z, storedZr.z));
	if (printEverything)
	{
		printXMFLOAT3X3("rotMatrix", R);
	}

	

	XMVECTOR y = XMVector3Dot(Yr, T);
	if (printEverything)
	{
		printFloat("y", y);
	}
	float storedY;
	XMStoreFloat(&storedY, y);




	//TailPos = NewTailPosByIndex(storedL, storedY, storedLengthS, storedS, storedX, storedXh, storedXp, R, printEverything);
	//TailPos = NewTailPosByMafs(storedL, storedY, storedLengthS, storedS, storedX, storedXh, storedXp, R, printEverything);
	TailPos = NewTailPosByMafsCleaned(storedL, storedY, storedLengthS, storedS, storedX, storedXh, storedXp, R, printEverything);

	HeadPos = DesiredHeadPos;

	if (printEverything)
	{
		std::cout << std::endl;
	}
}

void Tractrix::SimpleTractrix()
{
	std::cout << "Simple Tractrix" << std::endl;

	Strand strand;
	strand.NumberOfParticles = 3;
	strand.StrandIdx = 0;
	strand.DesiredHeadPosition = XMFLOAT3(1, -1.25, 0);
	strand.Particles[0].Position = XMFLOAT3(0, 1.25, 0);
	strand.Particles[1].Position = XMFLOAT3(0, 0, 0);
	strand.Particles[2].Position = XMFLOAT3(0, -1.25, 0);

	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, strand.DesiredHeadPosition, true);

	std::cout << std::endl;
}


void Tractrix::LerpTractrix()
{
	std::cout << "Lerp Tractrix" << std::endl;

	Strand strand;
	strand.NumberOfParticles = 3;
	strand.StrandIdx = 0;
	strand.DesiredHeadPosition = XMFLOAT3(1, -1.25, 0);
	strand.Particles[0].Position = XMFLOAT3(0, 1.25, 0);
	strand.Particles[1].Position = XMFLOAT3(0, 0, 0);
	strand.Particles[2].Position = XMFLOAT3(0, -1.25, 0);


	std::cout << "Manual steps:" << std::endl;

	std::cout << "###X-Dir###" << std::endl;

	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0.25, -1.25, 0), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0.5, -1.25, 0), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0.75, -1.25, 0), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(1, -1.25, 0), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0.75, -1.25, 0), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0.5, -1.25, 0), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0.25, -1.25, 0), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1.25, 0), false);

	std::cout << "###X-Dir-neg###" << std::endl;
	strand.Particles[0].Position = XMFLOAT3(0, 1.25, 0);
	strand.Particles[1].Position = XMFLOAT3(0, 0, 0);
	strand.Particles[2].Position = XMFLOAT3(0, -1.25, 0);

	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(-0.25, -1.25, 0), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(-0.5, -1.25, 0), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(-0.75, -1.25, 0), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(-1, -1.25, 0), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(-0.75, -1.25, 0), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(-0.5, -1.25, 0), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(-0.25, -1.25, 0), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1.25, 0), false);

	std::cout << "###Z-Dir###" << std::endl;

	strand.Particles[0].Position = XMFLOAT3(0, 1.25, 0);
	strand.Particles[1].Position = XMFLOAT3(0, 0, 0);
	strand.Particles[2].Position = XMFLOAT3(0, -1.25, 0);

	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1.25, 0.25), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1.25, 0.5), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1.25, 0.75), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1.25, 1), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1.25, 0.75), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1.25, 0.5), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1.25, 0.25), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1.25, 0), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1.25, -0.25), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1.25, -0.5), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1.25, -0.75), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1.25, -1), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1.25, -0.75), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1.25, -0.5), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1.25, -0.25), false);
	SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1.25, 0), false);

	//std::cout << "Y-Dir" << std::endl;

	//strand.Particles[0].Position = XMFLOAT3(0, 1.25, 0);
	//strand.Particles[1].Position = XMFLOAT3(0, 0, 0);
	//strand.Particles[2].Position = XMFLOAT3(0, -1.25, 0);

	//SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1.25, 0.1), false);
	//SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1, 0.2), false);
	//SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -.75, 0.3), false);
	//SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -.5, 0.4), false);
	//SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -.75, 0.3), false);
	//SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1, 0.2), false);
	//SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(0, -1.25, 0.1), false);

	std::cout << "Continuous" << std::endl;

	strand.Particles[0].Position = XMFLOAT3(0, 1.25, 0);
	strand.Particles[1].Position = XMFLOAT3(0, 0, 0);
	strand.Particles[2].Position = XMFLOAT3(0, -1.25, 0);


	float range = XM_PI * 20;
	int steps = 0;

	for (int i = 0; i < steps; i++)
	{
		if (i > 70)
		{
			//TEMP FOR DEBUGGING
			std::cout << "";
		}
		float targetX = sin(range * ((float)i / steps));
		float targetZ = sin(range * ((float)i / steps));
		targetZ = 0;
		SimpleTractrixStep(strand.Particles[2].Position, strand.Particles[1].Position, XMFLOAT3(targetX, -1.25, targetZ), false);

	}



	std::cout << std::endl;
}
