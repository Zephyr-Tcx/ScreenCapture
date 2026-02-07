// TrayManager.h
#pragma once

#include <windows.h>
#include <string>

class MainWindow;

class TrayManager {
public:
    TrayManager(MainWindow* pWindow);
    ~TrayManager();
    
    void CreateTrayIcon();
    void RemoveTrayIcon();
    void ShowNotification(const std::wstring& title, const std::wstring& message);
    void UpdateTrayIcon(bool capturing);
    
    // 托盘菜单
    void ShowTrayMenu();
    
private:
    MainWindow* m_pMainWindow;
    bool m_bTrayIconCreated;
    bool m_bCapturing;
};