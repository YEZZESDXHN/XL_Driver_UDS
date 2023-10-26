#ifndef _UDS_TP_H_
#define _UDS_TP_H_
#include<stdint.h>
#include"SID34_36_37TransferData.h"
typedef enum __NT_TIMER_T__
{
	TIMER_N_CR = 0,                 // N_CR 定时器，接收方收到连续帧间隔时间不能大于 TIMEOUT_N_CR，单位: ms
	TIMER_N_BS,                     // N_BS 定时器，发送方发送完成首帧后到接收到流控帧之间的时间不能大于 TIMEOUT_N_BS，单位: ms
	TIMER_STmin,                    // STmin 定时器，发送连续帧时，间隔时间最小为 g_rfc_stmin,单位: ms
	//TIMER_Response,
	TIMER_CNT                       // 定时器总个数
}nt_timer_t;



typedef enum __NETWORK_LAYER_STATUS_
{
	NWL_IDLE = 0,                   // 空闲状态
	NWL_XMIT,                       // 发送状态
	NWL_RECV,                       // 接收状态
	//NWL_DL,							// 下载状态，刷写
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


typedef enum _N_TATYPE_T_
{
	N_TATYPE_NONE = 0,                  // none
	N_TATYPE_PHYSICAL,                  // 物理寻址
	N_TATYPE_FUNCTIONAL                 // 功能寻址
}n_tatype_t;

typedef enum _N_RX_RESULT_
{
	
	N_OK = 0,
	N_TIMEOUT_Bs,                       // TIMER_N_BS 定时器超时
	N_TIMEOUT_Cr,                       // TIMER_N_CR 定时器超时
	N_TIMEROUT_Respone,
	N_WRONG_SN,                         // 接收到的连续帧帧序号错误
	N_INVALID_FS,                       // 接收到的流控帧中流状态非法
	N_UNEXP_PDU,                        // 不是期待的帧类型，比如在接收连续帧中莫名收到首帧
	N_BUFFER_OVFLW,                     // 接收到的流控帧中流状态为溢出
	N_FF_MSG,							//接收到首帧
	/*TX*/
	N_TX_OK,
}n_result_t;



//上层向 TP 层注册的发送报文接口函数
typedef int(*UDS_SEND_FRAME)(unsigned int, unsigned char*, unsigned char);

// 上层向 TP 层注册的一些接口函数，当 TP 层对数据做完处理后再通过这些接口函数将数据交由上层继续处理
//typedef void(*indication_func) (uint8_t* msg_buf, uint16_t msg_dlc, n_result_t n_result);


/******************************************************************************
* 函数名称: void uds_dataff_indication(uint8_t* msg_buf, uint16_t msg_dlc, n_result_t n_result);
* 功能说明: 上层回调函数
* 输入参数: 
* 输出参数: 
* 函数返回: 无
* 其它说明: 无
******************************************************************************/
void uds_data_indication(uint8_t* msg_buf, uint16_t msg_dlc, n_result_t n_result);

extern unsigned int task_cycle;
// 0:物理寻址; 1:功能寻址
extern uint8_t g_tatype;


//#define REQUEST_ID 0x724
//#define FUNCTION_ID 0x7df
//#define RESPONSE_ID 0x7a4
extern unsigned int REQUEST_ID;			// 请求 ID
extern unsigned int FUNCTION_ID;			// 功能 ID
extern unsigned int RESPONSE_ID;			// 应答 ID

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
#define NT_XMIT_FC_STMIN            (0x0A*1000)

// 接收方收到连续帧间隔时间不能大于 TIMEOUT_N_CR，单位: ms
#define TIMEOUT_N_CR                (1000*1000)

// 接收方收到响应时间不能大于 TIMEOUT_N_Response，单位: ms
#define TIMEOUT_Response                (1000*1000)

// 接收方收到响应时间不能大于 TIMEOUT_N_Response，单位: ms
#define TIMEOUT_SID                (1000*1000)

// 发送方发送完成首帧后到接收到流控帧之间的时间不能大于 TIMEOUT_N_BS，单位: ms
#define TIMEOUT_N_BS                (1000*1000)

#define FRAME_SIZE      8               // 帧长度



//#define REQUEST_ID      0x123           // 请求 ID
//#define FUNCTION_ID     0x7DF           // 功能 ID
//#define RESPONSE_ID     0x456           // 应答 ID

#define UDS_RX_MAX      1024            // 接收缓冲区长度 --UDS TP 层协议规定最大支持 4095 字节，但是可以根据实际需要改动，以免资源浪费
#define UDS_TX_MAX      0x802             // 发送缓冲区长度 --UDS TP 层协议规定最大支持 4095 字节，但是可以根据实际需要改动，以免资源浪费

#define PADDING_VAL     0x55            // 填充值，如果发送的有效数据不满一帧，则用该值填充

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


/*----------------------------------------------------------------------------------------------*/
typedef enum __UDS_NRC_ENUM__
{
	NRC_GENERAL_REJECT = 0x10, // 该服务响应不是协议里已支持的
	NRC_SERVICE_NOT_SUPPORTED = 0x11,	// ECU 压根就没做这个服务
	NRC_SUBFUNCTION_NOT_SUPPORTED = 0x12,	// ECU 不支持当前请求的子功能
	NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT = 0x13, // 请求报文的长度或者格式不正确
	NRC_CONDITIONS_NOT_CORRECT = 0x22, // 先决条件不满足
	NRC_REQUEST_SEQUENCE_ERROR = 0x24, // 请求报文的顺序不正确
	NRC_REQUEST_OUT_OF_RANGE = 0x31, // 参数超出范围/数据 ID 不支持
	NRC_SECURITY_ACCESS_DENIED = 0x33, // 不满足安全策略，请先解锁
	NRC_INVALID_KEY = 0x35, // 密钥不匹配
	NRC_EXCEEDED_NUMBER_OF_ATTEMPTS = 0x36, // 尝试解锁次数已达上限
	NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED = 0x37, // 安全访问失败，超时时间未到
	NRC_UPLOAD_DOWNLOAD_NOT_ACCEPTED = 0x70, // 不允许上传/下载
	NRC_TRANSFER_DATA_SUSPENDED = 0x71, // 数据传输终止
	NRC_GENERAL_PROGRAMMING_FAILURE = 0x72, // 擦除或烧写内存时错误
	NRC_WRONG_BLOCK_SEQUENCE_COUNTER = 0x73, // 块序列计数错误
	NRC_SERVICE_BUSY = 0x78, // 已正确接收请求消息，但会晚些回复
	NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION = 0x7E, // 当前会话下，该子功能不支持
	NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION = 0x7F, // 当前会话下，该服务不支持
	NRC_VOLTAGE_TOO_HIGH = 0x92, // 电压过高
	NRC_VOLTAGE_TOO_LOW = 0x93, // 电压过低
}uds_nrc_em;


// 第二个字节中的最高位 bit7 表示肯定响应抑制位
// 当其置 1 时，则表示不需要回复肯定响应，只进行服务处理即可
#define UDS_GET_SUB_FUNCTION_SUPPRESS_POSRSP(byte)    ((byte >> 7u)&0x01u)

// 获取子功能号 - 第二个字节中的低 7 位表示子功能号，范围：0 ~ 0x7F
#define UDS_GET_SUB_FUNCTION(byte)     (byte & 0x7fu)

// 肯定响应，服务 ID 需 +0x40
#define POSITIVE_RSP 			0x40
#define USD_GET_POSITIVE_RSP(server_id)         (POSITIVE_RSP + server_id)

// 否定响应
#define NEGATIVE_RSP 			0x7F

// 安全访问超时时间，单位：ms，如果安全访问种子匹配次数达到两次时，开启该定时器，在 TIMEOUT_FSA 时间内如果收到请求种子服务，则需要回复 NRC 37
#define TIMEOUT_FSA          	10000

// S3server 定时器超时时间，在非默认会话模式下，如果在 TIMEOUT_S3server 时间内没有收到任何消息的话，将自动回到默认会话，单位：ms
#define TIMEOUT_S3server     5000


typedef enum __UDS_TIMER_T__
{
	UDS_TIMER_FSA = 0,			// FSA 定时器，如果安全访问种子匹配次数达到两次时，开启该定时器，在 TIMEOUT_FSA 时间内如果收到请求种子服务，则需要回复 NRC 37
	UDS_TIMER_S3server,			// S3server 定时器，在非默认会话模式下，如果在 TIMEOUT_S3server 时间内没有收到任何消息的话，将自动回到默认会话
	UDS_TIMER_CNT				// 应用层定时器总个数
}uds_timer_t;


typedef enum __UDS_SESSION_T_
{
	UDS_SESSION_STD = 1,		// 默认会话
	UDS_SESSION_PROG,			// 编程会话
	UDS_SESSION_EXT				// 扩展会话
}uds_session_t;


typedef enum __UDS_SA_LV__
{
	UDS_SA_NON = 0,				// 安全访问等级 - 无
	UDS_SA_LV1,					// 安全访问等级 - 1 级
	UDS_SA_LV2,					// 安全访问等级 - 2 级
}uds_sa_lv;


//typedef struct __UDS_SERVICE_T__
//{
//	uint8_t uds_sid;									// 服务 ID
//	void(*uds_service)  (const uint8_t *, uint16_t);	// 服务处理函数
//	bool_t(*check_len)  (const uint8_t *, uint16_t);	// 检查数据长度是否合法
//	bool_t std_spt;   									// 是否支持默认会话
//	bool_t prog_spt;  									// 是否支持编程会话
//	bool_t ext_spt;   									// 是否支持扩展会话
//	bool_t fun_spt;   									// 是否支持功能寻址
//	bool_t ssp_spt;  									// 是否支持肯定响应抑制
//	uds_sa_lv uds_sa; 									// 安全访问等级
//}uds_service_t;
/*-------------------------------------------------------------------------------------------------------*/


/*-----------------------------------------service_cfg----------------------------------------------------------------*/
#define SID_NUM       16     // 当前共支持 16 个服务

#define SID_10        (0x10) /* SessionControl */
#define SID_11        (0x11) /* ECUReset */
#define SID_14        (0x14) /* ClearDTC */
#define SID_18        (0x18) /* KWPReadDTC */
#define SID_19        (0x19) /* ReadDTC */
#define SID_22        (0x22) /* ReadID */
#define SID_27        (0x27) /* SecurityAccess */
#define SID_2E        (0x2E) /* WriteID */
#define SID_2F        (0x2F) /* InputOutputControlID */
#define SID_28        (0x28) /* CommunicationControl */
#define SID_31        (0x31) /* RoutineControl */
#define SID_3E        (0x3E) /* TesterPresent */
#define SID_85        (0x85) /* ControlDTCSetting */
#define SID_34        (0x34) /* RequestDownload */
#define SID_36        (0x36) /* TransferData */
#define SID_37        (0x37) /* RequestTransferExit */



typedef struct __UDS_SERVICE_Info__
{
	uint8_t uds_sid;									// 服务 ID
	uint8_t timerflag;
	uint32_t TIMER_SID;
}uds_service_info_t;






typedef enum __SID_TIMER_T__
{
	TIMER_SID_10 = 0,
	TIMER_SID_11,
	TIMER_SID_14,
	TIMER_SID_18,
	TIMER_SID_19,
	TIMER_SID_22,
	TIMER_SID_27,
	TIMER_SID_2E,
	TIMER_SID_2F,
	TIMER_SID_28,
	TIMER_SID_31,
	TIMER_SID_3E,
	TIMER_SID_85,
	TIMER_SID_34,
	TIMER_SID_36,
	TIMER_SID_37,
	TIMER_SID_CNT                       // 定时器总个数
}sid_timer_t;

extern uds_service_info_t uds_service_list[SID_NUM];



/*---------------------------------------------------------------------------------------------------------*/



/******************************************************************************
* 函数名称: static void nt_timer_start(nt_timer_t num)
* 功能说明: 启动 TP 层定时器
* 输入参数: nt_timer_t num              --定时器
* 输出参数: 无
* 函数返回: 无
* 其它说明: 无
******************************************************************************/
static void nt_timer_start(nt_timer_t num);


/******************************************************************************
* 函数名称: static void nt_timer_start_wv(nt_timer_t num, uint32_t value)
* 功能说明: 重新设置定时器的计数值
* 输入参数: nt_timer_t num              --定时器
	　　　　uint32_t value              --计数值
* 输出参数: 无
* 函数返回: 无
* 其它说明: 当 value = 0，表示关闭定时器，等同于 nt_timer_stop() 函数
	　　　　当 value = 1，表示定时器超时已发生，接下来将处理超时事件
	　　　　当 value 为其它值时，定时器将根据 value 值重新开始计时
******************************************************************************/
static void nt_timer_start_wv(nt_timer_t num, uint32_t value);


/******************************************************************************
* 函数名称: static void nt_timer_stop (nt_timer_t num)
* 功能说明: 关闭定时器
* 输入参数: nt_timer_t num              --定时器
* 输出参数: 无
* 函数返回: 无
* 其它说明: 无
******************************************************************************/
static void nt_timer_stop(nt_timer_t num);

/******************************************************************************
* 函数名称: static int nt_timer_run(nt_timer_t num)
* 功能说明: 定时器计数运行
* 输入参数: nt_timer_t num              --定时器
* 输出参数: 无
* 函数返回: 0: 定时器已经被关闭; -1: 超时发生; 1: 定时器正在计时运行
* 其它说明: 该函数需要被 1ms 周期调用
******************************************************************************/
static int nt_timer_run(nt_timer_t num);


/******************************************************************************
* 函数名称: static int nt_timer_chk(nt_timer_t num)
* 功能说明: 检查定时器状态然后关闭定时器
* 输入参数: nt_timer_t num              --定时器
* 输出参数: 无
* 函数返回: 0: 定时器已停止运行;  1: 定时器正在计时运行
* 其它说明: 该函数执行后，无论定时器是否正在运行，都将被关闭
******************************************************************************/
static int nt_timer_chk(nt_timer_t num);


/******************************************************************************
* 函数名称: static void clear_network(void)
* 功能说明: 复位网络层状态
* 输入参数: 无
* 输出参数: 无
* 函数返回: 无
* 其它说明: 主要是复位各种状态变量，并且关闭所有相关定时器
******************************************************************************/
static void clear_network(void);



/******************************************************************************
* 函数名称: static int recv_singleframe (uint8_t* frame_buf, uint8_t frame_dlc)
* 功能说明: 处理单帧
* 输入参数: uint8_t*    frame_buf       --接收报文帧数据首地址
	　　　　uint8_t     frame_dlc       --接收报文帧数据长度
* 输出参数: 无
* 函数返回: 0: OK; -1: ERR
* 其它说明: 无
******************************************************************************/
int recv_singleframe(UDS_SEND_FRAME sendframefun, uint8_t* frame_buf, uint8_t frame_dlc);


/******************************************************************************
* 函数名称: recv_firstframe(UDS_SEND_FRAME sendframefun, uint8_t* frame_buf, uint8_t frame_dlc)
* 功能说明: 处理首帧
* 输入参数: UDS_SEND_FRAME sendframefun --发送报文函数指针，硬件驱动接口函数
			uint8_t*    frame_buf       --接收报文帧数据首地址
	　　　　uint8_t     frame_dlc       --接收报文帧数据长度
	　　　　
* 输出参数: 无
* 函数返回: 0: OK; other: -ERR
* 其它说明: 无
******************************************************************************/
int recv_firstframe(UDS_SEND_FRAME sendframefun, uint8_t* frame_buf, uint8_t frame_dlc);

/******************************************************************************
* 函数名称: int recv_consecutiveframe(UDS_SEND_FRAME sendframefun, uint8_t* frame_buf, uint8_t frame_dlc)
* 功能说明: 处理连续帧
* 输入参数: UDS_SEND_FRAME sendframefun --发送报文函数指针，硬件驱动接口函数
	　　　　uint8_t*    frame_buf       --接收报文帧数据首地址
	　　　　uint8_t     frame_dlc       --接收报文帧数据长度
* 输出参数: 无
* 函数返回: 0: 所有连续帧接收完成; 1: 继续接收; other: -ERR
* 其它说明: 无
******************************************************************************/
static int recv_consecutiveframe(UDS_SEND_FRAME sendframefun, uint8_t* frame_buf, uint8_t frame_dlc);

/******************************************************************************
* 函数名称: static int recv_flowcontrolframe(uint8_t* frame_buf, uint8_t frame_dlc)
* 功能说明: 处理流控帧
* 输入参数: uint8_t*    frame_buf       --接收报文帧数据首地址
	　　　　uint8_t     frame_dlc       --接收报文帧数据长度
* 输出参数: 无
* 函数返回: 0: 所有连续帧接收完成; 1: 继续接收; other: -ERR
* 其它说明: 无
******************************************************************************/
static int recv_flowcontrolframe(uint8_t* frame_buf, uint8_t frame_dlc);


/******************************************************************************
* 函数名称: void send_flowcontrol(UDS_SEND_FRAME sendframefun, network_flow_status_t flow_st)
* 功能说明: 发送流控帧
* 输入参数: UDS_SEND_FRAME sendframefun --发送报文函数指针，硬件驱动接口函数
	　　　　network_flow_status_t   flow_st       --流状态
* 输出参数: 无
* 函数返回: 0: 所有连续帧接收完成; 1: 继续接收; other: -ERR
* 其它说明: 无
******************************************************************************/
void send_flowcontrol(UDS_SEND_FRAME sendframefun, network_flow_status_t flow_st);

/******************************************************************************
* 函数名称: int send_singleframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint8_t msg_dlc)
* 功能说明: 发送单帧
* 输入参数: UDS_SEND_FRAME sendframefun --发送报文函数指针，硬件驱动接口函数
	　　　　uint8_t*    msg_buf         --发送数据首地址
	　　　　uint8_t     msg_dlc         --发送数据长度
* 输出参数: 无
* 函数返回: 0: OK; -1: ERR
* 其它说明: 无
******************************************************************************/
int send_singleframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint8_t msg_dlc);


