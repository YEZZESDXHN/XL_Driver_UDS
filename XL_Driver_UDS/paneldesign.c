#include"paneldesign.h"
#include"XL_Driver.h"
#include<stdint.h>
#include<time.h>
#include"uds_tp.h"
#include"SID34_36_37TransferData.h"
int isFirststart = 1;

LARGE_INTEGER fre = { 0 };//
LARGE_INTEGER startCount = { 0 };
LARGE_INTEGER endCount = { 0 };


double t1 = 0, t2 = 0;
//clock_t t1 = 0, t2 = 0;
void timer_tu_doing()//ʱ������ѭ������

{
	clock_t t;
	t= clock();
	//network_task(uds_send_can_farme);
	printf("time=%f,t=%d,(double)fre.QuadPart=%f,task_cycle=%d\n",t1, t,(double)fre.QuadPart, task_cycle);
}

void timer_tu_start(int n)

{
	/*while (1)

	{
		timer_tu_doing();

		Sleep(n);

	}*/

	while (1)
	{
		if (g_Run == 1)
		{
			//if (t2 - t1 >= n)
			//{
			//	t1 = t2;
			//	timer_tu_doing();
			//}
			//else
			//{
			//	t2 = clock();
			//}





			double dTimeTake = t2 - t1;
			if (dTimeTake >= task_cycle)
			{
				t1 = t2;
				timer_tu_doing();
			}
			else
			{
				QueryPerformanceCounter(&endCount);
				t2 = 1000000 * ((double)endCount.QuadPart) / (double)fre.QuadPart;//1/1000ms
			}




		}
		else
		{
			break;
		}




		
	}

}

int timer_tu(int mtime)
{

	int pd;

	if (mtime > 0)

	{
		QueryPerformanceFrequency(&fre);//
		pd = _beginthread(timer_tu_start, 0, mtime);

		return 1;

	}

	else

		return 0;

}






/******************************************************************************
* ��������: int Wchar2Char(char* charStr, const wchar_t* wcharStr)
* ����˵��: WCHAR�ַ���תCHAR
* �������:	char* charStr
			const wchar_t* wcharStr
* �������: 
* ��������: 
* ����˵��: 
******************************************************************************/
int Wchar2Char(char* charStr, const wchar_t* wcharStr) {
	int len = WideCharToMultiByte(CP_ACP, 0, wcharStr, wcslen(wcharStr), NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, wcharStr, wcslen(wcharStr), charStr, len, NULL, NULL);
	charStr[len] = '\0';
	return len;
}

/******************************************************************************
* ��������: int Char2Wchar(wchar_t* wcharStr, const char* charStr)
* ����˵��: CHAR�ַ���תWCHAR
* �������:	char* charStr
			const wchar_t* wcharStr
* �������:
* ��������:
* ����˵��:
******************************************************************************/
int Char2Wchar(wchar_t* wcharStr, const char* charStr) {
	int len = MultiByteToWideChar(CP_ACP, 0, charStr, strlen(charStr), NULL, 0);

	MultiByteToWideChar(CP_ACP, 0, charStr, strlen(charStr), wcharStr, len);
	wcharStr[len] = '\0';
	return len;
}





