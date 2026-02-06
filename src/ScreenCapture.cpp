//v0.0.1 - 初始版本，基本功能实现

#define UNICODE
#define _UNICODE

#include <windows.h>
#include <gdiplus.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")

using namespace Gdiplus;

// 全局变量
HWND g_hWnd = NULL;
HINSTANCE g_hInstance = NULL;
std::atomic<bool> g_bCapturing(false);
std::atomic<int> g_nCaptureCount(0);
std::wstring g_savePath;
int g_intervalMinutes = 5;
int g_selectedDisplay = 0; // 0 = 所有显示器
std::vector<DISPLAY_DEVICE> g_displays;
std::mutex g_captureMutex;

// 控件ID
#define IDC_START_BTN         101
#define IDC_MANUAL_BTN        102
#define IDC_BROWSE_BTN        103
#define IDC_INTERVAL_EDIT     104
#define IDC_DISPLAY_COMBO     105
#define IDC_PATH_EDIT         106
#define IDC_STATUS_TEXT       107
#define IDC_COUNT_TEXT        108
#define IDC_TIMER_ID          109
#define IDC_MINIMIZE_CHECK    110
#define IDC_TRAY_ICON         111
#define IDM_TRAY_EXIT         112
#define IDM_TRAY_SHOW         113
#define IDM_TRAY_MANUAL       114
#define IDM_TRAY_TOGGLE       115

// 自定义消息
#define WM_TRAYICON (WM_USER + 100)

// 函数声明
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL InitApplication(HINSTANCE hInstance);
HWND InitInstance(HINSTANCE, int);
void UpdateStatus(const std::wstring& text);
void BrowseSaveDirectory();
void CaptureScreen(bool manual = false);
void StartCapture();
void StopCapture();
void ToggleCapture();
void UpdateDisplayList(HWND hCombo);
void SaveSettings();
void LoadSettings();
void CreateTrayIcon(HWND hWnd);
void RemoveTrayIcon(HWND hWnd);
std::wstring GetTimestamp();
std::wstring GetAppDataPath();
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid); // 添加这个声明

// GDI+ 初始化
GdiplusStartupInput g_gdiplusStartupInput;
ULONG_PTR g_gdiplusToken;

// 入口点
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) {
    // 初始化GDI+
    GdiplusStartup(&g_gdiplusToken, &g_gdiplusStartupInput, NULL);
    
    // 初始化公共控件
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);
    
    g_hInstance = hInstance;
    
    // 注册窗口类
    if (!InitApplication(hInstance)) {
        return FALSE;
    }
    
    // 创建主窗口
    HWND hWnd = InitInstance(hInstance, nCmdShow);
    if (!hWnd) {
        return FALSE;
    }
    
    // 加载设置
    LoadSettings();
    
    // 消息循环
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    // 清理GDI+
    GdiplusShutdown(g_gdiplusToken);
    
    return (int)msg.wParam;
}

// 注册窗口类
BOOL InitApplication(HINSTANCE hInstance) {
    WNDCLASSEX wcex;
    
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"ScreenCaptureClass";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    
    return RegisterClassEx(&wcex);
}

