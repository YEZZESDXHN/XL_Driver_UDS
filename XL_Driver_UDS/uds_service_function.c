#include"uds_service_function.h"
#include"uds_tp.h"
#include"XL_Driver.h"

/******************************************************************************
* ��������: void network_task(UDS_SEND_FRAME sendframefun)
* ����˵��: TP ��������
* �������: UDS_SEND_FRAME sendframefun --���ͱ��ĺ���ָ�룬Ӳ�������ӿں���
* �������: ��
* ��������: ��
* ����˵��: �ú�����Ҫ�� 1ms ���ڵ���
******************************************************************************/
void sid_task()
{
	for (int i = 0; i < SID_NUM; i++)
	{





		if (sid_timer_run(i) < 0)
		{
			
			printf("SID_%02X��ʱ\n", uds_service_list[i].uds_sid);
		}

	}
	
}

int service_10_SessionControl(unsigned char session)
{
	if (sid_timer_chk(SID_10)==task_cycle)
	{
		printf("��һָ��δ�ظ�\n");
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