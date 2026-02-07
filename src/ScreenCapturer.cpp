// ScreenCapturer.cpp
#include "ScreenCapturer.h"
#include "MainWindow.h"
#include <sstream>
#include <iomanip>
#include <shlobj.h>
#include <commdlg.h>

#pragma comment(lib, "shell32.lib")

using namespace Gdiplus;

ScreenCapturer::ScreenCapturer(MainWindow* pWindow) 
    : m_pMainWindow(pWindow)
    , m_imageFormat(0)
    , m_imageQuality(90)
    , m_gdiplusToken(0)
    , m_captureCount(0) {
    
    InitializeGdiplus();
}

ScreenCapturer::~ScreenCapturer() {
    CleanupGdiplus();
}

bool ScreenCapturer::InitializeGdiplus() {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
    return (m_gdiplusToken != 0);
}

void ScreenCapturer::CleanupGdiplus() {
    if (m_gdiplusToken) {
        GdiplusShutdown(m_gdiplusToken);
    }
}

void ScreenCapturer::CaptureScreen(bool manual) {
    std::lock_guard<std::mutex> lock(m_captureMutex);
    
    std::wstring outputPath;
    bool success = false;
    
    // 获取显示器选择（需要通过主窗口）
    // 这里简化处理，实际应该从设置获取
    int displayIndex = 0; // 默认所有显示器
    
    if (displayIndex == 0) {
        success = CaptureAllDisplays(outputPath);
    } else {
        success = CaptureSpecificDisplay(displayIndex - 1, outputPath);
    }
    
    if (success && m_pMainWindow) {
        // 提取文件名
        size_t pos = outputPath.find_last_of(L"\\");
        std::wstring filename = (pos != std::wstring::npos) ? 
                               outputPath.substr(pos + 1) : outputPath;
        
        // 发送消息到主窗口
        std::wstring* pFilename = new std::wstring(filename);
        PostMessage(m_pMainWindow->GetHandle(), WM_CAPTURE_COMPLETE, 
                   (WPARAM)manual, (LPARAM)pFilename);
    }
}

bool ScreenCapturer::CaptureAllDisplays(std::wstring& outputPath) {
    HDC hdcScreen = GetDC(NULL);
    if (!hdcScreen) return false;
    
    // 获取虚拟屏幕尺寸
    int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    int screenX = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int screenY = GetSystemMetrics(SM_YVIRTUALSCREEN);
    
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
    SelectObject(hdcMem, hBitmap);
    
    // 拷贝屏幕
    BitBlt(hdcMem, 0, 0, screenWidth, screenHeight, hdcScreen, 
           screenX, screenY, SRCCOPY);
    
    // 生成保存路径
    std::wstring savePath = GetDefaultSavePath();
    CreateDirectoryIfNotExists(savePath);
    
    std::wstring timestamp = GetTimestamp();
    std::wstring filename = (timestamp + GetFormatExtension(m_imageFormat));
    outputPath = savePath + L"\\" + filename;
    
    // 使用GDI+保存
    Bitmap bitmap(hBitmap, NULL);
    CLSID encoderClsid;
    if (GetEncoderClsid(GetFormatMimeType(m_imageFormat).c_str(), &encoderClsid) == -1) {
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        ReleaseDC(NULL, hdcScreen);
        return false;
    }
    
    // 设置JPEG质量
    EncoderParameters encoderParameters;
    if (m_imageFormat == 1) { // JPEG
        encoderParameters.Count = 1;
        encoderParameters.Parameter[0].Guid = EncoderQuality;
        encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
        encoderParameters.Parameter[0].NumberOfValues = 1;
        ULONG quality = m_imageQuality;
        encoderParameters.Parameter[0].Value = &quality;
    }
    
    Status status = (m_imageFormat == 1) ?
        bitmap.Save(outputPath.c_str(), &encoderClsid, &encoderParameters) :
        bitmap.Save(outputPath.c_str(), &encoderClsid, NULL);
    
    // 清理资源
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
    
    return (status == Ok);
}

