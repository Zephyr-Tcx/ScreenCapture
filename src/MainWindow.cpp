// MainWindow.cpp
#include "MainWindow.h"
#include "ScreenCapturer.h"
#include "SettingsManager.h"
#include "TrayManager.h"
#include "Resource.h"
#include <commctrl.h>
#include <gdiplus.h>
#include <sstream>
#include <iomanip>

#pragma comment(lib, "comctl32.lib")

using namespace Gdiplus;

MainWindow* MainWindow::m_pInstance = nullptr;

MainWindow::MainWindow() 
    : m_hWnd(nullptr)
    , m_hInstance(nullptr)
    , m_bCapturing(false)
    , m_nCaptureCount(0)
    , m_bDarkMode(false)
    , m_hBackgroundBrush(nullptr)
    , m_hTitleFont(nullptr)
    , m_hNormalFont(nullptr)
    , m_nWindowWidth(600)
    , m_nWindowHeight(550) {
}

MainWindow::~MainWindow() {
    if (m_hTitleFont) DeleteObject(m_hTitleFont);
    if (m_hNormalFont) DeleteObject(m_hNormalFont);
    if (m_hBackgroundBrush) DeleteObject(m_hBackgroundBrush);
}

MainWindow* MainWindow::GetInstance() {
    if (!m_pInstance) {
        m_pInstance = new MainWindow();
    }
    return m_pInstance;
}

bool MainWindow::Initialize(HINSTANCE hInstance) {
    m_hInstance = hInstance;
    
    // 初始化公共控件
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES | ICC_USEREX_CLASSES;
    InitCommonControlsEx(&icex);
    
    // 创建窗口类
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"ScreenCaptureToolClass";
    wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON));
    
    if (!RegisterClassEx(&wcex)) {
        return false;
    }
    
    // 创建管理器对象
    m_pCapturer = std::make_unique<ScreenCapturer>(this);
    m_pSettings = std::make_unique<SettingsManager>();
    m_pTrayManager = std::make_unique<TrayManager>(this);
    
    return CreateMainWindow();
}

bool MainWindow::CreateMainWindow() {
    // 计算窗口位置（居中）
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenWidth - m_nWindowWidth) / 2;
    int y = (screenHeight - m_nWindowHeight) / 2;
    
    m_hWnd = CreateWindowEx(
        0,
        L"ScreenCaptureToolClass",
        L"屏幕截图工具",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN,
        x, y,
        m_nWindowWidth, m_nWindowHeight,
        nullptr,
        nullptr,
        m_hInstance,
        this
    );
    
    if (!m_hWnd) {
        return false;
    }
    
    // 创建字体
    m_hTitleFont = CreateFont(24, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    
    m_hNormalFont = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                              DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    
    // 创建控件
    if (!CreateControls()) {
        return false;
    }
    
    // 加载设置
    LoadSettings();
    
    // 创建托盘图标
    m_pTrayManager->CreateTrayIcon();
    
    return true;
}

