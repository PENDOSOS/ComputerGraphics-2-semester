#include "Postprocess.h"

Postprocess::Postprocess(ID3D11Device* device, int width, int height)
    : m_pDevice(device)
    , m_pPixelShader(nullptr)
    , m_pVertexShader(nullptr)
    , m_pScene(nullptr)
    , m_pSceneRTV(nullptr)
    , m_pSceneSRV(nullptr)
    , m_width(width)
    , m_height(height)
{
    initShaders();
    initResources();
}

Postprocess::~Postprocess()
{
	terminate();
}

void Postprocess::render(ID3D11DeviceContext* context, ID3D11RenderTargetView* backBuffer, ID3D11SamplerState* sampler)
{
    ID3D11RenderTargetView* views[] = { backBuffer };
    context->OMSetRenderTargets(1, views, nullptr);
    ID3D11SamplerState* samplers[] = { sampler };
    context->PSSetSamplers(0, 1, samplers);
    ID3D11ShaderResourceView* textures[] = { m_pSceneSRV };
    context->PSSetShaderResources(0, 1, textures);
    context->OMSetDepthStencilState(nullptr, 0);
    context->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
    context->RSSetState(nullptr);
    context->IASetInputLayout(nullptr);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->VSSetShader(m_pVertexShader, nullptr, 0);
    context->PSSetShader(m_pPixelShader, nullptr, 0);
    context->Draw(3, 0);
}

bool Postprocess::reinit(int width, int height)
{
    m_width = width;
    m_height = height;

    if (m_pSceneSRV != nullptr)
    {
        m_pSceneSRV->Release();
        m_pSceneSRV = nullptr;
    }

    if (m_pSceneRTV != nullptr)
    {
        m_pSceneRTV->Release();
        m_pSceneRTV = nullptr;
    }

    if (m_pScene != nullptr)
    {
        m_pScene->Release();
        m_pScene = nullptr;
    }

    if (!initResources())
        return false;

    return true;
}

bool Postprocess::initShaders()
{
    HRESULT result = S_OK;

    if (SUCCEEDED(result))
    {
        result = compileShader(m_pDevice, L"resources/shaders/filter_vs.hlsl", {}, shader_stage::Vertex, (ID3D11DeviceChild**)&m_pVertexShader);
    }
    if (SUCCEEDED(result))
    {
        result = compileShader(m_pDevice, L"resources/shaders/filter_ps.hlsl", {}, shader_stage::Pixel, (ID3D11DeviceChild**)&m_pPixelShader);
    }

	return false;
}

bool Postprocess::initResources()
{
    D3D11_TEXTURE2D_DESC filterDesc = {};
    filterDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    filterDesc.ArraySize = 1;
    filterDesc.CPUAccessFlags = 0;
    filterDesc.MipLevels = 0;
    filterDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    filterDesc.SampleDesc.Count = 1;
    filterDesc.SampleDesc.Quality = 0;
    filterDesc.Usage = D3D11_USAGE_DEFAULT;
    filterDesc.Height = m_height;
    filterDesc.Width = m_width;
    filterDesc.MipLevels = 1;

    HRESULT result = m_pDevice->CreateTexture2D(&filterDesc, nullptr, &m_pScene);
    if (FAILED(result))
    {
        return false;
    }

    //result = SetResourceName(m_pScene, "scene texture");

    // we will draw scene in this RTV
    result = m_pDevice->CreateRenderTargetView(m_pScene, nullptr, &m_pSceneRTV);
    if (FAILED(result))
    {
        return false;
    }

    //result = SetResourceName(m_pSceneRTV, "scene render target");

    result = m_pDevice->CreateShaderResourceView(m_pScene, nullptr, &m_pSceneSRV);
    if (FAILED(result))
    {
        return false;
    }

    //result = SetResourceName(m_pSceneRTV, "scene shader resource");

	return true;
}

void Postprocess::terminate()
{
    if (m_pSceneSRV != nullptr)
    {
        m_pSceneSRV->Release();
        m_pSceneSRV = nullptr;
    }

    if (m_pSceneRTV != nullptr)
    {
        m_pSceneRTV->Release();
        m_pSceneRTV = nullptr;
    }

    if (m_pScene != nullptr)
    {
        m_pScene->Release();
        m_pScene = nullptr;
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
}
