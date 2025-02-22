#include "Cube.h"

using namespace DirectX;

Cube::Cube(ID3D11Device* device)
{
	initBuffers();
	initInputLayout();
}

Cube::~Cube()
{
	terminate();
}

bool Cube::initBuffers()
{
    CubeVertex vertices[] =
    {
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
    };

    D3D11_BUFFER_DESC vertexBufferrDesc = {};
    vertexBufferrDesc.ByteWidth = sizeof(vertices);
    vertexBufferrDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferrDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferrDesc.CPUAccessFlags = 0;
    vertexBufferrDesc.MiscFlags = 0;
    vertexBufferrDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = &vertices;
    vertexData.SysMemPitch = sizeof(vertices);
    HRESULT result = m_pDevice->CreateBuffer(&vertexBufferrDesc, &vertexData, &m_pVertexBuffer);

    if (!SUCCEEDED(result))
        return false;

    result = SetResourceName(m_pVertexBuffer, "cube vertex buffer");

    WORD indices[] =
    {
        3,1,0,
        2,1,3,

        0,5,4,
        1,5,0,

        3,4,7,
        0,4,3,

        1,6,5,
        2,6,1,

        2,7,6,
        3,7,2,

        6,4,5,
        7,4,6,
    };

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = sizeof(indices);
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = &indices;
    indexData.SysMemPitch = sizeof(indices);
    result = m_pDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer);

    if (!SUCCEEDED(result))
        return false;

    result = SetResourceName(m_pIndexBuffer, "cube index buffer");

	return true;
}

bool Cube::initInputLayout()
{
    D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    HRESULT result = S_OK;
    
    return true;
}

