#include "TexturedCube.h"
#include "DirectXTex.h"

struct CubeVertex
{
    DirectX::XMFLOAT3 pos;
	//float x, y, z;
    DirectX::XMFLOAT3 tan;
    //float norm_x, norm_y, norm_z;
    DirectX::XMFLOAT3 norm;
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
    instanceCount = MAX_INST;
	initBuffers();
	initInputLayout();
	initTexture();
    initInstances();
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
        context->PSSetShaderResources(1, 1, &m_pNormalSRV);
        context->PSSetSamplers(0, 1, &samplerState);
        context->IASetInputLayout(m_pInputLayout);

        context->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        context->VSSetConstantBuffers(0, 1, &sceneBuffer);
        context->VSSetConstantBuffers(1, 1, &m_pGeomBufferInst);
        context->PSSetConstantBuffers(0, 1, &sceneBuffer);
        context->PSSetConstantBuffers(1, 1, &m_pGeomBufferInst);
        if (isCompute)
        {
            context->CopyResource(m_pIndirectArgs, m_pIndirectArgsSrc);
            context->DrawIndexedInstancedIndirect(m_pIndirectArgs, 0);
        }
        else
        {
            context->DrawIndexedInstanced(36, instanceCount, 0, 0, 0);
        }
    }
}

void TexturedCube::update(ID3D11DeviceContext* context, float angle, bool isCompute)
{
    this->isCompute = isCompute;
    if (!isCompute)
    {
        instanceCount = 0;
        std::vector<GeomBufferInst> visibleInstances;
        //visibleInstances.resize(20);
        for (int i = 0; i < MAX_INST; i++)
        {
            float offsetX = geomBuffers[i].M.r[3].m128_f32[0];
            float offsetY = geomBuffers[i].M.r[3].m128_f32[1];
            float offsetZ = geomBuffers[i].M.r[3].m128_f32[2];
            // if tiles texture - rotate cube
            if (geomBuffers[i].params.z)
            {
                geomBuffers[i].M = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationY(angle), DirectX::XMMatrixTranslation(offsetX, offsetY, offsetZ));
                geomBuffers[i].NormalM = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, geomBuffers[i].M));
            }
            instanceCount += geomBuffers[i].params.w;
            if ((int)geomBuffers[i].params.w)
            {
                visibleInstances.push_back(geomBuffers[i]);
            }
        }
        visibleInstances.resize(20);
        context->UpdateSubresource(m_pGeomBufferInst, 0, nullptr, visibleInstances.data(), 0, 0);
    }

    {
        ImGui::Begin("Culling stats");
        ImGui::Text("Instances: %d", MAX_INST);
        ImGui::Text("Visible instances %d", instanceCount);
        ImGui::End();
    }
}

void TexturedCube::cullInCompute(ID3D11DeviceContext* context, ID3D11Buffer* sceneBuffer)
{
    D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS args;
    args.IndexCountPerInstance = 36;
    args.InstanceCount = 0;
    args.StartIndexLocation = 0;
    args.BaseVertexLocation = 0;
    args.StartInstanceLocation = 0;

    context->UpdateSubresource(m_pIndirectArgsSrc, 0, nullptr, &args, 0, 0);

    UINT groupNumber = 1;

    ID3D11Buffer* constBuffers[3] = { sceneBuffer, m_pCullParams, m_pGeomBufferInstCompute };
    context->CSSetConstantBuffers(0, 3, constBuffers);

    ID3D11UnorderedAccessView* uavBuffers[2] = { m_pIndirectArgsUAV, m_pGeomBufferInstGPU_UAV };
    context->CSSetUnorderedAccessViews(0, 2, uavBuffers, nullptr);

    context->CSSetShader(m_pComputeShader, nullptr, 0);

    context->Dispatch(groupNumber, 1, 1);

    context->CopyResource(m_pGeomBufferInst, m_pGeomBufferInstGPU);
}

