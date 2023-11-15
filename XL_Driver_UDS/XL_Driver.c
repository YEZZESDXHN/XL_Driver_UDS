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
#include"uds_tp.h"
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
XLportHandle g_xlPortHandle = XL_INVALID_PORTHANDLE;      //!< Global porthandle (we use only one!)
XLaccess        g_xlPermissionMask=0;                          //!< Global permissionmask (includes all founded channels)
XLaccess        g_xlChannelCANMask=0;								//����֧��can��ͨ�����룬����CAN,CANFD ISO,CAN FD NO ISO(CANFD BOSCH)
XLaccess        g_xlChannelCANFDMask=0;							//֧��CANFD��ͨ������
XLaccess        g_xlChannelCANFDNOISOMask=0;						//֧��CANFD NO ISO(CANFD BOSCH)��ͨ������
XLaccess        g_xlChannelChooseMask=0;							//ѡ���ͱ��ĵ�ͨ��
unsigned int    g_BaudRate = 500000;


unsigned int    g_canFdModeNoIso = ENABLE_CAN_FD_MODE_NO_ISO;  //!< Global CAN FD ISO (default) / no ISO mode flag
unsigned int    g_canFdSupport=0;                          //Ӳ���Ƿ�֧��CANFD
unsigned int    g_canBusMode=1;                          //ѡ��CANFDģʽ��0��CAN;1CANFD
unsigned int    g_canMsgType=0;                          //ѡ����can��Ϣ����,0��CAN;1CANFD����������ΪCANFD����






unsigned int    g_Run = 0;                          //���б�־λ
unsigned int    g_ChannelChooes = 0xff;                          //ͨ��ѡ��






/******************************************************************************
* ��������: XLstatus GetVectorHWInfo()
* ����˵��: ��ȡӲ������
* �������:
* �������: ��
* ��������: XL_SUCCESS,XL_ERROR
* ����˵��:
******************************************************************************/
XLstatus GetVectorHWInfo()
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


		// ------------------------------------
		// select the wanted channels
		// ------------------------------------

		for (i = 0; i < g_xlDrvConfig.channelCount; i++) {

			// we take all hardware we found and supports CAN
			if (g_xlDrvConfig.channel[i].channelBusCapabilities & XL_BUS_ACTIVE_CAP_CAN) {

				// check if we can use CAN FD - the virtual CAN driver supports CAN-FD, but we don't use it
				if ((g_xlDrvConfig.channel[i].channelCapabilities & XL_CHANNEL_FLAG_CANFD_ISO_SUPPORT)
					/*&& (g_xlDrvConfig.channel[i].hwType != XL_HWTYPE_VIRTUAL)*/) {
					g_xlChannelCANFDMask |= g_xlDrvConfig.channel[i].channelMask;//֧��CANFD IOS
					g_xlChannelCANMask |= g_xlDrvConfig.channel[i].channelMask;//֧��CANFD IOS����Ҳ֧��CAN
					g_canFdSupport = 1;//֧��CANFD,����CANFD��־λ

					// check CAN FD NO ISO support
					if (g_xlDrvConfig.channel[i].channelCapabilities & XL_CHANNEL_FLAG_CANFD_BOSCH_SUPPORT) {
						g_xlChannelCANFDNOISOMask |= g_xlDrvConfig.channel[i].channelMask;//֧��CANFD NO IOS
					}
				}
				else {
					g_xlChannelCANMask |= g_xlDrvConfig.channel[i].channelMask;
				}

			}
		}

		////������֧��CANFD,��ôҲ֧��CAN,����g_xlChannelMask
		//if (g_xlChannelCANFDMask && g_canFdModeNoIso == 0) //֧��CANFD,CAN IOSģʽ
		//{
		//	g_xlChannelCANMask |= g_xlChannelCANFDMask;
		//	g_canFdSupport = 1;//֧��CANFD

		//}

		//if (g_xlChannelCANFDNOISOMask && g_canFdModeNoIso) //֧��CANFD,CAN NO IOSģʽ
		//{
		//	g_xlChannelCANMask |= g_xlChannelCANFDNOISOMask;
		//	g_canFdSupport = 1;//֧��CANFD
		//}

		if (!g_xlChannelCANMask) //��CANͨ��
		{
			xlStatus = XL_ERROR;
		}
	}
	return xlStatus;
}





