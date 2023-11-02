#include"uds_service_function.h"
#include"uds_tp.h"
#include"XL_Driver.h"
#include"readhex.h"
#include <stdarg.h>
#include"paneldesign.h"
//34 req
unsigned char dataFormatIdentifier = 0;//这是第二个字节的参数，其中高4个bit表示压缩方法，低4个bit表示加密方法，一般情况就是0x00
unsigned char addressAndLengthFormatIdentifier = 0;//请求刷写地址和长度格式，高4个bit表示下面的memorySize参数占几个字节，低4个bit表示下面的memoryAddress参数占几个字节。常规就是0x44，就是memorySize和memoryAddress各占4个字节
unsigned char memoryAddress = 0;//请求刷写的首地址，这个参数占几个字节由addressAndLengthFormatIdentifier参数的低4个bit决定的
unsigned char memorySize = 0;//请求刷写的字节长度，这个参数占几个字节由addressAndLengthFormatIdentifier参数的高4个bit决定的
//34 postive respones
unsigned char lengthFormatIdentifier = 0;//高4个bit表示下面的maxNumberOfBlockLength参数占几个字节，低4个bit默认0
unsigned short maxNumberOfBlockLength = 0;//目标ECU允许Tester传输最大的字节数,实际上，36服务传输可以小于maxNumberOfBlockLength，但不能大于

//36
unsigned char blockSequenceCounter = 0; //数据传输计数器，第一帧从1开始，到了0xFF后，再从0开始，循环往复，直到下载完毕
char Flash_path = {0};
IHexRecord record_t;
ConvertContext ctx_t;
uint32_t crc = 0;


uint32_t Crc_Table32[256] = {
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
	0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
	0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
	0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
	0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
	0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
	0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
	0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
	0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
	0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
	0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
	0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
	0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
	0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
	0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
	0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
	0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
	0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
	0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
	0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
	0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
	0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
	0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
	0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
	0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
	0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
	0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
	0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
	0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
	0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
	0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
	0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
	0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
	0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
	0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
	0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
	0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
	0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
	0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
	0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
	0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
	0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
	0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
	0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
	0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
	0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
	0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
	0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
	0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
	0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
	0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
	0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
	0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
	0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
	0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
	0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};
const uint32_t CRC_INITIAL_VALUE32 = 0xFFFFFFFF;
const uint32_t CRC_FINAL_XOR_CRC32 = 0xFFFFFFFF;
uint32_t Crc_CalCRC32(uint8_t Crc_DataPtr[], uint32_t Crc_Length, uint32_t Crc_StartValue32, uint32_t Crc_IsFirstCall)
{
	uint32_t Crc_Value;
	uint32_t Crc_LoopCounter;

	if (Crc_IsFirstCall == 1)
	{
		Crc_Value = CRC_INITIAL_VALUE32;
	}
	else
	{
		Crc_Value = (CRC_FINAL_XOR_CRC32 ^ Crc_StartValue32);
	}

	for (Crc_LoopCounter = 0; Crc_LoopCounter < Crc_Length; Crc_LoopCounter++)
	{
		Crc_Value ^= 0xFF & (uint32_t)Crc_DataPtr[Crc_LoopCounter];
		Crc_Value = (Crc_Value >> 8) ^ (Crc_Table32[(uint8_t)(Crc_Value)]);
	}
	return Crc_Value ^ CRC_FINAL_XOR_CRC32;
}



void loadflashfile(const char* rpath, const char* wpath)
{
	//uint32_t crc=0;
	ReadHex(rpath, wpath, &record_t, &ctx_t);
	crc = Crc_CalCRC32(ctx_t.rambuff, record_t.maxAddr - record_t.minAddr + 1, 0xFFFFFFFF, 1);
}





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


unsigned char uds_respone(const unsigned char *data)
{
	
	if (data[0] == 0x7f)
	{
		int temp = 0xff;

		for (int i = 0; i < SID_NUM; i++)
		{
			if (data[1] == uds_service_list[i].uds_sid)
			{
				temp = i;
				break;
			}
		}
		if (temp != 0xff)
		{
			if (data[2] == 0x78)//忙碌
			{
				//settexttocontrol(Edit_out, "NCR:", 1);
				//setHEXtocontrol(Edit_out, data[2], 0);
				uds_service_list[temp].NCR = data[2];
				uds_service_list[temp].TIMER_SID = 8000 * 1000;
				return data[2];
			}
			else
			{
				//settexttocontrol(Edit_out, "NCR:", 1);
				//setHEXtocontrol(Edit_out, data[2], 0);
				uds_service_list[temp].NCR = data[2];
				uds_service_list[temp].TIMER_SID = 0;
				return data[2];
			}
		}
		
	}
	else
	{
		
		int temp = 0xff;

		for (int i = 0; i < SID_NUM; i++)
		{
			
			if ((data[0]-0x40) == uds_service_list[i].uds_sid)
			{
				
				temp = i;
				break;
			}
		}
		if (temp != 0xff)
		{
			uds_service_list[temp].NCR = 0;
			uds_service_list[temp].TIMER_SID = 0;
			//printf("temp=%d", temp);
			return 0;
		}
	}
	
	
	//printf("sid_10 NCR=%d,TIMER=%d\n", uds_service_list[0].NCR, uds_service_list[0].TIMER_SID);

}


