#include "TexturedCube.h"

struct CubeVertex
{
    DirectX::XMFLOAT3 pos;
	//float x, y, z;
    DirectX::XMFLOAT3 norm;
    //float norm_x, norm_y, norm_z;
    DirectX::XMFLOAT3 tan;
    /*float tan_x, tan_y, tan_z;*/
    DirectX::XMFLOAT2 uv;
    /*float u, v;*/
};

struct GeomBuffer
{
    DirectX::XMMATRIX M;
    DirectX::XMFLOAT3 Size;
};

struct LightParams
{
    DirectX::XMFLOAT4 Params;
};

TexturedCube::TexturedCube(ID3D11Device* device)
	: m_pDevice(device)
	, m_pInputLayout(nullptr)
	, m_pPixelShader(nullptr)
	, m_pVertexBuffer(nullptr)
	, m_pVertexShader(nullptr)
    , m_pLightingParamBuffer(nullptr)
{
	initBuffers();
	initInputLayout();
	initTexture();
}

TexturedCube::~TexturedCube()
{
	terminate();
}

void TexturedCube::render(ID3D11DeviceContext* context, ID3D11Buffer* sceneBuffer, ID3D11Buffer* geomBuffer, ID3D11SamplerState* samplerState)
{
    if (m_pSRV)
    {
        UINT stride = { 44 };
        UINT offset = { 0 };

        context->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
        context->VSSetShader(m_pVertexShader, nullptr, 0);
        context->PSSetShader(m_pPixelShader, nullptr, 0);
        context->PSSetShaderResources(0, 1, &m_pSRV);
        context->PSSetSamplers(0, 1, &samplerState);
        context->IASetInputLayout(m_pInputLayout);

        context->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        context->VSSetConstantBuffers(0, 1, &sceneBuffer);
        context->VSSetConstantBuffers(1, 1, &geomBuffer);
        context->PSSetConstantBuffers(0, 1, &sceneBuffer);
        context->PSSetConstantBuffers(2, 1, &m_pLightingParamBuffer);

        context->DrawIndexed(36, 0, 0);
    }
}

