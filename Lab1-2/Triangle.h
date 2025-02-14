#pragma once

#include <set>

#include "framework.h"

struct triangleVertex
{
	float x, y, z;
	COLORREF color;
};

enum shader_stage
{
	Vertex,
	Pixel,
};

class Triangle
{
public:
	Triangle(ID3D11Device* device);
	~Triangle();
	
	bool initBuffers();
	bool initInputLayout();

	bool compileShader(LPCTSTR srcFilename, const std::vector<LPCSTR>& defines, const shader_stage& stage, ID3D11DeviceChild** ppShader, ID3DBlob** ppShaderBinary = nullptr);

	inline HRESULT SetResourceName(ID3D11DeviceChild* pResource, const std::string& name)
	{
		return pResource->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.length(), name.c_str());
	}

	void render(ID3D11DeviceContext* context, UINT width, UINT height);

private:
	void terminate();

	bool readFileContent(LPCTSTR filename, std::vector<char>& data);

private:
	ID3D11Device* m_pDevice;

	ID3D11Buffer* m_pIndexBuffer;
	ID3D11Buffer* m_pVertexBuffer;

	ID3D11PixelShader* m_pPixelShader;
	ID3D11VertexShader* m_pVertexShader;
	ID3D11InputLayout* m_pInputLayout;
};

