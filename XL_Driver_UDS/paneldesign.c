#include"paneldesign.h"
#include"XL_Driver.h"
#include<stdint.h>
#include<time.h>
#include"uds_tp.h"
clock_t t1 = 0, t2 = 0;
void timer_tu_doing()//时钟周期循环内容

{
	network_task(uds_send_can_farme);

}

void timer_tu_start(int n)

{
	//float t;
	//t = n;
	//while (1)

	//{
	//	timer_tu_doing();

	//	Sleep(t);

	//}

	while (1)
	{
		if (t2 - t1 >= n)
		{
			t1 = t2;
			timer_tu_doing();
		}
		else
		{
			t2 = clock();
		}
	}

}

int timer_tu(int mtime)
{

	int pd;

	if (mtime > 0)

	{

		pd = _beginthread(timer_tu_start, 0, mtime);

		return 1;

	}

	else

		return 0;

}






/******************************************************************************
* 函数名称: int Wchar2Char(char* charStr, const wchar_t* wcharStr)
* 功能说明: WCHAR字符串转CHAR
* 输入参数:	char* charStr
			const wchar_t* wcharStr
* 输出参数: 
* 函数返回: 
* 其它说明: 
******************************************************************************/
int Wchar2Char(char* charStr, const wchar_t* wcharStr) {
	int len = WideCharToMultiByte(CP_ACP, 0, wcharStr, wcslen(wcharStr), NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, wcharStr, wcslen(wcharStr), charStr, len, NULL, NULL);
	charStr[len] = '\0';
	return len;
}

/******************************************************************************
* 函数名称: int Char2Wchar(wchar_t* wcharStr, const char* charStr)
* 功能说明: CHAR字符串转WCHAR
* 输入参数:	char* charStr
			const wchar_t* wcharStr
* 输出参数:
* 函数返回:
* 其它说明:
******************************************************************************/
int Char2Wchar(wchar_t* wcharStr, const char* charStr) {
	int len = MultiByteToWideChar(CP_ACP, 0, charStr, strlen(charStr), NULL, 0);

	MultiByteToWideChar(CP_ACP, 0, charStr, strlen(charStr), wcharStr, len);
	wcharStr[len] = '\0';
	return len;
}





void settexttocontrol(HWND hwnd, char text[], int type)
{
	int textlen = GetWindowTextLengthA(hwnd);
	SendMessageA(hwnd, EM_SETSEL, (WPARAM)textlen, (LPARAM)textlen);
	if (type == 1)//换行
	{
		int len = GetWindowTextLengthA(hwnd);
		if (len > 0)
		{
			SendMessageA(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)"\r\n");
		}
		//wcscat_s(text, strlen(text) + 3, "\r\n");

		SendMessageA(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)text);
	}
	else if (type == 0)//不换行
	{
		SendMessageA(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)text);
	}


}
void setHEXtocontrol(HWND hwnd, unsigned short data, int type)
{
	int textlen = GetWindowTextLengthA(hwnd);
	SendMessageA(hwnd, EM_SETSEL, (WPARAM)textlen, (LPARAM)textlen);

	char text[128];
	snprintf(text, 5, "%3.2X", data);
	if (type == 1)//换行
	{
		//wcscat_s(text, strlen(text) + 3, "\r\n");
		int len = GetWindowTextLengthA(hwnd);
		if (len > 0)
		{
			SendMessageA(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)"\r\n");
		}
		SendMessageA(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)text);

	}
	else if (type == 0)//不换行
	{
		SendMessageA(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)text);
	}
}
void setHEXDatatocontrol(HWND hwnd, unsigned char data[], int length, int type)
{
	int textlen = GetWindowTextLengthA(hwnd);
	SendMessageA(hwnd, EM_SETSEL, (WPARAM)textlen, (LPARAM)textlen);

	char text[128];
	int i;
	if (type == 1)//换行
	{
		int len = GetWindowTextLengthA(hwnd);
		if (len > 0)
		{
			SendMessageA(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)"\r\n");
		}
		for (i = 0; i < length; i++)
		{
			setHEXtocontrol(hwnd, data[i], 0);
		}


	}
	else if (type == 0)//不换行
	{
		for (i = 0; i < length; i++)
		{
			//snprintf(text, 5, "%3.2X", hex[i]);
			//wcscat_s(text, strlen(text) + 3, "\r\n");
			//SendMessageA(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)"t_t\r\n");

			//SendMessageA(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)text);
			setHEXtocontrol(hwnd, data[i], 0);
		}
	}
}



void gettextwithoutspace(HWND hwnd, char *text)
{
	WCHAR * str = (WCHAR*)malloc(sizeof(WCHAR) * (128));
	//char temp[128];
	int len = GetWindowTextLengthA(hwnd);

	//SendMessageA(Text_in, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)" ");
	//MessageBox(hWnd, L"空格", L"提示", MB_OK);

	GetWindowText(hwnd, str, len + 1);
	Wchar2Char(text, str);

	int i = 0, j = 0;
	for (i = 0; i < strlen(text); i++) {
		if (text[i] != ' ') {
			text[j++] = text[i];
		}
		else if (text[i] == ' ') {
			continue;
		}
	}
	text[j] = '\0';


}



