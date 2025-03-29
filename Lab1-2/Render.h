#pragma once

//#include "Triangle.h"
//#include "Cube.h"
#include "TexturedCube.h"
#include "Skybox.h"
#include "TransparentRect.h"
#include "LightModel.h"
#include "Postprocess.h"

#define PI 3.14159265358979323846

struct Light
{
    DirectX::XMFLOAT4 Pos;
    DirectX::XMFLOAT4 Color;
};

struct SceneBuffer
{
    DirectX::XMMATRIX VP;
    DirectX::XMINT4 SceneParams; // x - light count
    Light lights[3];
    DirectX::XMFLOAT4 AmbientColor;
    DirectX::XMFLOAT3 CameraPos;
};

struct GeomBuffer
{
    DirectX::XMMATRIX M;
    DirectX::XMMATRIX NormalM;
    DirectX::XMFLOAT4 params; // x - shininess, y - use
};

struct Camera
{
    DirectX::XMFLOAT3 poi = {0, 0, 0};
    float r = 5.0f;        // Distance to POI
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
        , m_pSamplerState(nullptr)
        , m_pSkybox(nullptr)
        , m_pCube2(nullptr)
        , m_pRasterizerState(nullptr)
        , m_pDepthBuffer(nullptr)
        , m_pDepthBufferDSV(nullptr)
        , m_pGeomBuffer2(nullptr)
        , m_pDepthState(nullptr)
        , m_pRect1(nullptr)
        , m_pRect2(nullptr)
        , m_pBlendState(nullptr)
        , m_pTransparentDepthState(nullptr)
        , m_pTransparentBlendState(nullptr)
        , m_useNormalMap(true)
        , m_showNormals(false)
        , m_pPostprocess(nullptr)
        , m_useFilter(false)
    {
        for (int i = 0; i < 3; i++)
        {
            m_pLightSourceGeomBuffers[i] = nullptr;
        }
    }

    ~Render() { terminate(); }

    bool init(HWND window);
    
    bool render();
    bool resize(UINT width, UINT height);

    bool update();

    void mouseLeftButton(bool pressed, int posX, int posY);
    void mouseMove(int posX, int posY);
    void mouseWheel(int delta);

private:
    void terminate();

    HRESULT setupBackBuffer();
    HRESULT initScene();
    HRESULT initSamplers();
    HRESULT initDepthStencil();
    HRESULT initBlendState();

    void drawTransparentSorted();

private:
    ID3D11Device* m_pDevice;
    ID3D11DeviceContext* m_pDeviceContext;

    IDXGISwapChain* m_pSwapChain;
    ID3D11RenderTargetView* m_pBackBufferRTV;

    ID3D11RasterizerState* m_pRasterizerState;

    ID3D11Texture2D* m_pDepthBuffer;
    ID3D11DepthStencilView* m_pDepthBufferDSV;
    ID3D11DepthStencilState* m_pDepthState;
    ID3D11DepthStencilState* m_pTransparentDepthState;

    ID3D11BlendState* m_pBlendState;
    ID3D11BlendState* m_pTransparentBlendState;

    ID3D11Buffer* m_pSceneBuffer;
    ID3D11Buffer* m_pGeomBuffer;
    ID3D11Buffer* m_pGeomBuffer2;
    ID3D11Buffer* m_pGeomBufferInst;
    ID3D11Buffer* m_pLightSourceGeomBuffers[3];

    ID3D11SamplerState* m_pSamplerState;

    UINT m_width;
    UINT m_height;

    //Triangle* m_pTriangle;
    //Cube* m_pCube;
    TexturedCube* m_pCube;
    TexturedCube* m_pCube2;
    Skybox* m_pSkybox;
    TransparentRect* m_pRect1;
    TransparentRect* m_pRect2;
    Postprocess* m_pPostprocess;

    SceneBuffer m_sceneBuffer;

    Camera* m_pCamera;
    double m_angle = 0;

    size_t m_prevSec = 0;

    //bool m_isRotating;

    int m_mousePosX;
    int m_mousePosY;
    bool m_isButtonPressed;
    bool m_useNormalMap;
    bool m_showNormals;
    bool m_useFilter;

    std::vector<LightModel*> lights;
    std::vector<GeomBuffer> geomBuffers;
};

