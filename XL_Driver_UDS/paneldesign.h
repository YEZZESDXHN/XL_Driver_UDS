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

extern int isFirststart;


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

HWND BT_MSG_Type;
#define BTMSGType		1003

HWND BT_2;
#define BT1		1004
HWND BT_3;
#define BT1		1005
HWND BT_4;
#define BT1		1006

HWND BT_Send;
#define BTSend		1007




HWND Edit_in;
#define Editin		2001

HWND Edit_out;
#define Editout		2001


HWND CAN_Config;
#define CANConfig		3001



HWND Channel_List_Name;
#define ChannelListName		40011
HWND Channel_List;
#define ChannelList		4001



HWND ECU_List;
#define ECUList		4002

HWND Diag_List;
#define DiagList		4003

HWND CAN_Bud_Mode_List;
#define CANBudModeList		4004
HWND CAN_Bud_Mode_List_Name;
#define CANBudModeListName		40041

int Wchar2Char(char* charStr, const wchar_t* wcharStr);
int Char2Wchar(wchar_t* wcharStr, const char* charStr);
void settexttocontrol(HWND hwnd, char text[], int type);
void setHEXtocontrol(HWND hwnd, unsigned short hex, int type);
void setHEXDatatocontrol(HWND hwnd, unsigned char data[], int length, int type);
void gettextwithoutspace(HWND hwnd, char *text);
void char_to_hex(char src[], int len, unsigned char des[]);