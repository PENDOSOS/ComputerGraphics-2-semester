#pragma once

#include "framework.h"

class Postprocess
{
public:
	Postprocess(ID3D11Device* device, int width, int height);
	~Postprocess();

	void render(ID3D11DeviceContext* context, ID3D11RenderTargetView* backBuffer, ID3D11SamplerState* sampler);

	bool reinit(int width, int height);

	ID3D11RenderTargetView* getRenderTarget() { return m_pSceneRTV; }

private:
	bool initShaders();
	bool initResources();

	void terminate();

private:
	ID3D11Device* m_pDevice;

	ID3D11PixelShader* m_pPixelShader;
	ID3D11VertexShader* m_pVertexShader;

	ID3D11Texture2D* m_pScene;
	ID3D11RenderTargetView* m_pSceneRTV;
	ID3D11ShaderResourceView* m_pSceneSRV;

	int m_width;
	int m_height;
};

