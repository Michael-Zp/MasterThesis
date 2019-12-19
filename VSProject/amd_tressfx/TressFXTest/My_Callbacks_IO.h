#pragma once

#include "AMD_TressFX.h"
#include <fstream>

void myRead(void* ptr, AMD::uint size, EI_StreamRef pFile)
{
	std::ifstream *in = reinterpret_cast<std::ifstream*>(pFile);
	in->read((char*)ptr, size);
}

void mySeek(EI_StreamRef pFile, AMD::uint offset)
{
	std::ifstream *in = reinterpret_cast<std::ifstream*>(pFile);
	in->seekg(offset);
}