@echo off
echo 正在编译屏幕截图工具...
echo.

:: 创建输出文件夹
if not exist "输出" mkdir 输出

:: 编译命令
cl /EHsc /Fe:输出/ScreenCapture.exe src/ScreenCapture.cpp ^
gdi32.lib gdiplus.lib user32.lib shell32.lib comctl32.lib ole32.lib

if %errorlevel% equ 0 (
    echo 编译成功！
    echo 程序在：输出/ScreenCapture.exe
) else (
    echo 编译失败，请检查错误信息
)

pause