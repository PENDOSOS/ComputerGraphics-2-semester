#include "framework.h"

#include "Render.h"

#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"

#pragma comment(lib, "dxgi.lib") // don't work on my computer without these libs
#pragma comment(lib, "d3d11.lib") // uncomment if it doesn't launch

using namespace DirectX;

static const float rotationSpeed = PI / 6;
static const float sensitivity = PI;

bool Render::init(HWND window)
{
    HRESULT result;

    // Create a DirectX graphics interface factory.
    IDXGIFactory* pFactory = nullptr;
    result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);

    // Select hardware adapter
    IDXGIAdapter* pSelectedAdapter = NULL;
    if (SUCCEEDED(result))
    {
        IDXGIAdapter* pCurrentAdapter = NULL;
        UINT adapterIdx = 0;
        while (SUCCEEDED(pFactory->EnumAdapters(adapterIdx, &pCurrentAdapter)))
        {
            DXGI_ADAPTER_DESC desc;
            pCurrentAdapter->GetDesc(&desc);

            if (wcscmp(desc.Description, L"Microsoft Basic Render Driver") != 0)
            {
                pSelectedAdapter = pCurrentAdapter;
                break;
            }

            pCurrentAdapter->Release();

            adapterIdx++;
        }
    }
    assert(pSelectedAdapter != NULL);

    // Create DirectX 11 device
    D3D_FEATURE_LEVEL level;
    D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
    if (SUCCEEDED(result))
    {
        UINT flags = 0;
#ifdef _DEBUG
        flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        result = D3D11CreateDevice(pSelectedAdapter, D3D_DRIVER_TYPE_UNKNOWN, NULL,
            flags, levels, 1, D3D11_SDK_VERSION, &m_pDevice, &level, &m_pDeviceContext);
        assert(level == D3D_FEATURE_LEVEL_11_0);
        assert(SUCCEEDED(result));
    }

    // Create swapchain
    if (SUCCEEDED(result))
    {
        DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
        swapChainDesc.BufferCount = 2;
        swapChainDesc.BufferDesc.Width = m_width;
        swapChainDesc.BufferDesc.Height = m_height;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.OutputWindow = window;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.Windowed = true;
        swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapChainDesc.Flags = 0;

        result = pFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
        assert(SUCCEEDED(result));
    }

    if (SUCCEEDED(result))
    {
        result = setupBackBuffer();
    }

    if (pSelectedAdapter != nullptr)
    {
        pSelectedAdapter->Release();
        pSelectedAdapter = nullptr;
    }

    if (pFactory != nullptr)
    {
        pFactory->Release();
        pFactory = nullptr;
    }

    result = initScene();
    if (!SUCCEEDED(result))
        return result;

    result = initSamplers();
    if (!SUCCEEDED(result))
        return result;

    result = initDepthStencil();
    if (!SUCCEEDED(result))
        return result;

    result = initBlendState();
    if (!SUCCEEDED(result))
        return result;

    if (SUCCEEDED(result))
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        ImGui_ImplWin32_Init(window);
        ImGui_ImplDX11_Init(m_pDevice, m_pDeviceContext);

        m_sceneBuffer.light.Color = { 1, 1, 0, 0 };
        m_sceneBuffer.light.Pos = { 0.2f, 0.7f, 0.2f, 0 };
        m_sceneBuffer.AmbientColor = { 0.57 * 1.0f / 3.0f, 0.541 * 1.0f / 3.0f, 0.722 * 1.0f / 4.0f, 1.0 };
    }

    //m_pTriangle = new Triangle(m_pDevice);
    m_pCamera = new Camera;
    //m_pCube = new Cube(m_pDevice);
    m_pCube = new TexturedCube(m_pDevice);
    m_pCube2 = new TexturedCube(m_pDevice);
    m_pSkybox = new Skybox(m_pDevice);
    m_pRect1 = new TransparentRect(m_pDevice, 1.0f, 0, 0, 128);
    m_pRect2 = new TransparentRect(m_pDevice, -1.0f, 128, 0, 0);

    return SUCCEEDED(result);
}

