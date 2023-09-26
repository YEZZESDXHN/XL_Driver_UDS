#include "uds_tp.h"
#include <stdint.h>
// 网络层状态, 共有 3 种 空闲状态(NWL_IDLE)、发送状态(NWL_XMIT)、接收状态(NWL_RECV)
// 当接收到首帧时，状态被置为 NWL_RECV，直到连续帧接收完成才置为 NWL_IDLE
// 当发送多帧时，该状态被置为 NWL_XMIT，直到发送完成后才置为 NWL_IDLE
static network_layer_st nwl_st = NWL_IDLE;

// 连续帧接收标志，从接收到首帧时置 1，直到连续帧接收完成置 0
static bool_t g_wait_cf = FALSE;

// 流控帧等待标志，当发送首帧时，该标志置 1，当收到流控帧时，该消息置 0，如果流控帧中的流状态为等待状态，则该标志依旧置 1
static bool_t g_wait_fc = FALSE;

// TP 层相关定时计数器
static uint32_t nt_timer[TIMER_CNT] = { 0 };

// 接收到流控帧中的 STmin 帧间隔时间，在向外发送连续帧时，发送间隔不能小于该值
static uint8_t g_rfc_stmin = 0;

// 接收到流控帧中的 bolck size 块大小，
// 如果该值不为 0，则每连续收到 g_rfc_bs 个连续帧后，需返回一帧流控帧
// 如果该值为 0，表明发送方可以一直无限制发送连续帧，直到发送完成所有连续帧
static uint8_t g_rfc_bs = 0;

// 连续帧发送计数
static uint8_t g_xcf_bc = 0;

// 连续帧帧序号，多帧发送时候使用，每发送一帧连续帧时其应该 +1，超过 0xf 复位为 0
static uint8_t g_xcf_sn = 0;

// 接收流控帧计数标志，当其 >= NT_XMIT_FC_BS (前提条件：NT_XMIT_FC_BS 不为 0) 时，应返回一帧流控帧，并清零复位
static uint8_t g_rcf_bc = 0;

// 连续帧帧序号，接收连续帧时候使用，在收到的连续帧中，其应该每次 +1，超过 0xf 复位为 0 
static uint8_t g_rcf_sn = 0;

// 发送数据缓冲区
static uint8_t remain_buf[UDS_TX_MAX];

// 剩余需要发送的有效数据长度，每向外发送一帧报文，remain_len 都需要减去其中有效数据的长度
static uint16_t remain_len = 0;

// 剩余需要发送的有效数据在 remain_buf 数组中的起始位置
static uint16_t remain_pos = 0;

// 接收数据缓冲区，多帧数据将会拼接，形成一个完整的有效数据存放到该数组中
static uint8_t recv_buf[UDS_RX_MAX];

// 单独给单帧数据设计一个独立的缓冲区，主要是为了在多帧接收的过程中，依旧能够正常接收并处理单帧
static uint8_t recv_buf_sf[FRAME_SIZE];

// 有效数据总长度，用于接收多帧报文，其在首帧中赋值，其应该等于首帧+后面所有连续帧中的有效数据长度
static uint16_t recv_fdl = 0;

// 实际接收到的有效数据总长度，用于接收多帧报文时累加计数
static uint16_t recv_len = 0;

// 0:物理寻址; 1:功能寻址
uint8_t g_tatype;






