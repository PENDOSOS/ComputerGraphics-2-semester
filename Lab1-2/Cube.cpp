#include "Cube.h"

using namespace DirectX;

Cube::Cube(ID3D11Device* device) : m_pDevice(device), m_pIndexBuffer(nullptr), m_pVertexBuffer(nullptr)
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
    CubeVertex cubeVertices[] =
    {
        { { -1.0f, 1.0f, -1.0f }, RGB(0, 0, 255)},
        { { 1.0f, 1.0f, -1.0f }, RGB(0, 255, 255) },
        { { 1.0f, 1.0f, 1.0f }, RGB(255, 255, 255) },
        { { -1.0f, 1.0f, 1.0f }, RGB(255, 0, 255)},
        { { -1.0f, -1.0f, -1.0f }, RGB(255, 255, 0)},
        { { 1.0f, -1.0f, -1.0f }, RGB(0, 255, 0) },
        { { 1.0f, -1.0f, 1.0f }, RGB(255, 0, 0) },
        { { -1.0f, -1.0f, 1.0f }, RGB(0, 0, 0)},
    };

    D3D11_BUFFER_DESC vertexBufferrDesc = {};
    vertexBufferrDesc.ByteWidth = sizeof(cubeVertices);
    vertexBufferrDesc.Usage = D3D11_USAGE_IMMUTABLE;//D3D11_USAGE_DEFAULT;
    vertexBufferrDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferrDesc.CPUAccessFlags = 0;
    vertexBufferrDesc.MiscFlags = 0;
    vertexBufferrDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = &cubeVertices;
    vertexData.SysMemPitch = sizeof(cubeVertices);
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
    D3D11_INPUT_ELEMENT_DESC inputDesc[] = 
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    HRESULT result = S_OK;

    ID3DBlob* pVertexShaderCode = nullptr;
    if (SUCCEEDED(result))
    {
        result = compileShader(m_pDevice, L"resources/shaders/cube_vs.hlsl", {}, shader_stage::Vertex, (ID3D11DeviceChild**)&m_pVertexShader, &pVertexShaderCode);
    }
    if (SUCCEEDED(result))
    {
        result = compileShader(m_pDevice, L"resources/shaders/cube_ps.hlsl", {}, shader_stage::Pixel, (ID3D11DeviceChild**)&m_pPixelShader);
    }

    if (SUCCEEDED(result))
    {
        result = m_pDevice->CreateInputLayout(inputDesc, 2, pVertexShaderCode->GetBufferPointer(), pVertexShaderCode->GetBufferSize(), &m_pInputLayout);
        if (SUCCEEDED(result))
        {
            result = SetResourceName(m_pInputLayout, "InputLayout");
        }
    }

    if (pVertexShaderCode != nullptr)
    {
        pVertexShaderCode->Release();
        pVertexShaderCode = nullptr;
    }

    return result;
}

void Cube::render(ID3D11DeviceContext* context, UINT width, UINT height, ID3D11Buffer* sceneBuffer, ID3D11Buffer* geomBuffer)
{
    context->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    ID3D11Buffer* vertexBuffers[] = { m_pVertexBuffer };
    UINT strides[] = { 16 };
    UINT offsets[] = { 0 };
    context->IASetVertexBuffers(0, 1, &m_pVertexBuffer, strides, offsets);
    context->IASetInputLayout(m_pInputLayout);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->VSSetShader(m_pVertexShader, nullptr, 0);
    context->VSSetConstantBuffers(0, 1, &sceneBuffer);
    context->VSSetConstantBuffers(1, 1, &geomBuffer);
    context->PSSetShader(m_pPixelShader, nullptr, 0);
    context->DrawIndexed(36, 0, 0);
}

void Cube::terminate()
{
    if (m_pInputLayout != nullptr)
    {
        m_pInputLayout->Release();
        m_pInputLayout = nullptr;
    }

    if (m_pPixelShader != nullptr)
    {
        m_pPixelShader->Release();
        m_pPixelShader = nullptr;
    }

    if (m_pVertexShader != nullptr)
    {
        m_pVertexShader->Release();
        m_pVertexShader = nullptr;
    }

    if (m_pIndexBuffer != nullptr)
    {
        m_pIndexBuffer->Release();
        m_pIndexBuffer = nullptr;
    }

    if (m_pVertexBuffer != nullptr)
    {
        m_pVertexBuffer->Release();
        m_pVertexBuffer = nullptr;
    }
}