// 创建窗口和控件
HWND InitInstance(HINSTANCE hInstance, int nCmdShow) {
    g_hWnd = CreateWindowW(
        L"ScreenCaptureClass",
        L"定时屏幕截图工具",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 450,
        NULL, NULL, hInstance, NULL
    );
    
    if (!g_hWnd) {
        return NULL;
    }
    
    // 创建控件
    // 标签：截图间隔
    CreateWindowW(L"STATIC", L"截图间隔 (分钟):", 
        WS_CHILD | WS_VISIBLE | SS_RIGHT,
        20, 20, 120, 25, g_hWnd, NULL, hInstance, NULL);
    
    // 编辑框：间隔时间
    HWND hIntervalEdit = CreateWindowW(L"EDIT", L"5",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
        150, 20, 80, 25, g_hWnd, (HMENU)IDC_INTERVAL_EDIT, hInstance, NULL);
    SendMessage(hIntervalEdit, EM_SETLIMITTEXT, 4, 0);
    
    // 按钮：开始/停止截图
    CreateWindowW(L"BUTTON", L"开始截图",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        250, 20, 100, 25, g_hWnd, (HMENU)IDC_START_BTN, hInstance, NULL);
    
    // 按钮：手动截图
    CreateWindowW(L"BUTTON", L"立即截图",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        370, 20, 100, 25, g_hWnd, (HMENU)IDC_MANUAL_BTN, hInstance, NULL);
    
    // 标签：保存目录
    CreateWindowW(L"STATIC", L"保存目录:", 
        WS_CHILD | WS_VISIBLE | SS_RIGHT,
        20, 60, 120, 25, g_hWnd, NULL, hInstance, NULL);
    
    // 编辑框：保存路径
    CreateWindowW(L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        150, 60, 290, 25, g_hWnd, (HMENU)IDC_PATH_EDIT, hInstance, NULL);
    
    // 按钮：浏览目录
    CreateWindowW(L"BUTTON", L"浏览...",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        450, 60, 60, 25, g_hWnd, (HMENU)IDC_BROWSE_BTN, hInstance, NULL);
    
    // 标签：显示器选择
    CreateWindowW(L"STATIC", L"选择显示器:", 
        WS_CHILD | WS_VISIBLE | SS_RIGHT,
        20, 100, 120, 25, g_hWnd, NULL, hInstance, NULL);
    
    // 下拉框：显示器列表
    HWND hDisplayCombo = CreateWindowW(L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        150, 100, 200, 200, g_hWnd, (HMENU)IDC_DISPLAY_COMBO, hInstance, NULL);
    
    // 复选框：最小化到托盘
    CreateWindowW(L"BUTTON", L"最小化到系统托盘",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        150, 140, 200, 25, g_hWnd, (HMENU)IDC_MINIMIZE_CHECK, hInstance, NULL);
    
    // 状态文本
    CreateWindowW(L"STATIC", L"状态: 就绪",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 180, 450, 25, g_hWnd, (HMENU)IDC_STATUS_TEXT, hInstance, NULL);
    
    // 计数文本
    CreateWindowW(L"STATIC", L"已截图: 0 次",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        20, 210, 450, 25, g_hWnd, (HMENU)IDC_COUNT_TEXT, hInstance, NULL);
    
    // 更新显示器列表
    UpdateDisplayList(hDisplayCombo);
    
    // 创建系统托盘图标
    CreateTrayIcon(g_hWnd);
    
    // 显示窗口
    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);
    
    return g_hWnd;
}

// 窗口过程
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            
            switch (wmId) {
            case IDC_START_BTN:
                ToggleCapture();
                break;
                
            case IDC_MANUAL_BTN:
                CaptureScreen(true);
                break;
                
            case IDC_BROWSE_BTN:
                BrowseSaveDirectory();
                break;
                
            case IDM_TRAY_EXIT:
                SaveSettings();
                DestroyWindow(hWnd);
                break;
                
            case IDM_TRAY_SHOW:
                ShowWindow(hWnd, SW_SHOW);
                SetForegroundWindow(hWnd);
                break;
                
            case IDM_TRAY_MANUAL:
                CaptureScreen(true);
                break;
                
            case IDM_TRAY_TOGGLE:
                ToggleCapture();
                break;
                
            case IDC_MINIMIZE_CHECK:
                // 复选框状态改变
                break;
            }
        }
        break;
        
    case WM_TIMER:
        if (wParam == IDC_TIMER_ID && g_bCapturing) {
            CaptureScreen(false);
        }
        break;
        
    case WM_CLOSE:
        {
            BOOL minimizeToTray = IsDlgButtonChecked(hWnd, IDC_MINIMIZE_CHECK);
            if (minimizeToTray) {
                ShowWindow(hWnd, SW_HIDE);
            } else {
                SaveSettings();
                DestroyWindow(hWnd);
            }
        }
        break;
        
    case WM_DESTROY:
        RemoveTrayIcon(hWnd);
        SaveSettings();
        PostQuitMessage(0);
        break;
        
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP) {
            // 显示托盘菜单
            POINT pt;
            GetCursorPos(&pt);
            
            HMENU hMenu = CreatePopupMenu();
            AppendMenuW(hMenu, MF_STRING, IDM_TRAY_SHOW, L"显示窗口");
            AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuW(hMenu, MF_STRING, IDM_TRAY_MANUAL, L"手动截图");
            
            if (g_bCapturing) {
                AppendMenuW(hMenu, MF_STRING, IDM_TRAY_TOGGLE, L"停止截图");
            } else {
                AppendMenuW(hMenu, MF_STRING, IDM_TRAY_TOGGLE, L"开始截图");
            }
            
            AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenuW(hMenu, MF_STRING, IDM_TRAY_EXIT, L"退出");
            
            SetForegroundWindow(hWnd);
            TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
            DestroyMenu(hMenu);
        }
        else if (lParam == WM_LBUTTONDBLCLK) {
            ShowWindow(hWnd, SW_SHOW);
            SetForegroundWindow(hWnd);
        }
        break;
        
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    
    return 0;
}

