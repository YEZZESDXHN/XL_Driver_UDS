#ifndef _XL_DRIVER_H_
#define _XL_DRIVER_H_


#if defined(_Windows) || defined(_MSC_VER) || defined (__GNUC__)
#define  STRICT
#include <windows.h>
#endif

#include <stdio.h>

#define UNUSED_PARAM(a) { a=a; }

#define RECEIVE_EVENT_SIZE         1        // DO NOT EDIT! Currently 1 is supported only
#define RX_QUEUE_SIZE              4096     // internal driver queue size in CAN events
#define RX_QUEUE_SIZE_FD           16384    // driver queue size for CAN-FD Rx events
#define ENABLE_CAN_FD_MODE_NO_ISO  0        // switch to activate no iso mode on a CAN FD channel

#include "vxlapi.h"
//#include"uds_tp.h"

#ifdef __GNUC__
static void strncpy_s(char *strDest, size_t numberOfElements, const char *strSource, size_t count)
{
	UNUSED_PARAM(numberOfElements);
	strncpy(strDest, strSource, count);
}

static void sscanf_s(const char *buffer, const char *format, ...)
{
	va_list argList;
	va_start(argList, format);
	sscanf(buffer, format, argList);
}
#endif

/////////////////////////////////////////////////////////////////////////////
// globals

extern char g_AppName[XL_MAX_APPNAME + 1]; //!< Application name which is displayed in VHWconf
extern XLportHandle    g_xlPortHandle;      //!< Global porthandle (we use only one!)
XLdriverConfig  g_xlDrvConfig;                                            //!< Contains the actual hardware configuration

extern unsigned int    g_BaudRate;                     //!< Default baudrate



extern XLaccess        g_xlPermissionMask;                          //!< Global permissionmask (includes all founded channels)
extern XLaccess        g_xlChannelCANMask;								//����֧��can��ͨ�����룬����CAN,CANFD ISO,CAN FD NO ISO(CANFD BOSCH)
extern XLaccess        g_xlChannelCANFDMask;							//֧��CANFD��ͨ������
extern XLaccess        g_xlChannelCANFDNOISOMask;						//֧��CANFD NO ISO(CANFD BOSCH)��ͨ������
extern XLaccess        g_xlChannelChooseMask;							//ѡ���ͱ��ĵ�ͨ��

extern unsigned int    g_canFdModeNoIso;  //CANFD NO ISO ��־λ
extern unsigned int    g_canFdSupport;                          //Ӳ���Ƿ�֧��CANFD
extern unsigned int    g_canBusMode;                          //ѡ��CAN��������,0��CAN;1CANFD
extern unsigned int    g_canMsgType;                          //ѡ����can��Ϣ����,0��CAN;1CANFD����������ΪCANFD����

extern unsigned int    g_Run;
extern unsigned int    g_ChannelChooes;
XLcanFdConf		g_canFdParams;							//CANFD����



XLhandle        g_hMsgEvent;                                          //!< notification handle for the receive queue
HANDLE          g_hRXThread;                                          //!< thread handle (RX)
HANDLE          g_hTXThread;                                          //!< thread handle (TX)
int             g_RXThreadRun;                                        //!< flag to start/stop the RX thread
int             g_TXThreadRun;                                        //!< flag to start/stop the TX thread (for the transmission burst)
int             g_RXCANThreadRun;                                     //!< flag to start/stop the RX thread
unsigned int    g_TXThreadCanId;                                     //!< CAN-ID the TX thread transmits under
XLaccess        g_TXThreadTxMask;                                     //!< channel mask the TX thread uses for transmitting


#define HW_CAN   (char)CAN
#define HW_CANFD   (char)CANFD
#define HW_LIN   (char)LIN
#define HW_ETH   (char)ETH

typedef struct _channel_
{
	unsigned char channelindex;//ͨ����ţ���1��ʼ
	char channeltype[128];
	char channelname[128];
	XLaccess channelMask;
}channel;

typedef struct _HW_
{
	unsigned char channelcount;
	channel ch[64];
	
}channelInfo;
channelInfo g_channel_info;



/******************************************************************************
* ��������: XLstatus GetVectorHWInfo()
* ����˵��: ��ȡӲ������
* �������:
* �������: ��
* ��������: XL_SUCCESS,XL_ERROR
* ����˵��:
******************************************************************************/
XLstatus GetVectorHWInfo();

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
	unsigned int  *BaudRate
);


/******************************************************************************
* ��������: CreateRxThread(void)
* ����˵��: ���������߳�
* �������: ��
* �������: ��
* ��������: XL_SUCCESS,XL_ERROR
* ����˵��:
******************************************************************************/
XLstatus CreateRxThread(void);


/******************************************************************************
* ��������: CreateRxThread(void)
* ����˵��: ����CAN��Ϣ�߳�
* �������: ��
* �������: ��
* ��������: XL_SUCCESS,XL_ERROR
* ����˵��:
******************************************************************************/
DWORD WINAPI RxThread(LPVOID par);

/******************************************************************************
* ��������: RxCanFdThread(void)
* ����˵��: ����CANFD��Ϣ�߳�
* �������: ��
* �������: ��
* ��������: XL_SUCCESS,XL_ERROR
* ����˵��:
******************************************************************************/
DWORD WINAPI RxCanFdThread(LPVOID par);

void initHWinfo(channelInfo *channel_info);

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
XLstatus XLTransmitMsg(unsigned int txID, unsigned int canType, unsigned char *MsgBuffer, unsigned char Msglen, XLaccess xlChanMaskTx);


/******************************************************************************
* ��������: uds_send_can_farme(unsigned short canId, unsigned char* farmeData, unsigned short farmelen)
* ����˵��: ����TP��Ľӿں��� typedef int(*UDS_SEND_FRAME)(unsigned short, unsigned char*, unsigned short);
* �������:  unsigned short canId					����id
			unsigned char* farmeData,				��������
			unsigned short farmelen					�������ݳ���
* �������: ��
* ��������: 1:���ͳɹ���-1������ʧ��
* ����˵��:Ĭ�Ϸ���can����(no canfd)
******************************************************************************/
int uds_send_can_farme(unsigned short canId, unsigned char* farmeData, unsigned short farmelen);







#endif