#include "InitD3D.h"



InitD3D::InitD3D(HINSTANCE hInstance) : D3DApp(hInstance)
{}

InitD3D::~InitD3D()
{}

bool InitD3D::Init()
{
	if (!D3DApp::Init())
	{
		return false;
	}

	return true;
}

void InitD3D::OnResize()
{
	D3DApp::OnResize();
}

void InitD3D::UpdateScene(float dt)
{

}

void InitD3D::DrawScene()
{
	assert(md3dImmediateContext);
	assert(mSwapChain);

	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView,
		reinterpret_cast<const float*>(&Colors::Blue));

	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	HR(mSwapChain->Present(0, 0));
}

