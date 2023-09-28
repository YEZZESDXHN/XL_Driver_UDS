#include <stdio.h>
#include "XL_Driver.h"
#include"uds_tp.h"

//void tp_indication(uint8_t* msg_buf, uint16_t msg_dlc, n_result_t n_result)
//{
//	printf("tp_indication");
//}
//
//void tp_ffindication_func(n_result_t n_result)
//{
//
//}
//
//
//void tp_confirm_func(n_result_t n_result)
//{
//
//}

int main()
{

	g_xlChannelChooseMask = 1;


	XLstatus          xlStatus;
	//memset(&g_canFdParams, 0, sizeof(g_canFdParams));
	//g_canFdParams.arbitrationBitRate = 500000;
	//g_canFdParams.tseg1Abr = 63;
	//g_canFdParams.tseg2Abr = 16;
	//g_canFdParams.sjwAbr = 2;

	//// data bitrate
	//g_canFdParams.dataBitRate = g_canFdParams.arbitrationBitRate * 4;
	//g_canFdParams.tseg1Dbr = 15;
	//g_canFdParams.tseg2Dbr = 4;
	//g_canFdParams.sjwDbr = 2;


	xlStatus = InitCANDriver(g_canFdParams, &g_BaudRate);
	if (xlStatus == XL_SUCCESS)
	{
		printf("XL_SUCCESS");
		xlStatus = CreateRxThread();
	}

	if (XL_SUCCESS == xlStatus) {
		// ------------------------------------
		// go with all selected channels on bus
		// ------------------------------------
		xlStatus = xlActivateChannel(g_xlPortHandle, g_xlChannelCANFDMask, XL_BUS_TYPE_CAN, XL_ACTIVATE_RESET_CLOCK);


	}

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
					printf("switch v");
					unsigned char msgdata[8] = { 1,8,7,6,4,8,2,9 };
					XLTransmitMsg(0x724, 0, msgdata, 8, 1);
					//send_singleframe(uds_send_can_farme, 0x724,msgdata, 3);
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


