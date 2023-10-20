#include "uds_tp.h"
#include <stdint.h>
#include<stdio.h>
#include<Windows.h>



// 网络层状态, 共有 3 种 空闲状态(NWL_IDLE)、发送状态(NWL_XMIT)、接收状态(NWL_RECV)
// 当接收到首帧时，状态被置为 NWL_RECV，直到连续帧接收完成才置为 NWL_IDLE
// 当发送多帧时，该状态被置为 NWL_XMIT，直到发送完成后才置为 NWL_IDLE
static network_layer_st nwl_st = NWL_IDLE;

unsigned int task_cycle = 1000;//单位微秒，1000表示1ms


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

// 需要发送有效数据总长度，由首帧获取
static uint8_t remain_buf_len = 0;

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
uint8_t g_tatype=0;

// 上层向 TP 层注册的一些接口函数将会记录在 N_USData 中，当 TP 层对数据做完处理后再通过这些接口函数将数据交由上层继续处理
//static nt_usdata_t N_USData = { NULL, NULL, NULL };

unsigned int REQUEST_ID = 0x726;			// 请求 ID
unsigned int FUNCTION_ID = 0x7df;			// 功能 ID
unsigned int RESPONSE_ID = 0x7A6;			// 应答 ID

/******************************************************************************
* 函数名称: static void nt_timer_start(nt_timer_t num)
* 功能说明: 启动 TP 层定时器
* 输入参数: nt_timer_t num              --定时器
* 输出参数: 无
* 函数返回: 无
* 其它说明: 无
******************************************************************************/
static void nt_timer_start(nt_timer_t num)
{
	// 检查参数合法性
	if (num >= TIMER_CNT) return;

	// 启动 N_CR 定时器
	if (num == TIMER_N_CR)
		nt_timer[TIMER_N_CR] = TIMEOUT_N_CR + 1;

	// 启动 N_BS 定时器
	if (num == TIMER_N_BS)
		nt_timer[TIMER_N_BS] = TIMEOUT_N_BS + 1;

	// 启动 STmin 定时器
	if (num == TIMER_STmin)
		nt_timer[TIMER_STmin] = g_rfc_stmin;

	// 启动 TIMEOUT_N_Response 定时器
	if (num == TIMER_Response)
		nt_timer[TIMER_Response] = TIMEOUT_Response+1;
}


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
static void nt_timer_start_wv(nt_timer_t num, uint32_t value)
{
	// 检查参数合法性
	if (num >= TIMER_CNT) return;

	// 重新设置 N_CR 定时器的计数值
	if (num == TIMER_N_CR)
		nt_timer[TIMER_N_CR] = value;

	// 重新设置 N_BS 定时器的计数值
	if (num == TIMER_N_BS)
		nt_timer[TIMER_N_BS] = value;

	// 重新设置 STmin 定时器的计数值
	if (num == TIMER_STmin)
		nt_timer[TIMER_STmin] = value;
}


/******************************************************************************
* 函数名称: static void nt_timer_stop (nt_timer_t num)
* 功能说明: 关闭定时器
* 输入参数: nt_timer_t num              --定时器
* 输出参数: 无
* 函数返回: 无
* 其它说明: 无
******************************************************************************/
static void nt_timer_stop(nt_timer_t num)
{
	// 检查参数合法性
	if (num >= TIMER_CNT) return;

	// 计数值清 0，表示关闭定时器
	nt_timer[num] = 0;
}

/******************************************************************************
* 函数名称: static int nt_timer_run(nt_timer_t num)
* 功能说明: 定时器计数运行
* 输入参数: nt_timer_t num              --定时器
* 输出参数: 无
* 函数返回: 0: 定时器已经被关闭; -1: 超时发生; 1: 定时器正在计时运行
* 其它说明: 该函数需要被 1ms 周期调用
******************************************************************************/
static int nt_timer_run(nt_timer_t num)
{
	// 检查参数合法性
	if (num >= TIMER_CNT) return 0;

	// 如果计数值为 0，表示定时器已经关闭，不再工作
	if (nt_timer[num] == 0)
	{
		return 0;                   // 返回 0，定时器已经被关闭
	}
	// 如果计数值为 1，表示定时器超时已发生
	else if (nt_timer[num] == 1)
	{
		nt_timer[num] = 0;          // 关闭定时器
		return -1;                  // 返回 -1，发生超时
	}
	// 其余情况则表示定时器正在运行
	else
	{
		nt_timer[num]--;            // 计数值 -1
		//nt_timer[num] = nt_timer[num] - task_cycle;
		//if (nt_timer[num] <= 17)
		//{
		//	nt_timer[num] = 18;
		//}
		//nt_timer[num]= nt_timer[num]-17;            // 计数值 -1
		//if(nt_timer[num])
		return 1;                   // 返回 1，定时器正在计时运行
	}
}


