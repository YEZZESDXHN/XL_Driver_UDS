#pragma once
#include"uds_tp.h"

extern unsigned short maxNumberOfBlockLength;
unsigned int blocknum;//ˢдʱ��Ҫ����Ŀ���
unsigned int blocknumcount;//ˢд�������
extern unsigned char blockSequenceCounter;

extern uint32_t flash_len;
extern uint32_t flash_len_temp;



typedef enum __NETWORK_FLASH_STATUS_
{
	FLASH_IDLE = 0,                   // ����״̬
	FLASH_REQUEST_Negitive,
	FLASH_REQUEST_Postive,
	FLASH_DOWNLOADING,                       // ���أ�ˢд�У�
	FLASH_36service_runing,
	FLASH_36service_finsh,
	FLASH_UPDATA                        // �ϴ�����

}network_flash_st;
extern network_flash_st nwf_st;