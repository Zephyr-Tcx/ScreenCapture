
**BUILD.md**（编译说明）：
```markdown
# 编译指南

## 环境要求

### Windows
- Visual Studio 2017 或更高版本
- 或 MinGW-w64 (GCC)
- Windows SDK

### 开发工具
- VS Code（推荐）或 Visual Studio
- Git（版本管理）

## 编译方法

### 方法1：使用 Visual Studio
1. 打开 Visual Studio
2. 创建新的 "Windows Desktop Wizard" 项目
3. 复制源代码到项目中
4. 项目属性 → 链接器 → 输入 → 附加依赖项：添加 `gdiplus.lib`
5. 编译运行

### 方法2：使用 VS Code
1. 安装 C/C++ 扩展
2. 按 F5 编译调试
3. 或使用预配置的任务

### 方法3：使用命令行
```cmd
# MSVC
cl /EHsc /Fe:ScreenCapture.exe src\ScreenCapture.cpp gdi32.lib gdiplus.lib user32.lib shell32.lib comctl32.lib ole32.lib

# MinGW
g++ -std=c++17 -static -mwindows -o ScreenCapture.exe src\ScreenCapture.cpp -lgdi32 -lgdiplus -luser32 -lshell32 -lcomctl32 -lole32