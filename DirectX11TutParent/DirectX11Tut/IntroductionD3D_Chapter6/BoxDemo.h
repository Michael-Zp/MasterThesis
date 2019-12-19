#pragma once

#include <Windows.h>
#include "d3dApp.h"
#include "GeometryGenerator.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Camera.h"
#include "ShapesDemo.h"

class BoxDemo : public D3DApp
{
public:
	BoxDemo(HINSTANCE hInstance);
	~BoxDemo();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();


	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:

	ID3D11Buffer *mIndexBuffer;
	ID3D11Buffer *mVertexBuffer;
	ID3D11Buffer *mConstBuffer;
	VertexShader *mVertexShader;
	PixelShader *mPixelShader;

	UINT mIndexCount = 0;


	float mRadius = 5.0f;
	float mPhi = 0.25f*MathHelper::Pi;
	float mTheta = 1.5f*MathHelper::Pi;

	POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	//BoxDemo app(hInstance);
	ShapesDemo app(hInstance);
	if (!app.Init())
		return 0;



	return app.Run();
}