void Render::terminate()
{
    delete m_pCamera;
    //delete m_pTriangle;
    delete m_pCube;
    delete m_pCube2;
    delete m_pSkybox;
    delete m_pRect1;
    delete m_pRect2;

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    
    if (m_pSamplerState != nullptr)
    {
        m_pSamplerState->Release();
        m_pSamplerState = nullptr;
    }

    if (m_pGeomBuffer2 != nullptr)
    {
        m_pGeomBuffer2->Release();
        m_pGeomBuffer2 = nullptr;
    }

    if (m_pGeomBuffer != nullptr)
    {
        m_pGeomBuffer->Release();
        m_pGeomBuffer = nullptr;
    }

    if (m_pSceneBuffer != nullptr)
    {
        m_pSceneBuffer->Release();
        m_pSceneBuffer = nullptr;
    }

    if (m_pTransparentBlendState != nullptr)
    {
        m_pTransparentBlendState->Release();
        m_pTransparentBlendState = nullptr;
    }

    if (m_pBlendState != nullptr)
    {
        m_pBlendState->Release();
        m_pBlendState = nullptr;
    }

    if (m_pTransparentDepthState != nullptr)
    {
        m_pTransparentDepthState->Release();
        m_pTransparentDepthState = nullptr;
    }

    if (m_pDepthState != nullptr)
    {
        m_pDepthState->Release();
        m_pDepthState = nullptr;
    }

    if (m_pDepthBufferDSV != nullptr)
    {
        m_pDepthBufferDSV->Release();
        m_pDepthBufferDSV = nullptr;
    }

    if (m_pDepthBuffer != nullptr)
    {
        m_pDepthBuffer->Release();
        m_pDepthBuffer = nullptr;
    }
    
    if (m_pRasterizerState != nullptr)
    {
        m_pRasterizerState->Release();
        m_pRasterizerState = nullptr;
    }

    if (m_pBackBufferRTV != nullptr)
    {
        m_pBackBufferRTV->Release();
        m_pBackBufferRTV = nullptr;
    }

    if (m_pSwapChain != nullptr)
    {
        m_pSwapChain->Release();
        m_pSwapChain = nullptr;
    }

    if (m_pDeviceContext != nullptr)
    {
        m_pDeviceContext->Release();
        m_pDeviceContext = nullptr;
    }

    if (m_pDevice != nullptr)
    {
        m_pDevice->Release();
        m_pDevice = nullptr;
    }

}

