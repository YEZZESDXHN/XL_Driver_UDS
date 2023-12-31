#include <stdio.h>
#include "XL_Driver.h"
#include"uds_tp.h"
#include<time.h>
#include"paneldesign.h"
#include"loadconfg.h"

int display = 1;
int             g_TXThreadRun_3E;                                        //!< flag to start/stop the TX thread (for the transmission burst)
HANDLE          g_hTXThread_3E;
int g_3e_flag = 0;
int service_3e_TesterPresent()
{
	unsigned char senddata[FRAME_SIZE];
	senddata[0] = 0x3e;
	senddata[1] = 0x80;
	send_singleframe(uds_send_can_farme, senddata, 2);
}

void demoStopTransmitBurst_3E();

DWORD WINAPI TxThread_3E(LPVOID par)
{

	while (g_TXThreadRun_3E) 
	{
		if (g_Run == 1)
		{
			if (g_3e_flag == 2)
			{
				if (g_3e_flag == 1)
				{
					service_3e_TesterPresent();
				}
				Sleep(3000);

			}
			else if (g_3e_flag == 1)
			{
				Sleep(3000);
				if (g_3e_flag == 1)
				{
					service_3e_TesterPresent();
				}
				
				Sleep(3000);
				g_3e_flag = 2;
			}
			else if (g_3e_flag == 0)
			{

			}
		}
		else
		{
			demoStopTransmitBurst_3E();
		}
		
		
		

	}
}

void demoStopTransmitBurst_3E()
{
	if (g_hTXThread_3E) {
		g_TXThreadRun_3E = 0;
		g_3e_flag = 0;
		WaitForSingleObject(g_hTXThread_3E, 10);
		g_hTXThread_3E = 0;
	}
}

