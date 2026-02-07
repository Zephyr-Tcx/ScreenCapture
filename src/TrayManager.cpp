// TrayManager.cpp
#include "TrayManager.h"
#include "MainWindow.h"
#include "Resource.h"
#include <shellapi.h>

TrayManager::TrayManager(MainWindow* pWindow) 
    : m_pMainWindow(pWindow)
    , m_bTrayIconCreated(false)
    , m_bCapturing(false) {
}

TrayManager::~TrayManager() {
    RemoveTrayIcon();
}

void TrayManager::CreateTrayIcon() {
    if (m_bTrayIconCreated) return;
    
    NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA) };
    nid.hWnd = m_pMainWindow->GetHandle();
    nid.uID = IDC_TRAY_ICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TRAY_ICON));
    wcscpy_s(nid.szTip, L"屏幕截图工具");
    
    Shell_NotifyIcon(NIM_ADD, &nid);
    
    m_bTrayIconCreated = true;
}

void TrayManager::RemoveTrayIcon() {
    if (!m_bTrayIconCreated) return;
    
    NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA) };
    nid.hWnd = m_pMainWindow->GetHandle();
    nid.uID = IDC_TRAY_ICON;
    
    Shell_NotifyIcon(NIM_DELETE, &nid);
    m_bTrayIconCreated = false;
}

void TrayManager::ShowNotification(const std::wstring& title, const std::wstring& message) {
    if (!m_bTrayIconCreated) return;
    
    NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA) };
    nid.hWnd = m_pMainWindow->GetHandle();
    nid.uID = IDC_TRAY_ICON;
    nid.uFlags = NIF_INFO;
    wcscpy_s(nid.szInfoTitle, title.c_str());
    wcscpy_s(nid.szInfo, message.c_str());
    nid.dwInfoFlags = NIIF_INFO;
    nid.uTimeout = 3000;
    
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void TrayManager::UpdateTrayIcon(bool capturing) {
    if (!m_bTrayIconCreated) return;
    
    m_bCapturing = capturing;
    
    NOTIFYICONDATA nid = { sizeof(NOTIFYICONDATA) };
    nid.hWnd = m_pMainWindow->GetHandle();
    nid.uID = IDC_TRAY_ICON;
    nid.uFlags = NIF_ICON;
    
    if (capturing) {
        nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_CAMERA_ICON));
    } else {
        nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TRAY_ICON));
    }
    
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void TrayManager::ShowTrayMenu() {
    POINT pt;
    GetCursorPos(&pt);
    
    HMENU hMenu = CreatePopupMenu();
    
    // 添加菜单项
    AppendMenu(hMenu, MF_STRING, IDM_TRAY_SHOW, L"显示窗口");
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    
    AppendMenu(hMenu, MF_STRING, IDM_TRAY_MANUAL, L"立即截图");
    
    if (m_bCapturing) {
        AppendMenu(hMenu, MF_STRING, IDM_TRAY_TOGGLE, L"停止截图");
    } else {
        AppendMenu(hMenu, MF_STRING, IDM_TRAY_TOGGLE, L"开始截图");
    }
    
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, IDM_TRAY_EXIT, L"退出");
    
    // 显示菜单
    SetForegroundWindow(m_pMainWindow->GetHandle());
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, m_pMainWindow->GetHandle(), NULL);
    DestroyMenu(hMenu);
}