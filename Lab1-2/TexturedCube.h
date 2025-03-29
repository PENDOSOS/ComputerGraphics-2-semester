#pragma once

#include "framework.h"

#define MAX_INST 20

struct GeomBufferInst
{
	DirectX::XMMATRIX M;
	DirectX::XMMATRIX NormalM;
	DirectX::XMFLOAT4 params; // x - shininess, y - use normal map, z - texture index, w - is cube visible
};

class TexturedCube
{
public:
	TexturedCube(ID3D11Device* device);
	~TexturedCube();

	void render(ID3D11DeviceContext* context, ID3D11Buffer* sceneBuffer, ID3D11Buffer* geomBuffer, ID3D11SamplerState* samplerState);

	void update(ID3D11DeviceContext* context, double angle);

private:
	bool initBuffers();
	bool initInputLayout();
	bool initTexture();
	bool initInstances();

	void terminate();

private:
	ID3D11Device* m_pDevice;

	ID3D11Buffer* m_pIndexBuffer;
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pLightingParamBuffer;

	ID3D11PixelShader* m_pPixelShader;
	ID3D11VertexShader* m_pVertexShader;
	ID3D11InputLayout* m_pInputLayout;

	ID3D11ShaderResourceView* m_pSRV;
	ID3D11ShaderResourceView* m_pNormalSRV;

	ID3D11Buffer* m_pGeomBufferInst;

	std::vector<GeomBufferInst> geomBuffers;
};