bool TexturedCube::initBuffers()
{
    // Textured cube
    static const CubeVertex cubeVertices[24] = 
    {
        // Bottom face
        {{-0.5, -0.5,  0.5}, {1, 0, 0}, {0, -1, 0}, {0, 1}},
        {{ 0.5, -0.5,  0.5}, {1, 0, 0}, {0, -1, 0}, {1, 1}},
        {{ 0.5, -0.5, -0.5}, {1, 0, 0}, {0, -1, 0}, {1, 0}},
        {{-0.5, -0.5, -0.5}, {1, 0, 0}, {0, -1, 0}, {0, 0}},
        // Top face
        {{-0.5,  0.5, -0.5}, {1, 0, 0}, {0, 1, 0}, {0, 1}},
        {{ 0.5,  0.5, -0.5}, {1, 0, 0}, {0, 1, 0}, {1, 1}},
        {{ 0.5,  0.5,  0.5}, {1, 0, 0}, {0, 1, 0}, {1, 0}},
        {{-0.5,  0.5,  0.5}, {1, 0, 0}, {0, 1, 0}, {0, 0}},
        // Front face
        {{ 0.5, -0.5, -0.5}, {0, 0, 1}, {1, 0, 0}, {0, 1}},
        {{ 0.5, -0.5,  0.5}, {0, 0, 1}, {1, 0, 0}, {1, 1}},
        {{ 0.5,  0.5,  0.5}, {0, 0, 1}, {1, 0, 0}, {1, 0}},
        {{ 0.5,  0.5, -0.5}, {0, 0, 1}, {1, 0, 0}, {0, 0}},
        // Back face
        {{-0.5, -0.5,  0.5}, {0, 0, -1}, {-1, 0, 0}, {0, 1}},
        {{-0.5, -0.5, -0.5}, {0, 0, -1}, {-1, 0, 0}, {1, 1}},
        {{-0.5,  0.5, -0.5}, {0, 0, -1}, {-1, 0, 0}, {1, 0}},
        {{-0.5,  0.5,  0.5}, {0, 0, -1}, {-1, 0, 0}, {0, 0}},
        // Left face
        {{ 0.5, -0.5,  0.5}, {-1, 0, 0}, {0, 0, 1}, {0, 1}},
        {{-0.5, -0.5,  0.5}, {-1, 0, 0}, {0, 0, 1}, {1, 1}},
        {{-0.5,  0.5,  0.5}, {-1, 0, 0}, {0, 0, 1}, {1, 0}},
        {{ 0.5,  0.5,  0.5}, {-1, 0, 0}, {0, 0, 1}, {0, 0}},
        // Right face
        {{-0.5, -0.5, -0.5}, {1, 0, 0}, {0, 0, -1}, {0, 1}},
        {{ 0.5, -0.5, -0.5}, {1, 0, 0}, {0, 0, -1}, {1, 1}},
        {{ 0.5,  0.5, -0.5}, {1, 0, 0}, {0, 0, -1}, {1, 0}},
        {{-0.5,  0.5, -0.5}, {1, 0, 0}, {0, 0, -1}, {0, 0}}
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
    params.Params = { 64.0f, 0.0f, 0, 0 };

    D3D11_SUBRESOURCE_DATA lightParamsData = {};
    lightParamsData.pSysMem = &params;
    lightParamsData.SysMemPitch = sizeof(LightParams);
    result = m_pDevice->CreateBuffer(&lightParamsBufferDesc, &lightParamsData, &m_pLightingParamBuffer);

    if (FAILED(result))
        return false;

    result = SetResourceName(m_pLightingParamBuffer, "lighting params buffer");

    D3D11_BUFFER_DESC desc;
    desc.ByteWidth = sizeof(D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS);
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = sizeof(UINT);

    result = m_pDevice->CreateBuffer(&desc, nullptr, &m_pIndirectArgsSrc);
    if (SUCCEEDED(result))
    {
        result = SetResourceName(m_pIndirectArgsSrc, "indirect args src buffer");
    }
    if (SUCCEEDED(result))
    {
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Buffer.NumElements = sizeof(D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS) / sizeof(UINT);
        uavDesc.Buffer.Flags = 0;

        result = m_pDevice->CreateUnorderedAccessView(m_pIndirectArgsSrc, &uavDesc, &m_pIndirectArgsUAV);
    }
    if (SUCCEEDED(result))
    {
        result = SetResourceName(m_pIndirectArgsSrc, "indirect args UAV");
    }

    if (SUCCEEDED(result))
    {
        D3D11_BUFFER_DESC desc;
        desc.ByteWidth = sizeof(D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS);
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = 0;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
        desc.StructureByteStride = 0;

        result = m_pDevice->CreateBuffer(&desc, nullptr, &m_pIndirectArgs);
        if (SUCCEEDED(result))
        {
            result = SetResourceName(m_pIndirectArgsSrc, "IndirectArgs");
        }
    }

    if (SUCCEEDED(result))
    {

        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = sizeof(GeomBufferInst) * MAX_INST;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        desc.StructureByteStride = sizeof(GeomBufferInst);

        result = m_pDevice->CreateBuffer(&desc, nullptr, &m_pGeomBufferInstGPU);
        if (SUCCEEDED(result))
        {
            result = SetResourceName(m_pGeomBufferInstGPU, "instance buffer GPU");
        }
        if (SUCCEEDED(result))
        {
            D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
            uavDesc.Format = DXGI_FORMAT_UNKNOWN;
            uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
            uavDesc.Buffer.FirstElement = 0;
            uavDesc.Buffer.NumElements = MAX_INST;
            uavDesc.Buffer.Flags = 0;

            result = m_pDevice->CreateUnorderedAccessView(m_pGeomBufferInstGPU, &uavDesc, &m_pGeomBufferInstGPU_UAV);
        }
        if (SUCCEEDED(result))
        {
            result = SetResourceName(m_pIndirectArgsSrc, "instance buffer GPU_UAV");
        }
    }

    return true;
}

bool TexturedCube::initInputLayout()
{
    D3D11_INPUT_ELEMENT_DESC inputDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
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
        result = compileShader(m_pDevice, L"resources/shaders/frustum_cull_cs.hlsl", {}, shader_stage::Compute, (ID3D11DeviceChild**)&m_pComputeShader);
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
    std::vector<DirectX::TexMetadata> textureInfo;
    std::vector<DirectX::ScratchImage> textures;
    textureInfo.resize(2);
    textures.resize(2);
    HRESULT result = S_OK;
    result = DirectX::LoadFromDDSFile(L"resources/textures/logo.dds", DirectX::DDS_FLAGS_NONE, &textureInfo[0], textures[0]);
    result = DirectX::LoadFromDDSFile(L"resources/textures/tiles.dds", DirectX::DDS_FLAGS_NONE, &textureInfo[1], textures[1]);

    D3D11_TEXTURE2D_DESC txDesc = {};
    txDesc.ArraySize = 2;
    txDesc.Format = textureInfo[0].format;
    txDesc.MipLevels = textureInfo[0].mipLevels;
    txDesc.Usage = D3D11_USAGE_IMMUTABLE;
    txDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    txDesc.CPUAccessFlags = 0;
    txDesc.MiscFlags = 0;
    txDesc.SampleDesc.Count = 1;
    txDesc.SampleDesc.Quality = 0;
    txDesc.Height = textureInfo[0].height;
    txDesc.Width = textureInfo[0].width;

    std::vector<D3D11_SUBRESOURCE_DATA> data;
    data.resize(textureInfo[0].mipLevels * 2);
    for (UINT32 j = 0; j < 2; j++)
    {
        UINT32 blockWidth = (txDesc.Width + 4u - 1u) / 4u;
        UINT32 blockHeight = (txDesc.Height + 4u - 1u) / 4u;
        UINT32 pitch = blockWidth * 8; // 8 bytes for DXGI_FORMAT_BC1_UNORM
        const char* pSrcData = reinterpret_cast<const char*>(textures[j].GetPixels());

        for (UINT32 i = 0; i < textureInfo[j].mipLevels; i++)
        {
            data[j * textureInfo[j].mipLevels + i].pSysMem = pSrcData;
            data[j * textureInfo[j].mipLevels + i].SysMemPitch = pitch;
            data[j * textureInfo[j].mipLevels + i].SysMemSlicePitch = 0;

            pSrcData += pitch * blockHeight;
            blockHeight = 1u > blockHeight / 2 ? 1u : blockHeight / 2;
            blockWidth = 1u > blockHeight / 2 ? 1u : blockWidth / 2;
            pitch = blockWidth * 8;
        }
    }
    ID3D11Texture2D* texture;
    texture = nullptr;
    result = m_pDevice->CreateTexture2D(&txDesc, &data[0], &texture);
    if (FAILED(result))
    {
        return false;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
    desc.Format = textureInfo[0].format;
    desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    desc.Texture2DArray.ArraySize = 2;
    desc.Texture2DArray.FirstArraySlice = 0;
    desc.Texture2DArray.MipLevels = textureInfo[0].mipLevels;
    desc.Texture2DArray.MostDetailedMip = 0;

    result = m_pDevice->CreateShaderResourceView(texture, &desc, &m_pSRV);

    texture->Release();
    
    /*result = DirectX::CreateDDSTextureFromFile(
        m_pDevice,
        L"resources/textures/tiles.dds",
        nullptr,
        &m_pSRV
    );

    if (FAILED(result))
    {
        return false;
    }*/

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

bool TexturedCube::initInstances()
{
    const float diag = sqrtf(2.0f) / 2.0f * 0.5f;

    geomBuffers.resize(MAX_INST);
    AABB.resize(MAX_INST);

    geomBuffers[0].M = DirectX::XMMatrixIdentity();
    geomBuffers[0].NormalM = DirectX::XMMatrixIdentity();
    geomBuffers[0].params.x = 64;
    geomBuffers[0].params.y = 1;
    geomBuffers[0].params.z = 1;
    geomBuffers[0].params.w = 1;
    AABB[0].first = {-diag, -0.5, -diag};
    AABB[0].second = { diag, 0.5, diag };

    geomBuffers[1].M = DirectX::XMMatrixTranslation(2.0, 0.0, 2.0);
    geomBuffers[1].NormalM = DirectX::XMMatrixIdentity();
    geomBuffers[1].params.x = 100;
    geomBuffers[1].params.y = 0;
    geomBuffers[1].params.z = 0;
    geomBuffers[1].params.w = 1;
    AABB[1].first = { 2.0f - diag, -0.5f, 2.0f - diag };
    AABB[1].second = { 2.0f + diag, 0.5f, 2.0f + diag };

    for (int i = 2; i < MAX_INST; i++)
    {
        DirectX::XMFLOAT3 pos = { float(randNormf() * 10 - 5.0), float(randNormf() * 10 - 5.0), float(randNormf() * 10 - 5.0) };
        geomBuffers[i].M = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
        geomBuffers[i].NormalM = DirectX::XMMatrixIdentity();
        geomBuffers[i].params.x = 50;
        geomBuffers[i].params.z = rand() % 2;
        geomBuffers[i].params.y = geomBuffers[i].params.z ? 1 : 0;
        geomBuffers[i].params.w = 1;
        AABB[i].first = {pos.x - diag, pos.y - 0.5f, pos.z - diag};
        AABB[i].second = { pos.x + diag, pos.y + 0.5f, pos.z + diag };
    }

    D3D11_BUFFER_DESC instanceBufferDesc = {};
    instanceBufferDesc.ByteWidth = sizeof(GeomBufferInst) * MAX_INST;
    instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    instanceBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    instanceBufferDesc.CPUAccessFlags = 0;
    instanceBufferDesc.MiscFlags = 0;
    instanceBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA instanceData = {};
    instanceData.pSysMem = &geomBuffers[0];
    instanceData.SysMemPitch = sizeof(GeomBufferInst) * MAX_INST;
    HRESULT result = m_pDevice->CreateBuffer(&instanceBufferDesc, &instanceData, &m_pGeomBufferInst);

    if (FAILED(result))
        return false;

    result = SetResourceName(m_pGeomBufferInst, "instance buffer");

    result = m_pDevice->CreateBuffer(&instanceBufferDesc, &instanceData, &m_pGeomBufferInstCompute);

    if (FAILED(result))
        return false;

    result = SetResourceName(m_pGeomBufferInst, "instance buffer for compute");

    D3D11_BUFFER_DESC cullParamsDesc;
    cullParamsDesc.ByteWidth = sizeof(CullParams);
    cullParamsDesc.Usage = D3D11_USAGE_DEFAULT;
    cullParamsDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cullParamsDesc.CPUAccessFlags = 0;
    cullParamsDesc.MiscFlags = 0;
    cullParamsDesc.StructureByteStride = 0;

    CullParams cp;
    for (int i = 0; i < MAX_INST; i++)
    {
        cp.bbMin[i] = { AABB[i].first.x, AABB[i].first.y, AABB[i].first.z, 0 };
        cp.bbMax[i] = { AABB[i].second.x, AABB[i].second.y, AABB[i].second.z, 0 };
    }

    D3D11_SUBRESOURCE_DATA cullData = {};
    cullData.pSysMem = &cp;
    cullData.SysMemPitch = sizeof(CullParams);
    result = m_pDevice->CreateBuffer(&cullParamsDesc, &cullData, &m_pCullParams);
    if (SUCCEEDED(result))
    {
        result = SetResourceName(m_pCullParams, "cull params buffer");
    }

    return true;
}

void TexturedCube::terminate()
{
    
    if (m_pGeomBufferInst != nullptr)
    {
        m_pGeomBufferInst->Release();
        m_pGeomBufferInst = nullptr;
    }

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