///******************************************************************************
//* 函数名称: static void nt_timer_start(nt_timer_t num)
//* 功能说明: 启动 TP 层定时器
//* 输入参数: nt_timer_t num              --定时器
//* 输出参数: 无
//* 函数返回: 无
//* 其它说明: 无
//******************************************************************************/
//static void nt_timer_start(nt_timer_t num)
//{
//	// 检查参数合法性
//	if (num >= TIMER_CNT) return;
//
//	// 启动 N_CR 定时器
//	if (num == TIMER_N_CR)
//		nt_timer[TIMER_N_CR] = TIMEOUT_N_CR + 1;
//
//	// 启动 N_BS 定时器
//	if (num == TIMER_N_BS)
//		nt_timer[TIMER_N_BS] = TIMEOUT_N_BS + 1;
//
//	// 启动 STmin 定时器
//	if (num == TIMER_STmin)
//		nt_timer[TIMER_STmin] = g_rfc_stmin;
//}
//
//
///******************************************************************************
//* 函数名称: static void nt_timer_start_wv(nt_timer_t num, uint32_t value)
//* 功能说明: 重新设置定时器的计数值
//* 输入参数: nt_timer_t num              --定时器
//	　　　　uint32_t value              --计数值
//* 输出参数: 无
//* 函数返回: 无
//* 其它说明: 当 value = 0，表示关闭定时器，等同于 nt_timer_stop() 函数
//	　　　　当 value = 1，表示定时器超时已发生，接下来将处理超时事件
//	　　　　当 value 为其它值时，定时器将根据 value 值重新开始计时
//******************************************************************************/
//static void nt_timer_start_wv(nt_timer_t num, uint32_t value)
//{
//	// 检查参数合法性
//	if (num >= TIMER_CNT) return;
//
//	// 重新设置 N_CR 定时器的计数值
//	if (num == TIMER_N_CR)
//		nt_timer[TIMER_N_CR] = value;
//
//	// 重新设置 N_BS 定时器的计数值
//	if (num == TIMER_N_BS)
//		nt_timer[TIMER_N_BS] = value;
//
//	// 重新设置 STmin 定时器的计数值
//	if (num == TIMER_STmin)
//		nt_timer[TIMER_STmin] = value;
//}
//
//
///******************************************************************************
//* 函数名称: static void nt_timer_stop (nt_timer_t num)
//* 功能说明: 关闭定时器
//* 输入参数: nt_timer_t num              --定时器
//* 输出参数: 无
//* 函数返回: 无
//* 其它说明: 无
//******************************************************************************/
//static void nt_timer_stop(nt_timer_t num)
//{
//	// 检查参数合法性
//	if (num >= TIMER_CNT) return;
//
//	// 计数值清 0，表示关闭定时器
//	nt_timer[num] = 0;
//}
//
///******************************************************************************
//* 函数名称: static int nt_timer_run(nt_timer_t num)
//* 功能说明: 定时器计数运行
//* 输入参数: nt_timer_t num              --定时器
//* 输出参数: 无
//* 函数返回: 0: 定时器已经被关闭; -1: 超时发生; 1: 定时器正在计时运行
//* 其它说明: 该函数需要被 1ms 周期调用
//******************************************************************************/
//static int nt_timer_run(nt_timer_t num)
//{
//	// 检查参数合法性
//	if (num >= TIMER_CNT) return 0;
//
//	// 如果计数值为 0，表示定时器已经关闭，不再工作
//	if (nt_timer[num] == 0)
//	{
//		return 0;                   // 返回 0，定时器已经被关闭
//	}
//	// 如果计数值为 1，表示定时器超时已发生
//	else if (nt_timer[num] == 1)
//	{
//		nt_timer[num] = 0;          // 关闭定时器
//		return -1;                  // 返回 -1，发生超时
//	}
//	// 其余情况则表示定时器正在运行
//	else
//	{
//		nt_timer[num]--;            // 计数值 -1
//		return 1;                   // 返回 1，定时器正在计时运行
//	}
//}
//
//
///******************************************************************************
//* 函数名称: static int nt_timer_chk(nt_timer_t num)
//* 功能说明: 检查定时器状态然后关闭定时器
//* 输入参数: nt_timer_t num              --定时器
//* 输出参数: 无
//* 函数返回: 0: 定时器已停止运行;  1: 定时器正在计时运行
//* 其它说明: 该函数执行后，无论定时器是否正在运行，都将被关闭
//******************************************************************************/
//static int nt_timer_chk(nt_timer_t num)
//{
//	// 检查参数合法性
//	if (num >= TIMER_CNT) return 0;
//
//	// 如果定时器计数值 > 0,表示定时器正在工作
//	if (nt_timer[num] > 0)
//	{
//		nt_timer[num] = 0;          // 关闭定时器
//		return 1;                   // 返回 1，定时器正在计时运行
//	}
//	else
//	{
//		nt_timer[num] = 0;          // 定时器已停止运行
//		return 0;                   // 返回 0，定时器已停止运行
//	}
//}