bool MainWindow::CreateControls() {
    // 设置窗口字体
    SendMessage(m_hWnd, WM_SETFONT, (WPARAM)m_hNormalFont, TRUE);
    
    // 创建分组框
    CreateGroupBox(L"截图设置", 15, 15, m_nWindowWidth - 30, 200);
    
    // 截图间隔
    CreateWindowEx(0, L"STATIC", L"截图间隔（分钟）:", 
                  WS_CHILD | WS_VISIBLE | SS_RIGHT,
                  30, 45, 120, 25, m_hWnd, nullptr, m_hInstance, nullptr);
    
    m_hIntervalEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"5",
                                    WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_NUMBER,
                                    160, 45, 80, 25, m_hWnd, (HMENU)IDC_INTERVAL_EDIT, 
                                    m_hInstance, nullptr);
    SendMessage(m_hIntervalEdit, EM_SETLIMITTEXT, 3, 0);
    
    // 保存路径
    CreateWindowEx(0, L"STATIC", L"保存路径:", 
                  WS_CHILD | WS_VISIBLE | SS_RIGHT,
                  30, 80, 120, 25, m_hWnd, nullptr, m_hInstance, nullptr);
    
    m_hPathEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                                160, 80, 250, 25, m_hWnd, (HMENU)IDC_PATH_EDIT, 
                                m_hInstance, nullptr);
    
    m_hBrowseBtn = CreateWindowEx(0, L"BUTTON", L"浏览...",
                                 WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                 420, 80, 80, 25, m_hWnd, (HMENU)IDC_BROWSE_BTN,
                                 m_hInstance, nullptr);
    
    // 显示器选择
    CreateWindowEx(0, L"STATIC", L"选择显示器:", 
                  WS_CHILD | WS_VISIBLE | SS_RIGHT,
                  30, 115, 120, 25, m_hWnd, nullptr, m_hInstance, nullptr);
    
    m_hDisplayCombo = CreateWindowEx(WS_EX_CLIENTEDGE, L"COMBOBOX", L"",
                                    WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
                                    160, 115, 340, 200, m_hWnd, (HMENU)IDC_DISPLAY_COMBO,
                                    m_hInstance, nullptr);
    
    // 文件格式
    CreateWindowEx(0, L"STATIC", L"文件格式:", 
                  WS_CHILD | WS_VISIBLE | SS_RIGHT,
                  30, 150, 120, 25, m_hWnd, nullptr, m_hInstance, nullptr);
    
    m_hFormatCombo = CreateWindowEx(WS_EX_CLIENTEDGE, L"COMBOBOX", L"",
                                   WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
                                   160, 150, 150, 200, m_hWnd, (HMENU)IDC_FORMAT_COMBO,
                                   m_hInstance, nullptr);
    
    SendMessage(m_hFormatCombo, CB_ADDSTRING, 0, (LPARAM)L"PNG");
    SendMessage(m_hFormatCombo, CB_ADDSTRING, 0, (LPARAM)L"JPEG");
    SendMessage(m_hFormatCombo, CB_ADDSTRING, 0, (LPARAM)L"BMP");
    SendMessage(m_hFormatCombo, CB_SETCURSEL, 0, 0);
    
    // 图片质量（仅JPEG）
    CreateWindowEx(0, L"STATIC", L"图片质量:", 
                  WS_CHILD | WS_VISIBLE | SS_RIGHT,
                  320, 150, 80, 25, m_hWnd, nullptr, m_hInstance, nullptr);
    
    m_hQualitySlider = CreateWindowEx(0, TRACKBAR_CLASS, L"",
                                     WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBS_AUTOTICKS,
                                     410, 150, 130, 25, m_hWnd, (HMENU)IDC_QUALITY_SLIDER,
                                     m_hInstance, nullptr);
    SendMessage(m_hQualitySlider, TBM_SETRANGE, TRUE, MAKELPARAM(10, 100));
    SendMessage(m_hQualitySlider, TBM_SETPOS, TRUE, 90);
    SendMessage(m_hQualitySlider, TBM_SETTICFREQ, 10, 0);
    
    m_hQualityText = CreateWindowEx(0, L"STATIC", L"90%", 
                                   WS_CHILD | WS_VISIBLE | SS_CENTER,
                                   545, 150, 40, 25, m_hWnd, (HMENU)IDC_QUALITY_TEXT,
                                   m_hInstance, nullptr);
    
    // 创建第二个分组框
    CreateGroupBox(L"控制选项", 15, 230, m_nWindowWidth - 30, 120);
    
    // 按钮
    HICON hPlayIcon = (HICON)LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_PLAY_ICON), 
                                       IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    HICON hStopIcon = (HICON)LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_STOP_ICON), 
                                       IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    HICON hCameraIcon = (HICON)LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_CAMERA_ICON), 
                                         IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    
    CreateModernButton(m_hStartBtn, L"开始截图", 30, 260, 120, 35, IDC_START_BTN, hPlayIcon);
    CreateModernButton(m_hManualBtn, L"立即截图", 170, 260, 120, 35, IDC_MANUAL_BTN, hCameraIcon);
    
    // 复选框
    m_hMinimizeCheck = CreateWindowEx(0, L"BUTTON", L"最小化到托盘",
                                     WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                                     310, 265, 150, 25, m_hWnd, (HMENU)IDC_MINIMIZE_CHECK,
                                     m_hInstance, nullptr);
    
    m_hStartMinimizedCheck = CreateWindowEx(0, L"BUTTON", L"启动时最小化",
                                           WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
                                           310, 290, 150, 25, m_hWnd, (HMENU)IDC_START_MINIMIZED,
                                           m_hInstance, nullptr);
    
    // 状态文本
    CreateGroupBox(L"状态信息", 15, 370, m_nWindowWidth - 30, 100);
    
    m_hStatusText = CreateWindowEx(0, L"STATIC", L"状态: 就绪",
                                  WS_CHILD | WS_VISIBLE | SS_LEFT,
                                  30, 400, m_nWindowWidth - 60, 25, m_hWnd, (HMENU)IDC_STATUS_TEXT,
                                  m_hInstance, nullptr);
    
    m_hCountText = CreateWindowEx(0, L"STATIC", L"已截图: 0 次",
                                 WS_CHILD | WS_VISIBLE | SS_LEFT,
                                 30, 430, m_nWindowWidth - 60, 25, m_hWnd, (HMENU)IDC_COUNT_TEXT,
                                 m_hInstance, nullptr);
    
    // 清理图标资源
    if (hPlayIcon) DestroyIcon(hPlayIcon);
    if (hStopIcon) DestroyIcon(hStopIcon);
    if (hCameraIcon) DestroyIcon(hCameraIcon);
    
    // 更新显示器列表
    m_pCapturer->UpdateDisplayList(m_hDisplayCombo);
    
    return true;
}