// 更新状态文本
void UpdateStatus(const std::wstring& text) {
    std::wstring status = L"状态: " + text;
    SetDlgItemTextW(g_hWnd, IDC_STATUS_TEXT, status.c_str());
}

// 更新计数文本
void UpdateCountText() {
    std::wstring count = L"已截图: " + std::to_wstring(g_nCaptureCount) + L" 次";
    SetDlgItemTextW(g_hWnd, IDC_COUNT_TEXT, count.c_str());
}

// 浏览保存目录
void BrowseSaveDirectory() {
    BROWSEINFOW bi = { 0 };
    bi.lpszTitle = L"选择截图保存目录";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    
    LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
    if (pidl != 0) {
        wchar_t path[MAX_PATH];
        if (SHGetPathFromIDListW(pidl, path)) {
            g_savePath = path;
            SetDlgItemTextW(g_hWnd, IDC_PATH_EDIT, path);
        }
        CoTaskMemFree(pidl);
    }
}

// 获取时间戳
std::wstring GetTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm tm_now;
    localtime_s(&tm_now, &now);
    
    std::wstringstream ss;
    ss << std::put_time(&tm_now, L"%Y-%m-%d_%H-%M-%S");
    return ss.str();
}

// 获取应用程序数据路径
std::wstring GetAppDataPath() {
    wchar_t path[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_MYDOCUMENTS, NULL, 0, path) == S_OK) {
        return std::wstring(path) + L"\\ScreenCaptures";
    }
    return L"C:\\ScreenCaptures";
}

// 获取PNG编码器的CLSID
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0;
    UINT size = 0;
    
    GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;
    
    ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL) return -1;
    
    GetImageEncoders(num, size, pImageCodecInfo);
    
    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;
        }
    }
    
    free(pImageCodecInfo);
    return -1;
}

// 截图函数
void CaptureScreen(bool manual) {
    std::lock_guard<std::mutex> lock(g_captureMutex);
    
    // 获取保存路径
    wchar_t pathBuffer[MAX_PATH];
    GetDlgItemTextW(g_hWnd, IDC_PATH_EDIT, pathBuffer, MAX_PATH);
    std::wstring savePath = pathBuffer;
    
    if (savePath.empty()) {
        savePath = GetAppDataPath();
        SetDlgItemTextW(g_hWnd, IDC_PATH_EDIT, savePath.c_str());
    }
    
    // 创建目录（如果不存在）
    CreateDirectoryW(savePath.c_str(), NULL);
    
    // 生成文件名
    std::wstring timestamp = GetTimestamp();
    std::wstring filename = (manual ? L"manual_" : L"auto_") + timestamp + L".png";
    std::wstring fullPath = savePath + L"\\" + filename;
    
    // 根据选择的显示器截图
    if (g_selectedDisplay == 0) {
        // 截取整个桌面（所有显示器）
        HDC hdcScreen = GetDC(NULL);
        HDC hdcMem = CreateCompatibleDC(hdcScreen);
        
        int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
        
        HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
        SelectObject(hdcMem, hBitmap);
        
        BitBlt(hdcMem, 0, 0, screenWidth, screenHeight, hdcScreen, 
               GetSystemMetrics(SM_XVIRTUALSCREEN), 
               GetSystemMetrics(SM_YVIRTUALSCREEN), SRCCOPY);
        
        // 使用GDI+保存为PNG
        Bitmap bitmap(hBitmap, NULL);
        CLSID pngClsid;
        GetEncoderClsid(L"image/png", &pngClsid);
        
        // 使用宽字符版本保存
        bitmap.Save(fullPath.c_str(), &pngClsid, NULL);
        
        // 清理资源
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdcScreen);
    } else {
        // 截取特定显示器
        DISPLAY_DEVICE displayDevice = g_displays[g_selectedDisplay - 1];
        DEVMODEW devMode;
        devMode.dmSize = sizeof(DEVMODEW);
        EnumDisplaySettingsW(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &devMode);
        
        HDC hdcScreen = CreateDCW(L"DISPLAY", displayDevice.DeviceName, NULL, NULL);
        HDC hdcMem = CreateCompatibleDC(hdcScreen);
        
        int screenWidth = devMode.dmPelsWidth;
        int screenHeight = devMode.dmPelsHeight;
        
        HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
        SelectObject(hdcMem, hBitmap);
        
        BitBlt(hdcMem, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY);
        
        // 使用GDI+保存为PNG
        Bitmap bitmap(hBitmap, NULL);
        CLSID pngClsid;
        GetEncoderClsid(L"image/png", &pngClsid);
        bitmap.Save(fullPath.c_str(), &pngClsid, NULL);
        
        // 清理资源
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        DeleteDC(hdcScreen);
    }
    
    // 更新计数和状态
    g_nCaptureCount++;
    UpdateCountText();
    
    std::wstring status = (manual ? L"手动截图已保存: " : L"自动截图已保存: ") + filename;
    UpdateStatus(status);
    
    // 显示系统通知
    NOTIFYICONDATAW nid = { sizeof(NOTIFYICONDATAW) };
    nid.hWnd = g_hWnd;
    nid.uID = IDC_TRAY_ICON;
    nid.uFlags = NIF_INFO;
    wcscpy_s(nid.szInfoTitle, L"截图保存成功");
    wcscpy_s(nid.szInfo, L"截图已保存到指定目录");
    nid.dwInfoFlags = NIIF_INFO;
    nid.uTimeout = 3000;
    Shell_NotifyIconW(NIM_MODIFY, &nid);
}

