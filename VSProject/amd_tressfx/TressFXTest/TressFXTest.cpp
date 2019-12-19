#include <iostream>
#include <stdio.h>
#include "TressFXLayouts.h"
#include "AMD_TressFX.h"
#include "AMD_Types.h"
#include "TressFXAsset.h"
#include "TressFXHairObject.h"
#include "d3d11.h"
#include <Windows.h>
#include <direct.h>
#include <exception>
#include <string>
#include <vector>
#include <DirectXMath.h>
#include "BonyBones.h"
#include <fstream>

#include "My_EI_BindSet.h"
#include "My_EI_CommandContext.h"
#include "My_EI_Device.h"
#include "My_EI_IndexBuffer.h"
#include "My_EI_PSO.h"
#include "My_EI_Resource.h"
#include "My_EI_LayoutManager.h"
#include "DeviceAndContext.h"

#include "My_Callbacks_2D.h"
#include "My_Callbacks_Allocation.h"
#include "My_Callbacks_Barriers.h"
#include "My_Callbacks_BindSets.h"
#include "My_Callbacks_Copy.h"
#include "My_Callbacks_Counter.h"
#include "My_Callbacks_Drawing.h"
#include "My_Callbacks_Error.h"
#include "My_Callbacks_IO.h"
#include "My_Callbacks_Layouts.h"
#include "My_Callbacks_Mapping.h"
#include "My_Callbacks_RT.h"
#include "My_Callbacks_Simulation.h"
#include "My_Callbacks_StructuredBuffers.h"

TressFXLayouts* g_TressFXLayouts = new TressFXLayouts();


void CreateAllLayouts(DeviceAndContext &dac, EI_PSO **pso, HINSTANCE hInstance) 
{
	EI_LayoutManager layoutManagerVS(dac.device, L".\\RenderStrands.hlsl", "VS_RenderHair_AA", AMD::EI_ShaderStage::EI_VS);
	CreateRenderPosTanLayout2(dac.device, layoutManagerVS);
	CreateRenderLayout2(dac.device, layoutManagerVS, false);
	EI_LayoutManager layoutManagerPS(dac.device, L".\\RenderStrands.hlsl", "StrandsPS", AMD::EI_ShaderStage::EI_PS);


	*pso = new EI_PSO(layoutManagerVS, layoutManagerPS, hInstance, dac);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	DeviceAndContext dac;
	//DestroyAllLayouts(pDevice);

	AMD::g_Callbacks.pfMalloc = malloc;
	AMD::g_Callbacks.pfFree = free;

	AMD::g_Callbacks.pfError = myError;

	AMD::g_Callbacks.pfRead = myRead;
	AMD::g_Callbacks.pfSeek = mySeek;

	AMD::g_Callbacks.pfCreateReadOnlySB = myCreateReadOnlySB;
	AMD::g_Callbacks.pfCreateReadWriteSB = myCreateReadWriteSB;

	AMD::g_Callbacks.pfCreateIndexBuffer = myCreateIndexBuffer;
	AMD::g_Callbacks.pfDraw = myDraw;

	AMD::g_Callbacks.pfCreateBindSet = myCreateBindSet;
	AMD::g_Callbacks.pfBind = myBind;

	AMD::g_Callbacks.pfCreateLayout = myCreateLayout;

	AMD::g_Callbacks.pfSubmitBarriers = mySubmitBarriers;
	AMD::g_Callbacks.pfMap = myMap;
	AMD::g_Callbacks.pfUnmap = myUnmap;

	char _buf[1024];
	_getcwd(_buf, sizeof(_buf));
	std::cout << _buf << std::endl;

	std::ifstream *ratboy = new std::ifstream(".\\Assets\\Ratboy\\Ratboy_short.tfx", std::ifstream::ate | std::ifstream::binary);
	std::ifstream *ratboyBones = new std::ifstream(".\\Assets\\Ratboy\\Ratboy_short.tfxbone", std::ifstream::ate | std::ifstream::binary);

	BonyBones bones(*ratboyBones);
	

	AMD::TressFXAsset asset;
	asset.LoadHairData(ratboy);
	//asset.GenerateFollowHairs();
	asset.LoadBoneData(bones, ratboyBones);

	asset.ProcessAsset();

	TressFXHairObject hair;
	hair.Create(&asset, dac.device, dac.GetContext(), "myHair", nullptr);

	EI_PSO *pso;
	CreateAllLayouts(dac, &pso, hInstance);

	ID3D11RasterizerState *rasterState;
	ID3D11DepthStencilState *dsState;
	ID3D11BlendState *blendState;

	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(rasterDesc));
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_BACK;


	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));
	dsDesc.DepthEnable = true;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dsDesc.StencilEnable = false;


	D3D11_RENDER_TARGET_BLEND_DESC blendTargetDesc;
	ZeroMemory(&blendTargetDesc, sizeof(blendTargetDesc));
	blendTargetDesc.BlendEnable = false;
	blendTargetDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.RenderTarget[0] = blendTargetDesc;

	float blendFactors[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	
	dac.device->device->CreateRasterizerState(&rasterDesc, &rasterState);
	dac.device->device->CreateDepthStencilState(&dsDesc, &dsState);
	dac.device->device->CreateBlendState(&blendDesc, &blendState);

	dac.context->context->RSSetState(rasterState);
	dac.context->context->OMSetDepthStencilState(dsState, 0);
	dac.context->context->OMSetBlendState(blendState, blendFactors, 0xffffffff);


	pso->mTimer.Reset();
	while (true) {
		hair.DrawStrands(dac.GetContext(), *pso);
		pso->FetchMessages();
		pso->UpdateConstantBuffers(dac);
	}
}