/******************************************************************************
* 函数名称: static int send_firstframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc)
* 功能说明: 发送首帧
* 输入参数: UDS_SEND_FRAME sendframefun --发送报文函数指针，硬件驱动接口函数
	　　　　uint8_t*    msg_buf         --发送数据首地址
	　　　　uint8_t     msg_dlc         --发送数据长度
* 输出参数: 无
* 函数返回: 成功发送的有效数据长度
* 其它说明: 无
******************************************************************************/
int send_firstframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc);

/******************************************************************************
* 函数名称: static int send_consecutiveframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc, uint8_t frame_sn)
* 功能说明: 发送连续帧
* 输入参数: UDS_SEND_FRAME sendframefun --发送报文函数指针，硬件驱动接口函数
	　　　　uint8_t*    msg_buf         --发送数据首地址
	　　　　uint8_t     msg_dlc         --发送数据长度
	　　　　uint8_t     frame_sn        --连续帧帧序号
* 输出参数: 无
* 函数返回: 成功发送的有效数据长度
* 其它说明: 无
******************************************************************************/
static int send_consecutiveframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc, uint8_t frame_sn);



/******************************************************************************
* 函数名称: static int send_multipleframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc)
* 功能说明: 发送多帧
* 输入参数: UDS_SEND_FRAME sendframefun --发送报文函数指针，硬件驱动接口函数
	　　　　uint8_t*    msg_buf         --发送数据首地址
	　　　　uint8_t     msg_dlc         --发送数据长度
* 输出参数: 无
* 函数返回: 0: OK; -1: ERR
* 其它说明: 无
******************************************************************************/
int send_multipleframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc);