// 开始截图
void StartCapture() {
    // 获取间隔时间
    wchar_t intervalBuffer[10];
    GetDlgItemTextW(g_hWnd, IDC_INTERVAL_EDIT, intervalBuffer, 10);
    g_intervalMinutes = _wtoi(intervalBuffer);
    
    if (g_intervalMinutes <= 0) {
        UpdateStatus(L"错误: 间隔时间必须大于0");
        return;
    }
    
    // 启动定时器（以秒为单位）
    SetTimer(g_hWnd, IDC_TIMER_ID, g_intervalMinutes * 60 * 1000, NULL);
    g_bCapturing = true;
    
    SetDlgItemTextW(g_hWnd, IDC_START_BTN, L"停止截图");
    UpdateStatus(L"定时截图已启动");
    
    // 立即截取第一张
    CaptureScreen(false);
}

// 停止截图
void StopCapture() {
    KillTimer(g_hWnd, IDC_TIMER_ID);
    g_bCapturing = false;
    
    SetDlgItemTextW(g_hWnd, IDC_START_BTN, L"开始截图");
    UpdateStatus(L"定时截图已停止");
}

// 切换截图状态
void ToggleCapture() {
    if (g_bCapturing) {
        StopCapture();
    } else {
        StartCapture();
    }
}

// 更新显示器列表
void UpdateDisplayList(HWND hCombo) {
    SendMessageW(hCombo, CB_RESETCONTENT, 0, 0);
    
    // 添加"所有显示器"选项
    SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)L"所有显示器");
    SendMessageW(hCombo, CB_SETCURSEL, 0, 0);
    
    // 枚举所有显示器
    DISPLAY_DEVICE displayDevice;
    displayDevice.cb = sizeof(DISPLAY_DEVICE);
    
    g_displays.clear();
    
    for (int i = 0; EnumDisplayDevicesW(NULL, i, &displayDevice, 0); i++) {
        if (displayDevice.StateFlags & DISPLAY_DEVICE_ACTIVE) {
            // 获取显示器设置
            DEVMODEW devMode;
            devMode.dmSize = sizeof(DEVMODEW);
            EnumDisplaySettingsW(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &devMode);
            
            // 构建显示器描述
            std::wstring desc = displayDevice.DeviceString;
            desc += L" (" + std::to_wstring(devMode.dmPelsWidth) + 
                    L"x" + std::to_wstring(devMode.dmPelsHeight) + L")";
            
            SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)desc.c_str());
            g_displays.push_back(displayDevice);
        }
    }
}

