@echo off
chcp 65001 >nul
echo ===========================================
echo          屏幕截图工具 - MinGW 编译脚本
echo ===========================================

REM 设置工作目录和变量
setlocal enabledelayedexpansion
set WORKSPACE=%~dp0..
set SRC_DIR=%WORKSPACE%\src
set RES_DIR=%WORKSPACE%\res
set OUTPUT_NAME=ScreenCaptureTool.exe

REM 检查是否需要清理旧构建
if "%1"=="clean" (
    echo 正在清理旧的构建文件...
    if exist "%WORKSPACE%\build" (
        rmdir /s /q "%WORKSPACE%\build"
    )
    echo 清理完成。
    pause
    exit /b 0
)

REM 创建构建目录
if not exist "%WORKSPACE%\build" (
    mkdir "%WORKSPACE%\build"
)
if not exist "%WORKSPACE%\build\Debug" (
    mkdir "%WORKSPACE%\build\Debug"
)
if not exist "%WORKSPACE%\build\Release" (
    mkdir "%WORKSPACE%\build\Release"
)

REM 检查是否指定了构建类型，默认为Debug
set BUILD_TYPE=%1
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Debug

REM 检查MinGW是否安装
echo 检查MinGW环境...
where g++ >nul 2>nul
if errorlevel 1 (
    echo 错误: 未找到MinGW (g++) 编译器
    echo.
    echo 请按照以下步骤操作:
    echo 1. 下载MinGW-w64: https://sourceforge.net/projects/mingw-w64/
    echo 2. 或使用MSYS2: https://www.msys2.org/
    echo 3. 安装后确保将bin目录添加到系统PATH
    echo.
    echo 例如，如果MinGW安装在 C:\mingw-w64:
    echo set PATH=C:\mingw-w64\x86_64-8.1.0-win32-seh-rt_v6-rev0\mingw64\bin;%%PATH%%
    pause
    exit /b 1
)

where windres >nul 2>nul
if errorlevel 1 (
    echo 警告: 未找到windres资源编译器
    echo 将尝试不包含资源文件编译...
    set NO_RESOURCES=1
) else (
    set NO_RESOURCES=0
)

echo.
echo 编译器信息:
g++ --version
if %NO_RESOURCES%==0 (
    windres --version
)
echo.

REM 显示编译配置
echo 构建配置:
echo   工作目录:  %WORKSPACE%
echo   源代码目录: %SRC_DIR%
echo   资源目录:   %RES_DIR%
echo   构建类型:   %BUILD_TYPE%
echo.

REM 根据构建类型设置编译选项
if /i "%BUILD_TYPE%"=="Release" (
    echo 构建发布版本 (优化，无调试信息)...
    set OPTIMIZE=-O2 -s
    set DEBUG_FLAGS=-DNDEBUG
    set OUTPUT_DIR=%WORKSPACE%\build\Release
) else (
    echo 构建调试版本 (包含调试信息)...
    set OPTIMIZE=-g -Og
    set DEBUG_FLAGS=-D_DEBUG
    set OUTPUT_DIR=%WORKSPACE%\build\Debug
)

set OUTPUT_FILE=%OUTPUT_DIR%\%OUTPUT_NAME%

REM ========== 编译资源文件 ==========
if %NO_RESOURCES%==0 (
    echo.
    echo [1/4] 编译资源文件...
    
    if not exist "%RES_DIR%\ScreenCaptureTool.rc" (
        echo 错误: 找不到资源文件 %RES_DIR%\ScreenCaptureTool.rc
        echo 请确保资源文件存在，或使用 --no-resources 参数跳过资源编译
        pause
        exit /b 1
    )
    
    REM 编译资源文件为COFF格式
    windres --use-temp-file -i "%RES_DIR%\ScreenCaptureTool.rc" ^
            -o "%OUTPUT_DIR%\ScreenCaptureTool.res" ^
            --include-dir "%SRC_DIR%" ^
            --define _WIN32 ^
            --define UNICODE ^
            --define _UNICODE
    
    if errorlevel 1 (
        echo 资源编译失败，尝试不带预处理器定义...
        windres --use-temp-file -i "%RES_DIR%\ScreenCaptureTool.rc" ^
                -o "%OUTPUT_DIR%\ScreenCaptureTool.res"
        if errorlevel 1 (
            echo 警告: 资源编译失败，将尝试无资源编译
            set NO_RESOURCES=1
        )
    )
    
    if %NO_RESOURCES%==0 (
        echo 资源文件编译成功: %OUTPUT_DIR%\ScreenCaptureTool.res
    )
) else (
    echo.
    echo [1/4] 跳过资源编译 (无windres或用户指定)
)

