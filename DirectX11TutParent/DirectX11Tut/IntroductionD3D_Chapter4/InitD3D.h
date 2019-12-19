#pragma once
#ifndef INITD3D
#define INITD3D


#include "../../Common/d3dApp.h"

class InitD3D : public D3DApp
{
public:
	InitD3D(HINSTANCE hInstance);
	~InitD3D();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	InitD3D app(hInstance);
	if (!app.Init())
		return 0;

	return app.Run();
}

#endif