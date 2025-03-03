#pragma once

#include "framework.h"

class TexturedCube
{
public:
	TexturedCube(ID3D11Device* device);
	~TexturedCube();

	void render(ID3D11DeviceContext* context, ID3D11Buffer* sceneBuffer, ID3D11Buffer* geomBuffer, ID3D11SamplerState* samplerState);

private:
	bool initBuffers();
	bool initInputLayout();
	bool initTexture();

	void terminate();

private:
	ID3D11Device* m_pDevice;

	ID3D11Buffer* m_pIndexBuffer;
	ID3D11Buffer* m_pVertexBuffer;

	ID3D11PixelShader* m_pPixelShader;
	ID3D11VertexShader* m_pVertexShader;
	ID3D11InputLayout* m_pInputLayout;

	ID3D11ShaderResourceView* m_pSRV;

	//ID3D11Buffer* m_pGeomBuffer;
};