/******************************************************************************
* 函数名称: static void clear_network(void)
* 功能说明: 复位网络层状态
* 输入参数: 无
* 输出参数: 无
* 函数返回: 无
* 其它说明: 主要是复位各种状态变量，并且关闭所有相关定时器
******************************************************************************/
static void clear_network(void)
{
	nt_timer_t num;
	nwl_st = NWL_IDLE;                      // 置网络层状态为空闲状态
	g_wait_cf = FALSE;                      // 清连续帧接收标志
	g_wait_fc = FALSE;                      // 清流控帧等待标志
	g_xcf_bc = 0;                           // 清连续帧发送计数
	g_xcf_sn = 0;                           // 清连续帧发送帧序号
	g_rcf_bc = 0;                           // 清接收流控帧计数标志
	g_rcf_sn = 0;                           // 清收到连续帧帧序号 

	// 关闭所有网络层相关定时器
	for (num = 0; num < TIMER_CNT; num++)
		nt_timer_stop(num);
}



/******************************************************************************
* 函数名称: static int recv_singleframe (uint8_t* frame_buf, uint8_t frame_dlc)
* 功能说明: 处理单帧
* 输入参数: uint8_t*    frame_buf       --接收报文帧数据首地址
	　　　　uint8_t     frame_dlc       --接收报文帧数据长度
* 输出参数: 无
* 函数返回: 0: OK; -1: ERR
* 其它说明: 无
******************************************************************************/
int recv_singleframe(uint8_t* frame_buf, uint8_t frame_dlc)
{
	uint16_t i, uds_dlc;

	// 单帧第一个字节的低 4 位为有效数据长度
	uds_dlc = NT_GET_SF_DL(frame_buf[0]);

	// 有效数据长度合法性检测
	if (uds_dlc > FRAME_SIZE - 1 || 0 == uds_dlc)
		return -1;

	// 将有效数据拷贝到 recv_buf_sf 中
	for (i = 0; i < uds_dlc; i++)
		recv_buf_sf[i] = frame_buf[1 + i];

	// TP 层单帧数据处理完成，将数据交由上层继续处理
	//N_USData.indication(recv_buf_sf, uds_dlc, N_OK);

	return 0;
}


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
int recv_firstframe(UDS_SEND_FRAME sendframefun, uint16_t REQUEST_ID, uint8_t* frame_buf, uint8_t frame_dlc)
{
	uint16_t i;
	uint16_t uds_dlc;

	// 首帧 byte0 的低 4 位以及 byte1 共拼成 12 位的有效数据长度，数据长度最大为 4095
	uds_dlc = ((uint16_t)(frame_buf[0] & 0x0f)) << 8;
	uds_dlc |= frame_buf[1];

	// 多帧有效数据长度不应该小于 FRAME_SIZE
	if (uds_dlc < FRAME_SIZE)
		return -1;

	// 如果有效数据长度大于 UDS_RX_MAX， 则返回流控帧(溢出)
	if (uds_dlc > UDS_RX_MAX)
	{
		send_flowcontrol(sendframefun, REQUEST_ID,FS_OVFLW);
		return -2;
	}

	// 有效数据总长度
	recv_fdl = uds_dlc;

	// 将首帧中的有效数据拷贝到 recv_buf 数组中
	for (i = 0; i < frame_dlc - 2; i++)
		recv_buf[i] = frame_buf[2 + i];

	// recv_len: 实际接收到的有效数据长度，其在后续的连续帧中累加
	recv_len = frame_dlc - 2;

	// 返回流控帧(允许继续无限制发送)，发送最小间隔为 NT_XMIT_FC_STMIN
	send_flowcontrol(sendframefun, REQUEST_ID,FS_CTS);

	// 清接收流控帧计数标志
	g_rcf_bc = 0;

	// 接收到首帧后意味着接下来将要收到连续帧，这里置连续帧接收标志
	g_wait_cf = TRUE;

	// 开启 N_CR 定时器，接收方收到连续帧间隔时间不应大于 TIMEOUT_N_CR
	//nt_timer_start(TIMER_N_CR);

	// 清连续帧帧序号 
	g_rcf_sn = 0;

	// TP 层首帧数据处理完成，通知上层
	//N_USData.ffindication(N_OK);

	return 0;
}


