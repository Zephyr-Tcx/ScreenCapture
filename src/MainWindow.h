// MainWindow.h
#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <atomic>
#include <memory>

// 前向声明
class ScreenCapturer;
class SettingsManager;
class TrayManager;

class MainWindow {
public:
    static MainWindow* GetInstance();
    
    bool Initialize(HINSTANCE hInstance);
    int Run();
    
    HWND GetHandle() const { return m_hWnd; }
    HINSTANCE GetInstanceHandle() const { return m_hInstance; }
    
    void UpdateStatus(const std::wstring& text);
    void UpdateCount(int count);
    void OnCaptureComplete(const std::wstring& filename, bool manual);
    void ToggleCapture();
    
private:
    MainWindow();
    ~MainWindow();
    
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);
    
    bool CreateMainWindow();
    bool CreateControls();
    void LayoutControls();
    void UpdateButtonState();
    
    void OnCommand(WPARAM wParam, LPARAM lParam);
    void OnTimer(WPARAM wParam, LPARAM lParam);
    void OnPaint();
    void OnSize(WPARAM wParam, LPARAM lParam);
    void OnClose();
    void OnDestroy();
    
    void LoadSettings();
    void SaveSettings();
    
    // UI助手函数
    void CreateModernButton(HWND& hButton, const std::wstring& text, int x, int y, 
                           int width, int height, int id, HICON hIcon = nullptr);
    void CreateGroupBox(const std::wstring& text, int x, int y, int width, int height);
    void UpdateTheme();
    
private:
    static MainWindow* m_pInstance;
    
    HWND m_hWnd;
    HINSTANCE m_hInstance;
    
    // 子窗口句柄
    HWND m_hStartBtn;
    HWND m_hManualBtn;
    HWND m_hBrowseBtn;
    HWND m_hIntervalEdit;
    HWND m_hDisplayCombo;
    HWND m_hPathEdit;
    HWND m_hStatusText;
    HWND m_hCountText;
    HWND m_hMinimizeCheck;
    HWND m_hFormatCombo;
    HWND m_hQualitySlider;
    HWND m_hQualityText;
    HWND m_hStartMinimizedCheck;
    
    // 管理器对象
    std::unique_ptr<ScreenCapturer> m_pCapturer;
    std::unique_ptr<SettingsManager> m_pSettings;
    std::unique_ptr<TrayManager> m_pTrayManager;
    
    // 状态变量
    std::atomic<bool> m_bCapturing;
    std::atomic<int> m_nCaptureCount;
    
    // UI主题
    bool m_bDarkMode;
    HBRUSH m_hBackgroundBrush;
    HFONT m_hTitleFont;
    HFONT m_hNormalFont;
    
    // 窗口尺寸
    int m_nWindowWidth;
    int m_nWindowHeight;
};