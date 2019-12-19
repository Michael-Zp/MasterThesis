#include "My_EI_LayoutManager.h"

#include "Utils.h"

EI_LayoutManager::EI_LayoutManager(EI_Device *device, LPCWSTR filePath, LPCSTR functionName, AMD::EI_ShaderStage stage)
{
	UINT flags1 = 0;
#define DEBUG
#ifdef DEBUG
	flags1 = (D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION);
#endif
#undef DEBUG
	ID3D10Blob *errorMsgs;

	LPCSTR target;

	switch (stage)
	{
		case AMD::EI_ShaderStage::EI_CS:
			target = "cs_5_0";
			break;
		case AMD::EI_ShaderStage::EI_PS:
			target = "ps_5_0";
			break;
		case AMD::EI_ShaderStage::EI_VS:
			target = "vs_5_0";
			break;
		default:
			target = "";
			DebugBreak();
	}


	HR(D3DCompileFromFile(filePath, NULL, NULL, functionName, target, flags1, 0, &mShaderBlob, &errorMsgs));

	if (errorMsgs != 0)
	{
		MessageBoxA(0, (char*)errorMsgs->GetBufferPointer(), 0, 0);
		if (errorMsgs) 
		{
			errorMsgs->Release();
		}
	}

	switch (stage)
	{
		case AMD::EI_ShaderStage::EI_CS:
			device->device->CreateComputeShader(mShaderBlob->GetBufferPointer(), mShaderBlob->GetBufferSize(), NULL, &mComputeShader);
			break;
		case AMD::EI_ShaderStage::EI_PS:
			device->device->CreatePixelShader(mShaderBlob->GetBufferPointer(), mShaderBlob->GetBufferSize(), NULL, &mPixelShader);
			break;
		case AMD::EI_ShaderStage::EI_VS:
			device->device->CreateVertexShader(mShaderBlob->GetBufferPointer(), mShaderBlob->GetBufferSize(), NULL, &mVertexShader);
			break;
		default:
			target = "";
			DebugBreak();
	}

	ID3D11ShaderReflection *reflector;
	D3DReflect(mShaderBlob->GetBufferPointer(), mShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflector);

	D3D11_SHADER_DESC shaderDesc;
	reflector->GetDesc(&shaderDesc);


	for (int i = 0; i < shaderDesc.BoundResources; i++)
	{
		D3D11_SHADER_INPUT_BIND_DESC shadInBindDesc;
		ZeroMemory(&shadInBindDesc, sizeof(shadInBindDesc));
		reflector->GetResourceBindingDesc(i, &shadInBindDesc);
	
		if (shadInBindDesc.Type == D3D_SIT_CBUFFER)
		{
			D3D11_SHADER_BUFFER_DESC shadBufDesc;
			ZeroMemory(&shadBufDesc, sizeof(shadBufDesc));

			ID3D11ShaderReflectionConstantBuffer *cb = reflector->GetConstantBufferByIndex(shadInBindDesc.BindPoint);
			cb->GetDesc(&shadBufDesc);

			mConstNameToSize.push_back(new ConstNameToSize{ shadBufDesc.Name, shadBufDesc.Size });

			for (int k = 0; k < shadBufDesc.Variables; k++)
			{
				D3D11_SHADER_VARIABLE_DESC shadVarDesc;
				ID3D11ShaderReflectionVariable *srv = cb->GetVariableByIndex(k);
				srv->GetDesc(&shadVarDesc);

				mConstNameToBind.push_back(new ConstNameToBind{
					shadVarDesc.Name,
					shadBufDesc.Name,
					shadInBindDesc.BindPoint,
					shadVarDesc.StartOffset,
					shadVarDesc.Size,
					true
					});
			}


			mNameToBind.push_back(new NameToBind{ shadBufDesc.Name, shadInBindDesc.BindPoint, true });
		}
		else
		{
			D3D11_SHADER_INPUT_BIND_DESC shadInBindDesc;
			ZeroMemory(&shadInBindDesc, sizeof(shadInBindDesc));

			reflector->GetResourceBindingDesc(i, &shadInBindDesc);

			mNameToBind.push_back(new NameToBind{ shadInBindDesc.Name, shadInBindDesc.BindPoint, true });
		}
	}

	
	//ID3D11ShaderReflectionConstantBuffer *cb = reflector->GetConstantBufferByName("Camera");
	//ID3D11ShaderReflectionConstantBuffer *cb2 = reflector->GetConstantBufferByName("Camera2");
	//D3D11_SHADER_BUFFER_DESC shadBufDesc;
	//cb->GetDesc(&shadBufDesc);
	//D3D11_SHADER_BUFFER_DESC shadBufDesc2;
	//cb->GetDesc(&shadBufDesc2);
}

BindSlot EI_LayoutManager::GetBindSlotByName(std::string name)
{
	for (int i = 0; i < mNameToBind.size(); i++)
	{
		if (name == mNameToBind[i]->Name) {
			return { mNameToBind[i]->BindSlot, true };
		}
	}

	return { (UINT)-1, false };
}


ConstBindSlot EI_LayoutManager::GetConstVarByName(std::string bufName, std::string varName)
{
	for (int i = 0; i < mConstNameToBind.size(); i++)
	{
		if (bufName == mConstNameToBind[i]->CBufferName && varName == mConstNameToBind[i]->Name) {
			return { mConstNameToBind[i]->BindSlot, mConstNameToBind[i]->Offset, mConstNameToBind[i]->Size, true };
		}
	}

	return { (UINT)-1, 0, 0, false };
}


UINT EI_LayoutManager::GetConstBufferSizeByName(std::string bufName)
{
	for (int i = 0; i < mConstNameToSize.size(); i++)
	{
		if (bufName == mConstNameToSize[i]->Name) {
			return mConstNameToSize[i]->Size;
		}
	}

	return 0;
}