/******************************************************************************
* 函数名称: static int nt_timer_chk(nt_timer_t num)
* 功能说明: 检查定时器状态然后关闭定时器
* 输入参数: nt_timer_t num              --定时器
* 输出参数: 无
* 函数返回: 0: 定时器已停止运行;  1: 定时器正在计时运行
* 其它说明: 该函数执行后，无论定时器是否正在运行，都将被关闭
******************************************************************************/
static int nt_timer_chk(nt_timer_t num)
{
	// 检查参数合法性
	if (num >= TIMER_CNT)
	{
		return 0;
	}

	// 如果定时器计数值 > 0,表示定时器正在工作
	if (nt_timer[num] > 0)
	{
		nt_timer[num] = 0;          // 关闭定时器
		return 1;                   // 返回 1，定时器正在计时运行
	}
	else
	{
		nt_timer[num] = 0;          // 定时器已停止运行
		return 0;                   // 返回 0，定时器已停止运行
	}
}


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

int service_27_SecurityAccess(UDS_SEND_FRAME sendframefun, char *iFilename, uint8_t* msg_buf, uint16_t msg_dlc)
{
	unsigned char iSeed[FRAME_SIZE - 3];
	unsigned int iSeedSize;
	unsigned int iSecurityLevel;
	unsigned int iKeyArrayMaxSize = FRAME_SIZE - 3;
	unsigned char oKey[FRAME_SIZE - 3];
	unsigned int oKeyLen = 0;

	unsigned char senddata[FRAME_SIZE];

	char dllname_char[128];
	WCHAR dllname[128];
	int ret;

	if (msg_buf[0] != 0x67)
		return -3;

	if (msg_buf[1] % 2 == 0)
		return -4;
	iSeedSize = msg_dlc - 2;
	for (int i = 0; i < iSeedSize; i++)
	{
		iSeed[i] = msg_buf[i + 2];
	}
	iSecurityLevel = msg_buf[1];

	snprintf(dllname_char, 128, "./SecurityAccessDLL/");
	strncat_s(dllname_char, 128, iFilename, 128);
	Char2Wchar(dllname, dllname_char);
	ret = SecurityAccessWithDLL(dllname, iSeed, iSeedSize, iSecurityLevel, oKey, iKeyArrayMaxSize, &oKeyLen);
	if (ret >= 0)
	{
		senddata[0] = 0x27;
		senddata[1] = iSecurityLevel + 1;
		//setHEXtocontrol(Edit_out, oKeyLen, 1);
		for (int i = 0; i < 4; i++)
		{
			senddata[i + 2] = oKey[i];
		}


		send_singleframe(sendframefun, senddata, iSeedSize + 2);
		return ret;
	}

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
int recv_singleframe(UDS_SEND_FRAME sendframefun, uint8_t* frame_buf, uint8_t frame_dlc)
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
	////N_USData.indication(recv_buf_sf, uds_dlc, N_OK);


	if (recv_buf_sf[0] == 0x67 && recv_buf_sf[1] % 2 == 1)//收到种子，回复密钥解锁
	{
		service_27_SecurityAccess(sendframefun, "SeednKeyMR", recv_buf_sf, uds_dlc);
	}
	else if (recv_buf_sf[0] == 0x74)//下载请求正响应，Flash状态置FLASH_DOWNLOAD
	{
		nwf_st = FLASH_REQUEST_Postive;
	}
	else if (recv_buf_sf[0] == 0x77)//退出下载请求正响应，Flash状态置FLASH_IDLE
	{
		nwf_st = FLASH_IDLE;
	}
	else if (recv_buf_sf[0] == 0x76)//退出下载请求正响应，Flash状态置FLASH_IDLE
	{
		nwf_st = FLASH_36service_finsh;
	}


	uds_data_indication(recv_buf_sf, uds_dlc, N_OK);
	
	return 0;
}


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
int recv_firstframe(UDS_SEND_FRAME sendframefun, uint8_t* frame_buf, uint8_t frame_dlc)
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
		send_flowcontrol(sendframefun, FS_OVFLW);
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
	send_flowcontrol(sendframefun, FS_CTS);

	// 清接收流控帧计数标志
	g_rcf_bc = 0;

	// 接收到首帧后意味着接下来将要收到连续帧，这里置连续帧接收标志
	g_wait_cf = TRUE;

	// 开启 N_CR 定时器，接收方收到连续帧间隔时间不应大于 TIMEOUT_N_CR
	nt_timer_start(TIMER_N_CR);

	// 清连续帧帧序号 
	g_rcf_sn = 0;

	// TP 层首帧数据处理完成，通知上层
	//N_USData.ffindication(N_OK);
	uds_data_indication(frame_buf, frame_dlc, N_FF_MSG);
	//uds_data_indication(frame_buf, frame_dlc, N_OK);
	return 0;
}

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
static int recv_consecutiveframe(UDS_SEND_FRAME sendframefun, uint8_t* frame_buf, uint8_t frame_dlc)
{
	uint8_t cf_sn;      // 连续帧的帧序号
	uint16_t i;

	// 连续帧报文第一个字节的低 4 位表示帧序号
	cf_sn = NT_GET_CF_SN(frame_buf[0]);

	// 检查并停止 N_CR 定时器，接收方收到连续帧间隔时间不能大于 TIMEOUT_N_CR，如果超时，将不再处理该连续帧
	if (nt_timer_chk(TIMER_N_CR) <= 0)
	{
		//printf("return -1;");
		return -1;
	}

	// 帧序号 +1，超过 0xF 后从 0 开始重新计数
	g_rcf_sn++;
	if (g_rcf_sn > 0x0f)
		g_rcf_sn = 0;

	// 收到的连续帧帧序号应该每次 +1，如果不是，则抛弃这一帧，并通知上层做异常处理
	if (g_rcf_sn != cf_sn)
	{
		//N_USData.indication(recv_buf, recv_len, N_WRONG_SN);
		uds_data_indication(frame_buf, frame_dlc, N_WRONG_SN);
		return -2;
	}

	// 拷贝报文帧中的有效数据到 recv_buf 数组中
	for (i = 0; i < (FRAME_SIZE - 1); i++)
	{
		if (recv_len + i < UDS_RX_MAX)     // 防止数组越界
			recv_buf[recv_len + i] = frame_buf[1 + i];
	}

	// recv_len：实际接收到的有效数据总长度，用于接收多帧报文时累加计数
	recv_len += (FRAME_SIZE - 1);

	// 当实际接收到的有效数据总长度大于或等于首帧中获得的有效数据总长度
	// 说明连续帧接收完成，这是最后一个连续帧了，清连续帧接收标志，并将数据交由上层继续处理
	if (recv_len >= recv_fdl)
	{
		g_wait_cf = FALSE;              // 清连续帧接收标志
		uds_data_indication(recv_buf, recv_fdl, N_OK);
		return 0;
	}
	else
	{
		// 程序运行到这里说明后续还有连续帧需要接收

		// 当 NT_XMIT_FC_BS 不为 0 时，每收到 NT_XMIT_FC_BS 个连续帧后需返回一帧流控帧 
		if (NT_XMIT_FC_BS > 0)
		{
			// 接收流控帧计数 +1
			g_rcf_bc++;

			if (g_rcf_bc >= NT_XMIT_FC_BS)
			{
				// 返回一帧流控帧
				send_flowcontrol(sendframefun, FS_CTS);

				// 接收流控帧计数清 0
				g_rcf_bc = 0;
			}
		}

		// 连续帧接收标志置 1
		g_wait_cf = TRUE;

		// 启动 N_CR 定时器，接收方收到连续帧间隔时间不能大于 TIMEOUT_N_CR
		nt_timer_start(TIMER_N_CR);
		return 1;
	}
}

