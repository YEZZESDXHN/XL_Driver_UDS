#include <stdio.h>
#include "XL_Driver.h"
#include"uds_tp.h"





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
		printf("RX SUCCEED:ID:%4X\tDatalen:%d\tData:", RESPONSE_ID, msg_dlc);
		for (int i = 0; i < msg_dlc; i++)
		{
			printf("%02X ", msg_buf[i]);
		}
		printf("\n");
		
	}
	else if(n_result == N_FF_MSG)//接收到首帧
	{
		printf("接收首帧\n");
	}
	else if (n_result == N_TIMEOUT_Bs)                       // TIMER_N_BS 定时器超时
	{
		printf("TIMER_N_BS 定时器超时\n");
	}
	else if (n_result == N_TIMEOUT_Cr)                       // TIMER_N_CR 定时器超时
	{
		printf("TIMER_N_CR 定时器超时\n");
	}
	else if (n_result == N_WRONG_SN)                         // 接收到的连续帧帧序号错误
	{
		printf("接收到的连续帧帧序号错误\n");
	}
	else if (n_result == N_INVALID_FS)                       // 接收到的流控帧中流状态非法
	{
		printf("接收到的流控帧中流状态非法\n");
	}
	else if (n_result == N_UNEXP_PDU)                        // 不是期待的帧类型，比如在接收连续帧中莫名收到首帧
	{
		printf("不是期待的帧类型，比如在接收连续帧中莫名收到首帧\n");
	}
	else if (n_result == N_BUFFER_OVFLW)                     // 接收到的流控帧中流状态为溢出
	{
		printf("接收到的流控帧中流状态为溢出\n");
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



void timer_tu_doing()//时钟周期循环内容

{
	network_task(uds_send_can_farme);

}

void timer_tu_start(int n)

{
	float t;
	t = n;
	while (1)

	{
		timer_tu_doing();

		Sleep(t);

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



















int main()
{
	int choose=0;
	

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

	g_xlChannelChooseMask = g_channel_info.ch[choose].channelMask;
	

	int           c;
	while (1) {

		unsigned long n;
		INPUT_RECORD ir;

		ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &ir, 1, &n);

		if ((n == 1) && (ir.EventType == KEY_EVENT)) {

			if (ir.Event.KeyEvent.bKeyDown) {

				c = ir.Event.KeyEvent.uChar.AsciiChar;
				switch (c)
				{
				case 't':
				{
					unsigned char msgdata[128] = { 0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9, 0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9 };
					//XLTransmitMsg(0x724, 0, msgdata, 8, 1);
					//send_singleframe(uds_send_can_farme, msgdata, 3);
					network_send_udsmsg(uds_send_can_farme, msgdata, 5);
					break;
				}
				case 'i':
				{
					for (int i = 0; i < g_channel_info.channelcount; i++)
					{
						printf("channel:%d  channel name:%s  type:%s\n", g_channel_info.ch[i].channelindex, g_channel_info.ch[i].channelname, g_channel_info.ch[i].channeltype);
					}
					break;
				}

				case '+':
				{
					choose++;
					if (choose >= g_channel_info.channelcount)
					{
						choose = g_channel_info.channelcount-1;
						
					}
					printf("choose=%d\n", choose);
					g_xlChannelChooseMask = g_channel_info.ch[choose].channelMask;
					break;
				}
				case '-':
				{
					choose--;
					if (choose <0)
					{
						choose = 0;
					}
					g_xlChannelChooseMask = g_channel_info.ch[choose].channelMask;
					printf("choose=%d\n", choose);
					break;
				}

				default:
					break;
					// end switch
				}
			}
		}
	}
}


