#pragma once

#ifndef _MY_EI_LAYOUTMANAGER_
#define _MY_EI_LAYOUTMANAGER_


#include "My_EI_Device.h"
#include "AMD_TressFX.h"
#include "My_EI_CommandContext.h"
#include <d3d11.h>
#include <string>
#include <d3dcompiler.h>
#include <Windows.h>
#include <vector>
#include "My_EI_BindLayout.h"




class EI_LayoutManager {
public:
	EI_LayoutManager(EI_Device *device, LPCWSTR filePath, LPCSTR functionName, AMD::EI_ShaderStage stage);
	ID3D10Blob *mShaderBlob;

	struct NameToBind {
		std::string Name;
		UINT BindSlot;
		bool Valid;
	};

	struct ConstNameToBind {
		std::string Name;
		std::string CBufferName;
		UINT BindSlot;
		UINT Offset;
		UINT Size;
		bool Valid;
	};

	struct ConstNameToSize {
		std::string Name;
		UINT Size;
	};

	BindSlot GetBindSlotByName(std::string name);
	ConstBindSlot EI_LayoutManager::GetConstVarByName(std::string bufName, std::string varName);
	UINT GetConstBufferSizeByName(std::string bufName);


	ID3D11VertexShader* GetVertexShader() { return mVertexShader; }
	ID3D11PixelShader* GetPixelShader() { return mPixelShader; }
	ID3D11ComputeShader* GetComputeShader() { return mComputeShader; }

private:
	ID3D11VertexShader *mVertexShader = nullptr;
	ID3D11PixelShader *mPixelShader = nullptr;
	ID3D11ComputeShader *mComputeShader = nullptr;

	std::vector<NameToBind*> mNameToBind;
	std::vector<ConstNameToBind*> mConstNameToBind;
	std::vector<ConstNameToSize*> mConstNameToSize;
};

#endif