/******************************************************************************
* 函数名称: int network_send_udsmsg(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc)
* 功能说明: 发送数据
* 输入参数: UDS_SEND_FRAME sendframefun --发送报文函数指针，硬件驱动接口函数
	　　　　uint8_t*    msg_buf         --发送数据首地址
	　　　　uint8_t     msg_dlc         --发送数据长度
* 输出参数: 无
* 函数返回: 0: OK; -1: ERR
* 其它说明: TP 层向上层提供的数据发送接口
******************************************************************************/
int network_send_udsmsg(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc);


/******************************************************************************
* 函数名称: void network_task(UDS_SEND_FRAME sendframefun)
* 功能说明: TP 层任务处理
* 输入参数: UDS_SEND_FRAME sendframefun --发送报文函数指针，硬件驱动接口函数
* 输出参数: 无
* 函数返回: 无
* 其它说明: 该函数需要被 1ms 周期调用
******************************************************************************/
void network_task(UDS_SEND_FRAME sendframefun);



/******************************************************************************
* 函数名称: void uds_tp_recv_frame(UDS_SEND_FRAME sendframefun, uint8_t func_addr, uint8_t* frame_buf, uint8_t frame_dlc)
* 功能说明: 接收到一帧报文并处理
* 输入参数: UDS_SEND_FRAME sendframefun --发送报文函数指针，硬件驱动接口函数
	　　　　uint8_t     func_addr       --0:物理寻址; 1:功能寻址
	　　　　uint8_t*    frame_buf       --接收报文帧数据首地址
	　　　　uint8_t     frame_dlc       --接收报文帧数据长度
* 输出参数: 无
* 函数返回: 无
* 其它说明: frame_dlc 长度必须等于 FRAME_SIZE，否则判断为无效帧
******************************************************************************/
void uds_tp_recv_frame(UDS_SEND_FRAME sendframefun, uint8_t* frame_buf, uint8_t frame_dlc);



