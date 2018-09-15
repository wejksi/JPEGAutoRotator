#include "stdafx.h"
#include "resource.h"

#include <windowsx.h>
#include <RotationInterfaces.h>
#include <RotationUI.h>
#include <RotationManager.h>

#define MAX_LOADSTRING 100

HINSTANCE g_hInst;

class CJPEGAutoRotatorTestUI: 
    public IDropTarget,
    public IRotationManagerEvents
{
public:
    CJPEGAutoRotatorTestUI()
    {
        OleInitialize(0);
    }

    HRESULT DoModal(__in_opt HWND hwnd)
    {
        HRESULT hr = S_OK;
        INT_PTR ret = DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_MAIN), hwnd, s_DlgProc, (LPARAM)this);
        if (ret < 0)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        return hr;
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(__in REFIID riid, __deref_out void **ppv)
    {
        static const QITAB qit[] =
        {
            QITABENT(CJPEGAutoRotatorTestUI, IDropTarget),
            QITABENT(CJPEGAutoRotatorTestUI, IRotationManagerEvents),
            {},
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = InterlockedDecrement(&m_cRef);
        if (!cRef)
            delete this;
        return cRef;
    }

    // IDropTarget
    IFACEMETHODIMP DragEnter(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    IFACEMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    IFACEMETHODIMP DragLeave();
    IFACEMETHODIMP Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

    // IRotationManagerEvents
    IFACEMETHODIMP OnAdded(__in UINT uIndex);
    IFACEMETHODIMP OnRotated(__in UINT uIndex);
    IFACEMETHODIMP OnProgress(__in UINT uCompleted, __in UINT uTotal);
    IFACEMETHODIMP OnCanceled();
    IFACEMETHODIMP OnCompleted();

    static INT_PTR CALLBACK s_DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        CJPEGAutoRotatorTestUI* pJPEGAutoRotatorTestUI = reinterpret_cast<CJPEGAutoRotatorTestUI *>(GetWindowLongPtr(hdlg, DWLP_USER));
        if (uMsg == WM_INITDIALOG)
        {
            pJPEGAutoRotatorTestUI = reinterpret_cast<CJPEGAutoRotatorTestUI *>(lParam);
            pJPEGAutoRotatorTestUI->m_hdlg = hdlg;
            SetWindowLongPtr(hdlg, DWLP_USER, reinterpret_cast<LONG_PTR>(pJPEGAutoRotatorTestUI));
        }
        return pJPEGAutoRotatorTestUI ? pJPEGAutoRotatorTestUI->_DlgProc(uMsg, wParam, lParam) : FALSE;
    }

private:
    ~CJPEGAutoRotatorTestUI()
    {
        OleUninitialize();
    }

    INT_PTR _DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    void _OnInit();
    void _OnDestroy();
    void _OnCommand(const int commandId);

    void _OnStart();
    void _OnStop();
    void _OnClear();

    void _AddItemToListView(__in IRotationItem* pri);
    void _UpdateItemInListView(__in int index, __in IRotationItem* pri);

    long m_cRef = 1;
    HWND m_hdlg = 0;
    HWND m_hwndLV = 0;

    DWORD m_adviseCookie = 0;

    CComPtr<IDropTargetHelper> m_spdth;
    CComPtr<IDataObject> m_spdtobj;

    CComPtr<IRotationManager> m_sprm;
    CComPtr<IRotationUI> m_sprui;
};

// IDropTarget
IFACEMETHODIMP CJPEGAutoRotatorTestUI::DragEnter(IDataObject *pdtobj, DWORD /* grfKeyState */, POINTL pt, DWORD *pdwEffect)
{
    if (m_spdth)
    {
        POINT ptT = { pt.x, pt.y };
        m_spdth->DragEnter(m_hdlg, pdtobj, &ptT, *pdwEffect);
    }
    
    m_spdtobj = pdtobj;
    return S_OK;
}

IFACEMETHODIMP CJPEGAutoRotatorTestUI::DragOver(DWORD /* grfKeyState */, POINTL pt, DWORD *pdwEffect)
{
    // leave *pdwEffect unchanged, we support all operations
    if (m_spdth)
    {
        POINT ptT = { pt.x, pt.y };
        m_spdth->DragOver(&ptT, *pdwEffect);
    }

    return S_OK;
}

IFACEMETHODIMP CJPEGAutoRotatorTestUI::DragLeave()
{
    if (m_spdth)
    {
        m_spdth->DragLeave();
    }

    m_spdtobj = nullptr;
    return S_OK;
}

IFACEMETHODIMP CJPEGAutoRotatorTestUI::Drop(IDataObject *pdtobj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
    if (m_spdth)
    {
        POINT ptT = { pt.x, pt.y };
        m_spdth->Drop(pdtobj, &ptT, *pdwEffect);
    }

    // TODO: We got the IDataObject we can pass to our code

    return S_OK;
}

// IRotationManagerEvents
IFACEMETHODIMP CJPEGAutoRotatorTestUI::OnAdded(__in UINT uIndex)
{
    // Get the IRotationItem and add it to our list view
    if (m_sprm)
    {
        CComPtr<IRotationItem> spri;
        m_sprm->GetItem(uIndex, &spri);
        _AddItemToListView(spri);
    }
    
    return S_OK;
}