void MainWindow::CreateModernButton(HWND& hButton, const std::wstring& text, 
                                   int x, int y, int width, int height, 
                                   int id, HICON hIcon) {
    DWORD style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON | BS_TEXT;
    if (hIcon) style |= BS_ICON;
    
    hButton = CreateWindowEx(0, L"BUTTON", text.c_str(),
                            style, x, y, width, height,
                            m_hWnd, (HMENU)id, m_hInstance, nullptr);
    
    if (hIcon) {
        SendMessage(hButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
    }
    
    // 设置字体
    SendMessage(hButton, WM_SETFONT, (WPARAM)m_hNormalFont, TRUE);
}

void MainWindow::CreateGroupBox(const std::wstring& text, int x, int y, int width, int height) {
    CreateWindowEx(0, L"BUTTON", text.c_str(),
                  WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                  x, y, width, height,
                  m_hWnd, nullptr, m_hInstance, nullptr);
}

LRESULT CALLBACK MainWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = nullptr;
    
    if (message == WM_NCCREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (MainWindow*)pCreate->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
    } else {
        pThis = (MainWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    }
    
    if (pThis) {
        return pThis->HandleMessage(message, wParam, lParam);
    }
    
    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT MainWindow::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_COMMAND:
            OnCommand(wParam, lParam);
            break;
            
        case WM_TIMER:
            OnTimer(wParam, lParam);
            break;
            
        case WM_PAINT:
            OnPaint();
            break;
            
        case WM_SIZE:
            OnSize(wParam, lParam);
            break;
            
        case WM_CLOSE:
            OnClose();
            break;
            
        case WM_DESTROY:
            OnDestroy();
            break;
            
        case WM_CAPTURE_COMPLETE:
            {
                std::wstring* pFilename = (std::wstring*)lParam;
                bool manual = (wParam == 1);
                OnCaptureComplete(*pFilename, manual);
                delete pFilename;
            }
            break;
            
        case WM_UPDATE_STATUS:
            {
                std::wstring* pText = (std::wstring*)lParam;
                UpdateStatus(*pText);
                delete pText;
            }
            break;
            
        default:
            return DefWindowProc(m_hWnd, message, wParam, lParam);
    }
    return 0;
}

