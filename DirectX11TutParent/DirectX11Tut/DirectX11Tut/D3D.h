#pragma once
#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <directxmath.h>

class D3D
{
public:
	D3D();
	D3D(const D3D&);
	~D3D();

	bool Initialize(int, int, bool, HWND, bool, float, float);
	void Shutdown();

	void BeginScene(float, float, float, float);
	void EndScene();

	ID3D11Device* GetDevice() {
		return m_device;
	}

	ID3D11DeviceContext* GetDeviceContext() {
		return m_deviceContext;
	}

	void GetProjectionMatrix(DirectX::XMMATRIX& projectionMatrix) {
		projectionMatrix = m_projectionMatrix;
	}

	void GetWorldMatrix(DirectX::XMMATRIX& worldMatrix) {
		worldMatrix = m_worldMatrix;
	}

	void GetOrthoMatrix(DirectX::XMMATRIX& orthoMatrix) {
		orthoMatrix = m_orthoMatrix;
	}

	void GetVideoCardInfo(char* cardName, int& memory) {
		strcpy_s(cardName, 128, m_videoCardDescription);
		memory = m_videoCardMemory;
	}

private:
	bool m_vsync_enabled;
	int m_videoCardMemory;
	char m_videoCardDescription[128];
	IDXGISwapChain* m_swapChain;
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11RasterizerState* m_rasterState;
	DirectX::XMMATRIX m_projectionMatrix;
	DirectX::XMMATRIX m_worldMatrix;
	DirectX::XMMATRIX m_orthoMatrix;
};

#endif