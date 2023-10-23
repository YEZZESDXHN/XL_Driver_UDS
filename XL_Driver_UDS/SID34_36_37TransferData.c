#include"SID34_36_37TransferData.h"
#include"uds_tp.h"
#include"XL_Driver.h"
#include"readhex.h"
//34 req
unsigned char dataFormatIdentifier = 0;//���ǵڶ����ֽڵĲ��������и�4��bit��ʾѹ����������4��bit��ʾ���ܷ�����һ���������0x00
unsigned char addressAndLengthFormatIdentifier = 0;//����ˢд��ַ�ͳ��ȸ�ʽ����4��bit��ʾ�����memorySize����ռ�����ֽڣ���4��bit��ʾ�����memoryAddress����ռ�����ֽڡ��������0x44������memorySize��memoryAddress��ռ4���ֽ�
unsigned char memoryAddress = 0;//����ˢд���׵�ַ���������ռ�����ֽ���addressAndLengthFormatIdentifier�����ĵ�4��bit������
unsigned char memorySize = 0;//����ˢд���ֽڳ��ȣ��������ռ�����ֽ���addressAndLengthFormatIdentifier�����ĸ�4��bit������
//34 postive respones
unsigned char lengthFormatIdentifier = 0;//��4��bit��ʾ�����maxNumberOfBlockLength����ռ�����ֽڣ���4��bitĬ��0
unsigned short maxNumberOfBlockLength = 0;//Ŀ��ECU����Tester���������ֽ���,ʵ���ϣ�36���������С��maxNumberOfBlockLength�������ܴ���

//36
unsigned char blockSequenceCounter = 0; //���ݴ������������һ֡��1��ʼ������0xFF���ٴ�0��ʼ��ѭ��������ֱ���������
unsigned char downloaddata[1];

network_flash_st nwf_st=FLASH_IDLE;






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

IHexRecord record_t;
ConvertContext ctx_t;
uint32_t crc = 0;



unsigned int calcblocknum(unsigned short maxNumberOfBlockLength, unsigned int hexlen)
{
	unsigned int count = 0;
	if (hexlen % (maxNumberOfBlockLength - 2) == 0)
	{
		count = hexlen / (maxNumberOfBlockLength - 2);
	}
	else
	{
		count = hexlen / (maxNumberOfBlockLength - 2)+1;
	}
	return count;


	
}



void loadflashfile(const char* rpath, const char* wpath)
{
	//uint32_t crc=0;
	ReadHex(rpath, wpath, &record_t, &ctx_t);
	crc=Crc_CalCRC32(ctx_t.rambuff, record_t.maxAddr - record_t.minAddr + 1, 0xFFFFFFFF, 1);
	printf("crc=%X\n", crc);
}

void sid10(unsigned char session)
{
	unsigned char data[8] = {0x10,session };
	network_send_udsmsg(uds_send_can_farme, data, 2);
}
void sid3e(unsigned char subFunction)
{
	unsigned char data[8] = { 0x3e,subFunction };
	network_send_udsmsg(uds_send_can_farme, data, 2);
}

void sid27(unsigned char level)
{
	unsigned char data[8] = { 0x27,level };
	network_send_udsmsg(uds_send_can_farme, data, 2);
}

void sid2e()
{
	unsigned char data[12] = { 0x2e,0xf1,0x84,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11 };
	network_send_udsmsg(uds_send_can_farme, data, 12);
}

void sid34()
{
	//record_t.minAddr
	unsigned char start[4];
	
	start[0] = (record_t.minAddr >> 24) & 0xff;
	start[1] = (record_t.minAddr >> 16) & 0xff;
	start[2] = (record_t.minAddr >> 8 ) & 0xff;
	start[3] = (record_t.minAddr      ) & 0xff;
	printf("start:%02X %02X %02X %02X \n", start[0], start[1], start[2], start[3]);
	unsigned char len[4];
	uint32_t hexlen = 0;
	hexlen = record_t.maxAddr - record_t.minAddr + 1;
	
	len[0] = (hexlen >> 24) & 0xff;
	len[1] = (hexlen >> 16) & 0xff;
	len[2] = (hexlen >> 8) & 0xff;
	len[3] = (hexlen) & 0xff;
	printf("len:%02X %02X %02X %02X \n", len[0], len[1], len[2], len[3]);
	unsigned char data[12] = { 0x34,0x00,0x44, start[0] ,start[1] ,start[2] ,start[3] ,len[0] ,len[1] ,len[2] ,len[3] };
	network_send_udsmsg(uds_send_can_farme, data, 11);
}



