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
extern XLaccess        g_xlChannelCANMask;								//所有支持can的通道掩码，包括CAN,CANFD ISO,CAN FD NO ISO(CANFD BOSCH)
extern XLaccess        g_xlChannelCANFDMask;							//支持CANFD的通道掩码
extern XLaccess        g_xlChannelCANFDNOISOMask;						//支持CANFD NO ISO(CANFD BOSCH)的通道掩码
extern XLaccess        g_xlChannelChooseMask;							//选择发送报文的通道

extern unsigned int    g_canFdModeNoIso;  //CANFD NO ISO 标志位
extern unsigned int    g_canFdSupport;                          //硬件是否支持CANFD
extern unsigned int    g_canBusMode;                          //选择CAN总线类型,0：CAN;1CANFD
extern unsigned int    g_canMsgType;                          //选择发送can消息类型,0：CAN;1CANFD，总线类型为CANFD可用

extern unsigned int    g_Run;
extern unsigned int    g_ChannelChooes;
XLcanFdConf		g_canFdParams;							//CANFD参数



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
	unsigned char channelindex;//通道编号，从1开始
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
* 函数名称: XLstatus GetVectorHWInfo()
* 功能说明: 获取硬件配置
* 输入参数:
* 输出参数: 无
* 函数返回: XL_SUCCESS,XL_ERROR
* 其它说明:
******************************************************************************/
XLstatus GetVectorHWInfo();

/******************************************************************************
* 函数名称: InitCANDriver()
* 功能说明: 初始化CANoe驱动
* 输入参数:	XLcanFdConf canfdParams,
			unsigned int  BaudRate,
* 输出参数: 无
* 函数返回: XL_SUCCESS,XL_ERROR
* 其它说明: g_canFdSupport=0,硬件不支持CANFD;g_canBusMode=0,使用CAN1.0模式；
			默认会打开所有支持的通道
			需要先调用GetDriverConfig()
******************************************************************************/
XLstatus InitCANDriver(

	XLcanFdConf canParams,
	unsigned int  *BaudRate
);


/******************************************************************************
* 函数名称: CreateRxThread(void)
* 功能说明: 创建接收线程
* 输入参数: 无
* 输出参数: 无
* 函数返回: XL_SUCCESS,XL_ERROR
* 其它说明:
******************************************************************************/
XLstatus CreateRxThread(void);


/******************************************************************************
* 函数名称: CreateRxThread(void)
* 功能说明: 接收CAN消息线程
* 输入参数: 无
* 输出参数: 无
* 函数返回: XL_SUCCESS,XL_ERROR
* 其它说明:
******************************************************************************/
DWORD WINAPI RxThread(LPVOID par);

/******************************************************************************
* 函数名称: RxCanFdThread(void)
* 功能说明: 接收CANFD消息线程
* 输入参数: 无
* 输出参数: 无
* 函数返回: XL_SUCCESS,XL_ERROR
* 其它说明:
******************************************************************************/
DWORD WINAPI RxCanFdThread(LPVOID par);

void initHWinfo(channelInfo *channel_info);

/******************************************************************************
* 函数名称: XLTransmitMsg(unsigned int txID, unsigned int canType, unsigned char *MsgBuffer, unsigned char Msglen, XLaccess xlChanMaskTx)
* 功能说明: 调用Vector XL发送CAN/CANFD报文
* 输入参数:  unsigned int txID					发送报文ID
			unsigned int canType				发送报文类型CAN/CANFD(需硬件支持CANFD)
			unsigned char *MsgBuffer			发送报文数据
			unsigned char Msglen				发送报文数据长度
			XLaccess xlChanMaskTx				发送报文硬件通道
* 输出参数: 无
* 函数返回: XL_SUCCESS,XL_ERROR
* 其它说明:
******************************************************************************/
XLstatus XLTransmitMsg(unsigned int txID, unsigned int canType, unsigned char *MsgBuffer, unsigned char Msglen, XLaccess xlChanMaskTx);


/******************************************************************************
* 函数名称: uds_send_can_farme(unsigned short canId, unsigned char* farmeData, unsigned short farmelen)
* 功能说明: 传给TP层的接口函数 typedef int(*UDS_SEND_FRAME)(unsigned short, unsigned char*, unsigned short);
* 输入参数:  unsigned short canId					发送id
			unsigned char* farmeData,				发送数据
			unsigned short farmelen					发送数据长度
* 输出参数: 无
* 函数返回: 1:发送成功；-1：发送失败
* 其它说明:默认发送can报文(no canfd)
******************************************************************************/
int uds_send_can_farme(unsigned short canId, unsigned char* farmeData, unsigned short farmelen);







#endif