// 创建系统托盘图标
void CreateTrayIcon(HWND hWnd) {
    NOTIFYICONDATAW nid = { sizeof(NOTIFYICONDATAW) };
    nid.hWnd = hWnd;
    nid.uID = IDC_TRAY_ICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
    wcscpy_s(nid.szTip, L"定时屏幕截图工具");
    
    Shell_NotifyIconW(NIM_ADD, &nid);
    
    if (nid.hIcon) {
        DestroyIcon(nid.hIcon);
    }
}

// 移除系统托盘图标
void RemoveTrayIcon(HWND hWnd) {
    NOTIFYICONDATAW nid = { sizeof(NOTIFYICONDATAW) };
    nid.hWnd = hWnd;
    nid.uID = IDC_TRAY_ICON;
    
    Shell_NotifyIconW(NIM_DELETE, &nid);
}

// 保存设置
void SaveSettings() {
    std::wstring iniPath = GetAppDataPath() + L"\\settings.ini";
    
    // 保存保存路径
    wchar_t pathBuffer[MAX_PATH];
    GetDlgItemTextW(g_hWnd, IDC_PATH_EDIT, pathBuffer, MAX_PATH);
    WritePrivateProfileStringW(L"Settings", L"SavePath", pathBuffer, iniPath.c_str());
    
    // 保存间隔时间
    wchar_t intervalBuffer[10];
    GetDlgItemTextW(g_hWnd, IDC_INTERVAL_EDIT, intervalBuffer, 10);
    WritePrivateProfileStringW(L"Settings", L"Interval", intervalBuffer, iniPath.c_str());
    
    // 保存显示器选择
    int displayIndex = (int)SendDlgItemMessageW(g_hWnd, IDC_DISPLAY_COMBO, CB_GETCURSEL, 0, 0);
    WritePrivateProfileStringW(L"Settings", L"Display", 
                              std::to_wstring(displayIndex).c_str(), iniPath.c_str());
    
    // 保存复选框状态
    BOOL minimizeToTray = IsDlgButtonChecked(g_hWnd, IDC_MINIMIZE_CHECK);
    WritePrivateProfileStringW(L"Settings", L"MinimizeToTray", 
                              minimizeToTray ? L"1" : L"0", iniPath.c_str());
}

// 加载设置
void LoadSettings() {
    std::wstring iniPath = GetAppDataPath() + L"\\settings.ini";
    std::wstring defaultPath = GetAppDataPath();
    
    // 加载保存路径
    wchar_t pathBuffer[MAX_PATH];
    GetPrivateProfileStringW(L"Settings", L"SavePath", defaultPath.c_str(), 
                           pathBuffer, MAX_PATH, iniPath.c_str());
    g_savePath = pathBuffer;
    SetDlgItemTextW(g_hWnd, IDC_PATH_EDIT, pathBuffer);
    
    // 创建目录
    CreateDirectoryW(pathBuffer, NULL);
    
    // 加载间隔时间
    wchar_t intervalBuffer[10];
    GetPrivateProfileStringW(L"Settings", L"Interval", L"5", 
                           intervalBuffer, 10, iniPath.c_str());
    SetDlgItemTextW(g_hWnd, IDC_INTERVAL_EDIT, intervalBuffer);
    g_intervalMinutes = _wtoi(intervalBuffer);
    
    // 加载显示器选择
    wchar_t displayBuffer[10];
    GetPrivateProfileStringW(L"Settings", L"Display", L"0", 
                           displayBuffer, 10, iniPath.c_str());
    int displayIndex = _wtoi(displayBuffer);
    SendDlgItemMessageW(g_hWnd, IDC_DISPLAY_COMBO, CB_SETCURSEL, displayIndex, 0);
    g_selectedDisplay = displayIndex;
    
    // 加载复选框状态
    wchar_t minimizeBuffer[10];
    GetPrivateProfileStringW(L"Settings", L"MinimizeToTray", L"1", 
                           minimizeBuffer, 10, iniPath.c_str());
    CheckDlgButton(g_hWnd, IDC_MINIMIZE_CHECK, 
                  (_wtoi(minimizeBuffer) == 1) ? BST_CHECKED : BST_UNCHECKED);
}

//v0.0.1 - 初始版本，基本功能实现