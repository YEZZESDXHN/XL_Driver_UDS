#include <stdio.h>
#include "XL_Driver.h"
#include"uds_tp.h"
#include<time.h>
#include"paneldesign.h"


/******************************************************************************
* 函数名称: void uds_data_indication(uint8_t* msg_buf, uint16_t msg_dlc, n_result_t n_result)
* 功能说明: 接收UDS报文数据
* 输入参数: 无
* 输出参数:	uint8_t*		msg_buf		接收到的UDS数据，单帧或多帧
			uint16_t		msg_dlc		接收到的UDS数据长度
			n_result_t		n_result	接收状态
* 函数返回: XL_SUCCESS,XL_ERROR
* 其它说明:
******************************************************************************/
void uds_data_indication(uint8_t* msg_buf, uint16_t msg_dlc, n_result_t n_result)
{
	
	if (n_result == N_OK)//消息接收完成，包括成功接收完成多帧
	{
		//printf("RX SUCCEED:ID:%4X\tDatalen:%d\tData:", RESPONSE_ID, msg_dlc);
		//for (int i = 0; i < msg_dlc; i++)
		//{
		//	printf("%02X ", msg_buf[i]);
		//}
		//printf("\n");
		settexttocontrol(Edit_out, "RX:", 1);
		setHEXDatatocontrol(Edit_out, msg_buf, msg_dlc,0);
	}
	else if(n_result == N_FF_MSG)//接收到首帧
	{
		//printf("接收首帧\n");
	}
	else if (n_result == N_TIMEOUT_Bs)                       // TIMER_N_BS 定时器超时
	{
		//printf("TIMER_N_BS 定时器超时\n");
		settexttocontrol(Edit_out, "TIMER_N_BS 定时器超时", 1);
	}
	else if (n_result == N_TIMEOUT_Cr)                       // TIMER_N_CR 定时器超时
	{
		//printf("TIMER_N_CR 定时器超时\n");
		settexttocontrol(Edit_out, "TIMER_N_CR 定时器超时", 1);
	}
	else if (n_result == N_WRONG_SN)                         // 接收到的连续帧帧序号错误
	{
		//printf("接收到的连续帧帧序号错误\n");
		settexttocontrol(Edit_out, "接收到的连续帧帧序号错误", 1);
	}
	else if (n_result == N_INVALID_FS)                       // 接收到的流控帧中流状态非法
	{
		//printf("接收到的流控帧中流状态非法\n");
		settexttocontrol(Edit_out, "接收到的流控帧中流状态非法", 1);
	}
	else if (n_result == N_UNEXP_PDU)                        // 不是期待的帧类型，比如在接收连续帧中莫名收到首帧
	{
		//printf("不是期待的帧类型，比如在接收连续帧中莫名收到首帧\n");
		settexttocontrol(Edit_out, "不是期待的帧类型", 1);
	}
	else if (n_result == N_BUFFER_OVFLW)                     // 接收到的流控帧中流状态为溢出
	{
		//printf("接收到的流控帧中流状态为溢出\n");
		settexttocontrol(Edit_out, "接收到的流控帧中流状态为溢出", 1);
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

//clock_t t1 = 0, t2 = 0;
//void timer_tu_doing()//时钟周期循环内容
//
//{
//	network_task(uds_send_can_farme);
//
//}
//
//void timer_tu_start(int n)
//
//{
//	//float t;
//	//t = n;
//	//while (1)
//
//	//{
//	//	timer_tu_doing();
//
//	//	Sleep(t);
//
//	//}
//
//	while (1)
//	{
//		if (t2 - t1 >= n)
//		{
//			t1 = t2;
//			timer_tu_doing();
//		}
//		else
//		{
//			t2 = clock();
//		}
//	}
//
//}
//
//int timer_tu(int mtime)
//{
//
//	int pd;
//
//	if (mtime > 0)
//
//	{
//
//		pd = _beginthread(timer_tu_start, 0, mtime);
//
//		return 1;
//
//	}
//
//	else
//
//		return 0;
//
//}











int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	setlocale(LC_ALL, "");


	// 初始化全局字符串
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_VSWIN, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);






	// 执行应用程序初始化:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VSWIN));

	LoadLibrary(L"Msftedit.dll");
	LoadLibrary(L"Comctl32.dll");

	BT_start = CreateWindowExW(
		0, L"button", L"start", WS_CHILDWINDOW | WS_VISIBLE | BS_PUSHBUTTON, 10, 10,
		50, 30, GUI, (HMENU)BTstart, GetModuleHandle(NULL), NULL
	);

	BT_Send = CreateWindowExW(
		0, L"button", L"Send", WS_CHILDWINDOW | WS_VISIBLE | BS_PUSHBUTTON, 350, 100,
		50, 30, GUI, (HMENU)BTSend, GetModuleHandle(NULL), NULL
	);

	Groupbox_1 = CreateWindowExW(
		0, L"button", L"Groupbox1", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON|WS_GROUP|BS_GROUPBOX, 600, 10,
		100, 100, GUI, (HMENU)Groupbox1, GetModuleHandle(NULL), NULL
	);



	hTabCtel = CreateWindowExW(
		0, WC_TABCONTROL, NULL, WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER| TCS_BOTTOM, 10, 100,
		500, 500, GUI, (HMENU)TabCtel, GetModuleHandle(NULL), NULL
	);

	TCITEM tcItem = { 0 };
	tcItem.mask = TCIF_IMAGE | TCIF_TEXT;
	tcItem.pszText = (LPTSTR)TEXT("诊断");
	tcItem.iImage = 0;
	SendMessage(hTabCtel, TCM_INSERTITEM, 1, (LPARAM)&tcItem);
	tcItem.pszText = (LPTSTR)TEXT("刷写");
	tcItem.iImage = 0;
	SendMessage(hTabCtel, TCM_INSERTITEM, 2, (LPARAM)&tcItem);
	tcItem.pszText = (LPTSTR)TEXT("item3");
	tcItem.iImage = 0;
	SendMessage(hTabCtel, TCM_INSERTITEM, 3, (LPARAM)&tcItem);

	Tab_1 = CreateWindowW(szWindowClass, "tab1", WS_CHILDWINDOW| WS_VISIBLE| WS_BORDER,
		0, 0, 498, 473, hTabCtel, (HMENU)Tab1, hInstance, NULL);
	Tab_2 = CreateWindowW(szWindowClass, "tab2", WS_CHILDWINDOW | WS_BORDER,
		0, 0, 498, 473, hTabCtel, (HMENU)Tab2, hInstance, NULL);
	
	Tab_3 = CreateWindowW(szWindowClass, "tab3", WS_CHILDWINDOW | WS_BORDER,
		0, 0, 498, 473, hTabCtel, (HMENU)Tab3, hInstance, NULL);
	Tab_4 = CreateWindowW(szWindowClass, "tab4", WS_CHILDWINDOW | WS_BORDER,
		0, 0, 498, 473, hTabCtel, (HMENU)Tab4, hInstance, NULL);

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

	

	Channel_List = CreateWindowEx(0, TEXT("comboBOX"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL  | CBS_DROPDOWNLIST,

		100, 10, 200, 200, GUI, (HMENU)ChannelList, GetModuleHandle(NULL), 0);


	Edit_in = CreateWindow(MSFTEDIT_CLASS, NULL,
		WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE |
		ES_UPPERCASE | ES_AUTOVSCROLL, 350, 10, 200, 80, GUI, (HMENU)Editin, hInstance, NULL);

	Edit_out = CreateWindow(MSFTEDIT_CLASS, NULL,
		WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | /*WS_DISABLED|*/
		ES_UPPERCASE | ES_AUTOVSCROLL, 800, 10, 400, 300, GUI, (HMENU)Editout, hInstance, NULL);

	
	XLstatus          xlStatus;
	memset(&g_canFdParams, 0, sizeof(g_canFdParams));
	g_canFdParams.arbitrationBitRate = 500000;
	g_canFdParams.tseg1Abr = 63;
	g_canFdParams.tseg2Abr = 16;
	g_canFdParams.sjwAbr = 2;
	
	// data bitrate
	g_canFdParams.dataBitRate = g_canFdParams.arbitrationBitRate * 4;
	g_canFdParams.tseg1Dbr = 15;
	g_canFdParams.tseg2Dbr = 4;
	g_canFdParams.sjwDbr = 2;
	
	
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
	
	
	timer_tu(1);
	
		
	getHWinfo(&g_channel_info);
	
	for (int i = 0; i < g_channel_info.channelcount; i++)
	{
		WCHAR temp_wchar[128];
		char temp_char[128];
		snprintf(temp_char, 128, g_channel_info.ch[i].channelname);
		strncat_s(temp_char, 128, " ", 3);
		strncat_s(temp_char, 128, g_channel_info.ch[i].channeltype, 6);
		Char2Wchar(temp_wchar, temp_char);
		SendMessage(Channel_List, CB_ADDSTRING, 0, (LPARAM)temp_wchar);
	}







	MSG msg;

	// 主消息循环:
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


