// main.cpp
#include "MainWindow.h"
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, int nCmdShow) {
    
    // 创建主窗口实例
    MainWindow* pMainWindow = MainWindow::GetInstance();
    
    if (!pMainWindow->Initialize(hInstance)) {
        MessageBox(NULL, L"程序初始化失败!", L"错误", MB_ICONERROR);
        return 1;
    }
    
    // 运行消息循环
    return pMainWindow->Run();
}