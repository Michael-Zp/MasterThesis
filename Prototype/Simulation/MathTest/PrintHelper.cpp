#include "PrintHelper.h"

#include <string>
#include <iostream>
#include <DirectXMath.h>

using namespace DirectX;

void printFloat(std::string label, float out)
{
	std::cout << label << "= \t(" << out << ")" << std::endl;
}

void printFloat(std::string label, XMVECTOR out)
{
	float storedOut;
	XMStoreFloat(&storedOut, out);
	printFloat(label, storedOut);
}

void printXMFLOAT3(std::string label, XMFLOAT3 &out)
{
	std::cout << label << "= \t(" << out.x << "; " << out.y << "; " << out.z << ")" << std::endl;
}

void printXMFLOAT3(std::string label, XMVECTOR &out)
{
	XMFLOAT3 storedOut;
	XMStoreFloat3(&storedOut, out);
	printXMFLOAT3(label, storedOut);
}

void printXMFLOAT4(std::string label, XMFLOAT4 &out)
{
	std::cout << label << "= \t(" << out.x << "; " << out.y << "; " << out.z << "; " << out.w << ")" << std::endl;
}


void printXMFLOAT3X3(std::string label, XMMATRIX &out)
{
	XMFLOAT3X3 storedOut;
	XMStoreFloat3x3(&storedOut, out);
	printXMFLOAT3X3(label, storedOut);
}

void printXMFLOAT3X3(std::string label, XMFLOAT3X3 &out)
{
	std::cout << label << "= \t(" << out._11 << "; " << out._12 << "; " << out._13 << ")" << std::endl;
	std::cout << "\t\t(" << out._21 << "; " << out._22 << "; " << out._23 << ")" << std::endl;
	std::cout << "\t\t(" << out._31 << "; " << out._32 << "; " << out._33 << ")" << std::endl;
}