/******************************************************************************
* 函数名称: static int recv_flowcontrolframe(uint8_t* frame_buf, uint8_t frame_dlc)
* 功能说明: 处理流控帧
* 输入参数: uint8_t*    frame_buf       --接收报文帧数据首地址
	　　　　uint8_t     frame_dlc       --接收报文帧数据长度
* 输出参数: 无
* 函数返回: 0: 所有连续帧接收完成; 1: 继续接收; other: -ERR
* 其它说明: 无
******************************************************************************/
static int recv_flowcontrolframe(uint8_t* frame_buf, uint8_t frame_dlc)
{
	uint8_t fc_fs;

	// 获取流状态
	fc_fs = NT_GET_FC_FS(frame_buf[0]);

	// 检查并停止 N_BS 定时器，发送方发送完成首帧后到接收到流控帧之间的时间不应大于 TIMEOUT_N_BS
	// 如果超时，将不再处理该流控帧
	if (nt_timer_chk(TIMER_N_BS) <= 0) return -1;

	// 功能寻址是不应该发送流控帧的，如果收到则应该丢弃
	if (g_tatype == N_TATYPE_FUNCTIONAL) return -1;

	// 已经等到了流控帧，所以流控帧等待标志就可以清 0 了
	g_wait_fc = FALSE;

	// 流状态非法，通知上层并退出
	if (fc_fs >= FS_RESERVED)
	{
		//N_USData.confirm(N_INVALID_FS);
		uds_data_indication(frame_buf, frame_dlc, N_INVALID_FS);
		return -2;
	}

	// 流状态为溢出状态，通知上层并退出
	if (fc_fs == FS_OVFLW)
	{
		//N_USData.confirm(N_BUFFER_OVFLW);
		uds_data_indication(frame_buf, frame_dlc, N_BUFFER_OVFLW);
		return -3;
	}

	// 流状态为等待状态，流控帧等待标志重新置 1，重新开启 N_BS 定时器，退出
	if (fc_fs == FS_WT)
	{
		g_wait_fc = TRUE;
		nt_timer_start(TIMER_N_BS);
		return 1;
	}

	// 接收到流控帧中的 bolck size 块大小
	// 如果该值不为 0，则每连续收到 g_rfc_bs 个连续帧后，需返回一帧流控帧
	// 如果该值为 0，则不需要再回复流控帧了，发送方可以一直无限制发送连续帧，直到发送完成所有连续帧
	g_rfc_bs = frame_buf[1];

	// 接收到流控帧中的 STmin 帧间隔时间，在向外发送连续帧时，发送间隔不能小于该值
	// 0~7F 表示: 0~127ms, F1~F9 表示: 100~900us（这里统一按 1ms 处理），其它值均为无效值，统一处理成 127ms
	// 因为定时器计数值为 1 的时候表示超时，所以这里设置的计定时计数值都是 +1 的
	if (frame_buf[2] <= 0x7F)
		g_rfc_stmin = frame_buf[2] + 1;
		//g_rfc_stmin = frame_buf[2] * 1000 + 1 * 1000;
	else if (frame_buf[2] >= 0xF0 && frame_buf[2] <= 0xF9)
	{
		//task_cycle = 100;//修改周期
		//g_rfc_stmin = (frame_buf[2] - 0xF0) * 100 + 1 * 100;
		g_rfc_stmin = 1;
	}
		
	else
		//g_rfc_stmin = 0x7F * 1000 + 1 * 1000;
		g_rfc_stmin = 0x7F + 1 ;

	// 清连续帧发送计数
	g_xcf_bc = 0;

	// 重新设置 STmin 定时器计数值为 1，表示定时器超时已发生，接下来将处理超时事件
	nt_timer_start_wv(TIMER_STmin, 1);

	return 0;
}


