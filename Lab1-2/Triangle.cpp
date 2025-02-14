#include "Triangle.h"

#include <stdlib.h>

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment( lib, "dxguid.lib")
 
Triangle::Triangle(ID3D11Device* device) : m_pDevice(device), m_pIndexBuffer(nullptr), m_pVertexBuffer(nullptr)
{
	initBuffers();
    initInputLayout();
}

bool Triangle::initBuffers()
{
	static const triangleVertex vertices[] = {
        {-0.5f, -0.5f, 0.0f, RGB(255, 0, 0)},
        { 0.5f, -0.5f, 0.0f, RGB(0, 255, 0)},
        { 0.0f,  0.5f, 0.0f, RGB(0, 0, 255)}
    };

	D3D11_BUFFER_DESC vertexBufferrDesc = {};
	vertexBufferrDesc.ByteWidth = sizeof(vertices);
	vertexBufferrDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferrDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferrDesc.CPUAccessFlags = 0;
	vertexBufferrDesc.MiscFlags = 0;
	vertexBufferrDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = &vertices;
	vertexData.SysMemPitch = sizeof(vertices);
	HRESULT result = m_pDevice->CreateBuffer(&vertexBufferrDesc, &vertexData, &m_pVertexBuffer);

	if (!SUCCEEDED(result))
		return false;

	result = SetResourceName(m_pVertexBuffer, "vertex buffer");

	static const USHORT indices[] = { 0, 2, 1 };

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

	if (!SUCCEEDED(result))
		return false;

	result = SetResourceName(m_pIndexBuffer, "index buffer");

	return true;
}

bool Triangle::initInputLayout()
{
    static const D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    HRESULT result = S_OK;

    ID3DBlob* pVertexShaderCode = nullptr;
    if (SUCCEEDED(result))
    {
        result = compileShader(L"shaders/vertex_shader.hlsl", {}, shader_stage::Vertex, (ID3D11DeviceChild**)&m_pVertexShader, &pVertexShaderCode);
    }
    if (SUCCEEDED(result))
    {
        result = compileShader(L"shaders/pixel_shader.hlsl", {}, shader_stage::Pixel, (ID3D11DeviceChild**)&m_pPixelShader);
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

bool Triangle::readFileContent(LPCTSTR filename, std::vector<char>& data)
{
    DWORD error = NO_ERROR;
    HANDLE hFile = CreateFile(
        filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    error = GetLastError();
    if (hFile != INVALID_HANDLE_VALUE)
    {
        // Only work with files less than 2Gb. We don't need larger
        DWORD size = GetFileSize(hFile, nullptr);
        error = GetLastError();
        if (error == NO_ERROR)
        {
            data.resize(size);

            DWORD readBytes = 0;
            ReadFile(hFile, data.data(), size, &readBytes, nullptr);
            error = GetLastError();
            if (readBytes != size)
            {
                OutputDebugString(_T("File "));
                OutputDebugString(filename);
                OutputDebugString(_T(" wrong number of bytes is read.\n"));
                if (error == NO_ERROR)
                {
                    error = ERROR_READ_FAULT; // We need to mirror error somehow, as we only expect the given number of bytes
                }
            }
        }

        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

    switch (error)
    {
    case NO_ERROR:
        // Do nothing, all is OK
        break;

    case ERROR_FILE_NOT_FOUND:
        OutputDebugString(_T("File "));
        OutputDebugString(filename);
        OutputDebugString(_T(" is not found.\n"));
        break;

    case ERROR_ACCESS_DENIED:
        OutputDebugString(_T("Access is denied for file "));
        OutputDebugString(filename);
        OutputDebugString(_T(".\n"));
        break;

    default:
        OutputDebugString(_T("File I/O error while working with file "));
        OutputDebugString(filename);
        OutputDebugString(_T(".\n"));
        break;
    }

    return error == NO_ERROR;
}

bool Triangle::compileShader(LPCTSTR srcFilename, const std::vector<LPCSTR>& defines, const shader_stage& stage, ID3D11DeviceChild** ppShader, ID3DBlob** ppShaderBinary)
{
    std::vector<char> data;
    bool res = readFileContent(srcFilename, data);
    if (res)
    {
        std::vector<D3D_SHADER_MACRO> macros;
        for (int i = 0; i < defines.size(); i++)
        {
            macros.push_back({ defines[i], nullptr });
        }
        macros.push_back({ nullptr, nullptr });

        UINT flags = 0;

#ifdef _DEBUG
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif // _DEBUG

        HRESULT result = S_OK;

        ID3DBlob* pCode = nullptr;
        ID3DBlob* pErrMsg = nullptr;
        switch (stage)
        {
        case Vertex:
        {
            ID3D11VertexShader* pVertexShader = nullptr;
            result = D3DCompile(data.data(), data.size(), "", nullptr, nullptr, "VS", "vs_5_0", flags, 0, &pCode, &pErrMsg);
            if (!SUCCEEDED(result))
            {
                return false;
            }
            result = m_pDevice->CreateVertexShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), nullptr, &pVertexShader);
            if (SUCCEEDED(result))
            {
                *ppShader = pVertexShader;
            }
            break;
        }
        case Pixel:
        {
            ID3D11PixelShader* pPixelShader = nullptr;
            result = D3DCompile(data.data(), data.size(), "", nullptr, nullptr, "PS", "ps_5_0", flags, 0, &pCode, &pErrMsg);
            if (!SUCCEEDED(result))
            {
                return false;
            }
            result = m_pDevice->CreatePixelShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), nullptr, &pPixelShader);
            if (SUCCEEDED(result))
            {
                *ppShader = pPixelShader;
            }
            break;
        }
        }
        if (pErrMsg != nullptr && !SUCCEEDED(result))
        {
            const char* pMsg = (const char*)pErrMsg->GetBufferPointer();
            OutputDebugStringA(pMsg);
            OutputDebugString(_T("\n"));
        }
        assert(SUCCEEDED(result));

        if (pErrMsg != nullptr)
        {
            pErrMsg->Release();
            pErrMsg = nullptr;
        }

        if (SUCCEEDED(result))
        {
            std::wstring name = std::wstring(srcFilename);
            result = SetResourceName(*ppShader, std::string(name.begin(), name.end()));
        }

        if(ppShaderBinary)
        {
            *ppShaderBinary = pCode;
        }
        else
        {
            if (pCode != nullptr)
            {
                pCode->Release();
                pCode = nullptr;
            }
        }
        res = SUCCEEDED(result);
    }
    return res;
}

void Triangle::render(ID3D11DeviceContext* context, UINT width, UINT height)
{
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (FLOAT)width;
    viewport.Height = (FLOAT)height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    context->RSSetViewports(1, &viewport);

    D3D11_RECT rect;
    rect.left = 0;
    rect.top = 0;
    rect.right = width;
    rect.bottom = height;
    context->RSSetScissorRects(1, &rect);

    context->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    ID3D11Buffer* vertexBuffers[] = { m_pVertexBuffer };
    UINT strides[] = { 16 };
    UINT offsets[] = { 0 };
    context->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
    context->IASetInputLayout(m_pInputLayout);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->VSSetShader(m_pVertexShader, nullptr, 0);
    context->PSSetShader(m_pPixelShader, nullptr, 0);
    context->DrawIndexed(3, 0, 0);
}

void Triangle::terminate()
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

Triangle::~Triangle()
{
    terminate();
}