REM ========== 编译C++源文件 ==========
echo.
echo [2/4] 编译C++源文件...

REM 定义公共编译标志
set COMMON_FLAGS=^
    -std=c++17 ^
    -Wall ^
    -Wextra ^
    -Wpedantic ^
    -Werror ^
    -Wno-unused-parameter ^
    -DUNICODE ^
    -D_UNICODE ^
    -DWIN32 ^
    -D_WIN32_WINNT=0x0601 ^
    -DWINVER=0x0601 ^
    -I"%SRC_DIR%"

REM 列出所有源文件
set SOURCE_FILES=^
    "%SRC_DIR%\main.cpp" ^
    "%SRC_DIR%\MainWindow.cpp" ^
    "%SRC_DIR%\ScreenCapturer.cpp" ^
    "%SRC_DIR%\SettingsManager.cpp" ^
    "%SRC_DIR%\TrayManager.cpp"

echo 编译以下文件:
for %%f in (%SOURCE_FILES%) do echo   %%~nxf

REM 编译每个源文件为目标文件
set OBJ_FILES=
for %%f in (%SOURCE_FILES%) do (
    set OBJ_NAME=%%~nf.obj
    set OBJ_FILES=!OBJ_FILES! "%OUTPUT_DIR%\%%~nf.obj"
    
    echo 正在编译 %%~nxf...
    g++ %COMMON_FLAGS% %DEBUG_FLAGS% %OPTIMIZE% ^
        -c "%%f" ^
        -o "%OUTPUT_DIR%\%%~nf.obj"
    
    if errorlevel 1 (
        echo 错误: 编译失败 - %%~nxf
        pause
        exit /b 1
    )
)

REM ========== 链接可执行文件 ==========
echo.
echo [3/4] 链接生成可执行文件...

REM 构建链接器选项
set LINKER_FLAGS=^
    -mwindows ^
    -static-libgcc ^
    -static-libstdc++ ^
    -Wl,--subsystem,windows

REM 构建库链接列表
set LIBRARIES=^
    -lgdiplus ^
    -lgdi32 ^
    -luser32 ^
    -lkernel32 ^
    -lcomctl32 ^
    -lshell32 ^
    -lole32 ^
    -luuid ^
    -lcomdlg32 ^
    -ladvapi32 ^
    -lwinspool

REM 执行链接
echo 正在链接...
g++ %OBJ_FILES% ^
    %NO_RESOURCES%==0 && echo "%OUTPUT_DIR%\ScreenCaptureTool.res" ^
    -o "%OUTPUT_FILE%" ^
    %LINKER_FLAGS% ^
    %LIBRARIES%

if errorlevel 1 (
    echo 错误: 链接失败
    pause
    exit /b 1
)

echo 可执行文件创建成功: %OUTPUT_FILE%

REM ========== 后处理步骤 ==========
echo.
echo [4/4] 执行后处理步骤...

REM 复制必要的DLL文件（如果使用动态链接）
REM 检查是否需要复制GDI+ DLL
where gdiplus.dll >nul 2>nul
if not errorlevel 1 (
    echo 发现gdiplus.dll，正在复制到输出目录...
    copy gdiplus.dll "%OUTPUT_DIR%\" >nul
)

REM 显示文件信息
echo.
echo 文件信息:
dir /b "%OUTPUT_FILE%"
echo.
echo 文件大小: 
for %%F in ("%OUTPUT_FILE%") do echo   %%~zF 字节

if %NO_RESOURCES%==0 (
    echo 构建包含资源文件。
) else (
    echo 构建不包含资源文件，程序图标将使用默认图标。
)

REM ========== 验证可执行文件 ==========
echo.
echo 验证可执行文件...
if exist "%OUTPUT_FILE%" (
    echo ✓ 构建成功！
    echo.
    echo 运行程序: "%OUTPUT_FILE%"
    echo 或双击执行。
    
    REM 询问是否立即运行
    set /p RUN_PROGRAM="是否立即运行程序? (Y/N): "
    if /i "!RUN_PROGRAM!"=="Y" (
        echo 启动程序...
        start "" "%OUTPUT_FILE%"
    )
) else (
    echo ✗ 构建失败，输出文件不存在。
)

echo.
echo ===========================================
echo          编译完成
echo ===========================================

REM 清理临时文件（可选）
echo.
set /p CLEANUP="是否清理临时文件(.obj, .res)? (Y/N): "
if /i "!CLEANUP!"=="Y" (
    echo 清理临时文件...
    del /q "%OUTPUT_DIR%\*.obj" 2>nul
    del /q "%OUTPUT_DIR%\*.res" 2>nul
    echo 临时文件已清理。
)

pause
exit /b 0