void sid36_download()
{
	//IHexRecord record_t;
	//ConvertContext ctx_t;

	uint32_t hexlen = 0;
	maxNumberOfBlockLength = 0x402;
	blockSequenceCounter = 1;
	
	int count = 0;
	hexlen = record_t.maxAddr - record_t.minAddr + 1;
	uint32_t index = 0;

	unsigned char data[0x402];

	while (1)
	{
		if ((hexlen - index) > maxNumberOfBlockLength-2)
		{
			data[0] = 0x36;
			data[1] = blockSequenceCounter;
			for (int n = 0; n < maxNumberOfBlockLength - 2; n++)
			{
				data[2 + n] = ctx_t.rambuff[index];
				index++;
			}
			network_send_udsmsg(uds_send_can_farme, data, maxNumberOfBlockLength);
			blockSequenceCounter++;
		}
		else
		{
			data[0] = 0x36;
			data[1] = blockSequenceCounter;
			for (int n = 0; n < hexlen - index-1; n++)
			{
				data[2 + n] = ctx_t.rambuff[index];
				index++;
			}
			network_send_udsmsg(uds_send_can_farme, data, maxNumberOfBlockLength);
			return;
		}
		
	}

	//��ʼ
	unsigned char blockSequenceCounter = 1;












	if (ctx_t.binbuff) free(ctx_t.binbuff);
	if (ctx_t.strbuff) free(ctx_t.strbuff);
	if (ctx_t.rambuff) free(ctx_t.rambuff);

}
void sid37()
{
	unsigned char data[8] = { 0x37};
	network_send_udsmsg(uds_send_can_farme, data, 1);
}
void flash31010202(unsigned int start_,unsigned int datalen,unsigned int crc_)
{
	unsigned char start[4];
	start[0] = (start_ >> 24) & 0xff;
	start[1] = (start_ >> 16) & 0xff;
	start[2] = (start_ >> 8) & 0xff;
	start[3] = (start_) & 0xff;
	unsigned char len[4];
	len[0] = (datalen >> 24) & 0xff;
	len[1] = (datalen >> 16) & 0xff;
	len[2] = (datalen >> 8) & 0xff;
	len[3] = (datalen) & 0xff;
	unsigned char crc[4];
	crc[0] = (crc_ >> 24) & 0xff;
	crc[1] = (crc_ >> 16) & 0xff;
	crc[2] = (crc_ >> 8) & 0xff;
	crc[3] = (crc_) & 0xff;


	unsigned char data[16] = { 0x31,0x1,0x02,0x02, start[0] ,start[1] ,start[2] ,start[3] , len[0] ,len[1] ,len[2] ,len[3] , crc[0] ,crc[1] ,crc[2] ,crc[3]};
	network_send_udsmsg(uds_send_can_farme, data, 16);

}
void flash31010203()
{
	


	unsigned char data[16] = { 0x31,0x1,0x02,0x03};
	network_send_udsmsg(uds_send_can_farme, data, 4);

}
void flash3101ff00(unsigned int start_, unsigned int datalen)
{
	unsigned char start[4];
	start[0] = (start_ >> 24) & 0xff;
	start[1] = (start_ >> 16) & 0xff;
	start[2] = (start_ >> 8) & 0xff;
	start[3] = (start_) & 0xff;
	unsigned char len[4];
	len[0] = (datalen >> 24) & 0xff;
	len[1] = (datalen >> 16) & 0xff;
	len[2] = (datalen >> 8) & 0xff;
	len[3] = (datalen) & 0xff;


	unsigned char data[16] = { 0x31,0x1,0xff,0x00, start[0] ,start[1] ,start[2] ,start[3] , len[0] ,len[1] ,len[2] ,len[3]};
	network_send_udsmsg(uds_send_can_farme, data, 12);

}
void flash3101ff01()
{
	
	unsigned char data[16] = { 0x31,0x1,0xff,0x01};
	network_send_udsmsg(uds_send_can_farme, data, 4);

}
void sid11(unsigned char subFunction)
{
	unsigned char data[8] = { 0x11,subFunction };
	network_send_udsmsg(uds_send_can_farme, data, 2);
}


void timer_count_up(int t)

