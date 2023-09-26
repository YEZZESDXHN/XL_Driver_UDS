/*------------------------------------------------------------------------------
| File:
|   xlCANdemo.C
| Project:
|   Sample for XL - Driver Library
|   Example application using 'vxlapi.dll'
|-------------------------------------------------------------------------------
| $Author: visjb $    $Locker: $   $Revision: 101442 $
|-------------------------------------------------------------------------------
| Copyright (c) 2014 by Vector Informatik GmbH.  All rights reserved.
 -----------------------------------------------------------------------------*/

 //#if defined(_Windows) || defined(_MSC_VER) || defined (__GNUC__)
 //#define  STRICT
 //#include <windows.h>
 //#endif
 //
 //#include <stdio.h>



 //#include "vxlapi.h"
#include "XL_Driver.h"


//#ifdef __GNUC__
//static void strncpy_s(char *strDest, size_t numberOfElements, const char *strSource, size_t count)
//{
//	UNUSED_PARAM(numberOfElements);
//	strncpy(strDest, strSource, count);
//}
//
//static void sscanf_s(const char *buffer, const char *format, ...)
//{
//	va_list argList;
//	va_start(argList, format);
//	sscanf(buffer, format, argList);
//}
//#endif

char            g_AppName[XL_MAX_APPNAME + 1] = "xlCANdemo";               //!< Application name which is displayed in VHWconf
g_xlPortHandle = XL_INVALID_PORTHANDLE;      //!< Global porthandle (we use only one!)
//g_xlPermissionMask = (XLaccess)0;                          //!< Global permissionmask (includes all founded channels)
//g_BaudRate = (unsigned int)500000;                     //!< Default baudrate


g_canFdModeNoIso = ENABLE_CAN_FD_MODE_NO_ISO;  //!< Global CAN FD ISO (default) / no ISO mode flag





g_canFdSupport = 0;                          //硬件是否支持CANFD
g_canBusMode = 1;                          //选择CANFD模式























