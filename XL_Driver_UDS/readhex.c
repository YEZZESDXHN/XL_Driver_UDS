// hex2bin.cpp : �������̨Ӧ�ó������ڵ㡣
//

// **************************************************************************
// ͷ�ļ�
//#include "IntelHex.h"
//#include "Srecord.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include"readhex.h"
//// **************************************************************************
//// ���Ͷ���
//typedef struct
//{
//	FILE* rfile;        // ��ȡ�ļ����
//	FILE* wfile;        // д���ļ����
//
//	uint8_t* binbuff;   // �����ƻ�����
//	uint8_t* rambuff;   // ram���ݻ�����
//	char* strbuff;      // �ַ���������
//}ConvertContext;

// **************************************************************************
//  �ڲ����ݶ���

// �ַ�����Դ����
static const char* STRING_OPEN_FILEERROR = "���ļ�:\"%s\" ʱ���ִ���!\r\n";
static const char* STRING_MALLO_MEMERROR = "�����ڴ����!\r\n";
static const char* STRING_READ_ERROREND = "��ȡ�ļ�ʱ����������ļ���β!\r\n";
static const char* STRING_FILE_ERROR = "�ļ����ݴ��󣬴�����Ϊ:%d ������:%d\r\n";

// **************************************************************************
//  �ڲ�����

// ��ʼ�������Ķ���
static void InitContext(ConvertContext* pctx)
{
	if (pctx)
	{
		pctx->rfile = NULL;
		pctx->wfile = NULL;
		pctx->strbuff = NULL;
		pctx->binbuff = NULL;
		pctx->rambuff = NULL;
	}
}
// ������
static bool CheckSpaceLine(const char* linstr)
{
	for (char chr; '\0' != (chr = *linstr); ++linstr)
	{
		if ('\n' == chr || '\r' == chr ||
			' ' == chr || '\t' == chr)
			continue;
		else
			return false;
	}
	return true;
}
// �˳�Ӧ�ó���
static void ExitApplication(ConvertContext* pctx, const char*fmt, ...)
{
	// ��ӡ��Ϣ�ַ���
	if (fmt)
	{
		va_list ap;
		va_start(ap, fmt);
		vprintf(fmt, ap);
		va_end(ap);
	}

	// ����ʹ�õ���Դ
	if (pctx)
	{
		if (pctx->rfile) fclose(pctx->rfile);
		if (pctx->wfile) fclose(pctx->wfile);
		if (pctx->binbuff) free(pctx->binbuff);
		if (pctx->strbuff) free(pctx->strbuff);
		if (pctx->rambuff) free(pctx->rambuff);
	}

	getchar();
	exit(0);
}
// ���·����Ч
static void CheckPatchValid(const char* rpath, const char* wpath)
{
	if (NULL == rpath || *rpath == '\0' ||
		NULL == wpath || *wpath == '\0')
		ExitApplication(NULL, "�����ļ�·��Ϊ��!...\r\n");
}

// **************************************************************************
// ����ת�����ܺ���

