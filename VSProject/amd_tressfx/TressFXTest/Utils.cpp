#include "Utils.h"
#include <windows.h>
#include <string>

void HR(HRESULT x)
{
	HRESULT hr = (x);
	if (FAILED(hr))
	{
		std::string text(std::string("Error in file ") + std::string(__FILE__) + std::string(" at line ") + std::to_string(__LINE__));
		std::string s = std::system_category().message(hr);
		text += std::string("\n\n") + s;
		int len;
		int slength = (int)text.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, text.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, text.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		MessageBoxW(GetForegroundWindow(), r.c_str(), L"Unexpected Error", MB_YESNO | MB_ICONERROR);
		DebugBreak();
	}
}