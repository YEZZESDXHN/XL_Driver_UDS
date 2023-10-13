#pragma once
#include "IntelHex.h"
#include "Srecord.h"
#include <stdio.h>
// **************************************************************************
// ���Ͷ���
typedef struct
{
	FILE* rfile;        // ��ȡ�ļ����
	FILE* wfile;        // д���ļ����

	uint8_t* binbuff;   // �����ƻ�����
	uint8_t* rambuff;   // ram���ݻ�����
	char* strbuff;      // �ַ���������
}ConvertContext;

//void ReadHex(const char* rpath, const char* wpath, IHexRecord *record, ConvertContext *ctx);


void ReadHex(const char* rpath, const char* wpath, IHexRecord *record, ConvertContext *ctx);

void Hex2Bin(const char* rpath, const char* wpath);