/******************************************************************************
* 函数名称: void send_flowcontrol(UDS_SEND_FRAME sendframefun, network_flow_status_t flow_st)
* 功能说明: 发送流控帧
* 输入参数: UDS_SEND_FRAME sendframefun --发送报文函数指针，硬件驱动接口函数
	　　　　network_flow_status_t   flow_st       --流状态
* 输出参数: 无
* 函数返回: 0: 所有连续帧接收完成; 1: 继续接收; other: -ERR
* 其它说明: 无
******************************************************************************/
void send_flowcontrol(UDS_SEND_FRAME sendframefun, network_flow_status_t flow_st)
{
	uint16_t i;
	uint8_t send_buf[FRAME_SIZE] = { 0 };
	unsigned int request_id = 0;
	request_id = REQUEST_ID;
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
	sendframefun(request_id, send_buf, FRAME_SIZE);
}

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
int send_singleframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint8_t msg_dlc)
{
	uint16_t i;
	uint8_t send_buf[FRAME_SIZE] = { 0 };
	unsigned int request_id = 0;
	request_id = REQUEST_ID;
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
	if (sendframefun(request_id, send_buf, FRAME_SIZE) == 1)
	{
		nt_timer_start(TIMER_Response);    // 启动  定时器
		
		uds_data_indication(msg_buf, msg_dlc, N_TX_OK);
	}

	return 0;

}


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
int send_firstframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc)
{
	uint16_t i;
	uint8_t send_buf[FRAME_SIZE] = { 0 };
	unsigned int request_id = 0;
	request_id = REQUEST_ID;
	// 检查参数合法性
	if (msg_dlc < FRAME_SIZE || msg_dlc > UDS_TX_MAX) return 0;

	// 首帧的第一个字节高 4 位表示帧类型；低 4 位和第二个字节共组成 12 位，表示帧有效数据长度 
	send_buf[0] = NT_SET_PCI_TYPE_FF((uint8_t)(msg_dlc >> 8));
	send_buf[1] = (uint8_t)(msg_dlc & 0x00ff);


	// 拷贝有效数据到 send_buf 中
	for (i = 0; i < FRAME_SIZE - 2; i++)
		send_buf[2 + i] = msg_buf[i];
	
	// 发送
	sendframefun(request_id, send_buf, FRAME_SIZE);
	//uds_send_can_farme(0x724, send_buf, FRAME_SIZE);
	//uint8_t data[8] = { 0x10,0xd,0x2e,0xf1,0x8c,0x00,0x11,0x22 };
	//XLTransmitMsg(0x726, 0, data, 8, 1);
	

	// 发送完首帧后，流控帧等待标志需置 1，接下来等待接收流控帧
	g_wait_fc = TRUE;

	// 启动 N_BS 定时器，发送方发送完成首帧后到接收到流控帧之间的时间不能大于 TIMEOUT_N_BS，单位: ms
	nt_timer_start(TIMER_N_BS);

	// 返回已发送的有效数据长度
	return FRAME_SIZE - 2;
}

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
static int send_consecutiveframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc, uint8_t frame_sn)
{
	uint16_t i;
	uint8_t send_buf[FRAME_SIZE] = { 0 };
	unsigned int request_id = 0;
	request_id = REQUEST_ID;
	// 连续帧的第一个字节的高 4 位表示帧类型，低 4 位表示帧序号，每发送一帧连续帧时其应该 +1，超过 0xf 复位为 0
	send_buf[0] = NT_SET_PCI_TYPE_CF(frame_sn);

	// 拷贝有效数据到 send_buf 中
	for (i = 0; i < msg_dlc && i < (FRAME_SIZE - 1); i++)
		send_buf[1 + i] = msg_buf[i];

	// 连续帧的最后一帧可能是不满的，需填充默认值 PADDING_VAL
	for (; i < (FRAME_SIZE - 1); i++)
		send_buf[1 + i] = PADDING_VAL;
	// 发送
	sendframefun(request_id, send_buf, FRAME_SIZE);
	//uds_send_can_farme(0x724, send_buf, FRAME_SIZE);
	// 如果 msg_dlc > FRAME_SIZE - 1，说明这不是最后一个连续帧，实际发送的有效数据长度为 FRAME_SIZE - 1
	// 否则说明这是连续帧的最后一帧，返回实际发送最后剩余的有效数据长度
	if (msg_dlc > (FRAME_SIZE - 1))
		return (FRAME_SIZE - 1);
	else
		return msg_dlc;
}



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
int send_multipleframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc)
{
	uint16_t i;
	uint8_t send_len;

	// 检查参数合法性
	if (msg_dlc < FRAME_SIZE || msg_dlc > UDS_TX_MAX)
		return -1;

	// 将要发送的数据拷贝到 remain_buf 数组中
	for (i = 0; i < msg_dlc; i++)
		remain_buf[i] = msg_buf[i];
	



	//将多帧数据长度赋给全局变量
	remain_buf_len = msg_dlc;

	// 连续帧帧序号清 0，多帧发送时候使用，每发送一帧连续帧时其应该 +1，超过 0xf 复位为 0
	g_xcf_sn = 0;


	//if (remain_buf[2] == 0x36 && remain_buf[3] == 0x1 && nwf_st == FLASH_IDLE)
	//{
	//	nwf_st = FLASH_DOWNLOADING;

	//}


	// 多帧发送时先将首帧发送出去，接下来在 network_task 任务函数中将剩下的数据拆分成连续帧继续发送
	send_len = send_firstframe(sendframefun,msg_buf, msg_dlc);
	
	// 已经发送出去一帧了，所以剩余需要发送的有效数据在 remain_buf 数组中的起始位置需要向后移动 send_len
	remain_pos = send_len;

	// 剩余需要发送的有效数据长度 = 所有需要发送的有效数据总长度(msg_dlc) - 首帧中已发送出去的有效数据长度(send_len)
	remain_len = msg_dlc - send_len;

	// 当多帧发送时，在发送完成首帧后并没有设计出一个连续帧发送标志，以便在周期任务中继续发送连续帧
	// 那么，连续帧的发送是如何设计的呢？
	// 在收到流控帧 recv_flowcontrolframe 函数的最后调用 nt_timer_start_wv(TIMER_STmin, 1);
	// 重新设置 STmin 定时器计数值为 1，表示 STmin 定时器超时已发生
	// 在 network_task 任务函数中检查 STmin 定时器是否超时，如果超时，则继续发送连续帧，每发送一帧连续帧 STmin 定时器都重新开始计数
	// 直到剩余需要发送的有效数据长度 remain_len 为 0(所有数据发送完成)，调用 clear_network 函数复位网络层状态

	return 0;
}


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
int network_send_udsmsg(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc)
{
	// 检查参数合法性
	if (NULL == msg_buf || 0 == msg_dlc || msg_dlc > UDS_TX_MAX)
		return -1;

	// 根据数据长度判断应为单帧发送还是多帧发送
	if (msg_dlc < FRAME_SIZE)
	{
		send_singleframe(sendframefun,msg_buf, msg_dlc);
	}
	else
	{
		// 如果是多帧发送，则将当前网络层状态设置为发送状态
		nwl_st = NWL_XMIT;
		send_multipleframe(sendframefun,msg_buf, msg_dlc);
	}

	return 0;
}


