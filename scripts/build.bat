@echo off
setlocal enabledelayedexpansion

echo ========================================
echo   屏幕截图工具编译脚本
echo ========================================

:: 检查编译器
where cl >nul 2>nul
if %errorlevel% equ 0 (
    echo [✓] 找到 MSVC 编译器
    set COMPILER=msvc
) else (
    where g++ >nul 2>nul
    if %errorlevel% equ 0 (
        echo [✓] 找到 MinGW 编译器
        set COMPILER=mingw
    ) else (
        echo [✗] 未找到编译器！请安装 MSVC 或 MinGW
        pause
        exit /b 1
    )
)

:: 创建输出目录
if not exist "output" mkdir output

:: 编译
if "%COMPILER%"=="msvc" (
    echo 使用 MSVC 编译...
    
    :: 查找 Windows SDK 路径
    for /f "tokens=*" %%i in ('reg query "HKLM\SOFTWARE\Microsoft\Windows Kits\Installed Roots" /v KitsRoot10 2^>nul ^| findstr KitsRoot10') do (
        for /f "tokens=2,*" %%j in ("%%i") do set KITSPATH=%%k
    )
    
    if not defined KITSPATH (
        echo [警告] 未找到 Windows SDK，尝试默认路径...
        set KITSPATH=C:\Program Files (x86)\Windows Kits\10
    )
    
    :: 编译命令
    cl /EHsc /Fe:output\ScreenCapture.exe /O2 /MT /std:c++17 ^
       /I"%KITSPATH%\Include\10.0.22000.0\shared" ^
       /I"%KITSPATH%\Include\10.0.22000.0\um" ^
       /I"%KITSPATH%\Include\10.0.22000.0\winrt" ^
       src\ScreenCapture.cpp ^
       gdi32.lib gdiplus.lib user32.lib shell32.lib comctl32.lib ole32.lib
    
    if %errorlevel% equ 0 (
        echo [✓] 编译成功！
        echo 输出文件: output\ScreenCapture.exe
    ) else (
        echo [✗] 编译失败！
    )
    
) else if "%COMPILER%"=="mingw" (
    echo 使用 MinGW 编译...
    
    g++ -std=c++17 -static -mwindows -o output\ScreenCapture.exe ^
        src\ScreenCapture.cpp ^
        -lgdi32 -lgdiplus -luser32 -lshell32 -lcomctl32 -lole32
    
    if %errorlevel% equ 0 (
        echo [✓] 编译成功！
        echo 输出文件: output\ScreenCapture.exe
    ) else (
        echo [✗] 编译失败！
    )
)

echo.
pause