unsigned char ChartoHex(char asciiChar)
{
	unsigned char finalHexByte;

	finalHexByte = 0x00;
	if (asciiChar >= 0x30 && asciiChar <= 0x39)
	{
		finalHexByte = asciiChar - 0x30;
	}
	else if (asciiChar >= 0x41 && asciiChar <= 0x46)
	{
		finalHexByte = asciiChar - 0x37;
	}
	else if (asciiChar >= 0x61 && asciiChar <= 0x66)
	{
		finalHexByte = asciiChar - 0x57;
	}

	return finalHexByte;
}

void char_to_hex(char src[], int len, unsigned char des[])
{
	/*char char_table[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	uint8_t hex_table[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };
	while (len--)
	{
		*(des++) = char_table[(*src) >> 4];
		*(des++) = char_table[*(src++) & 0x0f];
	}*/
	for (int i = 0; i < len / 2; i++)
	{
		des[i] = ((ChartoHex(src[i * 2])) << 4) + (ChartoHex(src[i * 2 + 1]) & 0x0F);
	}


}







//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VSWIN));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_VSWIN);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 将实例句柄存储在全局变量中

	GUI = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!GUI)
	{
		return FALSE;
	}

	ShowWindow(GUI, nCmdShow);
	UpdateWindow(GUI);

	return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;

		case BTstart:
		{
			//g_xlChannelChooseMask = g_channel_info.ch[g_ChannelChooes].channelMask;
			if (g_ChannelChooes == 0xff)
			{
				MessageBox(hWnd, L"请选择通道", L"提示", MB_OK);
			}
			else
			{
				timer_tu(1);
				g_Run = 1;
			}
			
			break;
		}
		case BTSend:
		{
			if (g_Run = 1)
			{
				char temp[128];
				//MessageBox(hWnd, L"列表---", L"提示", MB_OK);
				gettextwithoutspace(Edit_in, temp);
				int len = strlen(temp);
				if ((len % 2) == 0)
				{
					unsigned char data[128];
					char_to_hex(temp, len, data);
					network_send_udsmsg(uds_send_can_farme, data, len / 2);
				}
				else
				{
					MessageBox(hWnd, L"输入数据长度有误，请检查", L"提示", MB_OK);
				}
				
			}
			else
			{
				MessageBox(hWnd, L"请先运行工程", L"提示", MB_OK);
			}
			
			break;
		}
		case ChannelList:
		{
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
			{
				int temp;
				temp = SendMessage(Channel_List, CB_GETCURSEL, 0, 0);
				g_ChannelChooes = g_channel_info.ch[temp].channelindex - 1;
				g_xlChannelChooseMask = g_channel_info.ch[temp].channelMask;
				//xlChanMaskTx = g_xlDrvConfig.channel[channel_choose].channelMask;
				//setHEXtocontrol(debug_info, channel_choose, 1);

				break;


			}
			default:
				break;
			}
			
			break;
		}


		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	}
	
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此处添加使用 hdc 的任何绘图代码...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CTLCOLORBTN:
	{
		
		break;
	}
	case WM_DRAWITEM:
	{
		
		break;
	}
	case WM_NOTIFY:
	{
		NMHDR *lParam_t;
		NMHDR *wParam_t;

		lParam_t = lParam;
		wParam_t = wParam;



		if((*lParam_t).idFrom==TabCtel)
		{
			switch ((*lParam_t).code)
			{
			case TCN_SELCHANGE:
			{
				int nSel = SendMessage(hTabCtel, TCM_GETCURSEL, 0, 0);
				//MessageBox(hWnd, L"TCN_SELCHANGE", L"提示", MB_OK);
				switch (nSel)
				{
				case 0:
				{
					ShowWindow(Tab_4, SW_HIDE);
					ShowWindow(Tab_3, SW_HIDE);
					ShowWindow(Tab_2, SW_HIDE);
					ShowWindow(Tab_1, SW_SHOW);
					break;
				}
				case 1:
				{
					ShowWindow(Tab_4, SW_HIDE);
					ShowWindow(Tab_3, SW_HIDE);
					ShowWindow(Tab_2, SW_SHOW);
					ShowWindow(Tab_1, SW_HIDE);
					break;
				}
				case 2:
				{
					ShowWindow(Tab_4, SW_HIDE);
					ShowWindow(Tab_3, SW_SHOW);
					ShowWindow(Tab_2, SW_HIDE);
					ShowWindow(Tab_1, SW_HIDE);
					break;
				}
				case 3:
				{
					ShowWindow(Tab_4, SW_SHOW);
					ShowWindow(Tab_3, SW_HIDE);
					ShowWindow(Tab_2, SW_HIDE);
					ShowWindow(Tab_1, SW_HIDE);
					break;
				}
				default:
					break;
				}
				break;
			}
			default:
				break;
			}
		}

	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	
	return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}