/******************************************************************************
* 函数名称: void network_task(UDS_SEND_FRAME sendframefun)
* 功能说明: TP 层任务处理
* 输入参数: UDS_SEND_FRAME sendframefun --发送报文函数指针，硬件驱动接口函数
* 输出参数: 无
* 函数返回: 无
* 其它说明: 该函数需要被 1ms 周期调用
******************************************************************************/
void network_task(UDS_SEND_FRAME sendframefun)
{
	uint8_t send_len;

	// 如果 N_CR 定时器超时，复位网络层状态，并通知上层做异常处理
	if (nt_timer_run(TIMER_N_CR) < 0)
	{
		clear_network();
		//N_USData.indication(recv_buf, recv_len, N_TIMEOUT_Cr);
		uds_data_indication(NULL, NULL, N_TIMEOUT_Cr);
	}

	// 如果 N_BS 定时器超时，复位网络层状态，并通知上层做异常处理
	if (nt_timer_run(TIMER_N_BS) < 0)
	{
		clear_network();
		//N_USData.confirm(N_TIMEOUT_Bs);
		uds_data_indication(NULL, NULL, N_TIMEOUT_Bs);
	}

	// 如果 N_BS 定时器超时，复位网络层状态，并通知上层做异常处理
	if (nt_timer_run(TIMER_Response) < 0)
	{
		clear_network();
		//N_USData.confirm(N_TIMEOUT_Bs);
		uds_data_indication(NULL, NULL, N_TIMEROUT_Respone);
	}



	// 如果 STmin 定时器超时，表示可以继续发送连续帧
	if (nt_timer_run(TIMER_STmin) < 0)
	{
		// 连续帧帧序号 +1，超过 0xf 复位为 0
		g_xcf_sn++;
		if (g_xcf_sn > 0x0f)
			g_xcf_sn = 0;

		// 发送一帧连续帧
		send_len = send_consecutiveframe(sendframefun, &remain_buf[remain_pos], remain_len, g_xcf_sn);
		// 剩余需要发送的有效数据在 remain_buf 数组中的起始位置 remain_pos 移动 send_len
		remain_pos += send_len;

		// 剩余需要发送的有效数据长度需减去 send_len
		remain_len -= send_len;

		// 判断剩余需要发送的有效数据长度是否大于 0，若大于 0，表示还有连续帧需要继续发送，否则，表示连续帧已全部发送完毕，复位网络层状态
		if (remain_len > 0)
		{
			// g_rfc_bs: 接收到流控帧中的 bolck size 块大小
			// 如果该值不为 0，则每连续收到 g_rfc_bs 个连续帧后，需返回一帧流控帧
			// 如果该值为 0，则不需要再回复流控帧了，发送方可以一直无限制发送连续帧，直到发送完成所有连续帧
			if (g_rfc_bs > 0)
			{
				// 连续帧发送计数 +1
				g_xcf_bc++;
				if (g_xcf_bc < g_rfc_bs)
				{
					nt_timer_start(TIMER_STmin);// 启动 STmin 定时器
				}
				else
				{
					// 连续发送 g_rfc_bs 个连续帧后，需要再次等待流控帧 
					g_wait_fc = TRUE;
					nt_timer_start(TIMER_N_BS); // 启动 N_BS 定时器
				}
			}
			else
			{
				nt_timer_start(TIMER_STmin);    // 启动 STmin 定时器
			}
		}
		else
		{
			uds_data_indication(remain_buf, remain_buf_len, N_TX_OK);
			clear_network();                    // 复位网络层状态
			nt_timer_start(TIMER_Response);    // 启动  定时器
		}
	}
	
}






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
void uds_tp_recv_frame(UDS_SEND_FRAME sendframefun, uint8_t* frame_buf, uint8_t frame_dlc)
{
	uint8_t pci_type;
	int ret = -1;
	
	// 检查参数合法性
	if (NULL == frame_buf || FRAME_SIZE != frame_dlc)
		return;

	

	// 每帧报文第一个字节的高 4 位表示帧类型，共 4 种：单帧(SF)、首帧(SF)、连续帧(CF)、流控帧(FC)
	pci_type = NT_GET_PCI_TYPE(frame_buf[0]);

	


	// 接下来根据帧类型分别处理
	switch (pci_type)
	{
	case PCI_SF:                            // 单帧
		// 处理单帧，这里不做限制，这意味着即便是在多帧收发的过程中，我们也可以处理单帧
		// 为了不影响多帧数据的接收，我们为单帧数据单独设计一个独立的缓冲区 recv_buf_sf
		if (NWL_IDLE == nwl_st)
		{
			nt_timer_t num;
			for (num = 0; num < TIMER_CNT; num++)
				nt_timer_stop(num);
			recv_singleframe(sendframefun, frame_buf, frame_dlc);
		}
			
		
		break;
	case PCI_FF:                            // 首帧
		// 当网络层状态为接收或者空闲状态时
		if (NWL_RECV == nwl_st || NWL_IDLE == nwl_st)
		{
			// 复位网络层状态，主要是复位各种状态变量，并且关闭所有相关定时器
			clear_network();

			// 如果在接收连续帧时插入一条首帧，这是不正常的，通知上层做异常处理
			// 接下来在 recv_firstframe 函数中将收到的连续帧数据破坏，重新处理首帧数据
			if (NWL_RECV == nwl_st)
				//N_USData.indication(recv_buf, recv_len, N_UNEXP_PDU);
				uds_data_indication(frame_buf, frame_dlc, N_UNEXP_PDU);

			// 处理首帧
			ret = recv_firstframe(sendframefun,frame_buf, frame_dlc);

			// 当首帧处理完成时，置网络层状态为接收状态，若帧格式错误，则置网络层状态为空闲状态
			if (0 == ret)
				nwl_st = NWL_RECV;
			else
				nwl_st = NWL_IDLE;
		}
		break;
	case PCI_CF:                            // 连续帧
		// 当网络层状态为接收状态并且处在连续帧接收状态时
		if (NWL_RECV == nwl_st && TRUE == g_wait_cf)
		{
			// 处理连续帧
			ret = recv_consecutiveframe(sendframefun,frame_buf, frame_dlc);

			// 当连续帧处理完成或者帧格式错误时
			if (ret <= 0)
			{
				// 复位网络层状态，主要是复位各种状态变量，并且关闭所有相关定时器
				clear_network();
				nwl_st = NWL_IDLE;          // 置网络层状态为空闲状态
			}
		}
		break;
	case PCI_FC:                            // 流控帧
		// 当网络层状态为发送状态并且处在等待流控帧状态时
		if (/*NWL_XMIT == nwl_st && */TRUE == g_wait_fc)
		{
			// 处理流控帧
			ret = recv_flowcontrolframe(frame_buf, frame_dlc);

			// 当流控帧处理完成或者帧格式错误时
			if (ret < 0)
			{
				// 复位网络层状态，主要是复位各种状态变量，并且关闭所有相关定时器
				clear_network();
				nwl_st = NWL_IDLE;          // 置网络层状态为空闲状态
			}
		}
		break;
	default:
		// 非法无效帧
		break;
	}
}








