#pragma once

#include <set>

#include "framework.h"

struct triangleVertex
{
	float x, y, z;
	COLORREF color;
};

class Triangle
{
public:
	Triangle(ID3D11Device* device);
	~Triangle();
	
	bool initBuffers();
	bool initInputLayout();

	void render(ID3D11DeviceContext* context, UINT width, UINT height);

private:
	void terminate();

private:
	ID3D11Device* m_pDevice;

	ID3D11Buffer* m_pIndexBuffer;
	ID3D11Buffer* m_pVertexBuffer;

	ID3D11PixelShader* m_pPixelShader;
	ID3D11VertexShader* m_pVertexShader;
	ID3D11InputLayout* m_pInputLayout;
};