//HEX2BINת��
void Hex2Bin(const char* rpath, const char* wpath)
{
	IHexResult result;
	IHexRecord record;

	ConvertContext ctx;
	InitContext(&ctx);

	// ���·����Ч
	CheckPatchValid(rpath, wpath);
	// ��HEX�ļ�
	if (NULL == (ctx.rfile = fopen(rpath, "r")))
		ExitApplication(&ctx, STRING_OPEN_FILEERROR, rpath);

	// ������Ӧ�Ļ����ڴ�ռ�
	if (NULL == (ctx.strbuff = (char*)malloc(IHEX_STRING_BUFF_SIZE)) ||
		NULL == (ctx.binbuff = (uint8_t*)malloc(IHEX_MAX_BINDATA_NUM)))
		ExitApplication(&ctx, STRING_MALLO_MEMERROR);

	// ��ȡHEX��Ϣ�Է����ڴ�ռ�
	uint32_t startaddress = 0;
	IHex_InitRecord(&record, ctx.binbuff);
	for (int clen = 0;;)
	{
		if (NULL == fgets(ctx.strbuff, IHEX_RECORD_STR_LEN, ctx.rfile))
			ExitApplication(&ctx, STRING_READ_ERROREND);

		++clen;
		if (CheckSpaceLine(ctx.strbuff))       // ����
			continue;

		result = IHex_GetNextHeadInfo(&record, ctx.strbuff);
		if (IHEX_RESULT_NOERROR != result)
			ExitApplication(&ctx, STRING_FILE_ERROR, result, clen);
		else if (record.type == IHEX_TYPE_STARTADDRESS)
		{
			startaddress = record.fileAddr;
		}
		else if (record.type == IHEX_TYPE_ENDFILE)
		{
			printf("�ļ�:\"%s\" ��ȡ�ɹ�\r\n", rpath);
			printf("�ļ���¼���ݷ�ΧΪ:0X%08X - 0X%08X\r\n", record.minAddr, record.maxAddr);
			printf("��: %d �ֽ����� %d ��\r\n", record.maxAddr - record.minAddr + 1, clen);
			if (startaddress)
				printf("��ʼ��ַΪ��0X%08X\r\n", startaddress);
			break;
		}
	}

	const uint32_t bytenum = record.maxAddr - record.minAddr + 1;
	//�����ڴ沢���0xff
	if (NULL == (ctx.rambuff = (uint8_t*)malloc(bytenum)))
		ExitApplication(&ctx, "�����ڴ����!\r\n");
	memset(ctx.rambuff, 0xff, bytenum);
	// ��д���ļ�
	if (NULL == (ctx.wfile = fopen(wpath, "wb")))
		ExitApplication(&ctx, STRING_OPEN_FILEERROR, wpath);

	// ��ʼ����hex�ļ�����������
	rewind(ctx.rfile);              // �ض�λ��ȡ�ļ�
	IHex_InitRecord(&record, ctx.binbuff);
	for (int clen = 0;;)
	{
		if (NULL == fgets(ctx.strbuff, IHEX_RECORD_STR_LEN, ctx.rfile))
			ExitApplication(&ctx, STRING_READ_ERROREND);

		++clen;
		if (CheckSpaceLine(ctx.strbuff))       // ����
			continue;

		if (IHEX_RESULT_NOERROR != (result = IHex_GetNextBindata(&record, ctx.strbuff)))
			ExitApplication(&ctx, STRING_FILE_ERROR, result, clen);
		else if (record.type == IHEX_TYPE_ENDFILE)
		{
			fwrite(ctx.rambuff, 1, bytenum, ctx.wfile);
			ExitApplication(&ctx, "ת���������\r\n");
		}
		else if (record.type == IHEX_TYPE_DATARECORD)
			memcpy(ctx.rambuff + (record.fileAddr - record.minAddr), ctx.binbuff, record.count);
	}
}
//IHexRecord record;
//ConvertContext ctx;
//HEX��ȡ
void ReadHex(const char* rpath, const char* rpath2, IHexRecord *record, ConvertContext *ctx)
{
	IHexResult result;



	InitContext(ctx);

	// ���·����Ч
	CheckPatchValid(rpath, rpath2);
	// ��HEX�ļ�
	if (NULL == ((*ctx).rfile = fopen(rpath, "r")))
		ExitApplication(ctx, STRING_OPEN_FILEERROR, rpath);

	// ������Ӧ�Ļ����ڴ�ռ�
	if (NULL == ((*ctx).strbuff = (char*)malloc(IHEX_STRING_BUFF_SIZE)) ||
		NULL == ((*ctx).binbuff = (uint8_t*)malloc(IHEX_MAX_BINDATA_NUM)))
		ExitApplication(ctx, STRING_MALLO_MEMERROR);

	// ��ȡHEX��Ϣ�Է����ڴ�ռ�
	uint32_t startaddress = 0;
	IHex_InitRecord(record, (*ctx).binbuff);
	for (int clen = 0;;)
	{
		if (NULL == fgets((*ctx).strbuff, IHEX_RECORD_STR_LEN, (*ctx).rfile))
			ExitApplication(ctx, STRING_READ_ERROREND);

		++clen;
		if (CheckSpaceLine((*ctx).strbuff))       // ����
			continue;

		result = IHex_GetNextHeadInfo(record, (*ctx).strbuff);
		if (IHEX_RESULT_NOERROR != result)
			ExitApplication(ctx, STRING_FILE_ERROR, result, clen);
		else if ((*record).type == IHEX_TYPE_STARTADDRESS)
		{
			startaddress = (*record).fileAddr;
		}
		else if ((*record).type == IHEX_TYPE_ENDFILE)
		{
			printf("�ļ�:\"%s\" ��ȡ�ɹ�\r\n", rpath);
			printf("�ļ���¼���ݷ�ΧΪ:0X%08X - 0X%08X\r\n", (*record).minAddr, (*record).maxAddr);
			printf("��: %d �ֽ����� %d ��\r\n", (*record).maxAddr - (*record).minAddr + 1, clen);
			if (startaddress)
				printf("��ʼ��ַΪ��0X%08X\r\n", startaddress);
			break;
		}
	}

	const uint32_t bytenum = (*record).maxAddr - (*record).minAddr + 1;
	//�����ڴ沢���0xff
	if (NULL == ((*ctx).rambuff = (uint8_t*)malloc(bytenum)))
		ExitApplication(ctx, "�����ڴ����!\r\n");
	memset((*ctx).rambuff, 0xff, bytenum);
	// ��д���ļ�
	//if (NULL == (ctx.wfile = fopen(wpath, "wb")))
	//	ExitApplication(ctx, STRING_OPEN_FILEERROR, wpath);

	// ��ʼ����hex�ļ�����������
	rewind((*ctx).rfile);              // �ض�λ��ȡ�ļ�
	IHex_InitRecord(record, (*ctx).binbuff);
	for (int clen = 0;;)
	{
		if (NULL == fgets((*ctx).strbuff, IHEX_RECORD_STR_LEN, (*ctx).rfile))
			ExitApplication(ctx, STRING_READ_ERROREND);

		++clen;
		if (CheckSpaceLine((*ctx).strbuff))       // ����
			continue;

		if (IHEX_RESULT_NOERROR != (result = IHex_GetNextBindata(record, (*ctx).strbuff)))
			ExitApplication(ctx, STRING_FILE_ERROR, result, clen);
		else if ((*record).type == IHEX_TYPE_ENDFILE)
		{
			//fwrite(ctx.rambuff, 1, bytenum, ctx.wfile);
			//ExitApplication(ctx, "ת���������\r\n");
			if ((*ctx).rfile) fclose((*ctx).rfile);
			if ((*ctx).wfile) fclose((*ctx).wfile);
			//if (ctx.binbuff) free(ctx.binbuff);
			//if (ctx.strbuff) free(ctx.strbuff);
			//if ((*ctx).rambuff) free((*ctx).rambuff);
			return;
		}
		else if ((*record).type == IHEX_TYPE_DATARECORD)
			memcpy((*ctx).rambuff + ((*record).fileAddr - (*record).minAddr), (*ctx).binbuff, (*record).count);
	}
}
//BIN2HEXת��
static void Bin2Hex(const char* rpath, const char* wpath, uint32_t offset)
{
	uint32_t fsize;
	uint32_t re;

	IHexWriteDescribe winfo;
	ConvertContext ctx;
	InitContext(&ctx);

	// ���·����Ч
	CheckPatchValid(rpath, wpath);

	if (NULL == (ctx.rfile = fopen(rpath, "rb")))
		ExitApplication(&ctx, STRING_OPEN_FILEERROR, rpath);


	// ��ȡ�ļ���С
	fseek(ctx.rfile, 0, SEEK_END);      // ��λ���ļ���β
	fsize = (uint32_t)ftell(ctx.rfile); // �����ļ���С

	// �����ڴ沢��ȡ����
	if (NULL == (ctx.rambuff = (uint8_t*)malloc(fsize)) ||
		NULL == (ctx.strbuff = (char*)malloc(IHEX_STRING_BUFF_SIZE)))
		ExitApplication(&ctx, STRING_MALLO_MEMERROR);
	rewind(ctx.rfile);      // �ض�λ�ļ�
	fread(ctx.rambuff, 1, fsize, ctx.rfile);
	fclose(ctx.rfile);
	ctx.rfile = NULL;

	// ת����д���ļ�
	winfo.pbuff = ctx.rambuff;
	winfo.linewidth = 32;
	winfo.startAddr = offset;
	winfo.size = fsize;
	IHex_InitWriteDesc(&winfo);
	if (NULL == (ctx.wfile = fopen(wpath, "w")))
		ExitApplication(&ctx, STRING_OPEN_FILEERROR, wpath);
	for (;;)
	{
		re = IHex_GetNextHexlinestr(&winfo, ctx.strbuff);
		fputs(ctx.strbuff, ctx.wfile);
		if (!re)
			ExitApplication(&ctx, "\"%s\" ת�����\r\n", rpath);
	}
}
//SREC2BINת��
static void SRec2Bin(const char* rpath, const char* wpath)
{
	SRecResult result;
	SRecRecord record;
	ConvertContext ctx;
	InitContext(&ctx);

	// ���·����Ч
	CheckPatchValid(rpath, wpath);
	// ��HEX�ļ�
	if (NULL == (ctx.rfile = fopen(rpath, "r")))
		ExitApplication(&ctx, STRING_OPEN_FILEERROR, rpath);

	// ������Ӧ�Ļ����ڴ�ռ�
	if (NULL == (ctx.strbuff = (char*)malloc(SREC_RECORD_STR_LEN)) ||
		NULL == (ctx.binbuff = (uint8_t*)malloc(SREC_MAX_BINDATA_NUM)))
		ExitApplication(&ctx, STRING_MALLO_MEMERROR);

	// ��ȡHEX��Ϣ�Է����ڴ�ռ�
	SRec_InitRecord(&record, ctx.binbuff);
	for (int clen = 0;;)
	{
		if (NULL == fgets(ctx.strbuff, IHEX_RECORD_STR_LEN, ctx.rfile))
			ExitApplication(&ctx, STRING_READ_ERROREND);

		++clen;
		if (CheckSpaceLine(ctx.strbuff))       // ����
			continue;

		result = SRec_GetNextHeadInfo(&record, ctx.strbuff);
		if (IHEX_RESULT_NOERROR != result)
			ExitApplication(&ctx, STRING_FILE_ERROR, result, clen);
		else if (record.type == SREC_TYPE_ENDBOOT)
		{
			printf("�ļ�:\"%s\" ��ȡ�ɹ�\r\n", rpath);
			printf("�ļ���¼���ݷ�ΧΪ:0X%08X - 0X%08X\r\n", record.minAddr, record.maxAddr);
			printf("��: %d �ֽ����� %d ��\r\n", record.maxAddr - record.minAddr + 1, clen);
			break;
		}
	}
	const uint32_t bytenum = record.maxAddr - record.minAddr + 1;
	//�����ڴ沢���0xff
	if (NULL == (ctx.rambuff = (uint8_t*)malloc(bytenum)))
		ExitApplication(&ctx, "�����ڴ����!\r\n");
	memset(ctx.rambuff, 0xff, bytenum);
	// ��д���ļ�
	if (NULL == (ctx.wfile = fopen(wpath, "wb")))
		ExitApplication(&ctx, STRING_OPEN_FILEERROR, wpath);

	// ��ʼ����hex�ļ�����������
	rewind(ctx.rfile);              // �ض�λ��ȡ�ļ�
	SRec_InitRecord(&record, ctx.binbuff);
	for (int clen = 0;;)
	{
		if (NULL == fgets(ctx.strbuff, IHEX_RECORD_STR_LEN, ctx.rfile))
			ExitApplication(&ctx, STRING_READ_ERROREND);

		++clen;
		if (CheckSpaceLine(ctx.strbuff))       // ����
			continue;

		if (IHEX_RESULT_NOERROR != (result = SRec_GetNextBindata(&record, ctx.strbuff)))
			ExitApplication(&ctx, STRING_FILE_ERROR, result, clen);
		else if (record.type == SREC_TYPE_ENDBOOT)
		{
			fwrite(ctx.rambuff, 1, bytenum, ctx.wfile);
			ExitApplication(&ctx, "ת���������\r\n");
		}
		else if (record.type == SREC_TYPE_DATA)
			memcpy(ctx.rambuff + (record.address - record.minAddr), ctx.binbuff, record.count);
	}
}
//SREC2BINת��
static void Bin2SRec(const char* rpath, const char* wpath, uint32_t offset)
{
	uint32_t fsize;
	uint32_t re;

	SRecWriteDescribe winfo;
	ConvertContext ctx;
	InitContext(&ctx);

	// ���·����Ч
	CheckPatchValid(rpath, wpath);

	if (NULL == (ctx.rfile = fopen(rpath, "rb")))
		ExitApplication(&ctx, STRING_OPEN_FILEERROR, rpath);


	// ��ȡ�ļ���С
	fseek(ctx.rfile, 0, SEEK_END);      // ��λ���ļ���β
	fsize = (uint32_t)ftell(ctx.rfile); // �����ļ���С

	// �����ڴ沢��ȡ����
	if (NULL == (ctx.rambuff = (uint8_t*)malloc(fsize)) ||
		NULL == (ctx.strbuff = (char*)malloc(SREC_RECORD_STR_LEN)))
		ExitApplication(&ctx, STRING_MALLO_MEMERROR);
	rewind(ctx.rfile);      // �ض�λ�ļ�
	fread(ctx.rambuff, 1, fsize, ctx.rfile);
	fclose(ctx.rfile);
	ctx.rfile = NULL;

	// ת����д���ļ�
	winfo.pbuff = ctx.rambuff;
	winfo.linewidth = 32;
	winfo.offsetAddr = offset;
	winfo.size = fsize;
	SRec_InitWriteDesc(&winfo);
	if (NULL == (ctx.wfile = fopen(wpath, "w")))
		ExitApplication(&ctx, STRING_OPEN_FILEERROR, wpath);
	for (;;)
	{
		re = SRec_GetNextlinestr(&winfo, ctx.strbuff);
		fputs(ctx.strbuff, ctx.wfile);
		if (!re)
			ExitApplication(&ctx, "\"%s\" ת�����\r\n", rpath);
	}
}
