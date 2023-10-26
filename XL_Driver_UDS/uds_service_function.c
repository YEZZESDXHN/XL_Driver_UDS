#include"uds_service_function.h"
#include"uds_tp.h"
#include"XL_Driver.h"

/******************************************************************************
* 函数名称: void network_task(UDS_SEND_FRAME sendframefun)
* 功能说明: TP 层任务处理
* 输入参数: UDS_SEND_FRAME sendframefun --发送报文函数指针，硬件驱动接口函数
* 输出参数: 无
* 函数返回: 无
* 其它说明: 该函数需要被 1ms 周期调用
******************************************************************************/
void sid_task()
{
	for (int i = 0; i < SID_NUM; i++)
	{





		if (sid_timer_run(i) < 0)
		{
			
			printf("SID_%02X超时\n", uds_service_list[i].uds_sid);
		}

	}
	
}

int service_10_SessionControl(unsigned char session)
{
	if (sid_timer_chk(SID_10)==task_cycle)
	{
		printf("上一指令未回复\n");
	}

	unsigned char data[16] = {0x10,session };

	network_send_udsmsg(uds_send_can_farme, data, 2);
	if (session & 0x80)
	{

	}
	else
	{
		sid_timer_start(SID_10);
	}
}