bool Render::render()
{
    m_pDeviceContext->ClearState();

    ID3D11RenderTargetView* views[] = { m_pBackBufferRTV };
    m_pDeviceContext->OMSetRenderTargets(1, views, m_pDepthBufferDSV);

    static const FLOAT BackColor[4] = { 0.25f, 0.25f, 0.25f, 1.0f };
    m_pDeviceContext->ClearRenderTargetView(m_pBackBufferRTV, BackColor);
    m_pDeviceContext->ClearDepthStencilView(m_pDepthBufferDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (FLOAT)m_width;
    viewport.Height = (FLOAT)m_height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    m_pDeviceContext->RSSetViewports(1, &viewport);

    D3D11_RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = m_width;
    rect.bottom = m_height;
    m_pDeviceContext->RSSetScissorRects(1, &rect);
    m_pDeviceContext->RSSetState(m_pRasterizerState);
    m_pDeviceContext->OMSetDepthStencilState(m_pDepthState, 0);
    m_pDeviceContext->OMSetBlendState(m_pBlendState, nullptr, 0xFFFFFFFF);

    //m_pTriangle->render(m_pDeviceContext, m_width, m_height);

    m_pCube->render(m_pDeviceContext, m_pSceneBuffer, m_pGeomBuffer, m_pSamplerState);
    m_pCube2->render(m_pDeviceContext, m_pSceneBuffer, m_pGeomBuffer2, m_pSamplerState);

    m_pSkybox->render(m_pDeviceContext, m_width, m_height, m_pSceneBuffer, m_pSamplerState);

    m_pDeviceContext->OMSetDepthStencilState(m_pTransparentDepthState, 0);
    m_pDeviceContext->OMSetBlendState(m_pTransparentBlendState, nullptr, 0xFFFFFFFF);
    //m_pDeviceContext->OMSetDepthStencilState(m_pTransparentDepthState, 0);

    //drawTransparentSorted();
    /*m_pRect1->render(m_pDeviceContext, m_pSceneBuffer);
    m_pRect2->render(m_pDeviceContext, m_pSceneBuffer);*/

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    {
        bool a = true;
        ImGui::Begin("Lights");

        ImGui::Checkbox("Show bulbs", &a);
        ImGui::Checkbox("Use normal maps", &a);
        ImGui::Checkbox("Show normals", &a);

        //m_sceneBuffer.lightCount.y = m_useNormalMaps ? 1 : 0;
        //m_sceneBuffer.lightCount.z = m_showNormals ? 1 : 0;

        bool add = ImGui::Button("+");
        ImGui::SameLine();
        bool remove = ImGui::Button("-");

        /*if (add && m_sceneBuffer.lightCount.x < 10)
        {
            ++m_sceneBuffer.lightCount.x;
            m_sceneBuffer.lights[m_sceneBuffer.lightCount.x - 1] = Light();
        }
        if (remove && m_sceneBuffer.lightCount.x > 0)
        {
            --m_sceneBuffer.lightCount.x;
        }*/

        char buffer[1024];
        for (int i = 0; i < 1; i++)
        {
            ImGui::Text("Light %d", i);
            sprintf_s(buffer, "Pos %d", i);
            ImGui::DragFloat3(buffer, (float*)&m_sceneBuffer.light.Pos, 0.1f, -10.0f, 10.0f);
            sprintf_s(buffer, "Color %d", i);
            ImGui::ColorEdit3(buffer, (float*)&m_sceneBuffer.light.Color);
        }

        ImGui::End();
    }

    // Rendering
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HRESULT result = m_pSwapChain->Present(0, 0);
    assert(SUCCEEDED(result));

    return SUCCEEDED(result);
}

bool Render::resize(UINT width, UINT height)
{
    if (width != m_width || height != m_height)
    {
        if (m_pBackBufferRTV != nullptr)
        {
            m_pBackBufferRTV->Release();
            m_pBackBufferRTV = nullptr;
        }

        if (m_pDepthBuffer != nullptr)
        {
            m_pDepthBuffer->Release();
            m_pDepthBuffer = nullptr;
        }

        if (m_pDepthBufferDSV != nullptr)
        {
            m_pDepthBufferDSV->Release();
            m_pDepthBufferDSV = nullptr;
        }

        HRESULT result = m_pSwapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
        assert(SUCCEEDED(result));

        if (SUCCEEDED(result))
        {
            m_width = width;
            m_height = height;

            result = setupBackBuffer();
            result = initDepthStencil();
        }

        return SUCCEEDED(result);
    }

    return true;
}

bool Render::update()
{
    size_t usec = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    if (m_prevSec == 0)
    {
        m_prevSec = usec; // Initial update
    }

    double deltaSec = (usec - m_prevSec) / 1000000.0;
    m_angle = m_angle + deltaSec * rotationSpeed;

    GeomBuffer geomBuffer;

    // Model matrix
    // Angle is reversed, as DirectXMath calculates it as clockwise
    DirectX::XMMATRIX m = DirectX::XMMatrixRotationAxis(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), -(float)m_angle);

    geomBuffer.M = m;
    m = DirectX::XMMatrixInverse(nullptr, m);
    m = DirectX::XMMatrixTranspose(m);
    geomBuffer.NormalM = m;
    geomBuffer.params.x = 64.0f;
    geomBuffer.params.y = 1;

    m_pDeviceContext->UpdateSubresource(m_pGeomBuffer, 0, nullptr, &geomBuffer, 0, 0);
   
    m_prevSec = usec;

    // Setup camera
    DirectX::XMFLOAT3 cameraPos;
    DirectX::XMMATRIX v;
    {
        DirectX::XMFLOAT3 pos = m_pCamera->poi;
        DirectX::XMFLOAT3 temp = DirectX::XMFLOAT3{ cosf(m_pCamera->theta) * cosf(m_pCamera->phi), sinf(m_pCamera->theta), cosf(m_pCamera->theta) * sinf(m_pCamera->phi) };
        DirectX::XMVECTOR vector = XMLoadFloat3(&temp);
        vector = XMVectorScale(vector, m_pCamera->r);
        DirectX::XMVECTOR posVector = XMLoadFloat3(&pos);
        posVector = XMVectorAdd(posVector, vector);
        XMStoreFloat3(&pos, posVector);
        float upTheta = m_pCamera->theta + (float)PI / 2;
        DirectX::XMFLOAT3 up = DirectX::XMFLOAT3{ cosf(upTheta) * cosf(m_pCamera->phi), sinf(upTheta), cosf(upTheta) * sinf(m_pCamera->phi) };

        v = DirectX::XMMatrixLookAtLH(
            DirectX::XMVectorSet(pos.x, pos.y, pos.z, 0.0f),
            DirectX::XMVectorSet(m_pCamera->poi.x, m_pCamera->poi.y, m_pCamera->poi.z, 0.0f),
            DirectX::XMVectorSet(up.x, up.y, up.z, 0.0f)
        );

        XMStoreFloat3(&cameraPos, posVector);
    }

    float f = 100.0f;
    float n = 0.1f;
    float fov = (float)PI / 3;
    float c = 1.0f / tanf(fov / 2);
    float aspectRatio = (float)m_height / m_width;
    DirectX::XMMATRIX p = DirectX::XMMatrixPerspectiveLH(tanf(fov / 2) * 2 * n, tanf(fov / 2) * 2 * n * aspectRatio, n, f);

    D3D11_MAPPED_SUBRESOURCE subresource;
    HRESULT result = m_pDeviceContext->Map(m_pSceneBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
    assert(SUCCEEDED(result));
    if (SUCCEEDED(result))
    {
        //m_sceneBuffer = reinterpret_cast<SceneBuffer*>(subresource.pData);

        m_sceneBuffer.VP = DirectX::XMMatrixMultiply(v, p);
        m_sceneBuffer.CameraPos = cameraPos;
        
        memcpy(subresource.pData, &m_sceneBuffer, sizeof(SceneBuffer));

        m_pDeviceContext->Unmap(m_pSceneBuffer, 0);
    }

    return SUCCEEDED(result);
}

void Render::mouseLeftButton(bool pressed, int posX, int posY)
{
    m_isButtonPressed = pressed;
    if (m_isButtonPressed)
    {
        m_mousePosX = posX;
        m_mousePosY = posY;
    }
}

void Render::mouseMove(int posX, int posY)
{
    if (m_isButtonPressed)
    {
        float dx = -(float)(posX - m_mousePosX) / m_width * sensitivity;
        float dy = (float)(posY - m_mousePosY) / m_width * sensitivity;

        m_pCamera->phi += dx;
        m_pCamera->theta += dy;

        if (m_pCamera->theta > PI / 2)
            m_pCamera->theta = PI / 2;
        if (m_pCamera->theta < -PI / 2)
            m_pCamera->theta = -PI / 2;

        m_mousePosX = posX;
        m_mousePosY = posY;
    }
}

void Render::mouseWheel(int delta)
{
    m_pCamera->r -= delta / 100.0f;
    if (m_pCamera->r < 1.0f)
    {
        m_pCamera->r = 1.0f;
    }
}

HRESULT Render::setupBackBuffer()
{
    ID3D11Texture2D* pBackBuffer = NULL;
    HRESULT result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
    assert(SUCCEEDED(result));
    if (SUCCEEDED(result))
    {
        result = m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pBackBufferRTV);
        assert(SUCCEEDED(result));

        if (pBackBuffer != nullptr)
        {
            pBackBuffer->Release();
            pBackBuffer = nullptr;
        }
    }

    return result;
}

