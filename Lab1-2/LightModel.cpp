#include "LightModel.h"

LightModel::LightModel(ID3D11Device* device)
	: m_pDevice(device)
    , m_pIndexBuffer(nullptr)
    , m_pInputLayout(nullptr)
    , m_pPixelShader(nullptr)
    , m_pVertexBuffer(nullptr)
    , m_pVertexShader(nullptr)
{
	bool initBuffers();
	bool initInputLayout();
}

LightModel::~LightModel()
{
	terminate();
}

void LightModel::render(ID3D11DeviceContext* context, UINT width, UINT height, ID3D11Buffer* sceneBuffer, ID3D11Buffer* geomBuffer)
{
    context->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    ID3D11Buffer* vertexBuffers[] = { m_pVertexBuffer };
    UINT strides[] = { 12 };
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

bool LightModel::initBuffers()
{
    static const size_t SphereSteps = 10;

    std::vector<DirectX::XMFLOAT3> sphereVertices;
    std::vector<UINT16> indices;

    size_t indexCount = (SphereSteps + 1) * (SphereSteps + 1);
    size_t vertexCount = SphereSteps * SphereSteps * 6;

    sphereVertices.resize(vertexCount);
    indices.resize(indexCount);

    for (size_t lat = 0; lat < SphereSteps + 1; lat++)
    {
        for (size_t lon = 0; lon < SphereSteps + 1; lon++)
        {
            int index = (int)(lat * (SphereSteps + 1) + lon);
            float lonAngle = 2.0f * (float)PI * lon / SphereSteps + (float)PI;
            float latAngle = -(float)PI / 2 + (float)PI * lat / SphereSteps;

            DirectX::XMFLOAT3 r = DirectX::XMFLOAT3{
                sinf(lonAngle) * cosf(latAngle),
                sinf(latAngle),
                cosf(lonAngle) * cosf(latAngle)
            };

            sphereVertices[index].x = r.x * 0.5f;
            sphereVertices[index].y = r.y * 0.5f;
            sphereVertices[index].z = r.z * 0.5f;
        }
    }

    for (size_t lat = 0; lat < SphereSteps; lat++)
    {
        for (size_t lon = 0; lon < SphereSteps; lon++)
        {
            size_t index = lat * SphereSteps * 6 + lon * 6;
            indices[index + 0] = (UINT16)(lat * (SphereSteps + 1) + lon + 0);
            indices[index + 2] = (UINT16)(lat * (SphereSteps + 1) + lon + 1);
            indices[index + 1] = (UINT16)(lat * (SphereSteps + 1) + SphereSteps + 1 + lon);
            indices[index + 3] = (UINT16)(lat * (SphereSteps + 1) + lon + 1);
            indices[index + 5] = (UINT16)(lat * (SphereSteps + 1) + SphereSteps + 1 + lon + 1);
            indices[index + 4] = (UINT16)(lat * (SphereSteps + 1) + SphereSteps + 1 + lon);
        }
    }

    for (auto& v : sphereVertices)
    {
        v.x = v.x * 0.125f;
        v.y = v.y * 0.125f;
        v.z = v.z * 0.125f;
    }

    D3D11_BUFFER_DESC vertexBufferrDesc = {};
    vertexBufferrDesc.ByteWidth = sizeof(sphereVertices);
    vertexBufferrDesc.Usage = D3D11_USAGE_IMMUTABLE;//D3D11_USAGE_DEFAULT;
    vertexBufferrDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferrDesc.CPUAccessFlags = 0;
    vertexBufferrDesc.MiscFlags = 0;
    vertexBufferrDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vertexData = {};
    vertexData.pSysMem = sphereVertices.data();
    vertexData.SysMemPitch = sizeof(sphereVertices);
    HRESULT result = m_pDevice->CreateBuffer(&vertexBufferrDesc, &vertexData, &m_pVertexBuffer);

    if (!SUCCEEDED(result))
        return false;

    result = SetResourceName(m_pVertexBuffer, "light source vertex buffer");

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = sizeof(indices);
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA indexData = {};
    indexData.pSysMem = indices.data();
    indexData.SysMemPitch = sizeof(indices);
    result = m_pDevice->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer);

    if (!SUCCEEDED(result))
        return false;

    result = SetResourceName(m_pIndexBuffer, "light source index buffer");

    return true;
}

bool LightModel::initInputLayout()
{
    static const D3D11_INPUT_ELEMENT_DESC inputDesc[] = 
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
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

void LightModel::terminate()
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
