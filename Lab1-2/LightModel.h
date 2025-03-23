#pragma once

#include "framework.h"

class LightModel
{
public:
	LightModel(ID3D11Device* device);
	~LightModel();

	void render(ID3D11DeviceContext* context, UINT width, UINT height, ID3D11Buffer* sceneBuffer, ID3D11Buffer* geomBuffer);

private:
	bool initBuffers();
	bool initInputLayout();

	void terminate();

private:
	ID3D11Device* m_pDevice;

	ID3D11Buffer* m_pIndexBuffer;
	ID3D11Buffer* m_pVertexBuffer;

	ID3D11PixelShader* m_pPixelShader;
	ID3D11VertexShader* m_pVertexShader;
	ID3D11InputLayout* m_pInputLayout;
};