HRESULT Render::initScene()
{
    D3D11_BUFFER_DESC sceneBufferDesc = {};
    sceneBufferDesc.ByteWidth = sizeof(SceneBuffer);
    sceneBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    sceneBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    sceneBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    sceneBufferDesc.MiscFlags = 0;
    sceneBufferDesc.StructureByteStride = 0;

    HRESULT result = m_pDevice->CreateBuffer(&sceneBufferDesc, nullptr, &m_pSceneBuffer);
    assert(SUCCEEDED(result));

    if (!SUCCEEDED(result))
    {
        return result;
    }
    result = SetResourceName(m_pSceneBuffer, "scene buffer");

    D3D11_BUFFER_DESC geomBufferDesc = {};
    geomBufferDesc.ByteWidth = sizeof(GeomBuffer);
    geomBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    geomBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    geomBufferDesc.CPUAccessFlags = 0;
    geomBufferDesc.MiscFlags = 0;
    geomBufferDesc.StructureByteStride = 0;

    GeomBuffer geomBuffer;
    geomBuffer.M = DirectX::XMMatrixIdentity();
    geomBuffer.NormalM = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixIdentity()));
    geomBuffer.params.x = 64;

    D3D11_SUBRESOURCE_DATA data;
    data.pSysMem = &geomBuffer;
    data.SysMemPitch = sizeof(geomBuffer);
    data.SysMemSlicePitch = 0;

    result = m_pDevice->CreateBuffer(&geomBufferDesc, &data, &m_pGeomBuffer);
    assert(SUCCEEDED(result));
    if (!SUCCEEDED(result))
    {
        return result;
    }
    result = SetResourceName(m_pGeomBuffer, "geom buffer");

    GeomBuffer geomBuffer2;
    geomBuffer2.M = DirectX::XMMatrixTranslation(2.0f, 0.0f, 2.0f);
    geomBuffer2.NormalM = DirectX::XMMatrixIdentity();
    geomBuffer2.params.x = 64;
    //geomBuffer2.params.x = 64;
    data.pSysMem = &geomBuffer2;

    result = m_pDevice->CreateBuffer(&geomBufferDesc, &data, &m_pGeomBuffer2);
    assert(SUCCEEDED(result));
    if (!SUCCEEDED(result))
    {
        return result;
    }
    result = SetResourceName(m_pGeomBuffer2, "geom buffer 2");

    if (SUCCEEDED(result))
    {
        D3D11_RASTERIZER_DESC desc = {};
        desc.AntialiasedLineEnable = TRUE;
        desc.FillMode = D3D11_FILL_SOLID;
        desc.CullMode = D3D11_CULL_NONE;
        desc.FrontCounterClockwise = FALSE;
        desc.DepthBias = 0;
        desc.SlopeScaledDepthBias = 0.0f;
        desc.DepthBiasClamp = 0.0f;
        desc.DepthClipEnable = TRUE;
        desc.ScissorEnable = FALSE;
        desc.MultisampleEnable = FALSE;

        result = m_pDevice->CreateRasterizerState(&desc, &m_pRasterizerState);
        assert(SUCCEEDED(result));
    }

    if (!SUCCEEDED(result))
    {
        return result;
    }
    result = SetResourceName(m_pRasterizerState, "rasterizer state");
    
    D3D11_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = TRUE;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    depthDesc.StencilEnable = FALSE;

    result = m_pDevice->CreateDepthStencilState(&depthDesc, &m_pDepthState);
    if (!SUCCEEDED(result))
    {
        return result;
    }
    result = SetResourceName(m_pDepthState, "depth state");

    D3D11_DEPTH_STENCIL_DESC transparentDepthDesc = {};
    transparentDepthDesc.DepthEnable = TRUE;
    transparentDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    transparentDepthDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    transparentDepthDesc.StencilEnable = FALSE;

    result = m_pDevice->CreateDepthStencilState(&transparentDepthDesc, &m_pTransparentDepthState);
    if (!SUCCEEDED(result))
    {
        return result;
    }
    result = SetResourceName(m_pTransparentDepthState, "depth state");

    return result;
}