/******************************************************************************
* 函数名称: void uds_1ms_task(void)
* 功能说明: UDS 周期任务
* 输入参数: 无
* 输出参数: 无
* 函数返回: 无
* 其它说明: 该函数需要被 1ms 周期调用
******************************************************************************/
void uds_1ms_task(UDS_SEND_FRAME sendframefun)
{
	network_task(sendframefun);
}





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
typedef int(*DLL_FUNCTION_GenerateKeyEx) (const unsigned char*, unsigned int, const unsigned int, const char*, unsigned char*, unsigned int, unsigned int*); //typedef定义一下函数指针，你不懂的话就记住末尾两个是你需要函数的形参。
typedef int(*DLL_FUNCTION_GenerateKeyExOpt) (const unsigned char*, unsigned int, const unsigned int, const char*, const char*, unsigned char*, unsigned int, unsigned int*); //typedef定义一下函数指针，你不懂的话就记住末尾两个是你需要函数的形参。
int SecurityAccessWithDLL(char *iFilename, const unsigned char iSeed[], unsigned int iSeedSize, const unsigned int iSecurityLevel, unsigned char *oKeyArray, unsigned int iKeyArrayMaxSize,unsigned int *oSize)
{
	//unsigned int keylen=0;

	HINSTANCE handle = LoadLibrary(iFilename);//LoadLibrary填入ddl文件名赋值给句柄
	if (handle == NULL)
	{
		return -2;
	}
	DLL_FUNCTION_GenerateKeyEx dll_GenerateKeyEx = (DLL_FUNCTION_GenerateKeyEx)GetProcAddress(handle, "GenerateKeyEx"); //使用GetProcAddress得到函数，参数是句柄名和函数名
	if (dll_GenerateKeyEx) //还是判断一下函数指针是否有效
	{

		//const unsigned char ipSeedArray[4];
		//unsigned int          iSeedArraySize = 4; /* Length of the array for the seed [in] */
		//const unsigned int    iSecurityLevel = 1;  /* Security level [in] */
		const char iVariant = 1;   /* Name of the active variant [in] */
	//const char ipOptions = 1;
		//unsigned char ioKeyArray[4];     /* Array for the key [in, out] */
		//unsigned int          iKeyArraySize = 4;  /* Maximum length of the array for the key [in] */
		//unsigned int oSize = 0;

		//SecurityInfo[ECU_Choose].SeedLen = 4;
		//SecurityInfo[ECU_Choose].Seed[0] = 1;
		//SecurityInfo[ECU_Choose].Seed[0] = 2;
		//SecurityInfo[ECU_Choose].Seed[0] = 3;
		//SecurityInfo[ECU_Choose].Seed[0] = 4;
		dll_GenerateKeyEx(iSeed, iSeedSize, iSecurityLevel, &iVariant, oKeyArray, iKeyArrayMaxSize, &oSize);
		
		FreeLibrary(handle); //卸载句柄，，
		return 0;
		
	}
	else
	{
		//HINSTANCE handle1 = LoadLibrary(_T("SeednKeyEx.dll"));//LoadLibrary填入ddl文件名赋值给句柄
		//typedef int(*DLL_FUNCTION_GenerateKeyExOpt) (const unsigned char*, unsigned int, const unsigned int, const char*, const char*, unsigned char*, unsigned int, unsigned int*); //typedef定义一下函数指针，你不懂的话就记住末尾两个是你需要函数的形参。

		DLL_FUNCTION_GenerateKeyExOpt dll_GenerateKeyExOpt = (DLL_FUNCTION_GenerateKeyExOpt)GetProcAddress(handle, "GenerateKeyExOpt"); //使用GetProcAddress得到函数，参数是句柄名和函数名

		if (dll_GenerateKeyExOpt) //还是判断一下函数指针是否有效
		{
			//const unsigned char  ipSeedArray[4] = { 0x1,0x2,0x3,0x4 };
			//unsigned int          iSeedArraySize = 4; /* Length of the array for the seed [in] */
			//const unsigned int    iSecurityLevel = 1;  /* Security level [in] */
			const char           iVariant = 1;   /* Name of the active variant [in] */
			const char ipOptions = 1;
			//unsigned char        ioKeyArray[4];     /* Array for the key [in, out] */
			//unsigned int          iKeyArraySize = 4;  /* Maximum length of the array for the key [in] */
			//unsigned int         oSize = 0;

			//printf("SeednKeyExOpt\n");
			dll_GenerateKeyExOpt(iSeed, iSeedSize, iSecurityLevel, &iVariant, &ipOptions, oKeyArray, iKeyArrayMaxSize, &oSize);
			FreeLibrary(handle); //卸载句柄，，
			return 1;
		}
		else
		{
			return -1;
		}
	}
}