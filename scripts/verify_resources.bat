@echo off
chcp 65001 >nul
echo ===========================================
echo          屏幕截图工具 - 资源验证脚本
echo ===========================================

setlocal enabledelayedexpansion
set WORKSPACE=%~dp0..
set SRC_DIR=%WORKSPACE%\src
set RES_DIR=%WORKSPACE%\res

echo 工作目录: %WORKSPACE%
echo 源代码目录: %SRC_DIR%
echo 资源目录: %RES_DIR%
echo.

set ERROR_COUNT=0
set WARNING_COUNT=0

REM ========== 检查必需的文件 ==========
echo [1/4] 检查必需文件...

echo 检查资源头文件...
if exist "%SRC_DIR%\Resource.h" (
    echo   ✓ Resource.h 存在
) else (
    echo   ✗ 错误: 缺少 Resource.h
    echo     此文件必须存在，包含资源ID定义
    set /a ERROR_COUNT+=1
)

echo 检查资源脚本文件...
if exist "%RES_DIR%\ScreenCaptureTool.rc" (
    echo   ✓ ScreenCaptureTool.rc 存在
) else (
    echo   ✗ 错误: 缺少 ScreenCaptureTool.rc
    echo     此文件必须存在，包含资源定义
    set /a ERROR_COUNT+=1
)

echo 检查主窗口源文件...
if exist "%SRC_DIR%\MainWindow.cpp" (
    echo   ✓ MainWindow.cpp 存在
) else (
    echo   ✗ 错误: 缺少 MainWindow.cpp
    set /a ERROR_COUNT+=1
)

if exist "%SRC_DIR%\MainWindow.h" (
    echo   ✓ MainWindow.h 存在
) else (
    echo   ✗ 错误: 缺少 MainWindow.h
    set /a ERROR_COUNT+=1
)

echo 检查主程序文件...
if exist "%SRC_DIR%\main.cpp" (
    echo   ✓ main.cpp 存在
) else (
    echo   ✗ 错误: 缺少 main.cpp
    set /a ERROR_COUNT+=1
)

REM ========== 检查可选的文件 ==========
echo.
echo [2/4] 检查可选文件...

echo 检查截图相关文件...
if exist "%SRC_DIR%\ScreenCapturer.cpp" (
    echo   ✓ ScreenCapturer.cpp 存在
) else (
    echo   ⚠ 警告: 缺少 ScreenCapturer.cpp
    set /a WARNING_COUNT+=1
)

if exist "%SRC_DIR%\ScreenCapturer.h" (
    echo   ✓ ScreenCapturer.h 存在
) else (
    echo   ⚠ 警告: 缺少 ScreenCapturer.h
    set /a WARNING_COUNT+=1
)

echo 检查设置管理文件...
if exist "%SRC_DIR%\SettingsManager.cpp" (
    echo   ✓ SettingsManager.cpp 存在
) else (
    echo   ⚠ 警告: 缺少 SettingsManager.cpp
    set /a WARNING_COUNT+=1
)

if exist "%SRC_DIR%\SettingsManager.h" (
    echo   ✓ SettingsManager.h 存在
) else (
    echo   ⚠ 警告: 缺少 SettingsManager.h
    set /a WARNING_COUNT+=1
)

echo 检查系统托盘管理文件...
if exist "%SRC_DIR%\TrayManager.cpp" (
    echo   ✓ TrayManager.cpp 存在
) else (
    echo   ⚠ 警告: 缺少 TrayManager.cpp
    set /a WARNING_COUNT+=1
)

if exist "%SRC_DIR%\TrayManager.h" (
    echo   ✓ TrayManager.h 存在
) else (
    echo   ⚠ 警告: 缺少 TrayManager.h
    set /a WARNING_COUNT+=1
)

REM ========== 检查图标文件 ==========
echo.
echo [3/4] 检查图标文件...

echo 注意: 本项目使用Windows系统内置图标
echo       无需外部图标文件即可编译
echo.

REM 检查是否有图标文件（可选）
set ICON_FILES=ScreenCaptureTool.ico tray.ico play.ico stop.ico camera.ico
set HAS_ICONS=0

for %%i in (%ICON_FILES%) do (
    if exist "%RES_DIR%\%%i" (
        if !HAS_ICONS!==0 (
            echo 检测到图标文件:
            set HAS_ICONS=1
        )
        echo   ✓ %%~i 存在
    )
)