{
	//task_cycle = 100;
	loadflashfile("flash_driver.hex", "flash_driver_1.bin");
	//loadflashfile("VIU_37FF_R520_RS1_178_20231011_BANK_1.hex", "flash_driver_1.bin");
	printf("--------------------start:%x len:%x\n", record_t.minAddr, record_t.maxAddr - record_t.minAddr + 1);
	sid10(0x81);
	Sleep(500);
	printf("sid10(0x83);\n");
	sid10(0x83);
	Sleep(500);
	flash31010203;
	Sleep(500);
	sid10(0x83);
	Sleep(500);
	printf("sid10(0x02);\n");
	sid10(0x02);
	Sleep(3000);

	sid3e(0x80);
	Sleep(3000);
	printf("sid27(0x09);\n");
	sid27(0x09);
	Sleep(500);
	sid3e(0x80);
	Sleep(500);
	printf("sid2e();\n");
	sid2e();
	Sleep(500);
	printf("sid34();\n");
	sid34();
	Sleep(500);





	printf("sid36_download();\n");
	uint32_t hexlen = 0;
	maxNumberOfBlockLength = 0x402;
	blockSequenceCounter = 1;

	//int count = 0;
	hexlen = record_t.maxAddr - record_t.minAddr + 1;
	uint32_t index = 0;
	uint32_t index_1 = 0;
	printf("hexlen=%d\n", hexlen);
	unsigned char data[0x803];
	//data = (uint8_t*)malloc(1030);
	//blockSequenceCounter = 1;
	while (1)
	{
		nwf_st = FLASH_36service_runing;
		if ((hexlen - index) > maxNumberOfBlockLength - 2)
		{
			printf("36_1 service\n");
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
			printf("36_2 service\n");
			nwf_st = FLASH_36service_runing;
			data[0] = 0x36;
			data[1] = blockSequenceCounter;
			for (int n = 0; n < hexlen - index_1; n++)
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
					break;
				}
			}
			break;
		}
		
	}
	
	if (ctx_t.binbuff) free(ctx_t.binbuff);
	if (ctx_t.strbuff) free(ctx_t.strbuff);
	if (ctx_t.rambuff) free(ctx_t.rambuff);

	Sleep(500);
	sid37();
	Sleep(300);
	flash31010202(record_t.minAddr, record_t.maxAddr - record_t.minAddr + 1, crc);

	loadflashfile("VIU_37MR_R520_RD1_179_20231020_BANK_1.hex", "flash_driver_1.bin");
	//loadflashfile("VIU_37FF_R510_RC4_176_20230925_BANK_1.hex", "flash_driver_1.bin");
	Sleep(300);
	printf("--------------------start:%x len:%x\n", record_t.minAddr, record_t.maxAddr - record_t.minAddr + 1);
	flash3101ff00(record_t.minAddr, record_t.maxAddr - record_t.minAddr + 1);
	Sleep(1000);

	sid34();
	Sleep(500);


	printf("sid36_download();\n");
	hexlen = 0;
	maxNumberOfBlockLength = 0x802;
	blockSequenceCounter = 1;

	hexlen = record_t.maxAddr - record_t.minAddr + 1;
	index = 0;
	index_1 = 0;
	//printf("hexlen=%d\n", hexlen);
	//unsigned char data[0x803];
	//data = (uint8_t*)malloc(1030);
	//blockSequenceCounter = 1;
	while (1)
	{
		nwf_st = FLASH_36service_runing;
		if ((hexlen - index) > maxNumberOfBlockLength - 2)
		{
			printf("36_1 service\n");
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
				//break;
			}
		}
		else
		{
			printf("36_2 service\n");
			data[0] = 0x36;
			data[1] = blockSequenceCounter;
			for (int n = 0; n < hexlen - index_1; n++)
			{
				//printf("index=%d\n", index);
				data[2 + n] = ctx_t.rambuff[index];
				index++;
			}
			index_1 = index;
			network_send_udsmsg(uds_send_can_farme, data, maxNumberOfBlockLength);
			while (1)
			{
				Sleep(10);
				if (nwf_st == FLASH_36service_finsh)
				{
					break;
				}
			}
			break;
		}
		
	}

	if (ctx_t.binbuff) free(ctx_t.binbuff);
	if (ctx_t.strbuff) free(ctx_t.strbuff);
	if (ctx_t.rambuff) free(ctx_t.rambuff);

	Sleep(500);
	sid37();
	Sleep(300);
	flash31010202(record_t.minAddr, record_t.maxAddr - record_t.minAddr + 1, crc);
	Sleep(500);

	flash3101ff01();

	Sleep(3000);
	sid11(01);




	//task_cycle = 1000;
	//


}

int timer_start(int mtime)
{

	int pd;

	if (mtime > 0)

	{

		pd = _beginthread(timer_count_up, 0, mtime);

		return 1;

	}

	else

		return 0;

}