void sid_timer_start_flag(unsigned char siu_num)
{
	int temp = 0xff;

	for (int i = 0; i < SID_NUM; i++)
	{
		if (siu_num = uds_service_list[i].uds_sid)
		{
			temp = i;
			break;
		}
	}
	if (temp < SID_NUM)
	{

		uds_service_list[temp].timerflag = 1;
	}





}





void service_10_SessionControl(unsigned char session)
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
		sid_timer_start_flag(SID_10);
	}
}

void service_11_EcuReset(unsigned char Subfunctions)
{
	if (sid_timer_chk(SID_11) == task_cycle)
	{
		printf("上一指令未回复\n");
	}

	unsigned char data[16] = { 0x11,Subfunctions };

	network_send_udsmsg(uds_send_can_farme, data, 2);
	if (Subfunctions & 0x80)
	{

	}
	else
	{
		sid_timer_start_flag(SID_11);
	}
}

void service_27_SecurityAccess_request(unsigned char level)
{
	if (sid_timer_chk(SID_27) == task_cycle)
	{
		printf("上一指令未回复\n");
	}

	unsigned char data[16] = { 0x27,level };

	network_send_udsmsg(uds_send_can_farme, data, 2);
	sid_timer_start_flag(SID_27);
}

void service_28_CommunicationControl(unsigned char Subfunctions, unsigned char CommunicationContorlType)
{
	if (sid_timer_chk(SID_28) == task_cycle)
	{
		printf("上一指令未回复\n");
	}

	unsigned char data[16] = { 0x28,Subfunctions ,CommunicationContorlType};

	network_send_udsmsg(uds_send_can_farme, data, 3);
	sid_timer_start_flag(SID_28);
}

void service_3E_TesterPresent(unsigned char Subfunctions)
{
	if (sid_timer_chk(SID_3E) == task_cycle)
	{
		printf("上一指令未回复\n");
	}

	unsigned char data[16] = { 0x3E,Subfunctions };

	network_send_udsmsg(uds_send_can_farme, data, 2);
	if (Subfunctions & 0x80)
	{

	}
	else
	{
		sid_timer_start_flag(SID_3E);
	}
}

void service_85_ControlDTCSetting(unsigned char Subfunctions)
{
	if (sid_timer_chk(SID_85) == task_cycle)
	{
		printf("上一指令未回复\n");
	}

	unsigned char data[16] = { 0x85,Subfunctions };

	network_send_udsmsg(uds_send_can_farme, data, 2);
	sid_timer_start_flag(SID_85);
}

void service_22_ReadDataByIdentifier(unsigned short DID_NUM)
{
	if (sid_timer_chk(SID_22) == task_cycle)
	{
		printf("上一指令未回复\n");
	}

	unsigned char data[16] = { 0x22,(DID_NUM >> 8) & 0xff,DID_NUM & 0xff };

	network_send_udsmsg(uds_send_can_farme, data, 3);
	sid_timer_start_flag(SID_22);
}

void service_2E_WriteDataByIdentifier(unsigned short DID_NUM, unsigned char *exdata, unsigned char datalen)
{
	if (sid_timer_chk(SID_2E) == task_cycle)
	{
		printf("上一指令未回复\n");
	}

	unsigned char data[64] = { 0x2E,(DID_NUM >> 8) & 0xff,DID_NUM & 0xff };

	for (int i = 0; i < datalen; i++)
	{
		data[3 + i] = exdata[i];
	}


	network_send_udsmsg(uds_send_can_farme, data, datalen+3);
	sid_timer_start_flag(SID_2E);
}

void service_31_RoutineControl(unsigned char Subfunctions,unsigned short RID_NUM)
{
	if (sid_timer_chk(SID_31) == task_cycle)
	{
		printf("上一指令未回复\n");
	}

	unsigned char data[16] = { 0x31,Subfunctions,(RID_NUM >> 8) & 0xff,RID_NUM & 0xff };

	network_send_udsmsg(uds_send_can_farme, data, 4);
	sid_timer_start_flag(SID_31);
}

