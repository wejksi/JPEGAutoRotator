// JPEGAutoRotator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

HRESULT CreateDataObjectFromPath(_In_ PCWSTR pathToFile, _COM_Outptr_ IDataObject** dataObject)
{
    *dataObject = nullptr;

    CComPtr<IShellItem> spsi;
    HRESULT hr = SHCreateItemFromParsingName(pathToFile, nullptr /* IBindCtx */, IID_PPV_ARGS(&spsi));
    if (SUCCEEDED(hr))
    {
        hr = spsi->BindToHandler(nullptr /* IBindCtx */, BHID_DataObject, IID_PPV_ARGS(dataObject));
    }
    return hr;
}

HRESULT PerformOperation(_In_ PCWSTR pszPath)
{
    CComPtr<IDataObject> spdo;
    HRESULT hr = CreateDataObjectFromPath(pszPath, &spdo);
    if (SUCCEEDED(hr))
    {
        CComPtr<IRotationManager> sprm;
        hr = CRotationManager::s_CreateInstance(&sprm);
        if (SUCCEEDED(hr))
        {
            CComPtr<IRotationManagerDiagnostics> sprmd;
            hr = sprm->QueryInterface(IID_PPV_ARGS(&sprmd));
            if (SUCCEEDED(hr))
            {
                hr = EnumerateDataObject(spdo, sprm);
                if (SUCCEEDED(hr))
                {
                    sprm->Start();
                }
            }
        }
    }
 
    return hr;
}

void PrintUsage()
{
    // JPEGAutoRotator.exe /? <path>
    wprintf(L"\nUSAGE:\n");
    wprintf(L"\tJPEGAutoRotator [/?] [path]\n");
    wprintf(L"\n");
    wprintf(L"Options:\n");
    wprintf(L"\t/?\t\t\tDisplays this help message\n");
    wprintf(L"\tpath\t\t\tFull path to a file or folder to auto-rotate\n");
    wprintf(L"\n");
    wprintf(L"Example:\n");
    wprintf(L"\t> JPEGAutoRotator c:\\users\\chris\\pictures\\mypicturetorotate.jpg\n");
}

int main()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        int cArguments;
        PWSTR *ppszArguments = CommandLineToArgvW(GetCommandLine(), &cArguments);
        if (ppszArguments)
        {
            bool printUsage = false;
            int i = 0;
            while (i < cArguments)
            {
                if ((_wcsicmp(ppszArguments[i], L"/?") == 0) ||
                    (_wcsicmp(ppszArguments[i], L"?") == 0) ||
                    (_wcsicmp(ppszArguments[i], L"-?") == 0) ||
                    (_wcsicmp(ppszArguments[i], L"-help") == 0) ||
                    (_wcsicmp(ppszArguments[i], L"/help") == 0))
                {
                    printUsage = true;
                    break;
                }
                i++;
            }

            // Check if we should print the usage
            if (printUsage || (cArguments < 2))
            {
                PrintUsage();
                hr = PerformOperation(ppszArguments[1]);
            }

            if (FAILED(hr))
            {
                wchar_t message[200];
                StringCchPrintf(message, ARRAYSIZE(message), L"Error: 0x%x\n", hr);
                wprintf(message);
            }

            LocalFree(ppszArguments);
        }
        CoUninitialize();
    }
}