void MainWindow::OnCommand(WPARAM wParam, LPARAM lParam) {
    int wmId = LOWORD(wParam);
    int wmEvent = HIWORD(wParam);
    
    switch (wmId) {
        case IDC_START_BTN:
            ToggleCapture();
            break;
            
        case IDC_MANUAL_BTN:
            if (m_pCapturer) {
                m_pCapturer->CaptureScreen(true);
            }
            break;
            
        case IDC_BROWSE_BTN:
            m_pSettings->BrowseSaveDirectory(m_hWnd, m_hPathEdit);
            break;
            
        case IDC_QUALITY_SLIDER:
            if (wmEvent == TB_THUMBTRACK || wmEvent == TB_THUMBPOSITION) {
                int quality = (int)SendMessage(m_hQualitySlider, TBM_GETPOS, 0, 0);
                std::wstring text = std::to_wstring(quality) + L"%";
                SetWindowText(m_hQualityText, text.c_str());
                
                if (m_pCapturer) {
                    m_pCapturer->SetImageQuality(quality);
                }
            }
            break;
            
        case IDC_FORMAT_COMBO:
            if (wmEvent == CBN_SELCHANGE) {
                int format = (int)SendMessage(m_hFormatCombo, CB_GETCURSEL, 0, 0);
                if (m_pCapturer) {
                    m_pCapturer->SetImageFormat(format);
                    
                    // 显示/隐藏质量滑块
                    bool showQuality = (format == 1); // JPEG格式显示质量滑块
                    ShowWindow(m_hQualitySlider, showQuality ? SW_SHOW : SW_HIDE);
                    ShowWindow(m_hQualityText, showQuality ? SW_SHOW : SW_HIDE);
                }
            }
            break;
    }
}

void MainWindow::OnTimer(WPARAM wParam, LPARAM lParam) {
    if (wParam == IDC_TIMER_ID && m_bCapturing && m_pCapturer) {
        m_pCapturer->CaptureScreen(false);
    }
}

void MainWindow::OnPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hWnd, &ps);
    
    // 绘制标题
    RECT titleRect = { 20, 10, m_nWindowWidth - 20, 40 };
    HFONT hOldFont = (HFONT)SelectObject(hdc, m_hTitleFont);
    SetTextColor(hdc, RGB(0, 0, 0));
    SetBkMode(hdc, TRANSPARENT);
    DrawText(hdc, L"屏幕截图工具", -1, &titleRect, DT_LEFT | DT_SINGLELINE);
    
    SelectObject(hdc, hOldFont);
    
    EndPaint(m_hWnd, &ps);
}

void MainWindow::OnSize(WPARAM wParam, LPARAM lParam) {
    if (wParam == SIZE_MINIMIZED) {
        // 最小化到托盘
        BOOL minimizeToTray = IsDlgButtonChecked(m_hWnd, IDC_MINIMIZE_CHECK);
        if (minimizeToTray) {
            ShowWindow(m_hWnd, SW_HIDE);
        }
    }
}

void MainWindow::OnClose() {
    BOOL minimizeToTray = IsDlgButtonChecked(m_hWnd, IDC_MINIMIZE_CHECK);
    if (minimizeToTray) {
        ShowWindow(m_hWnd, SW_HIDE);
    } else {
        SaveSettings();
        DestroyWindow(m_hWnd);
    }
}

void MainWindow::OnDestroy() {
    m_pTrayManager->RemoveTrayIcon();
    SaveSettings();
    PostQuitMessage(0);
}

int MainWindow::Run() {
    ShowWindow(m_hWnd, SW_SHOW);
    UpdateWindow(m_hWnd);
    
    // 检查是否启动时最小化
    BOOL startMinimized = IsDlgButtonChecked(m_hWnd, IDC_START_MINIMIZED);
    if (startMinimized) {
        ShowWindow(m_hWnd, SW_HIDE);
    }
    
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}

void MainWindow::UpdateStatus(const std::wstring& text) {
    std::wstring status = L"状态: " + text;
    SetWindowText(m_hStatusText, status.c_str());
}

void MainWindow::UpdateCount(int count) {
    m_nCaptureCount = count;
    std::wstring countText = L"已截图: " + std::to_wstring(count) + L" 次";
    SetWindowText(m_hCountText, countText.c_str());
}

void MainWindow::OnCaptureComplete(const std::wstring& filename, bool manual) {
    // 更新计数
    UpdateCount(m_nCaptureCount + 1);
    
    // 更新状态
    std::wstring status = (manual ? L"手动截图: " : L"自动截图: ") + filename;
    UpdateStatus(status);
    
    // 显示通知
    m_pTrayManager->ShowNotification(L"截图保存成功", 
                                     L"截图已保存到指定目录");
}