void service_31_EXRoutineControl(unsigned char Subfunctions, unsigned short RID_NUM,unsigned char *exdata,unsigned char datalen)
{
	if (sid_timer_chk(SID_31) == task_cycle)
	{
		printf("上一指令未回复\n");
	}

	unsigned char data[32] = { 0x31,Subfunctions,(RID_NUM >> 8) & 0xff,RID_NUM & 0xff };

	for (int i = 0; i < datalen; i++)
	{
		data[4 + i] = exdata[i];
	}


	network_send_udsmsg(uds_send_can_farme, data, 4+ datalen);
	sid_timer_start_flag(SID_31);
}

void service_34_RequestDownload(unsigned int StartAddr, unsigned int DataLen)
{
	unsigned char start[4];

	start[0] = (StartAddr >> 24) & 0xff;
	start[1] = (StartAddr >> 16) & 0xff;
	start[2] = (StartAddr >> 8) & 0xff;
	start[3] = (StartAddr) & 0xff;

	unsigned char len[4];
	len[0] = (DataLen >> 24) & 0xff;
	len[1] = (DataLen >> 16) & 0xff;
	len[2] = (DataLen >> 8) & 0xff;
	len[3] = (DataLen) & 0xff;


	unsigned char data[16] ={ 0x34,0x00,0x44, start[0] ,start[1] ,start[2] ,start[3] ,len[0] ,len[1] ,len[2] ,len[3] };

	network_send_udsmsg(uds_send_can_farme, data, 11);
	sid_timer_start_flag(SID_34);

}


void service_36_TransferData(unsigned int DataLen)
{
	uint32_t index = 0;
	uint32_t index_1 = 0;
	unsigned char data[0x803];
	maxNumberOfBlockLength = 0x402;
	blockSequenceCounter = 1;
	display = 0;
	while (1)
	{
		nwf_st = FLASH_36service_runing;
		if ((DataLen - index) > maxNumberOfBlockLength - 2)
		{
			data[0] = 0x36;
			data[1] = blockSequenceCounter;
			for (int n = 0; n < maxNumberOfBlockLength - 2; n++)
			{
				data[2 + n] = ctx_t.rambuff[index];
				index++;
			}
			index_1 = index;
			network_send_udsmsg(uds_send_can_farme, data, maxNumberOfBlockLength);
			blockSequenceCounter++;
			while (1)
			{
				Sleep(10);
				if (nwf_st == FLASH_36service_finsh)
				{
					break;
				}
			}
			//break;
		}
		else
		{
			nwf_st = FLASH_36service_runing;
			data[0] = 0x36;
			data[1] = blockSequenceCounter;
			for (int n = 0; n < DataLen - index_1; n++)
			{
				//printf("index=%d\n", index);
				data[2 + n] = ctx_t.rambuff[index];
				index++;
			}
			index_1 = index;
			network_send_udsmsg(uds_send_can_farme, data, maxNumberOfBlockLength);
			nwf_st = FLASH_36service_runing;
			while (1)
			{
				Sleep(10);
				if (nwf_st == FLASH_36service_finsh)
				{
					display = 1;
					break;
				}
			}
			break;
		}

	}

	if (ctx_t.binbuff) free(ctx_t.binbuff);
	if (ctx_t.strbuff) free(ctx_t.strbuff);
	if (ctx_t.rambuff) free(ctx_t.rambuff);

}







void service_37_RequestTransferExit()
{
	unsigned char data[8] = { 0x37 };
	network_send_udsmsg(uds_send_can_farme, data, 1);
	sid_timer_start_flag(SID_37);
}









typedef struct _FILEINFO_
{
	char *driver_path;
	char *app_path;
}file_info_t;

network_flash_st nwf_st = FLASH_IDLE;






int sid_resp_chk(unsigned char siu_num)
{
	int temp = 0xff;

	for (int i = 0; i < SID_NUM; i++)
	{
		if (siu_num = uds_service_list[i].uds_sid)
		{
			temp = i;
			break;
		}
	}

	// 如果定时器计数值 > 0,表示定时器正在工作
	if (uds_service_list[temp].TIMER_SID > 0)
	{
		//uds_service_list[temp].TIMER_SID = 0;          // 关闭定时器
		return 1;                   // 返回 1，定时器正在计时运行
	}
	else
	{
		uds_service_list[temp].TIMER_SID = 0;          // 定时器已停止运行
		Sleep(300);
		return 0;                   // 返回 0，定时器已停止运行
	}


}


int sid_wait_resp(unsigned char sid_num)
{
	while (1)
	{
		Sleep(300);
		if (sid_resp_chk(sid_num) <= 0)
		{
			Sleep(200);
			break;
		}
	}
}