bool TexturedCube::initBuffers()
{
    // Textured cube
    static const CubeVertex cubeVertices[24] = 
    {
        // Bottom face
        { { -0.5, -0.5,  0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 1 } },
        { { 0.5, -0.5,  0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, { 1, 1 } },
        { { 0.5, -0.5, -0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, { 1, 0 } },
        { { -0.5, -0.5, -0.5 }, {0, 0, 0}, {0, 0, 0}, {0, 0}},
        // Top face
        { { -0.5,  0.5, -0.5 }, {0, 0, 0}, {0, 0, 0}, {0, 1}},
        { {  0.5,  0.5, -0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, {1, 1}},
        { {  0.5,  0.5,  0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, {1, 0}},
        { { -0.5,  0.5,  0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, {0, 0}},
        // Front face
        { { 0.5, -0.5, -0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, {0, 1}},
        { { 0.5, -0.5,  0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, {1, 1}},
        { { 0.5,  0.5,  0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, {1, 0}},
        { { 0.5,  0.5, -0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, {0, 0}},
        // Back face
        { {-0.5, -0.5,  0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, {0, 1}},
        { {-0.5, -0.5, -0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, {1, 1}},
        { {-0.5,  0.5, -0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, {1, 0}},
        { {-0.5,  0.5,  0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, {0, 0}},
        // Left face
        { {0.5, -0.5,  0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, {0, 1}},
        { {-0.5, -0.5,  0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, {1, 1}},
        { {-0.5,  0.5,  0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, {1, 0}},
        { {0.5,  0.5,  0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, {0, 0}},
        // Right face
        { {-0.5, -0.5, -0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 1}},
        { { 0.5, -0.5, -0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, { 1, 1}},
        { { 0.5,  0.5, -0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, {1, 0}},
        { {-0.5,  0.5, -0.5 }, { 0, 0, 0 }, { 0, 0, 0 }, {0, 0}},
    };
    static const UINT16 indices[36] = 
    {
        0, 2, 1, 0, 3, 2,
        4, 6, 5, 4, 7, 6,
        8, 10, 9, 8, 11, 10,
        12, 14, 13, 12, 15, 14,
        16, 18, 17, 16, 19, 18,
        20, 22, 21, 20, 23, 22
    };

    D3D11_BUFFER_DESC vertexBufferrDesc = {};
    vertexBufferrDesc.ByteWidth = sizeof(cubeVertices);
    vertexBufferrDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferrDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferrDesc.CPUAccessFlags = 0;
    vertexBufferrDesc.MiscFlags = 0;
    vertexBufferrDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = &cubeVertices;
    vertexData.SysMemPitch = sizeof(cubeVertices);
    HRESULT result = m_pDevice->CreateBuffer(&vertexBufferrDesc, &vertexData, &m_pVertexBuffer);

    if (FAILED(result))
    {
        return false;
    }

    result = SetResourceName(m_pVertexBuffer, "textured cube vertex buffer");

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

    if (FAILED(result))
        return false;

    result = SetResourceName(m_pIndexBuffer, "textured cube index buffer");

    D3D11_BUFFER_DESC lightParamsBufferDesc = {};
    lightParamsBufferDesc.ByteWidth = sizeof(LightParams);
    lightParamsBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    lightParamsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    lightParamsBufferDesc.CPUAccessFlags = 0;
    lightParamsBufferDesc.MiscFlags = 0;
    lightParamsBufferDesc.StructureByteStride = 0;

    LightParams params;
    params.Params = { 0.2f, 0.0f, 0, 0 };

    D3D11_SUBRESOURCE_DATA lightParamsData = {};
    lightParamsData.pSysMem = &params;
    lightParamsData.SysMemPitch = sizeof(LightParams);
    result = m_pDevice->CreateBuffer(&lightParamsBufferDesc, &lightParamsData, &m_pLightingParamBuffer);

    if (FAILED(result))
        return false;

    result = SetResourceName(m_pLightingParamBuffer, "lighting params buffer");

    return true;
}

bool TexturedCube::initInputLayout()
{
    D3D11_INPUT_ELEMENT_DESC inputDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    HRESULT result = S_OK;

    ID3DBlob* pVertexShaderCode = nullptr;
    if (SUCCEEDED(result))
    {
        result = compileShader(m_pDevice, L"resources/shaders/lighted_cube_vs.hlsl", {}, shader_stage::Vertex, (ID3D11DeviceChild**)&m_pVertexShader, &pVertexShaderCode);
    }
    if (SUCCEEDED(result))
    {
        result = compileShader(m_pDevice, L"resources/shaders/lighted_cube_ps.hlsl", {}, shader_stage::Pixel, (ID3D11DeviceChild**)&m_pPixelShader);
    }

    if (SUCCEEDED(result))
    {
        result = m_pDevice->CreateInputLayout(inputDesc, 4, pVertexShaderCode->GetBufferPointer(), pVertexShaderCode->GetBufferSize(), &m_pInputLayout);
        if (SUCCEEDED(result))
        {
            result = SetResourceName(m_pInputLayout, "input layout");
        }
    }

    if (pVertexShaderCode != nullptr)
    {
        pVertexShaderCode->Release();
        pVertexShaderCode = nullptr;
    }

    return result;
}

bool TexturedCube::initTexture()
{
    HRESULT result = DirectX::CreateDDSTextureFromFile(
        m_pDevice,
        L"resources/textures/tiles.dds",
        nullptr,
        &m_pSRV
    );

    if (FAILED(result))
    {
        return false;
    }

    result = DirectX::CreateDDSTextureFromFile(
        m_pDevice,
        L"resources/textures/tiles_normal.dds",
        nullptr,
        &m_pNormalSRV
    );

    if (FAILED(result))
    {
        return false;
    }

    return true;
}

void TexturedCube::terminate()
{
    if (m_pNormalSRV != nullptr)
    {
        m_pNormalSRV->Release();
        m_pNormalSRV = nullptr;
    }

    if (m_pSRV != nullptr)
    {
        m_pSRV->Release();
        m_pSRV = nullptr;
    }

    if (m_pInputLayout != nullptr)
    {
        m_pInputLayout->Release();
        m_pInputLayout = nullptr;
    }

    if (m_pVertexShader != nullptr)
    {
        m_pVertexShader->Release();
        m_pVertexShader = nullptr;
    }

    if (m_pPixelShader != nullptr)
    {
        m_pPixelShader->Release();
        m_pPixelShader = nullptr;
    }

    if (m_pLightingParamBuffer != nullptr)
    {
        m_pLightingParamBuffer->Release();
        m_pLightingParamBuffer = nullptr;
    }

    if (m_pVertexBuffer != nullptr)
    {
        m_pVertexBuffer->Release();
        m_pVertexBuffer = nullptr;
    }

    if (m_pIndexBuffer != nullptr)
    {
        m_pIndexBuffer->Release();
        m_pIndexBuffer = nullptr;
    }
}
