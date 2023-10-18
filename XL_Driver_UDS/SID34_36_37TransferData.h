#pragma once
#include"uds_tp.h"

extern unsigned short maxNumberOfBlockLength;
unsigned int blocknum;//刷写时需要传输的块数
unsigned int blocknumcount;//刷写块计数数
extern unsigned char blockSequenceCounter;

extern uint32_t flash_len;
extern uint32_t flash_len_temp;



typedef enum __NETWORK_FLASH_STATUS_
{
	FLASH_IDLE = 0,                   // 空闲状态
	FLASH_REQUEST_Negitive,
	FLASH_REQUEST_Postive,
	FLASH_DOWNLOADING,                       // 下载（刷写中）
	FLASH_36service_runing,
	FLASH_36service_finsh,
	FLASH_UPDATA                        // 上传（）

}network_flash_st;
extern network_flash_st nwf_st;