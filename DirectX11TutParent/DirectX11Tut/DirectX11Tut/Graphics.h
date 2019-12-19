#pragma once
#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_

#include <windows.h>
#include "D3D.h"
#include <fstream>
#include <iostream>
#include "Camera.h"
#include "Model.h"
#include "ColorShader.h"
#include "TextureShader.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class Graphics
{
public:
	Graphics();
	Graphics(const Graphics&);
	~Graphics();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame();

private:
	bool Render();

	D3D* m_Direct3D;
	Camera* m_Camera;
	Model* m_ModelColor;
	Model* m_ModelTexture;
	ColorShader* m_ColorShader;
	TextureShader* m_TextureShader;
};

#endif