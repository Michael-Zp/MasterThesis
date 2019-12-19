#pragma once
#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "Input.h"
#include "Graphics.h"

class System
{
public:
	System();
	System(const System& other);
	~System();

	bool Initialize();
	void Run();
	void Shutdown();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();

	LPCSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	Input* m_Input;
	Graphics* m_Graphics;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static System* ApplicationHandle = 0;

#endif
