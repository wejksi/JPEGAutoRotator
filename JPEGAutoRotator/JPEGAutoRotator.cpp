// JPEGAutoRotator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

HRESULT CreateDataObjectFromPath(_In_ PCWSTR pathToFile, _COM_Outptr_ IDataObject** dataObject)
{
    *dataObject = nullptr;

    CComPtr<IShellItem> spsi;
    HRESULT hr = SHCreateItemFromParsingName(pathToFile, nullptr, IID_PPV_ARGS(&spsi));
    if (SUCCEEDED(hr))
    {
        hr = spsi->BindToHandler(nullptr, BHID_DataObject, IID_PPV_ARGS(dataObject));
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
            // TODO: command line should allow access to diagnostic overrides
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

class CJPEGAutoRotator :
    public IRotationUI,
    public IRotationManagerEvents
{
public:
    CJPEGAutoRotator() {}

    // IUnknown
    IFACEMETHODIMP QueryInterface(_In_ REFIID riid, _COM_Outptr_ void** ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CJPEGAutoRotator, IRotationUI),
            QITABENT(CJPEGAutoRotator, IRotationManagerEvents),
            { 0, 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        LONG cRef = InterlockedDecrement(&m_cRef);
        if (cRef == 0)
        {
            delete this;
        }
        return cRef;
    }

    // IRotationUI
    IFACEMETHODIMP Initialize(_In_ IDataObject*)
    {
        return S_OK;
    }

    IFACEMETHODIMP Start()
    {
        return m_sprm->Start();
    }

    IFACEMETHODIMP Close()
    {
        return S_OK;
    }

    // IRotationManagerEvents
    IFACEMETHODIMP OnItemAdded(_In_ UINT uIndex)
    {
        return S_OK;
    }

    IFACEMETHODIMP OnItemProcessed(_In_ UINT uIndex)
    {

        return S_OK;
    }

    IFACEMETHODIMP OnProgress(_In_ UINT uCompleted, _In_ UINT uTotal)
    {
        return S_OK;
    }

    IFACEMETHODIMP OnCanceled()
    {
        return S_OK;
    }

    IFACEMETHODIMP OnCompleted()
    {
        return S_OK;
    }

private:
    ~CJPEGAutoRotator() {}

    HRESULT _ParseCommandLine()
    {
        HRESULT hr = E_FAIL;
        int cArguments;
        PWSTR *ppszArguments = CommandLineToArgvW(GetCommandLine(), &cArguments);
        if (ppszArguments)
        {
            bool printUsage = (cArguments < 2);
            if (!printUsage)
            {
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
                    else if ((_wcsicmp(ppszArguments[i], L"/verbose") == 0) ||
                        (_wcsicmp(ppszArguments[i], L"-verbose") == 0))
                    {
                        m_verboseMode = true;
                    }
                    else if ((_wcsicmp(ppszArguments[i], L"/silent") == 0) ||
                        (_wcsicmp(ppszArguments[i], L"-silent") == 0))
                    {
                        m_silentMode = true;
                    }
                    else if ((_wcsicmp(ppszArguments[i], L"/threadcount") == 0) ||
                        (_wcsicmp(ppszArguments[i], L"-threadcount") == 0))
                    {
                        printUsage = true;
                        if (i + 1 < cArguments)
                        {
                            m_threadCount = _wtoi(ppszArguments[i + 1]);
                            if (m_threadCount > 0 && m_threadCount != ERANGE)
                            {
                                printUsage = false;
                            }
                        }

                        if (printUsage)
                        {
                            break;
                        }
                    }
                    i++;
                }

                if (printUsage)
                {
                    _PrintUsage();
                }
                else
                {

                }
            }
        }

        return hr;
    }

    void _PrintUsage()
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

    HRESULT _Initialize(_In_ IRotationManager* prm);
    void _Cleanup();
    long m_cRef = 1;
    bool m_verboseMode = false;
    bool m_silentMode = false;
    int m_threadCount = -1;
    DWORD m_dwCookie = 0;
    CComPtr<IRotationManager> m_sprm;
    CComPtr<IDataObject> m_spdo;
};

int main()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        CJPEGAutoRotator *pRotator = new CJPEGAutoRotator();
        hr = pRotator->Start();
        pRotator->Release();
    }
       

            LocalFree(ppszArguments);
        }
        CoUninitialize();
    }
}

