#pragma once

#include "Triangle.h"
#include "Cube.h"

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
    DirectX::XMFLOAT3 poi = {0, 0, 0};
    float r = 7.0f;        // Distance to POI
    float phi = -(float)PI / 4;      // Angle in plane x0z
    float theta = (float)PI / 4;    // Angle from plane x0z
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
        //, m_pTriangle(nullptr)
        , m_pCube(nullptr)
        , m_pSceneBuffer(nullptr)
        , m_pCamera(nullptr)
        , m_pGeomBuffer(nullptr)
        //, m_isRotating(true)
        , m_prevSec(0)
        , m_mousePosX(0)
        , m_mousePosY(0)
        , m_isButtonPressed(false)
    {}

    ~Render() { terminate(); }

    bool init(HWND window);
    
    bool render();
    bool resize(UINT width, UINT height);

    bool update();

    void mouseLeftButton(bool pressed, int posX, int posY);
    void mouseMove(int posX, int posY);

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

    //Triangle* m_pTriangle;
    Cube* m_pCube;

    Camera* m_pCamera;
    double m_angle = 0;

    size_t m_prevSec = 0;

    //bool m_isRotating;

    int m_mousePosX;
    int m_mousePosY;
    bool m_isButtonPressed;
};