void flash(const char* rpath, const char* wpath)
{
	
	timer_start(50);
	//printf("flash31010202(record_t.minAddr, record_t.maxAddr - record_t.minAddr + 1, crc);\n");
	//flash31010202(record_t.minAddr, record_t.maxAddr - record_t.minAddr + 1, crc);
	//Sleep(500);
	//printf("flash3101ff00(record_t.minAddr, record_t.maxAddr - record_t.minAddr + 1);\n");
	//flash3101ff00(record_t.minAddr, record_t.maxAddr - record_t.minAddr + 1);
	//Sleep(500);
	//printf("loadflashfile(rpath, wpath);\n");
	//loadflashfile(rpath, wpath);
	//printf("sid34();\n");
	//sid34();
	//Sleep(500);
	//printf("sid36_download();\n");
	//sid36_download();
	//Sleep(500);
	//printf("sid37();\n");
	//sid37();
	//Sleep(500);
	//printf("flash31010202(record_t.minAddr, record_t.maxAddr - record_t.minAddr + 1, crc);\n");
	//flash31010202(record_t.minAddr, record_t.maxAddr - record_t.minAddr + 1, crc);
	//printf("flash3101ff01();\n");
	//flash3101ff01();
	//Sleep(2000);
	//printf("sid11(01);\n");
	//sid11(01);
	//printf("finsh\n");

	
}

void flash_test()
{
	loadflashfile("flash_driver.hex", "flash_driver_1.bin");
	//int count = 0;
	//while (1)
	//{
	//	printf("%04X: ", count);
	//	for (int i = 0; i < 4; i++)
	//	{
	//		printf("%02X ", ctx_t.rambuff[count]);
	//		count++;
	//	}
	//	printf(" ");

	//	for (int i = 0; i < 4; i++)
	//	{
	//		printf("%02X ", ctx_t.rambuff[count]);
	//		count++;
	//	}
	//	printf(" ");

	//	for (int i = 0; i < 4; i++)
	//	{
	//		printf("%02X ", ctx_t.rambuff[count]);
	//		count++;
	//	}
	//	printf(" ");

	//	for (int i = 0; i < 4; i++)
	//	{
	//		printf("%02X ", ctx_t.rambuff[count]);
	//		count++;
	//	}
	//	printf("\n");

	//	if (count >= 1023)
	//	{
	//		break;
	//	}
	//}





	uint32_t hexlen = 0;
	maxNumberOfBlockLength = 0x202;
	blockSequenceCounter = 1;

	//int count = 0;
	hexlen = record_t.maxAddr - record_t.minAddr + 1;
	uint32_t index = 0;
	uint32_t index_1 = 0;
	printf("hexlen=%d\n", hexlen);
	unsigned char data[0x803];
	//data = (uint8_t*)malloc(1030);
	//blockSequenceCounter = 1;
	while (1)
	{

		if ((hexlen - index) > maxNumberOfBlockLength - 2)
		{
			printf("36_1 service\n");
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

		}
		else
		{
			printf("36_2 service\n");
			data[0] = 0x36;
			data[1] = blockSequenceCounter;
			for (int n = 0; n < hexlen - index_1; n++)
			{
				//printf("index=%d\n", index);
				data[2 + n] = ctx_t.rambuff[index];
				index++;
			}
			index_1 = index;
			network_send_udsmsg(uds_send_can_farme, data, maxNumberOfBlockLength);
			break;
		}
		Sleep(10);
	}
	//int count = 0;
	//while (1)
	//{
	//	printf("%04X: ", count);
	//	for (int i = 0; i < 4; i++)
	//	{
	//		printf("%02X ", data[count+2]);
	//		count++;
	//	}
	//	printf(" ");

	//	for (int i = 0; i < 4; i++)
	//	{
	//		printf("%02X ", data[count + 2]);
	//		count++;
	//	}
	//	printf(" ");

	//	for (int i = 0; i < 4; i++)
	//	{
	//		printf("%02X ", data[count + 2]);
	//		count++;
	//	}
	//	printf(" ");

	//	for (int i = 0; i < 4; i++)
	//	{
	//		printf("%02X ", data[count + 2]);
	//		count++;
	//	}
	//	printf("\n");

	//	if (count >= 1023)
	//	{
	//		break;
	//	}
	//}

	//printf("%x %x %x %x\n", ctx_t.rambuff[0], ctx_t.rambuff[1], ctx_t.rambuff[2], ctx_t.rambuff[3]);
	//printf("%x %x %x %x\n", ctx_t.rambuff[record_t.maxAddr - record_t.minAddr-3], ctx_t.rambuff[record_t.maxAddr - record_t.minAddr-2], ctx_t.rambuff[record_t.maxAddr - record_t.minAddr-1], ctx_t.rambuff[record_t.maxAddr - record_t.minAddr]);
}