/******************************************************************************
* ��������: InitCANDriver()
* ����˵��: ��ʼ��CANoe����
* �������:	XLcanFdConf canfdParams,
			unsigned int  BaudRate,
* �������: ��
* ��������: XL_SUCCESS,XL_ERROR
* ����˵��: g_canFdSupport=0,Ӳ����֧��CANFD;g_canBusMode=0,ʹ��CAN1.0ģʽ��
			Ĭ�ϻ������֧�ֵ�ͨ��
			��Ҫ�ȵ���GetDriverConfig()
******************************************************************************/
XLstatus InitCANDriver(

	XLcanFdConf canParams,
	unsigned int  *BaudRate,
	XLaccess initChannelCM
)
{

	XLstatus          xlStatus;
	//===========================================================================
	//unsigned int      i;

	//// ------------------------------------
	//// open the driver
	//// ------------------------------------
	//xlStatus = xlOpenDriver();

	//// ------------------------------------
	//// get/print the hardware configuration
	//// ------------------------------------
	//if (XL_SUCCESS == xlStatus) {
	//	xlStatus = xlGetDriverConfig(&g_xlDrvConfig);
	//}

	//if (XL_SUCCESS == xlStatus) {

	//	
	//	// ------------------------------------
	//	// select the wanted channels
	//	// ------------------------------------

	//	for (i = 0; i < g_xlDrvConfig.channelCount; i++) {

	//		// we take all hardware we found and supports CAN
	//		if (g_xlDrvConfig.channel[i].channelBusCapabilities & XL_BUS_ACTIVE_CAP_CAN) {

	//			// check if we can use CAN FD - the virtual CAN driver supports CAN-FD, but we don't use it
	//			if ((g_xlDrvConfig.channel[i].channelCapabilities & XL_CHANNEL_FLAG_CANFD_ISO_SUPPORT)
	//				/*&& (g_xlDrvConfig.channel[i].hwType != XL_HWTYPE_VIRTUAL)*/) {
	//				g_xlChannelCANFDMask |= g_xlDrvConfig.channel[i].channelMask;//֧��CANFD IOS

	//				// check CAN FD NO ISO support
	//				if (g_xlDrvConfig.channel[i].channelCapabilities & XL_CHANNEL_FLAG_CANFD_BOSCH_SUPPORT) {
	//					g_xlChannelCANFDNOISOMask |= g_xlDrvConfig.channel[i].channelMask;//֧��CANFD NO IOS
	//				}
	//			}
	//			else {
	//				g_xlChannelCANMask |= g_xlDrvConfig.channel[i].channelMask;
	//			}

	//		}
	//	}

	//	//������֧��CANFD,��ôҲ֧��CAN,����g_xlChannelMask
	//	if (g_xlChannelCANFDMask && g_canFdModeNoIso == 0) //֧��CANFD,CAN IOSģʽ
	//	{
	//		g_xlChannelCANMask |= g_xlChannelCANFDMask;
	//		g_canFdSupport = 1;//֧��CANFD

	//	}

	//	if (g_xlChannelCANFDNOISOMask && g_canFdModeNoIso) //֧��CANFD,CAN NO IOSģʽ
	//	{
	//		g_xlChannelCANMask |= g_xlChannelCANFDNOISOMask;
	//		g_canFdSupport = 1;//֧��CANFD
	//	}

	//	if (!g_xlChannelCANMask) //��CANͨ��
	//	{
	//		xlStatus = XL_ERROR;
	//	}
	//}

	//g_xlPermissionMask = g_xlChannelMask;//��Ȩ��

	//===========================================================================




	
	// check if we can use CAN FD
	if (g_canFdSupport)
	{
		if (g_canBusMode == 1)
		{
			if (initChannelCM & g_xlChannelCANFDMask)
			{
				g_xlPermissionMask = initChannelCM;//Ĭ�ϸ�����Ȩ��
				xlStatus = xlOpenPort(&g_xlPortHandle, g_AppName, initChannelCM, &g_xlPermissionMask, RX_QUEUE_SIZE_FD, XL_INTERFACE_VERSION_V4, XL_BUS_TYPE_CAN);
				//printf("g_canBusMode=1,g_xlChannelCANFDMask=0x%I64x,g_xlPermissionMask=0x%I64x��initChannelCM=0x%I64x\n", g_xlChannelCANFDMask, g_xlPermissionMask, initChannelCM);
			}

			
		}
		else
		{
				
			g_xlPermissionMask = initChannelCM; //Ĭ�ϸ�����Ȩ��
			xlStatus = xlOpenPort(&g_xlPortHandle, g_AppName, initChannelCM, &g_xlPermissionMask, RX_QUEUE_SIZE, XL_INTERFACE_VERSION, XL_BUS_TYPE_CAN);
			//printf("g_canBusMode=0,g_xlChannelCANMask=0x%I64x,g_xlPermissionMask=0x%I64x\n", g_xlChannelCANMask, g_xlPermissionMask);
		}

	}
	// if not, we make 'normal' CAN
	else
	{
		g_canBusMode = 0;//Ӳ����֧��CANFD,��������ֻ����CAN

		g_xlPermissionMask = initChannelCM;//Ĭ�ϸ�����Ȩ��
		xlStatus = xlOpenPort(&g_xlPortHandle, g_AppName, initChannelCM, &g_xlPermissionMask, RX_QUEUE_SIZE, XL_INTERFACE_VERSION, XL_BUS_TYPE_CAN);

	}
	
	if ((XL_SUCCESS == xlStatus) && (XL_INVALID_PORTHANDLE != g_xlPortHandle)) {
		
		// ------------------------------------
		// if we have permission we set the
		// bus parameters (baudrate)
		// ------------------------------------




		//printf("g_xlPermissionMask=0x%I64x\ng_xlChannelCANFDMask=0x%I64x\n", g_xlPermissionMask, g_xlChannelCANFDMask);
		if (g_xlPermissionMask == initChannelCM)
		{
			if (g_canBusMode)
			{

				if (g_canFdModeNoIso) {
					canParams.options = CANFD_CONFOPT_NO_ISO;
				}
				else
				{
					canParams.options = 0;
				}


				xlStatus = xlCanFdSetConfiguration(g_xlPortHandle, g_xlPermissionMask, &canParams);
				//if (XL_SUCCESS == xlStatus)
				//{
				//	printf("CANFD Set Success,g_xlPortHandle=%d,g_xlPermissionMask=0x%I64x\n", g_xlPortHandle, g_xlPermissionMask);
				//}
				//else
				//{
				//	printf("CANFD Set error,g_xlPortHandle=%d,g_xlPermissionMask=0x%I64x\n", g_xlPortHandle, g_xlPermissionMask);
				//}
			}
			else {
				xlStatus = xlCanSetChannelBitrate(g_xlPortHandle, g_xlPermissionMask, g_BaudRate);
				//if (XL_SUCCESS == xlStatus)
				//{
				//	printf("CAN Set Success\n");
				//}
			}
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
* ��������: CreateRxThread(void)
* ����˵��: ���������߳�
* �������: ��
* �������: ��
* ��������: XL_SUCCESS,XL_ERROR
* ����˵��:
******************************************************************************/
XLstatus CreateRxThread(void)
{
	XLstatus      xlStatus = XL_ERROR;
	DWORD         ThreadId = 0;

	if (g_xlPortHandle != XL_INVALID_PORTHANDLE) {

		// Send a event for each Msg!!!
		xlStatus = xlSetNotification(g_xlPortHandle, &g_hMsgEvent, 1);

		if (g_canBusMode) {
			g_hRXThread = CreateThread(0, 0x1000, RxCanFdThread, (LPVOID)0, 0, &ThreadId);
		}
		else {
			g_hRXThread = CreateThread(0, 0x1000, RxThread, (LPVOID)0, 0, &ThreadId);
		}

	}
	return xlStatus;
}








/******************************************************************************
* ��������: RxThread(void)
* ����˵��: ����CAN��Ϣ�߳�
* �������: ��
* �������: ��
* ��������: XL_SUCCESS,XL_ERROR
* ����˵��:
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
				if (g_Run == 1)
				{
					if (xlEvent.chanIndex == g_ChannelChooes && g_ChannelChooes!=0xff)
					{
						uds_tp_recv_frame(uds_send_can_farme, xlEvent.tagData.msg.data, xlEvent.tagData.msg.dlc);

					}
				}
				

			}
		}

	}
	return NO_ERROR;
}




/******************************************************************************
* ��������: RxCanFdThread(void)
* ����˵��: ����CANFD��Ϣ�߳�
* �������: ��
* �������: ��
* ��������: XL_SUCCESS,XL_ERROR
* ����˵��:
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
			if (g_Run == 1)
			{
				if (xlCanRxEvt.channelIndex == g_ChannelChooes && g_ChannelChooes != 0xff && xlCanRxEvt.tagData.canRxOkMsg.canId== RESPONSE_ID)
				{
					
					uds_tp_recv_frame(uds_send_can_farme, xlCanRxEvt.tagData.canRxOkMsg.data, xlCanRxEvt.tagData.canRxOkMsg.dlc);
					//printf("xl RX:");
					//for (int i = 0; i < 8; i++)
					//{
					//	printf("%02X ", xlCanRxEvt.tagData.canRxOkMsg.data[i]);
					//}
					//printf("\n");
				}
			}
			


		} while (XL_SUCCESS == xlStatus);
	}

	return(NO_ERROR);
} // RxCanFdThread



void initHWinfo(channelInfo *channel_info)
{
	XLaccess cm = 1;
	uint8_t channelcount=1;//����canͨ������
	uint8_t index = 0;
	uint8_t i = 0;
	for (i = 0; i < 64; i++)
	{
		//printf("i=%d g_xlChannelCANFDMask=%I64x\n", i, g_xlChannelCANFDMask);

		if (cm & g_xlChannelCANMask)
		{
			
			//channelcount++;
			(*channel_info).ch[index].channelMask = cm;
			(*channel_info).ch[index].channelindex = i + 1;
			(*channel_info).channelcount = channelcount;


			snprintf((*channel_info).ch[index].channeltype, 6, "CANFD");
			(*channel_info).ch[index].channeltype[6] = "\0";


			snprintf((*channel_info).ch[index].channelname, strlen(g_xlDrvConfig.channel[i].name)+1, g_xlDrvConfig.channel[i].name);
			g_xlDrvConfig.channel[index].name[strlen(g_xlDrvConfig.channel[i].name) + 1] = '\0';
			
			//printf("i=%d,(*channel_info).ch[%d].channelindex=%d,channelcount=%d,name:%s\n", i, index, (*channel_info).ch[index].channelindex, channelcount,g_xlDrvConfig.channel[i].name);
			channelcount++;
			index++;
			
		}
		cm = cm << 1;
	}
}



/******************************************************************************
* ��������: XLTransmitMsg(unsigned int txID, unsigned int canType, unsigned char *MsgBuffer, unsigned char Msglen, XLaccess xlChanMaskTx)
* ����˵��: ����Vector XL����CAN/CANFD����
* �������:  unsigned int txID					���ͱ���ID
			unsigned int canType				���ͱ�������CAN/CANFD(��Ӳ��֧��CANFD)
			unsigned char *MsgBuffer			���ͱ�������
			unsigned char Msglen				���ͱ������ݳ���
			XLaccess xlChanMaskTx				���ͱ���Ӳ��ͨ��
* �������: ��
* ��������: XL_SUCCESS,XL_ERROR
* ����˵��:
******************************************************************************/

XLstatus XLTransmitMsg(unsigned int txID, unsigned int canType, unsigned char *MsgBuffer, unsigned char Msglen, XLaccess xlChanMaskTx)
{
	XLstatus             xlStatus;
	unsigned int         messageCount = 1;
	static int           cnt = 0;

	//
	if (g_canBusMode == 1)
	{
		

		XLcanTxEvent canTxEvt;
		unsigned int cntSent;
		unsigned int i;

		memset(&canTxEvt, 0, sizeof(canTxEvt));
		canTxEvt.tag = XL_CAN_EV_TAG_TX_MSG;//Event type. Set to XL_CAN_EV_TAG_TX_MSG.

		canTxEvt.tagData.canMsg.canId = txID; //CAN ID(11 or 29 bits).For extended IDs : canID = (XL_CAN_EXT_MSG_ID | id)


		/*
		Set to 0 to transmit a CAN 2.0 frame.
		XL_CAN_TXMSG_FLAG_BRS
		Baudrate switch.
		XL_CAN_TXMSG_FLAG_HIGHPRIO
		High priority message. Clears all send buffers then transmits.
		XL_CAN_TXMSG_FLAG_WAKEUP
		Generates a wake up message.
		XL_CAN_TXMSG_FLAG_EDL
		This flag is used to indicate an extended CAN FD data length according
		to the table below.
		XL_CAN_TXMSG_FLAG_RTR
		This flag is used for Remote-Transmission-Request.
		Only useable for Standard CAN messages.
		*/
		//unsigned int  fl[3] = {

		//  0 , // CAN (no FD)
		//  XL_CAN_TXMSG_FLAG_EDL,
		//  XL_CAN_TXMSG_FLAG_EDL | XL_CAN_TXMSG_FLAG_BRS,
		//};
		if (canType == 1)
		{
			canTxEvt.tagData.canMsg.msgFlags = 1;
		}
		else
		{
			canTxEvt.tagData.canMsg.msgFlags = 0;//Set to 0 to transmit a CAN 2.0 frame.
		}
		
		canTxEvt.tagData.canMsg.dlc = Msglen;
		for (i = 0; i < Msglen; i++)
		{
			canTxEvt.tagData.canMsg.data[i] = MsgBuffer[i];
		}

		//// if EDL is set, demonstrate transmit with DLC=15 (64 bytes)
		//if (canTxEvt.tagData.canMsg.msgFlags & XL_CAN_TXMSG_FLAG_EDL) {
		//	canTxEvt.tagData.canMsg.dlc = 15;
		//}

		//++cnt;

		//for (i = 1; i < XL_CAN_MAX_DATA_LEN; ++i) {
		//	canTxEvt.tagData.canMsg.data[i] = (unsigned char)i - 1;
		//}
		//canTxEvt.tagData.canMsg.data[0] = (unsigned char)cnt;
		xlStatus = xlCanTransmitEx(g_xlPortHandle, xlChanMaskTx, messageCount, &cntSent, &canTxEvt);
	}
	else {
		static XLevent       xlEvent;
		unsigned int i;
		memset(&xlEvent, 0, sizeof(xlEvent));

		xlEvent.tag = XL_TRANSMIT_MSG;
		xlEvent.tagData.msg.id = txID;
		xlEvent.tagData.msg.dlc = Msglen;
		xlEvent.tagData.msg.flags = 0;

		for (i = 0; i < Msglen; i++)
		{
			xlEvent.tagData.msg.data[i] = MsgBuffer[i];
		}

		xlStatus = xlCanTransmit(g_xlPortHandle, xlChanMaskTx, &messageCount, &xlEvent);

	}

	return xlStatus;
}










/******************************************************************************
* ��������: uds_send_can_farme(unsigned short canId, unsigned char* farmeData, unsigned short farmelen)
* ����˵��: ����TP��Ľӿں���
* �������:  unsigned short canId					����id
			unsigned char* farmeData,				��������
			unsigned short farmelen					�������ݳ���
* �������: ��
* ��������: 1:���ͳɹ���-1������ʧ��
* ����˵��:Ĭ�Ϸ���can����(no canfd)������ͨ����ȫ�ֱ���g_xlChannelChooseMask����
******************************************************************************/
int uds_send_can_farme(unsigned int canId, unsigned char* farmeData, unsigned char farmelen)
{
	XLstatus             xlStatus;
	//printf("uds_send_can_farme %02X %02X %02X %02X %02X %02X %02X %02X\n", farmeData[0], farmeData[1], farmeData[2], farmeData[3], farmeData[4], farmeData[5], farmeData[6], farmeData[7]);
	xlStatus=XLTransmitMsg(canId, g_canMsgType, farmeData, farmelen, g_xlChannelChooseMask);
	
	if (XL_SUCCESS == xlStatus)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}


//int service_3e_TesterPresent()
//{
//	unsigned char senddata[FRAME_SIZE];
//	senddata[0] = 0x3e;
//	senddata[1] = 0x80;
//	send_singleframe(uds_send_can_farme, senddata, 2);
//}
//
//
//
//DWORD WINAPI TxThread_3E(LPVOID par)
//{
//	
//	while (1) {
//
//		Sleep(3300);
//		service_3e_TesterPresent();
//
//	}
//}