HRESULT Render::initSamplers()
{
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;//D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;//D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;//D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MinLOD = -FLT_MAX;
    samplerDesc.MaxLOD = FLT_MAX;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 16;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.BorderColor[0] = samplerDesc.BorderColor[1] = samplerDesc.BorderColor[2] = samplerDesc.BorderColor[3] = 1.0f;

    HRESULT result = m_pDevice->CreateSamplerState(&samplerDesc, &m_pSamplerState);

    if (SUCCEEDED(result))
    {
        result = SetResourceName(m_pSamplerState, "sampler state");
    }

    return result;
}

HRESULT Render::initDepthStencil()
{
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Format = DXGI_FORMAT_D32_FLOAT;
    desc.ArraySize = 1;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.Height = m_height;
    desc.Width = m_width;
    desc.MipLevels = 1;

    HRESULT result = m_pDevice->CreateTexture2D(&desc, nullptr, &m_pDepthBuffer);
    
    if (FAILED(result))
    {
        return result;
    }

    result = SetResourceName(m_pDepthBuffer, "depth buffer");

    result = m_pDevice->CreateDepthStencilView(m_pDepthBuffer, nullptr, &m_pDepthBufferDSV);

    if (FAILED(result))
    {
        return result;
    }

    result = SetResourceName(m_pDepthBufferDSV, "depth buffer view");

    return result;
}

