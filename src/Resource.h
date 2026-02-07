// Resource.h
#pragma once

// 控件ID
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
#define IDM_TRAY_EXIT         112
#define IDM_TRAY_SHOW         113
#define IDM_TRAY_MANUAL       114
#define IDM_TRAY_TOGGLE       115
#define IDC_IMAGE_PREVIEW     116
#define IDC_CAPTURE_MODE      117
#define IDC_QUALITY_SLIDER    118
#define IDC_QUALITY_TEXT      119
#define IDC_FORMAT_COMBO      120
#define IDC_START_MINIMIZED   121

// 自定义消息
#define WM_TRAYICON (WM_USER + 100)
#define WM_CAPTURE_COMPLETE (WM_USER + 101)
#define WM_UPDATE_STATUS (WM_USER + 102)

// 图标资源
#define IDI_MAIN_ICON         1001
#define IDI_TRAY_ICON         1002
#define IDI_PLAY_ICON         1003
#define IDI_STOP_ICON         1004
#define IDI_CAMERA_ICON       1005

// 字符串资源
#define IDS_APP_TITLE         2001
#define IDS_STATUS_READY      2002
#define IDS_STATUS_CAPTURING  2003
#define IDS_STATUS_STOPPED    2004

// 菜单资源
#define IDM_MAIN_MENU         3001