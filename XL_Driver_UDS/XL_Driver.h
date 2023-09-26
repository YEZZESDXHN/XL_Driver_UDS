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


XLportHandle    g_xlPortHandle;      //!< Global porthandle (we use only one!)
XLdriverConfig  g_xlDrvConfig;                                            //!< Contains the actual hardware configuration

unsigned int    g_BaudRate;                     //!< Default baudrate



XLaccess        g_xlPermissionMask;                          //!< Global permissionmask (includes all founded channels)
XLaccess        g_xlChannelCANMask;								//所有支持can的通道掩码，包括CAN,CANFD ISO,CAN FD NO ISO(CANFD BOSCH)
XLaccess        g_xlChannelCANFDMask;							//支持CANFD的通道掩码
XLaccess        g_xlChannelCANFDNOISOMask;						//支持CANFD NO ISO(CANFD BOSCH)的通道掩码
unsigned int    g_canFdModeNoIso;  //CANFD NO ISO 标志位



unsigned int    g_canFdSupport;                          //硬件是否支持CANFD
unsigned int    g_canBusMode;                          //选择CANFD模式
XLcanFdConf		g_canFdParams;							//CANFD参数



XLhandle        g_hMsgEvent;                                          //!< notification handle for the receive queue
HANDLE          g_hRXThread;                                          //!< thread handle (RX)
HANDLE          g_hTXThread;                                          //!< thread handle (TX)
int             g_RXThreadRun;                                        //!< flag to start/stop the RX thread
int             g_TXThreadRun;                                        //!< flag to start/stop the TX thread (for the transmission burst)
int             g_RXCANThreadRun;                                     //!< flag to start/stop the RX thread
unsigned int    g_TXThreadCanId;                                     //!< CAN-ID the TX thread transmits under
XLaccess        g_TXThreadTxMask;                                     //!< channel mask the TX thread uses for transmitting







/******************************************************************************
* 函数名称: InitCANDriver()
* 功能说明: 初始化CANoe驱动
* 输入参数:	XLcanFdConf canfdParams,
			unsigned int  BaudRate
* 输出参数: 无
* 函数返回: XL_SUCCESS,XL_ERROR
* 其它说明: g_canFdSupport=0,硬件不支持CANFD;g_canBusMode=0,使用CAN1.0模式；
			默认会打开所有支持的通道
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










#endif