if !HAS_ICONS!==0 (
    echo   ℹ 未发现图标文件
    echo     将使用Windows系统内置图标
    echo.
    echo 如需使用自定义图标，请在res目录中添加:
    echo   • ScreenCaptureTool.ico (主程序图标)
    echo   • tray.ico (系统托盘图标)
    echo   • play.ico (开始按钮图标)
    echo   • stop.ico (停止按钮图标)
    echo   • camera.ico (截图按钮图标)
)

REM ========== 验证资源文件内容 ==========
echo.
echo [4/4] 验证资源文件内容...

echo 检查Resource.h文件内容...
if exist "%SRC_DIR%\Resource.h" (
    findstr /C:"#define IDI_MAIN_ICON" "%SRC_DIR%\Resource.h" >nul
    if errorlevel 1 (
        echo   ⚠ 警告: Resource.h中未找到IDI_MAIN_ICON定义
        set /a WARNING_COUNT+=1
    ) else (
        echo   ✓ Resource.h包含图标ID定义
    )
    
    findstr /C:"#define IDC_START_BTN" "%SRC_DIR%\Resource.h" >nul
    if errorlevel 1 (
        echo   ⚠ 警告: Resource.h中未找到控件ID定义
    ) else (
        echo   ✓ Resource.h包含控件ID定义
    )
)

echo 检查ScreenCaptureTool.rc文件内容...
if exist "%RES_DIR%\ScreenCaptureTool.rc" (
    findstr /C:"IDI_MAIN_ICON.*ICON" "%RES_DIR%\ScreenCaptureTool.rc" >nul
    if errorlevel 1 (
        echo   ⚠ 警告: .rc文件中未找到图标定义
        set /a WARNING_COUNT+=1
    ) else (
        echo   ✓ .rc文件包含图标定义
    )
    
    REM 检查是使用系统图标还是外部图标
    findstr /C:"IDI_APPLICATION" "%RES_DIR%\ScreenCaptureTool.rc" >nul
    if errorlevel 1 (
        findstr /C:".ico" "%RES_DIR%\ScreenCaptureTool.rc" >nul
        if not errorlevel 1 (
            echo   ℹ .rc文件引用外部.ico文件
            if !HAS_ICONS!==0 (
                echo   ✗ 错误: .rc引用.ico文件但图标文件不存在
                set /a ERROR_COUNT+=1
            )
        )
    ) else (
        echo   ℹ .rc文件使用Windows系统内置图标
    )
)

REM ========== 总结 ==========
echo.
echo ===========================================
echo          验证结果
echo ===========================================

if %ERROR_COUNT% gtr 0 (
    echo 状态: ✗ 失败 (发现 %ERROR_COUNT% 个错误)
    echo.
    echo 必需文件缺失，无法编译。请修复以下问题：
    echo.
    
    REM 重新显示错误
    echo 必需文件检查:
    if not exist "%SRC_DIR%\Resource.h" echo   • 缺少 Resource.h
    if not exist "%RES_DIR%\ScreenCaptureTool.rc" echo   • 缺少 ScreenCaptureTool.rc
    if not exist "%SRC_DIR%\MainWindow.cpp" echo   • 缺少 MainWindow.cpp
    if not exist "%SRC_DIR%\MainWindow.h" echo   • 缺少 MainWindow.h
    if not exist "%SRC_DIR%\main.cpp" echo   • 缺少 main.cpp
    
    echo.
    echo 请创建缺失的文件或从示例代码复制。
    set EXIT_CODE=1
) else (
    if %WARNING_COUNT% gtr 0 (
        echo 状态: ⚠ 警告 (发现 %WARNING_COUNT% 个警告)
        set EXIT_CODE=0
    ) else (
        echo 状态: ✓ 通过
        set EXIT_CODE=0
    )
)

echo.
echo 目录结构:
echo   %WORKSPACE%
echo   ├── src\
echo   │   ├── main.cpp
echo   │   ├── MainWindow.cpp/h
echo   │   ├── ScreenCapturer.cpp/h
echo   │   ├── SettingsManager.cpp/h
echo   │   ├── TrayManager.cpp/h
echo   │   └── Resource.h
echo   ├── res\
echo   │   └── ScreenCaptureTool.rc
echo   └── scripts\
echo       ├── build_mingw.bat
echo       └── verify_resources.bat

echo.
echo 下一步操作:
if %EXIT_CODE%==0 (
    echo 1. 运行编译脚本: build_mingw.bat
    echo 2. 如果需要自定义图标，在res目录中添加.ico文件
    echo 3. 修改.rc文件引用你的图标文件
) else (
    echo 1. 先解决上述错误，创建缺失的文件
    echo 2. 然后重新运行此验证脚本
    echo 3. 验证通过后再运行编译脚本
)

echo.
echo 按任意键退出...
pause >nul
exit /b %EXIT_CODE%