IFACEMETHODIMP CJPEGAutoRotatorTestUI::OnRotated(__in UINT uIndex)
{
    // Update the list item in our list view
    if (m_sprm)
    {
        CComPtr<IRotationItem> spri;
        m_sprm->GetItem(uIndex, &spri);
        _UpdateItemInListView(uIndex, spri);
    }

    return S_OK;
}

IFACEMETHODIMP CJPEGAutoRotatorTestUI::OnProgress(__in UINT uCompleted, __in UINT uTotal)
{
    return S_OK;
}

IFACEMETHODIMP CJPEGAutoRotatorTestUI::OnCanceled()
{
    return S_OK;
}

IFACEMETHODIMP CJPEGAutoRotatorTestUI::OnCompleted()
{
    return S_OK;
}

INT_PTR CJPEGAutoRotatorTestUI::_DlgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR ret = TRUE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        _OnInit();
        break;

    case WM_COMMAND:
        _OnCommand(GET_WM_COMMAND_ID(wParam, lParam));
        break;
    
    case WM_DESTROY:
        _OnDestroy();
        ret = FALSE;
        break;

    default:
        ret = FALSE;
    }
    return ret;
}

void CJPEGAutoRotatorTestUI::_OnInit()
{
    CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC, IID_PPV_ARGS(&m_spdth));

    // Initialize list view
    m_hwndLV = GetDlgItem(m_hdlg, IDC_RENAMEITEMLIST);

    // Initialize columns

    // Initialize the listview
    LV_COLUMN lvc = {};
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = 300;
    lvc.pszText = L"Path";
    ListView_InsertColumn(m_hwndLV, 0, &lvc);

    lvc.cx = 100;
    lvc.pszText = L"Result";
    ListView_InsertColumn(m_hwndLV, 1, &lvc);
}

void CJPEGAutoRotatorTestUI::_OnCommand(const int commandId)
{
    switch (commandId)
    {
    case IDC_START:
        _OnStart();
        break;
    case IDC_STOP:
        _OnStop();
        break;
    case IDC_CLEAR:
        _OnClear();
        break;
    case IDCANCEL:
        EndDialog(m_hdlg, commandId);
        break;
    }
}

void CJPEGAutoRotatorTestUI::_OnDestroy()
{
    _OnStop();

    SetWindowLongPtr(m_hdlg, GWLP_USERDATA, NULL);
    m_hdlg = NULL;
}

void CJPEGAutoRotatorTestUI::_OnStart()
{
    // Clear the list view
    if (m_hwndLV)
    {
        ListView_DeleteAllItems(m_hwndLV);
    }

    HRESULT hr = CRotationManager::s_CreateInstance(&m_sprm);
    if (SUCCEEDED(hr))
    {
        hr = m_sprm->Advise(this, &m_adviseCookie);
        if (SUCCEEDED(hr))
        {
            hr = CRotationUI::s_CreateInstance(m_spdtobj, m_sprm, &m_sprui);
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = m_sprui->Start();
    }

    if (FAILED(hr))
    {
        MessageBox(m_hdlg, L"Failed to start rotation operation.", L"Rotation Failed", MB_OK | MB_ICONERROR);
    }
}

void CJPEGAutoRotatorTestUI::_OnStop()
{
    // Cancel via the rotation manager
    if (m_sprm)
    {
        m_sprm->Cancel();
    }
}

void CJPEGAutoRotatorTestUI::_OnClear()
{
    // Clear the list view
    if (m_hwndLV)
    {
        ListView_DeleteAllItems(m_hwndLV);
    }

    // Clear our data object
    m_spdtobj = nullptr;
}

void CJPEGAutoRotatorTestUI::_AddItemToListView(__in IRotationItem* pri)
{
    LV_ITEM lvi = { 0 };
    lvi.iItem = ListView_GetItemCount(m_hwndLV);
    lvi.iSubItem = 0;

    PWSTR itemPath = nullptr;
    pri->GetPath(&itemPath);
    lvi.pszText = itemPath;

    ListView_InsertItem(m_hwndLV, &lvi);

    // Update the sub items
    _UpdateItemInListView(lvi.iItem, pri);
}

void CJPEGAutoRotatorTestUI::_UpdateItemInListView(__in int index, __in IRotationItem* pri)
{
    PWSTR itemPath = nullptr;
    pri->GetPath(&itemPath);

    LV_ITEM lvi = { 0 };

    lvi.mask = LVIF_TEXT;
    lvi.pszText = itemPath;
    lvi.iSubItem = 0;
    lvi.pszText = itemPath;
    ListView_SetItem(m_hwndLV, &lvi);

    CoTaskMemFree(itemPath);

    HRESULT hrResult;
    pri->GetResult(&hrResult);
    lvi.iSubItem = 1;
    lvi.pszText = SUCCEEDED(hrResult) ? L"SUCCEEDED" : L"FAILED";
    ListView_SetItem(m_hwndLV, &lvi);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int)
{
    g_hInst = hInstance;
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
        CJPEGAutoRotatorTestUI *pdlg = new CJPEGAutoRotatorTestUI();
        if (pdlg)
        {
            pdlg->DoModal(NULL);
            pdlg->Release();
        }
        CoUninitialize();
    }
    return 0;
}