#include "TransparentRect.h"

struct GeomBuffer
{
    DirectX::XMMATRIX M;
};

TransparentRect::TransparentRect(ID3D11Device* device, float offset, int colorRed, int colorGreen, int colorBlue) : m_pDevice(device), m_offset(offset), m_colorRed(colorRed), m_colorGreen(colorGreen), m_colorBlue(colorBlue)
{
	initBuffers();
	initInputLayout();
}

TransparentRect::~TransparentRect()
{
	terminate();
}

void TransparentRect::render(ID3D11DeviceContext* context, ID3D11Buffer* sceneBuffer)
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
    context->VSSetConstantBuffers(1, 1, &m_pGeomBuffer);
    context->PSSetShader(m_pPixelShader, nullptr, 0);
    context->DrawIndexed(6, 0, 0);
}

bool TransparentRect::initBuffers()
{
    const RectVertex Vertices[] =
    {
        { { 0.0, -0.75, -0.75 }, RGB(m_colorRed, m_colorGreen, m_colorBlue) },
        { { 0.0,  0.75, -0.75 }, RGB(m_colorRed, m_colorGreen, m_colorBlue) },
        { { 0.0,  0.75,  0.75 }, RGB(m_colorRed, m_colorGreen, m_colorBlue) },
        { { 0.0, -0.75,  0.75 }, RGB(m_colorRed, m_colorGreen, m_colorBlue) }
    };

    static const UINT16 Indices[] = 
    {
        0, 1, 2,
        0, 2, 3
    };

    D3D11_BUFFER_DESC vertexBufferrDesc = {};
    vertexBufferrDesc.ByteWidth = sizeof(Vertices);
    vertexBufferrDesc.Usage = D3D11_USAGE_IMMUTABLE;//D3D11_USAGE_DEFAULT;
    vertexBufferrDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferrDesc.CPUAccessFlags = 0;
    vertexBufferrDesc.MiscFlags = 0;
    vertexBufferrDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = &Vertices;
    vertexData.SysMemPitch = sizeof(Vertices);
    HRESULT result = m_pDevice->CreateBuffer(&vertexBufferrDesc, &vertexData, &m_pVertexBuffer);

    if (FAILED(result))
        return false;

    result = SetResourceName(m_pVertexBuffer, "rect vertex buffer");

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = sizeof(Indices);
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = &Indices;
    indexData.SysMemPitch = sizeof(Indices);
    result = m_pDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer);

    if (!SUCCEEDED(result))
        return false;

    result = SetResourceName(m_pIndexBuffer, "rect index buffer");

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = sizeof(GeomBuffer);
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.StructureByteStride = 0;

    GeomBuffer geomBuffer;
    geomBuffer.M = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationY(-3.14 / 4), DirectX::XMMatrixTranslation(m_offset, 0.0f, m_offset));

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = &geomBuffer;
    data.SysMemPitch = sizeof(geomBuffer);
    data.SysMemSlicePitch = 0;

    result = m_pDevice->CreateBuffer(&desc, &data, &m_pGeomBuffer);
    assert(SUCCEEDED(result));
    if (SUCCEEDED(result))
    {
        result = SetResourceName(m_pGeomBuffer, "rect geom buffer");
    }

    return true;
}

bool TransparentRect::initInputLayout()
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
            result = SetResourceName(m_pInputLayout, "rect input layout");
        }
    }

    if (pVertexShaderCode != nullptr)
    {
        pVertexShaderCode->Release();
        pVertexShaderCode = nullptr;
    }

    return result;
}

void TransparentRect::terminate()
{
    if (m_pGeomBuffer != nullptr)
    {
        m_pGeomBuffer->Release();
        m_pGeomBuffer = nullptr;
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
