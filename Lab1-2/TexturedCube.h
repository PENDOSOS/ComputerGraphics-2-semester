#pragma once

#include "framework.h"

#define MAX_INST 20

struct GeomBufferInst
{
	DirectX::XMMATRIX M;
	DirectX::XMMATRIX NormalM;
	DirectX::XMFLOAT4 params; // x - shininess, y - use normal map, z - texture index, w - is cube visible
};

struct CullParams
{
	//DirectX::XMINT4 shapeCount; // x - shapes count
	DirectX::XMFLOAT4 bbMin[MAX_INST];
	DirectX::XMFLOAT4 bbMax[MAX_INST];
};

class TexturedCube
{
public:
	TexturedCube(ID3D11Device* device);
	~TexturedCube();

	void render(ID3D11DeviceContext* context, ID3D11Buffer* sceneBuffer, ID3D11Buffer* geomBuffer, ID3D11SamplerState* samplerState);

	void update(ID3D11DeviceContext* context, float angle, bool isCompute);

	void cullInCompute(ID3D11DeviceContext* context, ID3D11Buffer* sceneBuffer);

	std::vector<GeomBufferInst>& getInstances() { return geomBuffers; }
	std::vector<std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3>>& getAABB() { return AABB; }
	ID3D11UnorderedAccessView* getIndirectArgsUAV() { return m_pIndirectArgsUAV; }
	ID3D11UnorderedAccessView* getInstUAV() { return m_pGeomBufferInstGPU_UAV; }

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
	ID3D11ComputeShader* m_pComputeShader;
	ID3D11InputLayout* m_pInputLayout;

	ID3D11ShaderResourceView* m_pSRV;
	ID3D11ShaderResourceView* m_pNormalSRV;

	ID3D11Buffer* m_pGeomBufferInst;
	ID3D11Buffer* m_pCullParams;

	ID3D11Buffer* m_pGeomBufferInstCompute;
	ID3D11Buffer* m_pGeomBufferInstGPU;
	ID3D11UnorderedAccessView* m_pGeomBufferInstGPU_UAV;

	ID3D11Buffer* m_pIndirectArgs;
	ID3D11Buffer* m_pIndirectArgsSrc;
	ID3D11UnorderedAccessView* m_pIndirectArgsUAV;

	std::vector<GeomBufferInst> geomBuffers;
	std::vector<DirectX::XMINT4> visible;

	int instanceCount;
	std::vector<std::pair<DirectX::XMFLOAT3, DirectX::XMFLOAT3>> AABB;
	bool isCompute;
};