void MainWindow::ToggleCapture() {
    if (!m_pCapturer) return;
    
    if (m_bCapturing) {
        // 停止截图
        KillTimer(m_hWnd, IDC_TIMER_ID);
        m_bCapturing = false;
        
        SetWindowText(m_hStartBtn, L"开始截图");
        UpdateStatus(L"定时截图已停止");
        
        // 更新按钮图标
        HICON hPlayIcon = (HICON)LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_PLAY_ICON), 
                                           IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
        if (hPlayIcon) {
            SendMessage(m_hStartBtn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hPlayIcon);
            DestroyIcon(hPlayIcon);
        }
    } else {
        // 获取间隔时间
        wchar_t intervalBuffer[10];
        GetWindowText(m_hIntervalEdit, intervalBuffer, 10);
        int interval = _wtoi(intervalBuffer);
        
        if (interval <= 0) {
            UpdateStatus(L"错误: 间隔时间必须大于0");
            return;
        }
        
        // 启动定时器
        SetTimer(m_hWnd, IDC_TIMER_ID, interval * 60 * 1000, NULL);
        m_bCapturing = true;
        
        SetWindowText(m_hStartBtn, L"停止截图");
        UpdateStatus(L"定时截图已启动");
        
        // 更新按钮图标
        HICON hStopIcon = (HICON)LoadImage(m_hInstance, MAKEINTRESOURCE(IDI_STOP_ICON), 
                                           IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
        if (hStopIcon) {
            SendMessage(m_hStartBtn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hStopIcon);
            DestroyIcon(hStopIcon);
        }
        
        // 立即截取第一张
        m_pCapturer->CaptureScreen(false);
    }
}

void MainWindow::LoadSettings() {
    if (m_pSettings) {
        m_pSettings->LoadSettings();
        
        // 应用设置到UI
        SetWindowText(m_hPathEdit, m_pSettings->GetSavePath().c_str());
        SetWindowText(m_hIntervalEdit, std::to_wstring(m_pSettings->GetInterval()).c_str());
        
        // 选择显示器
        SendMessage(m_hDisplayCombo, CB_SETCURSEL, m_pSettings->GetDisplayIndex(), 0);
        
        // 选择格式
        SendMessage(m_hFormatCombo, CB_SETCURSEL, m_pSettings->GetImageFormat(), 0);
        
        // 设置质量
        SendMessage(m_hQualitySlider, TBM_SETPOS, TRUE, m_pSettings->GetImageQuality());
        std::wstring qualityText = std::to_wstring(m_pSettings->GetImageQuality()) + L"%";
        SetWindowText(m_hQualityText, qualityText.c_str());
        
        // 设置复选框
        CheckDlgButton(m_hWnd, IDC_MINIMIZE_CHECK, 
                      m_pSettings->GetMinimizeToTray() ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(m_hWnd, IDC_START_MINIMIZED,
                      m_pSettings->GetStartMinimized() ? BST_CHECKED : BST_UNCHECKED);
        
        // 更新捕获器设置
        if (m_pCapturer) {
            m_pCapturer->SetImageFormat(m_pSettings->GetImageFormat());
            m_pCapturer->SetImageQuality(m_pSettings->GetImageQuality());
        }
    }
}

void MainWindow::SaveSettings() {
    if (m_pSettings) {
        // 从UI获取设置
        wchar_t buffer[MAX_PATH];
        
        GetWindowText(m_hPathEdit, buffer, MAX_PATH);
        m_pSettings->SetSavePath(buffer);
        
        GetWindowText(m_hIntervalEdit, buffer, 10);
        m_pSettings->SetInterval(_wtoi(buffer));
        
        m_pSettings->SetDisplayIndex((int)SendMessage(m_hDisplayCombo, CB_GETCURSEL, 0, 0));
        m_pSettings->SetImageFormat((int)SendMessage(m_hFormatCombo, CB_GETCURSEL, 0, 0));
        m_pSettings->SetImageQuality((int)SendMessage(m_hQualitySlider, TBM_GETPOS, 0, 0));
        
        m_pSettings->SetMinimizeToTray(IsDlgButtonChecked(m_hWnd, IDC_MINIMIZE_CHECK));
        m_pSettings->SetStartMinimized(IsDlgButtonChecked(m_hWnd, IDC_START_MINIMIZED));
        
        // 保存设置
        m_pSettings->SaveSettings();
    }
}