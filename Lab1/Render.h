#pragma once

#include <dxgi.h>
#include <d3d11.h>

class Render
{
public:
    Render()
        : m_pDevice(nullptr)
        , m_pDeviceContext(nullptr)
        , m_pSwapChain(nullptr)
        , m_pBackBufferRTV(nullptr)
        , m_width(16)
        , m_height(16)
    {}

    ~Render() { terminate(); }

    bool init(HWND window);
    

    bool render();
    bool resize(UINT width, UINT height);

private:
    void terminate();

    HRESULT setupBackBuffer();

private:
    ID3D11Device* m_pDevice;
    ID3D11DeviceContext* m_pDeviceContext;

    IDXGISwapChain* m_pSwapChain;
    ID3D11RenderTargetView* m_pBackBufferRTV;

    UINT m_width;
    UINT m_height;
};

