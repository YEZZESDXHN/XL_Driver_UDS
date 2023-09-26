#ifndef _UDS_TP_H_
#define _UDS_TP_H_
#include<stdint.h>
typedef enum __NT_TIMER_T__
{
	TIMER_N_CR = 0,                 // N_CR 定时器，接收方收到连续帧间隔时间不能大于 TIMEOUT_N_CR，单位: ms
	TIMER_N_BS,                     // N_BS 定时器，发送方发送完成首帧后到接收到流控帧之间的时间不能大于 TIMEOUT_N_BS，单位: ms
	TIMER_STmin,                    // STmin 定时器，发送连续帧时，间隔时间最小为 g_rfc_stmin,单位: ms
	TIMER_CNT                       // 定时器总个数
}nt_timer_t;

typedef enum __NETWORK_LAYER_STATUS_
{
	NWL_IDLE = 0,                   // 空闲状态
	NWL_XMIT,                       // 发送状态
	NWL_RECV,                       // 接收状态
	NWL_CNT                         // 状态数量
}network_layer_st;


typedef enum __NETWORK_PCI_TYPE_
{
	PCI_SF = 0,                     // 单帧
	PCI_FF,                         // 首帧
	PCI_CF,                         // 连续帧
	PCI_FC                          // 流控帧
}network_pci_type_t;


typedef enum __NETWORK_FLOW_STATUS__
{
	FS_CTS = 0,                    // 允许继续发送
	FS_WT,                         // 等待
	FS_OVFLW,                      // 溢出
	FS_RESERVED                    // 非法
}network_flow_status_t;
//typedef int(*DLL_FUNCTION_GenerateKeyEx) (const unsigned char*, unsigned int, const unsigned int, const char*, unsigned char*, unsigned int, unsigned int*);
typedef void(*UDS_SEND_FRAME)(unsigned short, unsigned char*, unsigned short);


//uint16_t REQUEST_ID = 0x724;			// 请求 ID
//uint16_t FUNCTION_ID = 0x7DF;			// 功能 ID
//uint16_t RESPONSE_ID = 0x7A4;			// 应答 ID

// // 填充值，如果发送的有效数据不满一帧，则用该值填充
// #define PADDING_VAL                 (0x55)

// 设置帧类型为单帧 
#define NT_SET_PCI_TYPE_SF(low)     (0x00 | (low & 0x0f))

// 设置帧类型为首帧
#define NT_SET_PCI_TYPE_FF(low)     (0x10 | (low & 0x0f))

// 设置帧类型为连续帧
#define NT_SET_PCI_TYPE_CF(low)     (0x20 | (low & 0x0f))

// 设置帧类型为流控帧
#define NT_SET_PCI_TYPE_FC(low)     (0x30 | (low & 0x0f))

// 获取帧类型
#define NT_GET_PCI_TYPE(n_pci)      (n_pci >> 4)

// 获取单帧长度
#define NT_GET_SF_DL(n_pci)         (0x0f & n_pci)

// 获取连续帧帧序号
#define NT_GET_CF_SN(n_pci)         (0x0f & n_pci)

// 获取流状态
#define NT_GET_FC_FS(n_pci)         (0x0f & n_pci)

// 允许发送连续帧的个数，若为 0，则表示发送方可以一直无限制发送连续帧，直到发送完成所有的连续帧
// 若不为 0，则表示当发送方发送的连续帧个数为 NT_XMIT_FC_BS 后需等待接收方回复一帧流控帧，发送方根据流控帧决定接下来的发送情况
#define NT_XMIT_FC_BS               (0)

// 通知发送方发送连续帧的帧间隔最小时间，单位: ms
#define NT_XMIT_FC_STMIN            (0x0A)

// 接收方收到连续帧间隔时间不能大于 TIMEOUT_N_CR，单位: ms
#define TIMEOUT_N_CR                (1000)

// 发送方发送完成首帧后到接收到流控帧之间的时间不能大于 TIMEOUT_N_BS，单位: ms
#define TIMEOUT_N_BS                (1000)

#define FRAME_SIZE      8               // 帧长度



#define REQUEST_ID      0x123           // 请求 ID
#define FUNCTION_ID     0x7DF           // 功能 ID
#define RESPONSE_ID     0x456           // 应答 ID

#define UDS_RX_MAX      1024            // 接收缓冲区长度 --UDS TP 层协议规定最大支持 4095 字节，但是可以根据实际需要改动，以免资源浪费
#define UDS_TX_MAX      128             // 发送缓冲区长度 --UDS TP 层协议规定最大支持 4095 字节，但是可以根据实际需要改动，以免资源浪费

#define PADDING_VAL     0xAA            // 填充值，如果发送的有效数据不满一帧，则用该值填充

#ifndef TRUE
#define	TRUE	(1)
#endif


#ifndef FALSE
#define FALSE	(0)
#endif


#ifndef NULL
#define NULL  ((void *) 0)
#endif

typedef unsigned char		bool_t;







/******************************************************************************
* 函数名称: static int recv_singleframe (uint8_t* frame_buf, uint8_t frame_dlc)
* 功能说明: 处理单帧
* 输入参数: uint8_t*    frame_buf       --接收报文帧数据首地址
	　　　　uint8_t     frame_dlc       --接收报文帧数据长度
* 输出参数: 无
* 函数返回: 0: OK; -1: ERR
* 其它说明: 无
******************************************************************************/
int recv_singleframe(uint8_t* frame_buf, uint8_t frame_dlc);



/******************************************************************************
* 函数名称: static int recv_firstframe(uint8_t* frame_buf, uint8_t frame_dlc)
* 功能说明: 处理首帧
* 输入参数: uint8_t*    frame_buf       --接收报文帧数据首地址
	　　　　uint8_t     frame_dlc       --接收报文帧数据长度
	　　　　UDS_SEND_FRAME sendframefun --发送报文函数指针，硬件驱动接口函数
* 输出参数: 无
* 函数返回: 0: OK; other: -ERR
* 其它说明: 无
******************************************************************************/
int recv_firstframe(UDS_SEND_FRAME sendframefun, uint8_t* frame_buf, uint8_t frame_dlc);



/******************************************************************************
* 函数名称: static void send_flowcontrol(network_flow_status_t flow_st)
* 功能说明: 发送流控帧
* 输入参数: network_flow_status_t   flow_st       --流状态
* 输出参数: 无
* 函数返回: 0: 所有连续帧接收完成; 1: 继续接收; other: -ERR
* 其它说明: 无
******************************************************************************/
void send_flowcontrol(UDS_SEND_FRAME sendframefun, network_flow_status_t flow_st);


/******************************************************************************
* 函数名称: static int send_singleframe (uint8_t* msg_buf, uint16_t msg_dlc)
* 功能说明: 发送单帧
* 输入参数: uint8_t*    msg_buf         --发送数据首地址
	　　　　uint8_t     msg_dlc         --发送数据长度
* 输出参数: 无
* 函数返回: 0: OK; -1: ERR
* 其它说明: 无
******************************************************************************/
int send_singleframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint8_t msg_dlc);






#endif