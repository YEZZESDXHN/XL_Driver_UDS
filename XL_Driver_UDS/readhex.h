#pragma once
#include "IntelHex.h"
#include "Srecord.h"
#include <stdio.h>
// **************************************************************************
// 类型定义
typedef struct
{
	FILE* rfile;        // 读取文件句柄
	FILE* wfile;        // 写入文件句柄

	uint8_t* binbuff;   // 二进制缓冲区
	uint8_t* rambuff;   // ram数据缓冲区
	char* strbuff;      // 字符串缓冲区
}ConvertContext;

//void ReadHex(const char* rpath, const char* wpath, IHexRecord *record, ConvertContext *ctx);


void ReadHex(const char* rpath, const char* wpath, IHexRecord *record, ConvertContext *ctx);

void Hex2Bin(const char* rpath, const char* wpath);