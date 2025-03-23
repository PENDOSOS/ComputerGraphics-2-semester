#pragma once

#include "framework.h"

struct RectVertex 
{
	DirectX::XMFLOAT3 Pos;
	COLORREF Color;
	DirectX::XMFLOAT3 alpha;
};

class TransparentRect
{
public:
	TransparentRect(ID3D11Device* device, float offset, int colorRed, int colorGreen, int colorBlue);
	~TransparentRect();

	void render(ID3D11DeviceContext* context, ID3D11Buffer* sceneBuffer);

public:
	std::vector<DirectX::XMFLOAT3> coords;

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

	ID3D11Buffer* m_pGeomBuffer;

	float m_offset;
	int m_colorRed;
	int m_colorGreen;
	int m_colorBlue;
};

