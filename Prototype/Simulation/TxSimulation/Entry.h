#pragma once

#include <Windows.h>
#include "d3dApp.h"
#include "Camera.h"
#include <DirectXMath.h>
#include "IDrawable.h"

using namespace DirectX;

class Entry : public D3DApp
{
public:
	Entry(HINSTANCE hInstance);
	~Entry();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();


	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:

	std::vector<IDrawable*> mRenderItems;

	XMMATRIX mView;
	XMMATRIX mProj;

	float mRadius = 10.0f;
	float mPhi = 0.5f*MathHelper::Pi;
	float mTheta = 1.5f*MathHelper::Pi;

	POINT mLastMousePos;
};

