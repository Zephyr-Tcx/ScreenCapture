// SettingsManager.h
#pragma once

#include <windows.h>
#include <string>
#include <vector>

class SettingsManager {
public:
    SettingsManager();
    
    // 加载/保存设置
    void LoadSettings();
    void SaveSettings();
    
    // 目录浏览
    void BrowseSaveDirectory(HWND hParent, HWND hEdit);
    
    // Getter/Setter
    std::wstring GetSavePath() const { return m_savePath; }
    void SetSavePath(const std::wstring& path) { m_savePath = path; }
    
    int GetInterval() const { return m_interval; }
    void SetInterval(int interval) { m_interval = interval; }
    
    int GetDisplayIndex() const { return m_displayIndex; }
    void SetDisplayIndex(int index) { m_displayIndex = index; }
    
    int GetImageFormat() const { return m_imageFormat; }
    void SetImageFormat(int format) { m_imageFormat = format; }
    
    int GetImageQuality() const { return m_imageQuality; }
    void SetImageQuality(int quality) { m_imageQuality = quality; }
    
    bool GetMinimizeToTray() const { return m_minimizeToTray; }
    void SetMinimizeToTray(bool enable) { m_minimizeToTray = enable; }
    
    bool GetStartMinimized() const { return m_startMinimized; }
    void SetStartMinimized(bool enable) { m_startMinimized = enable; }
    
private:
    std::wstring GetAppDataPath() const;
    std::wstring GetDefaultSavePath() const;
    std::wstring GetIniPath() const;
    
private:
    std::wstring m_savePath;
    int m_interval;
    int m_displayIndex;
    int m_imageFormat;
    int m_imageQuality;
    bool m_minimizeToTray;
    bool m_startMinimized;
};