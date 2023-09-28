#include <stdio.h>
#include "XL_Driver.h"
#include"uds_tp.h"

void indication(uint8_t* msg_buf, uint16_t msg_dlc, n_result_t n_result)
{
	printf("tp_indication");
}


void uds_data_indication(uint8_t* msg_buf, uint16_t msg_dlc, n_result_t n_result)
{
	if (n_result == N_OK)
	{
		printf("RX:ID:%4X\tData:%02X %02X %02X %02X %02X %02X %02X %02X\n",
			RESPONSE_ID,
			msg_buf[0],
			msg_buf[1],
			msg_buf[2],
			msg_buf[3],
			msg_buf[4],
			msg_buf[5],
			msg_buf[6],
			msg_buf[7]);
	}
	
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

	getHWinfo(&g_channel_info);

	g_xlChannelChooseMask = g_channel_info.ch[choose].channelMask;
	//printf("g_xlChannelChooseMask=%d", g_xlChannelChooseMask);


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
				case 'v':
				{
					unsigned char msgdata[8] = { 1,8,7,6,4,8,2,9 };
					//XLTransmitMsg(0x724, 0, msgdata, 8, 1);
					send_singleframe(uds_send_can_farme, msgdata, 3);
					break;
				}
				case 'i':
				{
					for (int i = 0; i < g_channel_info.channelcount; i++)
					{
						printf("channel:%d  channel name:%s  type:%s\n", g_channel_info.ch[i].channelindex, g_channel_info.ch[i].channelname, g_channel_info.ch[i].channeltype);
					}
					printf("count=%d\n", g_channel_info.channelcount);
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