bool ScreenCapturer::CaptureSpecificDisplay(int displayIndex, std::wstring& outputPath) {
    if (displayIndex < 0 || displayIndex >= (int)m_displays.size()) {
        return false;
    }
    
    DISPLAY_DEVICE displayDevice = m_displays[displayIndex];
    DEVMODE devMode;
    devMode.dmSize = sizeof(DEVMODE);
    EnumDisplaySettings(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &devMode);
    
    HDC hdcScreen = CreateDC(NULL, displayDevice.DeviceName, NULL, NULL);
    if (!hdcScreen) return false;
    
    int screenWidth = devMode.dmPelsWidth;
    int screenHeight = devMode.dmPelsHeight;
    
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screenWidth, screenHeight);
    SelectObject(hdcMem, hBitmap);
    
    BitBlt(hdcMem, 0, 0, screenWidth, screenHeight, hdcScreen, 0, 0, SRCCOPY);
    
    // 生成保存路径
    std::wstring savePath = GetDefaultSavePath();
    CreateDirectoryIfNotExists(savePath);
    
    std::wstring timestamp = GetTimestamp();
    std::wstring filename = (timestamp + L"_display" + std::to_wstring(displayIndex + 1) + 
                           GetFormatExtension(m_imageFormat));
    outputPath = savePath + L"\\" + filename;
    
    // 使用GDI+保存
    Bitmap bitmap(hBitmap, NULL);
    CLSID encoderClsid;
    if (GetEncoderClsid(GetFormatMimeType(m_imageFormat).c_str(), &encoderClsid) == -1) {
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        DeleteDC(hdcScreen);
        return false;
    }
    
    Status status = bitmap.Save(outputPath.c_str(), &encoderClsid, NULL);
    
    // 清理资源
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    DeleteDC(hdcScreen);
    
    return (status == Ok);
}

void ScreenCapturer::SetImageFormat(int format) {
    m_imageFormat = std::max(0, std::min(2, format));
}

void ScreenCapturer::SetImageQuality(int quality) {
    m_imageQuality = std::max(10, std::min(100, quality));
}

void ScreenCapturer::UpdateDisplayList(HWND hCombo) {
    SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
    
    // 添加"所有显示器"选项
    SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"所有显示器");
    
    // 枚举显示器
    m_displays.clear();
    DISPLAY_DEVICE displayDevice;
    displayDevice.cb = sizeof(DISPLAY_DEVICE);
    
    for (int i = 0; EnumDisplayDevices(NULL, i, &displayDevice, 0); i++) {
        if (displayDevice.StateFlags & DISPLAY_DEVICE_ACTIVE) {
            DEVMODE devMode;
            devMode.dmSize = sizeof(DEVMODE);
            EnumDisplaySettings(displayDevice.DeviceName, ENUM_CURRENT_SETTINGS, &devMode);
            
            std::wstring desc = displayDevice.DeviceString;
            desc += L" (" + std::to_wstring(devMode.dmPelsWidth) + 
                    L"x" + std::to_wstring(devMode.dmPelsHeight) + L")";
            
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)desc.c_str());
            m_displays.push_back(displayDevice);
        }
    }
    
    // 默认选择第一个
    SendMessage(hCombo, CB_SETCURSEL, 0, 0);
}

std::wstring ScreenCapturer::GetTimestamp() {
    std::time_t now = std::time(nullptr);
    std::tm tm_now;
    localtime_s(&tm_now, &now);
    
    std::wstringstream ss;
    ss << std::put_time(&tm_now, L"%Y-%m-%d_%H-%M-%S");
    return ss.str();
}

std::wstring ScreenCapturer::GetDefaultSavePath() {
    wchar_t path[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_PICTURES, NULL, 0, path) == S_OK) {
        return std::wstring(path) + L"\\ScreenCaptures";
    }
    return L"C:\\ScreenCaptures";
}

bool ScreenCapturer::CreateDirectoryIfNotExists(const std::wstring& path) {
    DWORD attrib = GetFileAttributes(path.c_str());
    if (attrib == INVALID_FILE_ATTRIBUTES) {
        return CreateDirectory(path.c_str(), NULL) || 
               GetLastError() == ERROR_ALREADY_EXISTS;
    }
    return (attrib & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

int ScreenCapturer::GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0;
    UINT size = 0;
    
    GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;
    
    ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL) return -1;
    
    GetImageEncoders(num, size, pImageCodecInfo);
    
    for (UINT i = 0; i < num; ++i) {
        if (wcscmp(pImageCodecInfo[i].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[i].Clsid;
            free(pImageCodecInfo);
            return i;
        }
    }
    
    free(pImageCodecInfo);
    return -1;
}

std::wstring ScreenCapturer::GetFormatExtension(int format) {
    switch (format) {
        case 0: return L".png";
        case 1: return L".jpg";
        case 2: return L".bmp";
        default: return L".png";
    }
}

std::wstring ScreenCapturer::GetFormatMimeType(int format) {
    switch (format) {
        case 0: return L"image/png";
        case 1: return L"image/jpeg";
        case 2: return L"image/bmp";
        default: return L"image/png";
    }
}