void demoTransmitBurst_3E()
{
	// first collect old TX-Thread
	//demoStopTransmitBurst_3E();
	if (g_3e_flag == 0)
	{
		g_3e_flag = 1;
		g_TXThreadRun_3E = 1;
		g_hTXThread_3E = CreateThread(0, 0x1000, TxThread_3E, NULL, 0, NULL);
	}
	
}






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
		if (display == 1)
		{
			if (display_ascii == 0)
			{
				settexttocontrol(Edit_out, "RX:", 1);
				setHEXDatatocontrol(Edit_out, msg_buf, msg_dlc, 0);
				SendMessageA(Edit_out, WM_VSCROLL, SB_BOTTOM, 0);//设置滚轮到末尾，这样就可以看到最新信息
			}
			else
			{
				settexttocontrol(Edit_out, "RX:", 1);
				setASCIItocontrol(Edit_out, msg_buf, msg_dlc, 0);
				//settexttocontrol(Edit_out,)
				//setHEXDatatocontrol(Edit_out, msg_buf, msg_dlc, 0);
				SendMessageA(Edit_out, WM_VSCROLL, SB_BOTTOM, 0);//设置滚轮到末尾，这样就可以看到最新信息
			}
			
		}
		

		//printf("main RX:");
		//for (int i = 0; i < msg_dlc; i++)
		//{
		//	printf("%02X ", msg_buf[i]);
		//}
		//printf("\n");

		if (msg_buf[0] == 0x67 && msg_buf[1]%2 == 1)
		{
			//Sleep(30);
			//service_27_SecurityAccess(uds_send_can_farme, gDiag_info.ECU_list[ECU_Choose].SecurityAccessDLL, msg_buf, msg_dlc);
			
		}
		else if (msg_buf[0] == 0x50 && msg_buf[1] == 03)
		{
			demoTransmitBurst_3E();
			SendMessage(BT_SEND_3E, BM_SETCHECK, BST_CHECKED, 0);
		}
		else if (msg_buf[0] == 0x50 && msg_buf[1] == 02)
		{
			demoTransmitBurst_3E();
			SendMessage(BT_SEND_3E, BM_SETCHECK, BST_CHECKED, 0);
		}
		else if (msg_buf[0] == 0x50 && msg_buf[1] == 01)
		{
			demoStopTransmitBurst_3E();
			SendMessage(BT_SEND_3E, BM_SETCHECK, BST_UNCHECKED, 0);
		}
		
	}
	else if(n_result == N_FF_MSG)//接收到首帧
	{
		//printf("接收首帧\n");
	}
	else if (n_result == N_TIMEOUT_Bs)                       // TIMER_N_BS 定时器超时
	{
		//printf("TIMER_N_BS 定时器超时\n");
		settexttocontrol(Edit_out, "TIMER_N_BS 定时器超时", 1);
		SendMessageA(Edit_out, WM_VSCROLL, SB_BOTTOM, 0);//设置滚轮到末尾，这样就可以看到最新信息
	}
	else if (n_result == N_TIMEOUT_Cr)                       // TIMER_N_CR 定时器超时
	{
		//printf("TIMER_N_CR 定时器超时\n");
		settexttocontrol(Edit_out, "TIMER_N_CR 定时器超时", 1);
		SendMessageA(Edit_out, WM_VSCROLL, SB_BOTTOM, 0);//设置滚轮到末尾，这样就可以看到最新信息
	}
	else if (n_result == N_WRONG_SN)                         // 接收到的连续帧帧序号错误
	{
		//printf("接收到的连续帧帧序号错误\n");
		settexttocontrol(Edit_out, "接收到的连续帧帧序号错误", 1);
		SendMessageA(Edit_out, WM_VSCROLL, SB_BOTTOM, 0);//设置滚轮到末尾，这样就可以看到最新信息
	}
	else if (n_result == N_INVALID_FS)                       // 接收到的流控帧中流状态非法
	{
		//printf("接收到的流控帧中流状态非法\n");
		settexttocontrol(Edit_out, "接收到的流控帧中流状态非法", 1);
		SendMessageA(Edit_out, WM_VSCROLL, SB_BOTTOM, 0);//设置滚轮到末尾，这样就可以看到最新信息
	}
	else if (n_result == N_UNEXP_PDU)                        // 不是期待的帧类型，比如在接收连续帧中莫名收到首帧
	{
		//printf("不是期待的帧类型，比如在接收连续帧中莫名收到首帧\n");
		settexttocontrol(Edit_out, "不是期待的帧类型", 1);
		SendMessageA(Edit_out, WM_VSCROLL, SB_BOTTOM, 0);//设置滚轮到末尾，这样就可以看到最新信息
	}
	else if (n_result == N_BUFFER_OVFLW)                     // 接收到的流控帧中流状态为溢出
	{
		//printf("接收到的流控帧中流状态为溢出\n");
		settexttocontrol(Edit_out, "接收到的流控帧中流状态为溢出", 1);
		SendMessageA(Edit_out, WM_VSCROLL, SB_BOTTOM, 0);//设置滚轮到末尾，这样就可以看到最新信息
	}
	else if (n_result == N_TIMEROUT_Respone)                     // 接收到的流控帧中流状态为溢出
	{
		//printf("接收到的流控帧中流状态为溢出\n");
		settexttocontrol(Edit_out, "Timeout -no receive message", 1);
		SendMessageA(Edit_out, WM_VSCROLL, SB_BOTTOM, 0);//设置滚轮到末尾，这样就可以看到最新信息
	}
	else if (n_result == N_TX_OK)
	{
		if (display == 1)
		{
			settexttocontrol(Edit_out, "TX:", 1);
			setHEXDatatocontrol(Edit_out, msg_buf, msg_dlc, 0);
			SendMessageA(Edit_out, WM_VSCROLL, SB_BOTTOM, 0);//设置滚轮到末尾，这样就可以看到最新信息
		}
		//printf("main TX:");
		//for (int i = 0; i < msg_dlc; i++)
		//{
		//	printf("%02X ", msg_buf[i]);
		//}
		//printf("\n");
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



//void loadcanconfg(char *filename)
//{
//	memset(&g_canFdParams, 0, sizeof(g_canFdParams));
//	GetIniKeyString("CAN", "arbitrationBitRate", "./Config/CANconfg.ini", CANConfgInfo.arbitrationBitRate);
//	g_canFdParams.arbitrationBitRate = 0;
//	for (int i = 0; i < strlen(CANConfgInfo.arbitrationBitRate) - 1; i++)
//	{
//
//		g_canFdParams.arbitrationBitRate = g_canFdParams.arbitrationBitRate + (CANConfgInfo.arbitrationBitRate[i] - '0') * pow(10, strlen(CANConfgInfo.arbitrationBitRate) - 2 - i);
//	}
//	GetIniKeyString("CAN", "tseg1Abr", "./Config/CANconfg.ini", CANConfgInfo.tseg1Abr);
//	g_canFdParams.tseg1Abr = 0;
//	for (int i = 0; i < strlen(CANConfgInfo.tseg1Abr) - 1; i++)
//	{
//
//		g_canFdParams.tseg1Abr = g_canFdParams.tseg1Abr + (CANConfgInfo.tseg1Abr[i] - '0') * pow(10, strlen(CANConfgInfo.tseg1Abr) - 2 - i);
//	}
//	GetIniKeyString("CAN", "tseg2Abr", "./Config/CANconfg.ini", CANConfgInfo.tseg2Abr);
//	g_canFdParams.tseg2Abr = 0;
//	for (int i = 0; i < strlen(CANConfgInfo.tseg2Abr) - 1; i++)
//	{
//
//		g_canFdParams.tseg2Abr = g_canFdParams.tseg2Abr + (CANConfgInfo.tseg2Abr[i] - '0') * pow(10, strlen(CANConfgInfo.tseg2Abr) - 2 - i);
//	}
//	GetIniKeyString("CAN", "sjwAbr", "./Config/CANconfg.ini", CANConfgInfo.sjwAbr);
//	g_canFdParams.sjwAbr = 0;
//	for (int i = 0; i < strlen(CANConfgInfo.sjwAbr) - 1; i++)
//	{
//
//		g_canFdParams.sjwAbr = g_canFdParams.sjwAbr + (CANConfgInfo.sjwAbr[i] - '0') * pow(10, strlen(CANConfgInfo.sjwAbr) - 2 - i);
//	}
//
//	GetIniKeyString("CAN", "dataBitRate", "./Config/CANconfg.ini", CANConfgInfo.dataBitRate);
//	g_canFdParams.dataBitRate = 0;
//	for (int i = 0; i < strlen(CANConfgInfo.dataBitRate) - 1; i++)
//	{
//
//		g_canFdParams.dataBitRate = g_canFdParams.dataBitRate + (CANConfgInfo.dataBitRate[i] - '0') * pow(10, strlen(CANConfgInfo.dataBitRate) - 2 - i);
//	}
//	GetIniKeyString("CAN", "tseg1Dbr", "./Config/CANconfg.ini", CANConfgInfo.tseg1Dbr);
//	g_canFdParams.tseg1Dbr = 0;
//	for (int i = 0; i < strlen(CANConfgInfo.tseg1Dbr) - 1; i++)
//	{
//
//		g_canFdParams.tseg1Dbr = g_canFdParams.tseg1Dbr + (CANConfgInfo.tseg1Dbr[i] - '0') * pow(10, strlen(CANConfgInfo.tseg1Dbr) - 2 - i);
//	}
//	GetIniKeyString("CAN", "tseg2Dbr", "./Config/CANconfg.ini", CANConfgInfo.tseg2Dbr);
//	g_canFdParams.tseg2Dbr = 0;
//	for (int i = 0; i < strlen(CANConfgInfo.tseg2Dbr) - 1; i++)
//	{
//
//		g_canFdParams.tseg2Dbr = g_canFdParams.tseg2Dbr + (CANConfgInfo.tseg2Dbr[i] - '0') * pow(10, strlen(CANConfgInfo.tseg2Dbr) - 2 - i);
//	}
//	GetIniKeyString("CAN", "sjwDbr", "./Config/CANconfg.ini", CANConfgInfo.sjwDbr);
//	g_canFdParams.sjwDbr = 0;
//	for (int i = 0; i < strlen(CANConfgInfo.sjwDbr) - 1; i++)
//	{
//
//		g_canFdParams.sjwDbr = g_canFdParams.sjwDbr + (CANConfgInfo.sjwDbr[i] - '0') * pow(10, strlen(CANConfgInfo.sjwDbr) - 2 - i);
//	}
//}







int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	setlocale(LC_ALL, "");

	////打开控制台
	//AllocConsole();
	//FILE* stream;
	////freopen_s(&stream, "CON", "r", stdin);//重定向输入流
	//freopen_s(&stream, "CON", "w", stdout);//重定向输入流



	// 初始化全局字符串
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_VSWIN, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	//IHexRecord record_t;
	//ConvertContext ctx_t;
	//ReadHex("VIU_37FF_R500_RX1_158_20230720_BANK_1.hex", "VIU_37FF_R500_RX1_158_20230720_BANK_1_t.bin", &record_t, &ctx_t);
	////Hex2Bin("VIU_37FF_R500_RX1_158_20230720_BANK_1.hex", "VIU_37FF_R500_RX1_158_20230720_BANK_1_t.bin");
	//sid36_download("VIU_37FF_R500_RX1_158_20230720_BANK_1.hex", "VIU_37FF_R500_RX1_158_20230720_BANK_1_t.bin");
	//flash("H37A3631809BD_VIU_37ML_R510_RC4_520_20230925_BANK_1", "VIU_37FF_R500_RX1_158_20230720_BANK_1_t.bin");

	// 执行应用程序初始化:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VSWIN));

	LoadLibrary(L"Msftedit.dll");
	LoadLibrary(L"Comctl32.dll");

	//CAN_Config = CreateWindowExW(
	//	0, szWindowClass, L"CAN_Config", WS_CHILD | WS_VISIBLE| WS_BORDER, 350, 10,
	//	300, 100, GUI, (HMENU)CANConfig, hInstance, 0
	//);

	BT_start = CreateWindowExW(
		0, L"button", L"Start", WS_CHILDWINDOW | WS_VISIBLE | BS_PUSHBUTTON, 10, 10,
		50, 30, GUI, (HMENU)BTstart, GetModuleHandle(NULL), NULL
	);

	//BT_stop= CreateWindowExW(
	//	0, L"button", L"stop", WS_CHILDWINDOW | WS_VISIBLE | BS_PUSHBUTTON, 10, 50,
	//	50, 30, GUI, (HMENU)BTstop, GetModuleHandle(NULL), NULL
	//);

	//ShowWindow(BT_stop, FALSE);    // 隐藏发送CANFD选项

	
	CAN_Bud_Mode_List = CreateWindowEx(0, TEXT("comboBOX"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,

		140, 10, 200, 100, GUI, (HMENU)CANBudModeList, hInstance, 0);
	CAN_Bud_Mode_List_Name = CreateWindow(MSFTEDIT_CLASS, NULL,
		WS_CHILD | ES_READONLY | WS_VISIBLE | ES_MULTILINE | ES_UPPERCASE, 70, 10, 70, 30, GUI, (HMENU)CANBudModeListName, hInstance, NULL);
	settexttocontrol(CAN_Bud_Mode_List_Name, "BusType:", 0);

	BT_Load_CAN_Confg = CreateWindowExW(
		0, L"button", L"加载CAN配置", WS_CHILDWINDOW | WS_VISIBLE | BS_PUSHBUTTON, 350, 10,
		100, 20, GUI, (HMENU)BTLoadCANConfg, GetModuleHandle(NULL), NULL
	);

	




	SendMessage(CAN_Bud_Mode_List, CB_ADDSTRING, 0, L"CAN");
	SendMessage(CAN_Bud_Mode_List, CB_ADDSTRING, 0, L"CANFD IOS");
	SendMessage(CAN_Bud_Mode_List, CB_ADDSTRING, 0, L"CANFD NO IOS");
	//SendMessage(CAN_Bud_Mode_List, CB_SETCUEBANNER, 0, "请选择");
	SendMessage(CAN_Bud_Mode_List, CB_SETCURSEL, 1, NULL);



	hTabCtel = CreateWindowExW(
		0, WC_TABCONTROL, NULL, WS_CHILDWINDOW | WS_VISIBLE | WS_BORDER| TCS_BOTTOM, 10, 100,
		1250, 580, GUI, (HMENU)TabCtel, GetModuleHandle(NULL), NULL
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
		0, 0, 1248, 558, hTabCtel, (HMENU)Tab1, hInstance, NULL);
	Tab_2 = CreateWindowW(szWindowClass, "tab2", WS_CHILDWINDOW | WS_BORDER,
		0, 0, 1248, 558, hTabCtel, (HMENU)Tab2, hInstance, NULL);
	
	Tab_3 = CreateWindowW(szWindowClass, "tab3", WS_CHILDWINDOW | WS_BORDER,
		0, 0, 1248, 558, hTabCtel, (HMENU)Tab3, hInstance, NULL);
	Tab_4 = CreateWindowW(szWindowClass, "tab4", WS_CHILDWINDOW | WS_BORDER,
		0, 0, 1248, 558, hTabCtel, (HMENU)Tab4, hInstance, NULL);

	

	

	Channel_List = CreateWindowEx(0, TEXT("comboBOX"), L"Name", WS_CHILD | WS_VISIBLE | WS_VSCROLL  | CBS_DROPDOWNLIST,

		140, 50, 300, 200, GUI, (HMENU)ChannelList, GetModuleHandle(NULL), 0);
	Channel_List_Name = CreateWindow(MSFTEDIT_CLASS, NULL,
		WS_CHILD | ES_READONLY | WS_VISIBLE | ES_MULTILINE | ES_UPPERCASE, 70, 50, 70, 30, GUI, (HMENU)ChannelListName, hInstance, NULL);
	settexttocontrol(Channel_List_Name, "Channel:", 0);
	

	Edit_in = CreateWindow(MSFTEDIT_CLASS, NULL,
		WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE |
		ES_UPPERCASE | ES_AUTOVSCROLL, 10, 10, 200, 80, Tab_1, (HMENU)Editin, hInstance, NULL);
	BT_MSG_Type = CreateWindowExW(
		0, L"button", L"SendCANFDMsg", WS_CHILDWINDOW | WS_VISIBLE | BS_AUTOCHECKBOX, 220, 10,
		130, 20, Tab_1, (HMENU)BTMSGType, GetModuleHandle(NULL), NULL
	);

	BT_Display_ASCII = CreateWindowExW(
		0, L"button", L"显示ASCII", WS_CHILDWINDOW | WS_VISIBLE | BS_AUTOCHECKBOX, 10, 120,
		130, 20, Tab_1, (HMENU)BTDisplayASCII, GetModuleHandle(NULL), NULL
	);
	
	BT_SEND_3E = CreateWindowExW(
		0, L"button", L"3E Present", WS_CHILDWINDOW | WS_VISIBLE | BS_AUTOCHECKBOX, 150, 120,
		130, 20, Tab_1, (HMENU)BTSEND3E, GetModuleHandle(NULL), NULL
	);




	BT_Send = CreateWindowExW(
		0, L"button", L"Send", WS_CHILDWINDOW | WS_VISIBLE | BS_PUSHBUTTON, 220, 50,
		50, 30, Tab_1, (HMENU)BTSend, GetModuleHandle(NULL), NULL
	);

	Edit_out = CreateWindow(MSFTEDIT_CLASS, NULL,
		WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | /*WS_DISABLED|*/
		ES_UPPERCASE | ES_AUTOVSCROLL, 10, 140, 400, 300, Tab_1, (HMENU)Editout, hInstance, NULL);


	ECU_List = CreateWindowEx(0, TEXT("comboBOX"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,

		450, 50, 200, 200, Tab_1, (HMENU)ECUList, GetModuleHandle(NULL), 0);

	Diag_List = CreateWindowEx(0, TEXT("ListBox"), NULL, WS_VISIBLE | WS_VSCROLL | WS_CHILD | WS_BORDER | LBS_NOTIFY,

		700, 50, 300, 500, Tab_1, (HMENU)DiagList, hInstance, 0);

	ECU_List_Name = CreateWindow(MSFTEDIT_CLASS, NULL,
		WS_CHILD | ES_READONLY | WS_VISIBLE | ES_MULTILINE | ES_UPPERCASE, 450, 10, 200, 30, Tab_1, (HMENU)ECUListName, hInstance, NULL);

	settexttocontrol(ECU_List_Name, "ECU List:", 0);

	Diag_List_Name = CreateWindow(MSFTEDIT_CLASS, NULL,
		WS_CHILD | ES_READONLY | WS_VISIBLE | ES_MULTILINE | ES_UPPERCASE, 700, 10, 200, 30, Tab_1, (HMENU)DiagListName, hInstance, NULL);

	settexttocontrol(Diag_List_Name, "DIAG List:", 0);





	

	






	Flash_file_display = CreateWindow(MSFTEDIT_CLASS, NULL,
		WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL| WS_HSCROLL | ES_MULTILINE | ES_READONLY|
		ES_UPPERCASE | ES_AUTOVSCROLL, 10, 10, 400, 50, Tab_2, (HMENU)Flashfiledisplay, hInstance, NULL);

	Flash_Info_display = CreateWindow(MSFTEDIT_CLASS, NULL,
		WS_CHILD | WS_BORDER | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY |
		ES_UPPERCASE | ES_AUTOVSCROLL, 420, 10, 300, 400, Tab_2, (HMENU)FlashInfodisplay, hInstance, NULL);

	BT_Choose_Flash_App = CreateWindowExW(
		0, L"button", L"请选择文件或拖入文件", WS_CHILDWINDOW | WS_VISIBLE | BS_PUSHBUTTON, 10, 100,
		180, 30, Tab_2, (HMENU)BTChooseFlashApp, GetModuleHandle(NULL), NULL
	);

	BT_flash = CreateWindowExW(
		0, L"button", L"flash", WS_CHILDWINDOW | WS_VISIBLE | BS_PUSHBUTTON, 200, 100,
		100, 30, Tab_2, (HMENU)BTflash, GetModuleHandle(NULL), NULL
	);


	


	//======================加载配置文件=====================================
	if (initDIAG("./Config/ECUlist.ini", &gDiag_info) == -1)
	{
		MessageBox(NULL, L"未添加配置文件", L"提示", MB_OK);
	}
	while (SendMessage(ECU_List, CB_DELETESTRING, 0, 0) > 0)
	{

	}
	for (int i = 0; i < gDiag_info.ECU_num; i++)
	{
		WCHAR test[128];
		Char2Wchar(test, gDiag_info.ECU_list[i].ECU_name);
		SendMessage(ECU_List, CB_ADDSTRING, 0, (LPARAM)test);
	}

	

	memset(&g_canFdParams, 0, sizeof(g_canFdParams));
	GetIniKeyString("CAN", "arbitrationBitRate", "./Config/CANconfg.ini", CANConfgInfo.arbitrationBitRate);
	g_canFdParams.arbitrationBitRate = 0;
	for (int i = 0; i < strlen(CANConfgInfo.arbitrationBitRate) - 1; i++)
	{
		
		g_canFdParams.arbitrationBitRate = g_canFdParams.arbitrationBitRate + (CANConfgInfo.arbitrationBitRate[i]-'0') * pow(10, strlen(CANConfgInfo.arbitrationBitRate) - 2 - i);
	}
	GetIniKeyString("CAN", "tseg1Abr", "./Config/CANconfg.ini", CANConfgInfo.tseg1Abr);
	g_canFdParams.tseg1Abr = 0;
	for (int i = 0; i < strlen(CANConfgInfo.tseg1Abr) - 1; i++)
	{

		g_canFdParams.tseg1Abr = g_canFdParams.tseg1Abr + (CANConfgInfo.tseg1Abr[i] - '0') * pow(10, strlen(CANConfgInfo.tseg1Abr) - 2 - i);
	}
	GetIniKeyString("CAN", "tseg2Abr", "./Config/CANconfg.ini", CANConfgInfo.tseg2Abr);
	g_canFdParams.tseg2Abr = 0;
	for (int i = 0; i < strlen(CANConfgInfo.tseg2Abr) - 1; i++)
	{

		g_canFdParams.tseg2Abr = g_canFdParams.tseg2Abr + (CANConfgInfo.tseg2Abr[i] - '0') * pow(10, strlen(CANConfgInfo.tseg2Abr) - 2 - i);
	}
	GetIniKeyString("CAN", "sjwAbr", "./Config/CANconfg.ini", CANConfgInfo.sjwAbr);
	g_canFdParams.sjwAbr = 0;
	for (int i = 0; i < strlen(CANConfgInfo.sjwAbr) - 1; i++)
	{

		g_canFdParams.sjwAbr = g_canFdParams.sjwAbr + (CANConfgInfo.sjwAbr[i] - '0') * pow(10, strlen(CANConfgInfo.sjwAbr) - 2 - i);
	}

	GetIniKeyString("CAN", "dataBitRate", "./Config/CANconfg.ini", CANConfgInfo.dataBitRate);
	g_canFdParams.dataBitRate = 0;
	for (int i = 0; i < strlen(CANConfgInfo.dataBitRate) - 1; i++)
	{

		g_canFdParams.dataBitRate = g_canFdParams.dataBitRate + (CANConfgInfo.dataBitRate[i] - '0') * pow(10, strlen(CANConfgInfo.dataBitRate) - 2 - i);
	}
	GetIniKeyString("CAN", "tseg1Dbr", "./Config/CANconfg.ini", CANConfgInfo.tseg1Dbr);
	g_canFdParams.tseg1Dbr = 0;
	for (int i = 0; i < strlen(CANConfgInfo.tseg1Dbr) - 1; i++)
	{

		g_canFdParams.tseg1Dbr = g_canFdParams.tseg1Dbr + (CANConfgInfo.tseg1Dbr[i] - '0') * pow(10, strlen(CANConfgInfo.tseg1Dbr) - 2 - i);
	}
	GetIniKeyString("CAN", "tseg2Dbr", "./Config/CANconfg.ini", CANConfgInfo.tseg2Dbr);
	g_canFdParams.tseg2Dbr = 0;
	for (int i = 0; i < strlen(CANConfgInfo.tseg2Dbr) - 1; i++)
	{

		g_canFdParams.tseg2Dbr = g_canFdParams.tseg2Dbr + (CANConfgInfo.tseg2Dbr[i] - '0') * pow(10, strlen(CANConfgInfo.tseg2Dbr) - 2 - i);
	}
	GetIniKeyString("CAN", "sjwDbr", "./Config/CANconfg.ini", CANConfgInfo.sjwDbr);
	g_canFdParams.sjwDbr = 0;
	for (int i = 0; i < strlen(CANConfgInfo.sjwDbr) - 1; i++)
	{

		g_canFdParams.sjwDbr = g_canFdParams.sjwDbr + (CANConfgInfo.sjwDbr[i] - '0') * pow(10, strlen(CANConfgInfo.sjwDbr) - 2 - i);
	}
	







	XLstatus          xlStatus;
	
	//g_canFdParams.arbitrationBitRate = 500000;
	//g_canFdParams.tseg1Abr = 63;
	//g_canFdParams.tseg2Abr = 16;
	//g_canFdParams.sjwAbr = 2;
	//// data bitrate
	//g_canFdParams.dataBitRate = g_canFdParams.arbitrationBitRate * 4;
	//g_canFdParams.tseg1Dbr = 15;
	//g_canFdParams.tseg2Dbr = 4;
	//g_canFdParams.sjwDbr = 2;
	




	xlStatus = GetVectorHWInfo();
	
	
	if (xlStatus == XL_SUCCESS)
	{
		initHWinfo(&g_channel_info);

		for (int i = 0; i < g_channel_info.channelcount; i++)
		{
			WCHAR temp_wchar[128];
			char temp_char[128];

			char serialNumber[8];
			snprintf(serialNumber, 128, "(%06d)", g_channel_info.ch[i].serialNumber);

			snprintf(temp_char, 128, g_channel_info.ch[i].channelname);

			strncat_s(temp_char, 128, " ", 3);
			strncat_s(temp_char, 128, g_channel_info.ch[i].channeltype, 6);
			strncat_s(temp_char, 128, serialNumber, 8);

			//strncat_s(temp_char, 128, "%06d", 3);
			Char2Wchar(temp_wchar, temp_char);
			SendMessage(Channel_List, CB_ADDSTRING, 0, (LPARAM)temp_wchar);
		}
	}
	
	//if (xlStatus == XL_SUCCESS)
	//{
	//
	//	xlStatus = CreateRxThread();
	//}
	//
	//if (XL_SUCCESS == xlStatus) {
	//	// ------------------------------------
	//	// go with all selected channels on bus
	//	// ------------------------------------
	//	xlStatus = xlActivateChannel(g_xlPortHandle, g_xlChannelCANFDMask, XL_BUS_TYPE_CAN, XL_ACTIVATE_RESET_CLOCK);
	//
	//}
	
	
	
		
	
	
	







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