/******************************************************************************
* 函数名称: InitCANDriver()
* 功能说明: 初始化CANoe驱动
* 输入参数:	XLcanFdConf canfdParams,
			unsigned int  BaudRate,
			unsigned int  canFdSupport,
			unsigned int  arbitrationBitRate,
			unsigned int  sjwAbr,              // CAN bus timing for nominal / arbitration bit rate
			unsigned int  tseg1Abr,
			unsigned int  tseg2Abr,
			unsigned int  dataBitRate,
			unsigned int  sjwDbr,              // CAN bus timing for data bit rate
			unsigned int  tseg1Dbr,
			unsigned int  tseg2Dbr,
			unsigned char options             // CANFD_CONFOPT_
* 输出参数: 无
* 函数返回: XL_SUCCESS,XL_ERROR
* 其它说明: g_canFdSupport=0,硬件不支持CANFD;g_canBusMode=0,使用CAN1.0模式；
			默认会打开所有支持的通道
******************************************************************************/
XLstatus InitCANDriver(

	XLcanFdConf canParams,
	unsigned int  *BaudRate
)
{

	XLstatus          xlStatus;
	unsigned int      i;

	// ------------------------------------
	// open the driver
	// ------------------------------------
	xlStatus = xlOpenDriver();

	// ------------------------------------
	// get/print the hardware configuration
	// ------------------------------------
	if (XL_SUCCESS == xlStatus) {
		xlStatus = xlGetDriverConfig(&g_xlDrvConfig);
	}

	if (XL_SUCCESS == xlStatus) {

		//init
		g_xlChannelCANMask = 0;
		g_xlChannelCANFDMask = 0;
		g_xlChannelCANFDNOISOMask = 0;
		g_xlPermissionMask = 0;
		//g_BaudRate = 500000;




		// ------------------------------------
		// select the wanted channels
		// ------------------------------------

		for (i = 0; i < g_xlDrvConfig.channelCount; i++) {

			// we take all hardware we found and supports CAN
			if (g_xlDrvConfig.channel[i].channelBusCapabilities & XL_BUS_ACTIVE_CAP_CAN) {

				// check if we can use CAN FD - the virtual CAN driver supports CAN-FD, but we don't use it
				if ((g_xlDrvConfig.channel[i].channelCapabilities & XL_CHANNEL_FLAG_CANFD_ISO_SUPPORT)
					/*&& (g_xlDrvConfig.channel[i].hwType != XL_HWTYPE_VIRTUAL)*/) {
					g_xlChannelCANFDMask |= g_xlDrvConfig.channel[i].channelMask;//支持CANFD IOS

					// check CAN FD NO ISO support
					if (g_xlDrvConfig.channel[i].channelCapabilities & XL_CHANNEL_FLAG_CANFD_BOSCH_SUPPORT) {
						g_xlChannelCANFDNOISOMask |= g_xlDrvConfig.channel[i].channelMask;//支持CANFD NO IOS
					}
				}
				else {
					g_xlChannelCANMask |= g_xlDrvConfig.channel[i].channelMask;
				}

			}
		}

		//如果检测支持CANFD,那么也支持CAN,设置g_xlChannelMask
		if (g_xlChannelCANFDMask && g_canFdModeNoIso == 0) //支持CANFD,CAN IOS模式
		{
			g_xlChannelCANMask |= g_xlChannelCANFDMask;
			g_canFdSupport = 1;//支持CANFD

		}

		if (g_xlChannelCANFDNOISOMask && g_canFdModeNoIso) //支持CANFD,CAN NO IOS模式
		{
			g_xlChannelCANMask |= g_xlChannelCANFDNOISOMask;
			g_canFdSupport = 1;//支持CANFD
		}

		if (!g_xlChannelCANMask) //无CAN通道
		{
			xlStatus = XL_ERROR;
		}
	}

	//g_xlPermissionMask = g_xlChannelMask;//给权限

	// ------------------------------------
	// 打开全部端口,默认给所有权限
	// ------------------------------------
	if (XL_SUCCESS == xlStatus) {

		// check if we can use CAN FD
		if (g_canFdSupport)
		{
			if (g_canBusMode == 1)
			{
				g_xlPermissionMask = g_xlChannelCANFDMask;//默认给所有权限
				xlStatus = xlOpenPort(&g_xlPortHandle, g_AppName, g_xlChannelCANFDMask, &g_xlPermissionMask, RX_QUEUE_SIZE_FD, XL_INTERFACE_VERSION_V4, XL_BUS_TYPE_CAN);
			}
			else
			{
				g_xlPermissionMask = g_xlChannelCANMask; //默认给所有权限
				xlStatus = xlOpenPort(&g_xlPortHandle, g_AppName, g_xlChannelCANMask, &g_xlPermissionMask, RX_QUEUE_SIZE, XL_INTERFACE_VERSION, XL_BUS_TYPE_CAN);
			}

		}
		// if not, we make 'normal' CAN
		else
		{
			g_xlPermissionMask = g_xlChannelCANMask;//默认给所有权限
			xlStatus = xlOpenPort(&g_xlPortHandle, g_AppName, g_xlChannelCANMask, &g_xlPermissionMask, RX_QUEUE_SIZE, XL_INTERFACE_VERSION, XL_BUS_TYPE_CAN);

		}

	}

	if ((XL_SUCCESS == xlStatus) && (XL_INVALID_PORTHANDLE != g_xlPortHandle)) {

		// ------------------------------------
		// if we have permission we set the
		// bus parameters (baudrate)
		// ------------------------------------
		if (1) //默认给所有权限
		{

			if (g_canFdSupport)
			{
				if (g_canFdModeNoIso) {
					canParams.options = CANFD_CONFOPT_NO_ISO;
				}
				else
				{
					canParams.options = 0;
				}
				xlStatus = xlCanFdSetConfiguration(g_xlPortHandle, g_xlChannelCANFDMask, &g_canFdParams);

			}
			else {
				xlStatus = xlCanSetChannelBitrate(g_xlPortHandle, g_xlChannelCANMask, g_BaudRate);
			}
		}
		else
		{

		}
	}
	else
	{

		xlClosePort(g_xlPortHandle);
		g_xlPortHandle = XL_INVALID_PORTHANDLE;
		xlStatus = XL_ERROR;
	}

	return xlStatus;

}

/******************************************************************************
* 函数名称: CreateRxThread(void)
* 功能说明: 创建接收线程
* 输入参数: 无
* 输出参数: 无
* 函数返回: XL_SUCCESS,XL_ERROR
* 其它说明:
******************************************************************************/
XLstatus CreateRxThread(void)
{
	XLstatus      xlStatus = XL_ERROR;
	DWORD         ThreadId = 0;

	if (g_xlPortHandle != XL_INVALID_PORTHANDLE) {

		// Send a event for each Msg!!!
		xlStatus = xlSetNotification(g_xlPortHandle, &g_hMsgEvent, 1);

		if (g_canFdSupport) {
			g_hRXThread = CreateThread(0, 0x1000, RxCanFdThread, (LPVOID)0, 0, &ThreadId);
		}
		else {
			g_hRXThread = CreateThread(0, 0x1000, RxThread, (LPVOID)0, 0, &ThreadId);
		}

	}
	return xlStatus;
}








/******************************************************************************
* 函数名称: CreateRxThread(void)
* 功能说明: 接收CAN消息线程
* 输入参数: 无
* 输出参数: 无
* 函数返回: XL_SUCCESS,XL_ERROR
* 其它说明:
******************************************************************************/
DWORD WINAPI RxThread(LPVOID par)
{
	XLstatus        xlStatus;

	unsigned int    msgsrx = RECEIVE_EVENT_SIZE;
	XLevent         xlEvent;

	UNUSED_PARAM(par);

	g_RXThreadRun = 1;

	while (g_RXThreadRun) {

		WaitForSingleObject(g_hMsgEvent, 10);

		xlStatus = XL_SUCCESS;

		while (!xlStatus) {

			msgsrx = RECEIVE_EVENT_SIZE;

			xlStatus = xlReceive(g_xlPortHandle, &msgsrx, &xlEvent);
			if (xlStatus != XL_ERR_QUEUE_IS_EMPTY)
			{
				printf("%s\n", xlGetEventString(&xlEvent));

			}
		}

	}
	return NO_ERROR;
}



