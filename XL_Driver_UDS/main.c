#include <stdio.h>
#include "XL_Driver.h"
#include"uds_tp.h"
#include<time.h>
#include"paneldesign.h"



/******************************************************************************
* ��������: void uds_data_indication(uint8_t* msg_buf, uint16_t msg_dlc, n_result_t n_result)
* ����˵��: ����UDS��������
* �������: ��
* �������:	uint8_t*		msg_buf		���յ���UDS���ݣ���֡���֡
			uint16_t		msg_dlc		���յ���UDS���ݳ���
			n_result_t		n_result	����״̬
* ��������: XL_SUCCESS,XL_ERROR
* ����˵��:
******************************************************************************/
void uds_data_indication(uint8_t* msg_buf, uint16_t msg_dlc, n_result_t n_result)
{
	if (n_result == N_OK)//��Ϣ������ɣ������ɹ�������ɶ�֡
	{
		printf("RX SUCCEED:ID:%4X\tDatalen:%d\tData:", RESPONSE_ID, msg_dlc);
		for (int i = 0; i < msg_dlc; i++)
		{
			printf("%02X ", msg_buf[i]);
		}
		printf("\n");
		
	}
	else if(n_result == N_FF_MSG)//���յ���֡
	{
		printf("������֡\n");
	}
	else if (n_result == N_TIMEOUT_Bs)                       // TIMER_N_BS ��ʱ����ʱ
	{
		printf("TIMER_N_BS ��ʱ����ʱ\n");
	}
	else if (n_result == N_TIMEOUT_Cr)                       // TIMER_N_CR ��ʱ����ʱ
	{
		printf("TIMER_N_CR ��ʱ����ʱ\n");
	}
	else if (n_result == N_WRONG_SN)                         // ���յ�������֡֡��Ŵ���
	{
		printf("���յ�������֡֡��Ŵ���\n");
	}
	else if (n_result == N_INVALID_FS)                       // ���յ�������֡����״̬�Ƿ�
	{
		printf("���յ�������֡����״̬�Ƿ�\n");
	}
	else if (n_result == N_UNEXP_PDU)                        // �����ڴ���֡���ͣ������ڽ�������֡��Ī���յ���֡
	{
		printf("�����ڴ���֡���ͣ������ڽ�������֡��Ī���յ���֡\n");
	}
	else if (n_result == N_BUFFER_OVFLW)                     // ���յ�������֡����״̬Ϊ���
	{
		printf("���յ�������֡����״̬Ϊ���\n");
	}
	else if (n_result == N_TX_OK)
	{
		printf("TX SUCCEED:ID:%4X\tDatalen:%d\tData:", REQUEST_ID, msg_dlc);
		for (int i = 0; i < msg_dlc; i++)
		{
			printf("%02X ", msg_buf[i]);
		}
		printf("\n");
	}
	
	
}

clock_t t1 = 0, t2 = 0;
void timer_tu_doing()//ʱ������ѭ������

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

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	setlocale(LC_ALL, "");


	// ��ʼ��ȫ���ַ���
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_VSWIN, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);






	// ִ��Ӧ�ó����ʼ��:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VSWIN));

	LoadLibrary(L"Msftedit.dll");
	BT_start = CreateWindowExW(
		0, L"button", L"start", WS_CHILDWINDOW | WS_VISIBLE | BS_PUSHBUTTON, 10, 10,
		50, 30, GUI, (HMENU)BTstart, GetModuleHandle(NULL), NULL
	);




	hTabCtel = CreateWindowExW(
		0, WC_TABCONTROL, NULL, WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER| TCS_TABS, 10, 100,
		500, 500, GUI, (HMENU)TabCtel, GetModuleHandle(NULL), NULL
	);

	TCITEM tcItem = { 0 };
	tcItem.mask = TCIF_IMAGE | TCIF_TEXT;
	tcItem.pszText = (LPTSTR)TEXT("���");
	tcItem.iImage = 0;
	SendMessage(hTabCtel, TCM_INSERTITEM, 1, (LPARAM)&tcItem);
	tcItem.pszText = (LPTSTR)TEXT("ˢд");
	tcItem.iImage = 0;
	SendMessage(hTabCtel, TCM_INSERTITEM, 2, (LPARAM)&tcItem);
	tcItem.pszText = (LPTSTR)TEXT("item3");
	tcItem.iImage = 0;
	SendMessage(hTabCtel, TCM_INSERTITEM, 3, (LPARAM)&tcItem);

	Tab_1 = CreateWindowW(szWindowClass, "tab1", WS_CHILDWINDOW| WS_VISIBLE| WS_BORDER,
		0, 25, 498, 473, hTabCtel, (HMENU)Tab1, hInstance, NULL);
	Tab_2 = CreateWindowW(szWindowClass, "tab2", WS_CHILDWINDOW | WS_BORDER,
		0, 25, 498, 473, hTabCtel, (HMENU)Tab2, hInstance, NULL);
	Tab_3 = CreateWindowW(szWindowClass, "tab3", WS_CHILDWINDOW | WS_BORDER,
		0, 25, 498, 473, hTabCtel, (HMENU)Tab3, hInstance, NULL);
	Tab_4 = CreateWindowW(szWindowClass, "tab4", WS_CHILDWINDOW | WS_BORDER,
		0, 25, 498, 473, hTabCtel, (HMENU)Tab4, hInstance, NULL);

	BT_1 = CreateWindowExW(
		0, L"button", L"bt1", WS_CHILDWINDOW | WS_VISIBLE | BS_PUSHBUTTON, 10, 10,
		50, 30, Tab_1, (HMENU)BT1, GetModuleHandle(NULL), NULL
	);
	BT_2 = CreateWindowExW(
		0, L"button", L"bt2", WS_CHILDWINDOW | WS_VISIBLE | BS_PUSHBUTTON, 10, 10,
		50, 30, Tab_2, (HMENU)BT1, GetModuleHandle(NULL), NULL
	);
	BT_3 = CreateWindowExW(
		0, L"button", L"bt3", WS_CHILDWINDOW | WS_VISIBLE | BS_PUSHBUTTON, 10, 10,
		50, 30, Tab_3, (HMENU)BT1, GetModuleHandle(NULL), NULL
	);
	BT_4 = CreateWindowExW(
		0, L"button", L"bt4", WS_CHILDWINDOW | WS_VISIBLE | BS_PUSHBUTTON, 10, 10,
		50, 30, Tab_4, (HMENU)BT1, GetModuleHandle(NULL), NULL
	);

	//Edit_in = CreateWindow(MSFTEDIT_CLASS, NULL,
	//	WS_CHILD | WS_BORDER /*| WS_VISIBLE*/ | WS_VSCROLL | ES_MULTILINE |
	//	ES_UPPERCASE | ES_AUTOVSCROLL, 10, 50, 200, 80, hTabCtel, (HMENU)Editin, hInstance, NULL);
	//Edit_out = CreateWindow(MSFTEDIT_CLASS, NULL,
	//	WS_CHILD | WS_BORDER /*| WS_VISIBLE*/ | WS_VSCROLL | ES_MULTILINE | ES_READONLY | /*WS_DISABLED|*/
	//	ES_UPPERCASE | ES_AUTOVSCROLL, 20, 200, 200, 80, hTabCtel, (HMENU)Editout, hInstance, NULL);
















	MSG msg;

	// ����Ϣѭ��:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

