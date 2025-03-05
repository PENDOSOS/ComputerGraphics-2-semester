#include "Skybox.h"

struct GeomBuffer
{
    DirectX::XMMATRIX M;
    DirectX::XMFLOAT3 Size;
};

Skybox::Skybox(ID3D11Device* device)
    : m_pDevice(device)
    , m_pVertexBuffer(nullptr)
    , m_pVertexShader(nullptr)
    , m_pPixelShader(nullptr)
    , m_pInputLayout(nullptr)
    , m_pCubemapView(nullptr)
    , m_pGeomBuffer(nullptr)
{
	initBuffers();
	initInputLayout();
    initTexture();
}

Skybox::~Skybox()
{
	terminate();
}

void Skybox::render(ID3D11DeviceContext* context, UINT width, UINT height, ID3D11Buffer* sceneBuffer, ID3D11SamplerState* samplerState)
{
    if (m_pCubemapView) 
    {
        UINT stride = { 12 };
        UINT offset = { 0 };

        context->VSSetShader(m_pVertexShader, nullptr, 0);
        context->PSSetShader(m_pPixelShader, nullptr, 0);
        context->PSSetShaderResources(0, 1, &m_pCubemapView);
        context->PSSetSamplers(0, 1, &samplerState);
        context->IASetInputLayout(m_pInputLayout);

        context->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        context->VSSetConstantBuffers(0, 1, &sceneBuffer);
        context->VSSetConstantBuffers(1, 1, &m_pGeomBuffer);

        context->Draw(36, 0);
    }
}

bool Skybox::initBuffers()
{
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    D3D11_BUFFER_DESC vertexBufferrDesc = {};
    vertexBufferrDesc.ByteWidth = sizeof(skyboxVertices);
    vertexBufferrDesc.Usage = D3D11_USAGE_IMMUTABLE;//D3D11_USAGE_DEFAULT;
    vertexBufferrDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferrDesc.CPUAccessFlags = 0;
    vertexBufferrDesc.MiscFlags = 0;
    vertexBufferrDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = &skyboxVertices;
    vertexData.SysMemPitch = sizeof(skyboxVertices);
    HRESULT result = m_pDevice->CreateBuffer(&vertexBufferrDesc, &vertexData, &m_pVertexBuffer);
    if (!SUCCEEDED(result))
    {
        return false;
    }

    result = SetResourceName(m_pVertexBuffer, "skybox vertex buffer");

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(GeomBuffer);
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    GeomBuffer geomBuffer;
    geomBuffer.M = DirectX::XMMatrixIdentity();
    geomBuffer.Size.x = 20.0f;
    geomBuffer.Size.y = 0.0f;
    geomBuffer.Size.z = 0.0f;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = &geomBuffer;
    data.SysMemPitch = sizeof(geomBuffer);
    data.SysMemSlicePitch = 0;

    result = m_pDevice->CreateBuffer(&desc, &data, &m_pGeomBuffer);
    assert(SUCCEEDED(result));
    if (SUCCEEDED(result))
    {
        result = SetResourceName(m_pGeomBuffer, "skybox geom buffer");
    }

    return true;
}

bool Skybox::initInputLayout()
{
    D3D11_INPUT_ELEMENT_DESC inputDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    HRESULT result = S_OK;

    ID3DBlob* pVertexShaderCode = nullptr;
    if (SUCCEEDED(result))
    {
        result = compileShader(m_pDevice, L"resources/shaders/skybox_vs.hlsl", {}, shader_stage::Vertex, (ID3D11DeviceChild**)&m_pVertexShader, &pVertexShaderCode);
    }
    if (SUCCEEDED(result))
    {
        result = compileShader(m_pDevice, L"resources/shaders/skybox_ps.hlsl", {}, shader_stage::Pixel, (ID3D11DeviceChild**)&m_pPixelShader);
    }

    if (!SUCCEEDED(result))
    {
        return false;
    }

    result = m_pDevice->CreateInputLayout(inputDesc, 1, pVertexShaderCode->GetBufferPointer(), pVertexShaderCode->GetBufferSize(), &m_pInputLayout);
    if (!SUCCEEDED(result))
    {
        return false;
    }

    result = SetResourceName(m_pInputLayout, "skybox input layout");

    if (pVertexShaderCode != nullptr)
    {
        pVertexShaderCode->Release();
        pVertexShaderCode = nullptr;
    }

    return true;
}

bool Skybox::initTexture()
{
    HRESULT hr = DirectX::CreateDDSTextureFromFile(
        m_pDevice,
        L"resources/textures/cubemap.dds",
        nullptr,
        &m_pCubemapView
    );

    if (FAILED(hr)) 
    {
        return false;
    }

    return true;
}

void Skybox::terminate()
{
    if (m_pGeomBuffer != nullptr)
    {
        m_pGeomBuffer->Release();
        m_pGeomBuffer = nullptr;
    }

    if (m_pCubemapView != nullptr)
    {
        m_pCubemapView->Release();
        m_pCubemapView = nullptr;
    }

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

    if (m_pVertexBuffer != nullptr)
    {
        m_pVertexBuffer->Release();
        m_pVertexBuffer = nullptr;
    }
}