/******************************************************************************
* 函数名称: RxCanFdThread(void)
* 功能说明: 接收CANFD消息线程
* 输入参数: 无
* 输出参数: 无
* 函数返回: XL_SUCCESS,XL_ERROR
* 其它说明:
******************************************************************************/
DWORD WINAPI RxCanFdThread(LPVOID par)
{
	XLstatus        xlStatus = XL_SUCCESS;
	DWORD           rc;
	XLcanRxEvent    xlCanRxEvt;

	UNUSED_PARAM(par);

	g_RXCANThreadRun = 1;

	while (g_RXCANThreadRun) {
		rc = WaitForSingleObject(g_hMsgEvent, 10);
		if (rc != WAIT_OBJECT_0) continue;

		do {
			xlStatus = xlCanReceive(g_xlPortHandle, &xlCanRxEvt);
			if (xlStatus == XL_ERR_QUEUE_IS_EMPTY) {
				break;
			}

			if (xlCanRxEvt.channelIndex == 0)
			{
				printf("t=%I64u\tID:%4X\tData:%02X %02X %02X %02X %02X %02X %02X %02X\n",
					xlCanRxEvt.timeStampSync,
					xlCanRxEvt.tagData.canRxOkMsg.canId,
					xlCanRxEvt.tagData.canRxOkMsg.data[0],
					xlCanRxEvt.tagData.canRxOkMsg.data[1],
					xlCanRxEvt.tagData.canRxOkMsg.data[2],
					xlCanRxEvt.tagData.canRxOkMsg.data[3],
					xlCanRxEvt.tagData.canRxOkMsg.data[4],
					xlCanRxEvt.tagData.canRxOkMsg.data[5],
					xlCanRxEvt.tagData.canRxOkMsg.data[6],
					xlCanRxEvt.tagData.canRxOkMsg.data[7]);
			}


		} while (XL_SUCCESS == xlStatus);
	}

	return(NO_ERROR);
} // RxCanFdThread



XLstatus XLTransmitMsg(unsigned int txID, unsigned char MsgBuffer, XLaccess xlChanMaskTx)
{
	XLstatus             xlStatus;
	unsigned int         messageCount = 1;
	static int           cnt = 0;

	if (g_canFdSupport) {
		unsigned int  fl[3] = {

		  0 , // CAN (no FD)
		  XL_CAN_TXMSG_FLAG_EDL,
		  XL_CAN_TXMSG_FLAG_EDL | XL_CAN_TXMSG_FLAG_BRS,
		};

		XLcanTxEvent canTxEvt;
		unsigned int cntSent;
		unsigned int i;

		memset(&canTxEvt, 0, sizeof(canTxEvt));
		canTxEvt.tag = XL_CAN_EV_TAG_TX_MSG;

		canTxEvt.tagData.canMsg.canId = txID;
		canTxEvt.tagData.canMsg.msgFlags = fl[cnt % (sizeof(fl) / sizeof(fl[0]))];
		canTxEvt.tagData.canMsg.dlc = 8;

		// if EDL is set, demonstrate transmit with DLC=15 (64 bytes)
		if (canTxEvt.tagData.canMsg.msgFlags & XL_CAN_TXMSG_FLAG_EDL) {
			canTxEvt.tagData.canMsg.dlc = 15;
		}

		++cnt;

		for (i = 1; i < XL_CAN_MAX_DATA_LEN; ++i) {
			canTxEvt.tagData.canMsg.data[i] = (unsigned char)i - 1;
		}
		canTxEvt.tagData.canMsg.data[0] = (unsigned char)cnt;
		xlStatus = xlCanTransmitEx(g_xlPortHandle, xlChanMaskTx, messageCount, &cntSent, &canTxEvt);
	}
	else {
		static XLevent       xlEvent;

		memset(&xlEvent, 0, sizeof(xlEvent));

		xlEvent.tag = XL_TRANSMIT_MSG;
		xlEvent.tagData.msg.id = txID;
		xlEvent.tagData.msg.dlc = 8;
		xlEvent.tagData.msg.flags = 0;
		++xlEvent.tagData.msg.data[0];
		xlEvent.tagData.msg.data[1] = 2;
		xlEvent.tagData.msg.data[2] = 3;
		xlEvent.tagData.msg.data[3] = 4;
		xlEvent.tagData.msg.data[4] = 5;
		xlEvent.tagData.msg.data[5] = 6;
		xlEvent.tagData.msg.data[6] = 7;
		xlEvent.tagData.msg.data[7] = 8;

		xlStatus = xlCanTransmit(g_xlPortHandle, xlChanMaskTx, &messageCount, &xlEvent);

	}

	printf("- Transmit         : CM(0x%I64x), %s\n", xlChanMaskTx, xlGetErrorString(xlStatus));

	return xlStatus;
}