void settexttocontrol(HWND hwnd, char text[], int type)
{
	//int textlen = GetWindowTextLengthA(hwnd);
	//SendMessageA(hwnd, EM_SETSEL, (WPARAM)textlen, (LPARAM)textlen);
	if (type == 1)//����
	{
		int len = GetWindowTextLengthA(hwnd);
		if (len > 0)
		{
			SendMessageA(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)"\r\n");
		}
		//wcscat_s(text, strlen(text) + 3, "\r\n");

		SendMessageA(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)text);
	}
	else if (type == 0)//������
	{
		SendMessageA(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)text);
	}


}
void setHEXtocontrol(HWND hwnd, unsigned short data, int type)
{
	//int textlen = GetWindowTextLengthA(hwnd);
	//SendMessageA(hwnd, EM_SETSEL, (WPARAM)textlen, (LPARAM)textlen);

	char text[128];
	snprintf(text, 5, "%3.2X", data);
	if (type == 1)//����
	{
		//wcscat_s(text, strlen(text) + 3, "\r\n");
		int len = GetWindowTextLengthA(hwnd);
		if (len > 0)
		{
			SendMessageA(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)"\r\n");
		}
		SendMessageA(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)text);

	}
	else if (type == 0)//������
	{
		SendMessageA(hwnd, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)text);
	}
}
void setHEXDatatocontrol(HWND hwnd, unsigned char data[], int length, int type)
{
	//int textlen = GetWindowTextLengthA(hwnd);
	//SendMessageA(hwnd, EM_SETSEL, (WPARAM)textlen, (LPARAM)textlen);

	char text[128];
	int i;
	if (type == 1)//����
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
	else if (type == 0)//������
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
	//MessageBox(hWnd, L"�ո�", L"��ʾ", MB_OK);

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
//  ����: MyRegisterClass()
//
//  Ŀ��: ע�ᴰ���ࡣ
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
//   ����: InitInstance(HINSTANCE, int)
//
//   Ŀ��: ����ʵ�����������������
//
//   ע��:
//
//        �ڴ˺����У�������ȫ�ֱ����б���ʵ�������
//        ��������ʾ�����򴰿ڡ�
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // ��ʵ������洢��ȫ�ֱ�����

	GUI = CreateWindowW(szWindowClass, L"XL_UDS", WS_OVERLAPPEDWINDOW,
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
//  ����: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  Ŀ��: ���������ڵ���Ϣ��
//
//  WM_COMMAND  - ����Ӧ�ó���˵�
//  WM_PAINT    - ����������
//  WM_DESTROY  - �����˳���Ϣ������
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
			if (g_Run == 0)
			{
				if (isFirststart == 1)
				{
					if (g_ChannelChooes == 0xff)
					{
						MessageBox(hWnd, L"��ѡ��ͨ��", L"��ʾ", MB_OK);
					}
					else
					{
						XLstatus xlStatus;
						xlStatus = InitCANDriver(g_canFdParams, &g_BaudRate);
						if (xlStatus == XL_SUCCESS)
						{

							xlStatus = CreateRxThread();
						}

						if (XL_SUCCESS == xlStatus) {
							// ------------------------------------
							// go with all selected channels on bus
							// ------------------------------------
							xlStatus = xlActivateChannel(g_xlPortHandle, g_xlChannelCANFDMask, XL_BUS_TYPE_CAN, XL_ACTIVATE_RESET_CLOCK);

						}

						g_Run = 1;
						timer_tu(1);
						SetWindowText(BT_start, TEXT("Stop"));
						isFirststart = 0;
					}
				}
				else
				{
					g_Run = 1;
					timer_tu(1);
					SetWindowText(BT_start, TEXT("Stop"));
				}
				
			}
			else
			{

				g_Run = 0;


				SetWindowText(BT_start, TEXT("Start"));
			}
			
			break;
		}
		case BTMSGType:
		{
			if (SendMessage(BT_MSG_Type, BM_GETCHECK, 0, 0) == BST_CHECKED)//ѡ��
			{
				g_canMsgType = 1;

			}
			else if (SendMessage(BT_MSG_Type, BM_GETCHECK, 0, 0) == BST_UNCHECKED)//δѡ��
			{
				g_canMsgType = 0;
			}
			break;
		}
		case BTSend:
		{
			if (g_Run = 1)
			{
				char temp[128];
				gettextwithoutspace(Edit_in, temp);
				int len = strlen(temp);
				if ((len % 2) == 0)
				{
					unsigned char data[128];
					char_to_hex(temp, len, data);
					network_send_udsmsg(uds_send_can_farme, data, len / 2);
					//if (len / 2 > 7)
					//{
					//	send_multipleframe(uds_send_can_farme, data, len / 2);
					//}
					//else
					//{

					//	network_send_udsmsg(uds_send_can_farme, data, len / 2);
					//}
				}
				else
				{
					MessageBox(hWnd, L"�������ݳ�����������", L"��ʾ", MB_OK);
				}
				
			}
			else
			{
				MessageBox(hWnd, L"�������й���", L"��ʾ", MB_OK);
			}
			
			break;
		}
		case BTflash:
		{
			if (g_Run = 1)
			{

				flash("VIU_37MR_R520_RD1_179_20231020_BANK_1", "VIU_37FF_R500_RX1_158_20230720_BANK_1_t.bin");
				

				//LPITEMIDLIST pil = NULL;
				//INITCOMMONCONTROLSEX InitCtrls = { 0 };
				//TCHAR szBuf[4096] = { 0 };
				//BROWSEINFO bi = { 0 };
				//bi.hwndOwner = NULL;
				//bi.iImage = 0;
				//bi.lParam = NULL;
				//bi.lpfn = NULL;
				//bi.lpszTitle = _T("��ѡ���ļ�·��");
				//bi.pszDisplayName = szBuf;
				//bi.ulFlags = BIF_BROWSEINCLUDEFILES;

				//InitCommonControlsEx(&InitCtrls);//�ڵ��ú���SHBrowseForFolder֮ǰ��Ҫ���øú�����ʼ����ػ���
				//pil = SHBrowseForFolder(&bi);
				//if (NULL != pil)//������ִ�гɹ��������û�ѡ���ʼ�·�������ȷ��
				//{
				//	SHGetPathFromIDList(pil, szBuf);//��ȡ�û�ѡ����ļ�·��
				//	wprintf_s(_T("%s"), szBuf);
				//	printf("%s\n", szBuf);
				//}

			}
			else
			{
				MessageBox(hWnd, L"�������й���", L"��ʾ", MB_OK);
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
		case CANBudModeList:
		{
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
			{
				int temp;
				temp = SendMessage(CAN_Bud_Mode_List, CB_GETCURSEL, 0, 0);
				if (temp == 0)//CAN
				{
					g_canBusMode = 0;
					ShowWindow(BT_MSG_Type, SW_HIDE);    // ���ط���CANFDѡ��

				}
				else if (temp == 1)//CANFD ISO
				{
					g_canBusMode = 1;
					g_canFdModeNoIso = 0;
					ShowWindow(BT_MSG_Type, SW_SHOW);    // ��ʾ����CANFDѡ��
				}
				else if (temp == 2)//CANFD ISO
				{
					g_canBusMode = 1;
					g_canFdModeNoIso = 1;
					ShowWindow(BT_MSG_Type, SW_SHOW);    // ��ʾ����CANFDѡ��
				}


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
		// TODO: �ڴ˴�����ʹ�� hdc ���κλ�ͼ����...
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
				//MessageBox(hWnd, L"TCN_SELCHANGE", L"��ʾ", MB_OK);
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

// �����ڡ������Ϣ��������
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