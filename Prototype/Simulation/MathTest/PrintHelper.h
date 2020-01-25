#pragma once

#include <string>
#include <DirectXMath.h>

using namespace DirectX;

void printFloat(std::string label, float out);
void printFloat(std::string label, XMVECTOR out);

void printXMFLOAT3(std::string label, XMFLOAT3 &out);
void printXMFLOAT3(std::string label, XMVECTOR &out);

void printXMFLOAT4(std::string label, XMFLOAT4 &out);

void printXMFLOAT3X3(std::string label, XMMATRIX &out);
void printXMFLOAT3X3(std::string label, XMFLOAT3X3 &out);