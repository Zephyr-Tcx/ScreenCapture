// SettingsManager.cpp
#include "SettingsManager.h"
#include <shlobj.h>
#include <fstream>
#include <sstream>

#pragma comment(lib, "shell32.lib")

SettingsManager::SettingsManager() 
    : m_interval(5)
    , m_displayIndex(0)
    , m_imageFormat(0)
    , m_imageQuality(90)
    , m_minimizeToTray(true)
    , m_startMinimized(false) {
    
    m_savePath = GetDefaultSavePath();
}

void SettingsManager::LoadSettings() {
    std::wstring iniPath = GetIniPath();
    
    wchar_t buffer[MAX_PATH];
    
    // 加载保存路径
    GetPrivateProfileString(L"Settings", L"SavePath", 
                           m_savePath.c_str(), buffer, MAX_PATH, iniPath.c_str());
    m_savePath = buffer;
    
    // 加载间隔时间
    m_interval = GetPrivateProfileInt(L"Settings", L"Interval", m_interval, iniPath.c_str());
    
    // 加载显示器索引
    m_displayIndex = GetPrivateProfileInt(L"Settings", L"DisplayIndex", m_displayIndex, iniPath.c_str());
    
    // 加载图像格式
    m_imageFormat = GetPrivateProfileInt(L"Settings", L"ImageFormat", m_imageFormat, iniPath.c_str());
    
    // 加载图像质量
    m_imageQuality = GetPrivateProfileInt(L"Settings", L"ImageQuality", m_imageQuality, iniPath.c_str());
    
    // 加载最小化到托盘设置
    m_minimizeToTray = GetPrivateProfileInt(L"Settings", L"MinimizeToTray", m_minimizeToTray, iniPath.c_str());
    
    // 加载启动最小化设置
    m_startMinimized = GetPrivateProfileInt(L"Settings", L"StartMinimized", m_startMinimized, iniPath.c_str());
}

void SettingsManager::SaveSettings() {
    std::wstring iniPath = GetIniPath();
    
    // 创建目录
    std::wstring dirPath = GetAppDataPath();
    CreateDirectory(dirPath.c_str(), NULL);
    
    // 保存设置
    WritePrivateProfileString(L"Settings", L"SavePath", m_savePath.c_str(), iniPath.c_str());
    
    WritePrivateProfileString(L"Settings", L"Interval", 
                             std::to_wstring(m_interval).c_str(), iniPath.c_str());
    
    WritePrivateProfileString(L"Settings", L"DisplayIndex", 
                             std::to_wstring(m_displayIndex).c_str(), iniPath.c_str());
    
    WritePrivateProfileString(L"Settings", L"ImageFormat", 
                             std::to_wstring(m_imageFormat).c_str(), iniPath.c_str());
    
    WritePrivateProfileString(L"Settings", L"ImageQuality", 
                             std::to_wstring(m_imageQuality).c_str(), iniPath.c_str());
    
    WritePrivateProfileString(L"Settings", L"MinimizeToTray", 
                             m_minimizeToTray ? L"1" : L"0", iniPath.c_str());
    
    WritePrivateProfileString(L"Settings", L"StartMinimized", 
                             m_startMinimized ? L"1" : L"0", iniPath.c_str());
}

void SettingsManager::BrowseSaveDirectory(HWND hParent, HWND hEdit) {
    BROWSEINFO bi = { 0 };
    bi.hwndOwner = hParent;
    bi.lpszTitle = L"选择截图保存目录";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl != NULL) {
        wchar_t path[MAX_PATH];
        if (SHGetPathFromIDList(pidl, path)) {
            m_savePath = path;
            SetWindowText(hEdit, path);
        }
        CoTaskMemFree(pidl);
    }
}

std::wstring SettingsManager::GetAppDataPath() const {
    wchar_t path[MAX_PATH];
    if (SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path) == S_OK) {
        return std::wstring(path) + L"\\ScreenCaptureTool";
    }
    return L"";
}

std::wstring SettingsManager::GetDefaultSavePath() const {
    wchar_t path[MAX_PATH];
    if (SHGetFolderPath(NULL, CSIDL_PICTURES, NULL, 0, path) == S_OK) {
        return std::wstring(path) + L"\\ScreenCaptures";
    }
    return L"C:\\ScreenCaptures";
}

std::wstring SettingsManager::GetIniPath() const {
    return GetAppDataPath() + L"\\settings.ini";
}