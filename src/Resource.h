// Resource.h
#pragma once

// =========================================
// 图标资源ID
// =========================================
#define IDI_MAIN_ICON         1001
#define IDI_TRAY_ICON         1002
#define IDI_PLAY_ICON         1003
#define IDI_STOP_ICON         1004
#define IDI_CAMERA_ICON       1005

// =========================================
// 控件ID定义
// =========================================
#define IDC_START_BTN         101
#define IDC_MANUAL_BTN        102
#define IDC_BROWSE_BTN        103
#define IDC_INTERVAL_EDIT     104
#define IDC_DISPLAY_COMBO     105
#define IDC_PATH_EDIT         106
#define IDC_STATUS_TEXT       107
#define IDC_COUNT_TEXT        108
#define IDC_TIMER_ID          109
#define IDC_MINIMIZE_CHECK    110
#define IDC_TRAY_ICON         111
#define IDC_FORMAT_COMBO      112
#define IDC_QUALITY_SLIDER    113
#define IDC_QUALITY_TEXT      114
#define IDC_START_MINIMIZED   115

// =========================================
// 托盘菜单命令ID
// =========================================
#define IDM_TRAY_EXIT         201
#define IDM_TRAY_SHOW         202
#define IDM_TRAY_MANUAL       203
#define IDM_TRAY_TOGGLE       204

// =========================================
// 字符串资源ID
// =========================================
#define IDS_APP_TITLE         301
#define IDS_APP_NAME          302
#define IDS_STATUS_READY      303
#define IDS_STATUS_CAPTURING  304
#define IDS_STATUS_STOPPED    305
#define IDS_STATUS_SAVED      306
#define IDS_BTN_START         307
#define IDS_BTN_STOP          308
#define IDS_BTN_MANUAL        309
#define IDS_BTN_BROWSE        310
#define IDS_LABEL_INTERVAL    311
#define IDS_LABEL_PATH        312
#define IDS_LABEL_DISPLAY     313
#define IDS_LABEL_FORMAT      314
#define IDS_LABEL_QUALITY     315
#define IDS_MENU_EXIT         316
#define IDS_MENU_SHOW         317
#define IDS_MENU_HIDE         318
#define IDS_MENU_MANUAL       319
#define IDS_MENU_TOGGLE_START 320
#define IDS_MENU_TOGGLE_STOP  321
#define IDS_FORMAT_PNG        322
#define IDS_FORMAT_JPEG       323
#define IDS_FORMAT_BMP        324
#define IDS_DISPLAY_ALL       325
#define IDS_NOTIFY_CAPTURE_OK 326
#define IDS_NOTIFY_CAPTURE_ERR 327

// =========================================
// 自定义窗口消息
// =========================================
#define WM_TRAYICON          (WM_USER + 100)
#define WM_CAPTURE_COMPLETE  (WM_USER + 101)
#define WM_UPDATE_STATUS     (WM_USER + 102)

// =========================================
// Windows系统内置图标标识符
// =========================================

// 这些是Windows SDK中定义的，我们直接使用
// IDI_APPLICATION - 默认应用程序图标
// IDI_INFORMATION - 信息图标（蓝色i）
// IDI_WARNING     - 警告图标（黄色!）
// IDI_ERROR       - 错误图标（红色×）
// IDI_WINLOGO     - Windows徽标
// IDI_SHIELD      - UAC盾牌图标
// IDI_ASTERISK    - 星号图标（同信息）
// IDI_QUESTION    - 问号图标
// IDI_EXCLAMATION - 感叹号图标（同警告）

// =========================================
// 系统内置光标标识符（参考）
// =========================================
// IDC_ARROW     - 标准箭头
// IDC_IBEAM     - I型光标（文本输入）
// IDC_WAIT      - 等待光标（沙漏）
// IDC_CROSS     - 十字光标
// IDC_UPARROW   - 向上箭头
// IDC_SIZE      - 调整大小（旧式）
// IDC_ICON      - 空白（已过时）
// IDC_SIZENWSE  - 西北-东南调整
// IDC_SIZENESW  - 东北-西南调整
// IDC_SIZEWE    - 水平调整
// IDC_SIZENS    - 垂直调整
// IDC_SIZEALL   - 四向调整
// IDC_NO        - 禁止（带斜线的圆圈）
// IDC_HAND      - 手型光标
// IDC_APPSTARTING - 应用程序启动
// IDC_HELP      - 帮助（带问号的箭头）