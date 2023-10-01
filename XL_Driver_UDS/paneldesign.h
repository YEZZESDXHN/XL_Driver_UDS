#pragma once

#include "framework.h"
#include "resource.h"
#include<Richedit.h>//富文本编辑控件的头文件
#include <locale.h>//编码格式
#include<commctrl.h>
#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


HWND GUI;//

HWND hTabCtel;
#define TabCtel		0010

HWND Tab_1;
#define Tab1		0011

HWND Tab_2;
#define Tab2		0012

HWND Tab_3;
#define Tab3		0013

HWND Tab_4;
#define Tab4		0014


HWND BT_start;
#define BTstart		1001

HWND BT_stop;
#define BTstop		1002

HWND BT_1;
#define BT1		1003
HWND BT_2;
#define BT1		1004
HWND BT_3;
#define BT1		1005
HWND BT_4;
#define BT1		1006

HWND Edit_in;
#define Editin		2001

HWND Edit_out;
#define Editout		2001