file_info_t file_path = { 0 };
void flash_flow()
{
	printf("flash info :%s,%s\n", file_path.driver_path, file_path.driver_path);
	//printf("=============falsh_;%s\n", path.driver_path);
	loadflashfile(file_path.driver_path, "flash_driver_1.bin");
	service_10_SessionControl(0x81);
	Sleep(500);

	service_10_SessionControl(0x83);
	Sleep(500);

	//service_31_RoutineControl(1, 0x0203);
	sid_wait_resp(SID_31);

	service_10_SessionControl(0x83);
	Sleep(500);

	service_10_SessionControl(0x02);
	sid_wait_resp(SID_10);

	service_3E_TesterPresent(0x80);
	Sleep(3000);

	service_27_SecurityAccess_request(0x09);
	Sleep(500);

	unsigned char data_f184[16] = { 0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11 };
	service_2E_WriteDataByIdentifier(0xf184, data_f184, 9);
	Sleep(500);

	service_3E_TesterPresent(0x80);
	Sleep(3000);

	service_34_RequestDownload(record_t.minAddr, record_t.maxAddr - record_t.minAddr + 1);
	sid_wait_resp(SID_34);

	service_36_TransferData(record_t.maxAddr - record_t.minAddr + 1);
	Sleep(500);

	service_37_RequestTransferExit();
	sid_wait_resp(SID_37);

	unsigned char data_0202[16] = { (record_t.minAddr >> 24) & 0xff,(record_t.minAddr >> 16) & 0xff,(record_t.minAddr >> 8) & 0xff,(record_t.minAddr) & 0xff ,(record_t.maxAddr - record_t.minAddr + 1 >> 24) & 0xff,(record_t.maxAddr - record_t.minAddr + 1 >> 16) & 0xff,(record_t.maxAddr - record_t.minAddr + 1 >> 8) & 0xff,(record_t.maxAddr - record_t.minAddr + 1) & 0xff ,(crc >> 24) & 0xff,(crc >> 16) & 0xff,(crc >> 8) & 0xff,(crc) & 0xff };
	service_31_EXRoutineControl(0x01, 0x0202, data_0202, 12);
	sid_wait_resp(SID_31);


	loadflashfile(file_path.app_path, "flash_driver_1.bin");

	unsigned char data_ff00[16] = { (record_t.minAddr >> 24) & 0xff,(record_t.minAddr >> 16) & 0xff,(record_t.minAddr >> 8) & 0xff,(record_t.minAddr) & 0xff ,(record_t.maxAddr - record_t.minAddr + 1 >> 24) & 0xff,(record_t.maxAddr - record_t.minAddr + 1 >> 16) & 0xff,(record_t.maxAddr - record_t.minAddr + 1 >> 8) & 0xff,(record_t.maxAddr - record_t.minAddr + 1) & 0xff};
	service_31_EXRoutineControl(1, 0xff00, data_ff00, 8);
	sid_wait_resp(SID_31);

	service_34_RequestDownload(record_t.minAddr, record_t.maxAddr - record_t.minAddr + 1);
	sid_wait_resp(SID_34);

	service_36_TransferData(record_t.maxAddr - record_t.minAddr + 1);
	Sleep(500);

	service_37_RequestTransferExit();
	sid_wait_resp(SID_37);

	unsigned char data_0202_app[16] = { (record_t.minAddr >> 24) & 0xff,(record_t.minAddr >> 16) & 0xff,(record_t.minAddr >> 8) & 0xff,(record_t.minAddr) & 0xff ,(record_t.maxAddr - record_t.minAddr + 1 >> 24) & 0xff,(record_t.maxAddr - record_t.minAddr + 1 >> 16) & 0xff,(record_t.maxAddr - record_t.minAddr + 1 >> 8) & 0xff,(record_t.maxAddr - record_t.minAddr + 1) & 0xff ,(crc >> 24) & 0xff,(crc >> 16) & 0xff,(crc >> 8) & 0xff,(crc) & 0xff };
	service_31_EXRoutineControl(0x01, 0x0202, data_0202_app, 12);
	sid_wait_resp(SID_31);


	service_31_RoutineControl(1, 0xff01);
	sid_wait_resp(SID_31);

	Sleep(3000);
	service_11_EcuReset(1);
}


void flash(const char *driver_file, const char *app_file)
{

	int pd;
	
	file_path.driver_path = driver_file;
	file_path.app_path = app_file;

	printf("---------------------------------------flash info :%s,%s\n", file_path.driver_path, file_path.app_path);
	
	if (1)
	{

		pd = _beginthread(flash_flow, 0, NULL);

		return 1;

	}

	else

		return 0;



}