HRESULT Render::initBlendState()
{
    D3D11_BLEND_DESC desc = {};
    desc.AlphaToCoverageEnable = FALSE;
    desc.IndependentBlendEnable = FALSE;
    desc.RenderTarget[0].BlendEnable = FALSE;
    desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;

    HRESULT result = m_pDevice->CreateBlendState(&desc, &m_pBlendState);

    if (SUCCEEDED(result))
    {
        result = SetResourceName(m_pBlendState, "blend state");
    }

    desc.RenderTarget[0].BlendEnable = TRUE;

    result = m_pDevice->CreateBlendState(&desc, &m_pTransparentBlendState);

    if (SUCCEEDED(result))
    {
        result = SetResourceName(m_pTransparentBlendState, "blend state");
    }

    return result;
}

void Render::drawTransparentSorted()
{
    float d0 = 0.0f, d1 = 0.0f;
    DirectX::XMFLOAT3 cameraPos;
    cameraPos.x = m_pCamera->poi.x + cosf(m_pCamera->theta) * cosf(m_pCamera->phi) * m_pCamera->r;
    cameraPos.y = m_pCamera->poi.y + sinf(m_pCamera->theta) * m_pCamera->r;
    cameraPos.z = m_pCamera->poi.z + cosf(m_pCamera->theta)* sinf(m_pCamera->phi)* m_pCamera->r;

    for (int i = 0; i < 4; i++)
    {
        d0 = (std::max)(d0, (float)(pow((cameraPos.x - m_pRect1->coords[i].x), 2)
                                  + pow((cameraPos.y - m_pRect1->coords[i].y), 2) 
                                  + pow((cameraPos.z - m_pRect1->coords[i].z), 2)));
        d1 = (std::max)(d1, (float)(pow((cameraPos.x - m_pRect2->coords[i].x), 2)
                                  + pow((cameraPos.y - m_pRect2->coords[i].y), 2)
                                  + pow((cameraPos.z - m_pRect2->coords[i].z), 2)));
    }

    if (d0 < d1)
    {
        m_pRect2->render(m_pDeviceContext, m_pSceneBuffer);
        m_pRect1->render(m_pDeviceContext, m_pSceneBuffer);
    }
    else
    {
        m_pRect1->render(m_pDeviceContext, m_pSceneBuffer);
        m_pRect2->render(m_pDeviceContext, m_pSceneBuffer);
    }
}
