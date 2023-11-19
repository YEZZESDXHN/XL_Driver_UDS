#pragma once
#include"uds_tp.h"
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
extern char Flash_path[256];

extern int display;

void flash(const char *driver_file, const char *app_file);
unsigned char uds_respone(unsigned char *data);
void service_31_RoutineControl(unsigned char Subfunctions, unsigned short RID_NUM);
void service_31_EXRoutineControl(unsigned char Subfunctions, unsigned short RID_NUM, unsigned char *exdata, unsigned char datalen);