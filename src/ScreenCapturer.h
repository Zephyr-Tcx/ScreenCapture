// ScreenCapturer.h
#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

#pragma comment(lib, "gdiplus.lib")

class MainWindow;

class ScreenCapturer {
public:
    ScreenCapturer(MainWindow* pWindow);
    ~ScreenCapturer();
    
    void CaptureScreen(bool manual = false);
    void SetImageFormat(int format);
    void SetImageQuality(int quality);
    void UpdateDisplayList(HWND hCombo);
    
private:
    bool InitializeGdiplus();
    void CleanupGdiplus();
    std::wstring GetTimestamp();
    std::wstring GetDefaultSavePath();
    bool CreateDirectoryIfNotExists(const std::wstring& path);
    int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
    std::wstring GetFormatExtension(int format);
    std::wstring GetFormatMimeType(int format);
    
    // 截图实现
    bool CaptureAllDisplays(std::wstring& outputPath);
    bool CaptureSpecificDisplay(int displayIndex, std::wstring& outputPath);
    
private:
    MainWindow* m_pMainWindow;
    std::vector<DISPLAY_DEVICE> m_displays;
    
    int m_imageFormat;  // 0: PNG, 1: JPEG, 2: BMP
    int m_imageQuality; // JPEG质量 (1-100)
    
    ULONG_PTR m_gdiplusToken;
    std::mutex m_captureMutex;
    std::atomic<int> m_captureCount;
};