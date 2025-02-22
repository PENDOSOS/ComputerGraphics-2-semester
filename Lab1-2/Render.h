#pragma once

#include "Triangle.h"

#define PI 3.14159265358979323846

struct SceneBuffer
{
    DirectX::XMMATRIX VP;
};

struct GeomBuffer
{
    DirectX::XMMATRIX M;
};

struct Camera
{
    DirectX::XMFLOAT3 poi;
    float r;        // Distance to POI
    float phi;      // Angle in plane x0z
    float theta;    // Angle from plane x0z
};

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
        , m_pTriangle(nullptr)
        , m_pSceneBuffer(nullptr)
        , m_pCamera(nullptr)
        , m_pGeomBuffer(nullptr)
    {}

    ~Render() { terminate(); }

    bool init(HWND window);
    

    bool render();
    bool resize(UINT width, UINT height);

private:
    void terminate();

    HRESULT setupBackBuffer();
    HRESULT initScene();

private:
    ID3D11Device* m_pDevice;
    ID3D11DeviceContext* m_pDeviceContext;

    IDXGISwapChain* m_pSwapChain;
    ID3D11RenderTargetView* m_pBackBufferRTV;

    ID3D11RasterizerState* m_pRasterizerState;

    ID3D11Buffer* m_pSceneBuffer;
    ID3D11Buffer* m_pGeomBuffer;

    UINT m_width;
    UINT m_height;

    Triangle* m_pTriangle;

    Camera* m_pCamera;
    double m_angle;
};