/******************************************************************************
* 函数名称: static void send_flowcontrol(network_flow_status_t flow_st)
* 功能说明: 发送流控帧
* 输入参数: network_flow_status_t   flow_st       --流状态
* 输出参数: 无
* 函数返回: 0: 所有连续帧接收完成; 1: 继续接收; other: -ERR
* 其它说明: 无
******************************************************************************/
void send_flowcontrol(UDS_SEND_FRAME sendframefun, uint16_t REQUEST_ID,network_flow_status_t flow_st)
{
	uint16_t i;
	uint8_t send_buf[FRAME_SIZE] = { 0 };

	// 填充默认值
	for (i = 0; i < FRAME_SIZE; i++)
		send_buf[i] = PADDING_VAL;

	// 高 4 位表示帧类型：流控帧; 低 4 位表示流状态，
	send_buf[0] = NT_SET_PCI_TYPE_FC(flow_st);

	// NT_XMIT_FC_BS: 允许发送连续帧的个数，若为 0，则表示发送方可以一直无限制发送连续帧
	// 若不为 0，则表示当发送方发送的连续帧个数为 NT_XMIT_FC_BS 后需等待接收方回复一帧流控帧，发送方根据流控帧决定接下来的发送情况
	send_buf[1] = NT_XMIT_FC_BS;

	// 帧间隔，发送方发送连续帧时，至少需要间隔 NT_XMIT_FC_STMIN 时间 
	send_buf[2] = NT_XMIT_FC_STMIN;

	// 发送
	sendframefun(REQUEST_ID, send_buf, FRAME_SIZE);
}

/******************************************************************************
* 函数名称: static int send_singleframe (uint8_t* msg_buf, uint16_t msg_dlc)
* 功能说明: 发送单帧
* 输入参数: uint8_t*    msg_buf         --发送数据首地址
	　　　　uint8_t     msg_dlc         --发送数据长度
* 输出参数: 无
* 函数返回: 0: OK; -1: ERR
* 其它说明: 无
******************************************************************************/
int send_singleframe(UDS_SEND_FRAME sendframefun, uint16_t REQUEST_ID,uint8_t* msg_buf, uint8_t msg_dlc)
{
	uint16_t i;
	uint8_t send_buf[FRAME_SIZE] = { 0 };
	
	// 填充默认值
	for (i = 0; i < FRAME_SIZE; i++)
		send_buf[i] = PADDING_VAL;

	// 检查参数合法性
	if (0 == msg_dlc || msg_dlc > (FRAME_SIZE - 1)) return -1;

	// 单帧第一个字节高 4 位表示帧类型；低 4 位表示帧有效数据长度 
	send_buf[0] = NT_SET_PCI_TYPE_SF((uint8_t)msg_dlc);

	// 拷贝有效数据到 send_buf 中
	for (i = 0; i < msg_dlc; i++)
		send_buf[1 + i] = msg_buf[i];

	// 发送
	sendframefun(REQUEST_ID, send_buf, FRAME_SIZE);

	return 0;

}