///******************************************************************************
//* 函数名称: int network_reg(nt_usdata_t* usdata)
//* 功能说明: 上层向 TP 层注册一些接口函数，当 TP 层对数据做完处理后再通过这些接口函数将数据交由上层继续处理
//* 输入参数: nt_usdata_t* usdata      --上层接口函数
//* 输出参数: 无
//* 函数返回: 0: OK; -1: ERR
//* 其它说明: 指示服务（Indication）：用于向更上层或应用层传递状态信息及接收到的数据
//	　　　　确认服务（Confirm）：用于向更上层或应用层传递状态信息
//	　　　　请求服务（Request）：用于上层向网络层传递控制报文信息及要发送的数据
//******************************************************************************/
//int network_reg(nt_usdata_t* usdata);


/******************************************************************************
* 函数名称: int SecurityAccessWithDLL(char *iFilename, const unsigned char iSeed[], unsigned int iSeedSize, const unsigned int iSecurityLevel, unsigned char *oKeyArray, unsigned int iKeyArrayMaxSize,unsigned int *oSize)
* 功能说明: 调用Vector Dll解锁
* 输入参数:  char *iFilename						Dll路径,文件名
			const unsigned char iSeed[]			输入种子
			unsigned int iSeedSize				输入种子长度
			const unsigned int iSecurityLevel	解锁等级

			unsigned int iKeyArrayMaxSize		key数组最大长度

* 输出参数:	unsigned char *oKeyArray			输出key
			unsigned int *oSize					输出key长度
* 函数返回:	0：调用GenerateKeyEx成功解锁
			1：调用GenerateKeyExOpt成功解锁
			-1:调用失败
* 其它说明:
******************************************************************************/
int SecurityAccessWithDLL(char *iFilename, const unsigned char iSeed[], unsigned int iSeedSize, const unsigned int iSecurityLevel, unsigned char *oKeyArray, unsigned int iKeyArrayMaxSize, unsigned int *oSize);

#endif