//int main()
//{
//	int choose=0;
//	
//
//	XLstatus          xlStatus;
//	memset(&g_canFdParams, 0, sizeof(g_canFdParams));
//	g_canFdParams.arbitrationBitRate = 500000;
//	g_canFdParams.tseg1Abr = 63;
//	g_canFdParams.tseg2Abr = 16;
//	g_canFdParams.sjwAbr = 2;
//
//	// data bitrate
//	g_canFdParams.dataBitRate = g_canFdParams.arbitrationBitRate * 4;
//	g_canFdParams.tseg1Dbr = 15;
//	g_canFdParams.tseg2Dbr = 4;
//	g_canFdParams.sjwDbr = 2;
//
//
//	xlStatus = InitCANDriver(g_canFdParams, &g_BaudRate);
//
//	if (xlStatus == XL_SUCCESS)
//	{
//
//		xlStatus = CreateRxThread();
//	}
//
//	if (XL_SUCCESS == xlStatus) {
//		// ------------------------------------
//		// go with all selected channels on bus
//		// ------------------------------------
//		xlStatus = xlActivateChannel(g_xlPortHandle, g_xlChannelCANFDMask, XL_BUS_TYPE_CAN, XL_ACTIVATE_RESET_CLOCK);
//
//	}
//
//
//	timer_tu(1);
//
//	
//	getHWinfo(&g_channel_info);
//
//	g_xlChannelChooseMask = g_channel_info.ch[choose].channelMask;
//	
//
//	int           c;
//	while (1) {
//
//		unsigned long n;
//		INPUT_RECORD ir;
//
//		ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &ir, 1, &n);
//
//		if ((n == 1) && (ir.EventType == KEY_EVENT)) {
//
//			if (ir.Event.KeyEvent.bKeyDown) {
//
//				c = ir.Event.KeyEvent.uChar.AsciiChar;
//				switch (c)
//				{
//				case 't':
//				{
//					unsigned char msgdata[128] = { 0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9, 0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9 };
//					//XLTransmitMsg(0x724, 0, msgdata, 8, 1);
//					//send_singleframe(uds_send_can_farme, msgdata, 3);
//					network_send_udsmsg(uds_send_can_farme, msgdata, 5);
//					break;
//				}
//				case 'i':
//				{
//					for (int i = 0; i < g_channel_info.channelcount; i++)
//					{
//						printf("channel:%d  channel name:%s  type:%s\n", g_channel_info.ch[i].channelindex, g_channel_info.ch[i].channelname, g_channel_info.ch[i].channeltype);
//					}
//					break;
//				}
//
//				case '+':
//				{
//					choose++;
//					if (choose >= g_channel_info.channelcount)
//					{
//						choose = g_channel_info.channelcount-1;
//						
//					}
//					printf("choose=%d\n", choose);
//					g_xlChannelChooseMask = g_channel_info.ch[choose].channelMask;
//					break;
//				}
//				case '-':
//				{
//					choose--;
//					if (choose <0)
//					{
//						choose = 0;
//					}
//					g_xlChannelChooseMask = g_channel_info.ch[choose].channelMask;
//					printf("choose=%d\n", choose);
//					break;
//				}
//
//				default:
//					break;
//					// end switch
//				}
//			}
//		}
//	}
//}


