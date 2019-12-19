#pragma once
#ifndef _TEXTURECLASS_H_
#define _TEXTURECLASS_H_


#include <d3d11.h>
#include <stdio.h>

class Texture
{
public:
	Texture();
	Texture(Texture&);
	~Texture();

	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, const char*);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture() {
		return m_textureView;
	}

private:
	struct TargaHeader
	{
		unsigned char data1[12];
		unsigned short width;
		unsigned short height;
		unsigned char bpp;
		unsigned char data2;
	};

	bool LoadTarga(const char*, int&, int&); 

	unsigned char* m_targaData;
	ID3D11Texture2D* m_texture;
	ID3D11ShaderResourceView* m_textureView;
};

#endif