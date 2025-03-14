// header.h: включаемый файл для стандартных системных включаемых файлов
// или включаемые файлы для конкретного проекта
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Исключите редко используемые компоненты из заголовков Windows
// Файлы заголовков Windows
#include <windows.h>
// Файлы заголовков среды выполнения C
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <assert.h>

#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include "thirdparty/DDSTextureLoader11.h"

enum shader_stage
{
	Vertex,
	Pixel,
};

inline HRESULT SetResourceName(ID3D11DeviceChild* pResource, const std::string& name)
{
    return pResource->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.length(), name.c_str());
}

inline bool readFileContent(LPCTSTR filename, std::vector<char>& data)
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

inline bool compileShader(ID3D11Device* device, LPCTSTR srcFilename, const std::vector<LPCSTR>& defines, const shader_stage& stage, ID3D11DeviceChild** ppShader, ID3DBlob** ppShaderBinary = nullptr)
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
            result = device->CreateVertexShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), nullptr, &pVertexShader);
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
            result = device->CreatePixelShader(pCode->GetBufferPointer(), pCode->GetBufferSize(), nullptr, &pPixelShader);
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

        if (ppShaderBinary)
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