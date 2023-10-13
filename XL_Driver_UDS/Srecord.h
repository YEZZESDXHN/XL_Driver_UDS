/***********************************************************************************************
_____________________________________���ݴ��ʿƼ����޹�˾_______________________________________
@�� �� ��: SRecord.h
@��    ��: 2016.09.16
@��    ��: ��ʫ��

@�ļ�˵��:

    SRecord�����ڴ���MOTOROL��S��¼�ļ��Ĳ�����

@�޶�˵��:

2016.09.16  ��ʼ�汾

***********************************************************************************************/
#ifndef _S_RECORD_H_
#define _S_RECORD_H_

/*----------------------------------------ͷ�ļ�----------------------------------------------*/
#include <stdint.h>

/*---------------------------------------��������---------------------------------------------*/
#define SREC_MAX_BINDATA_NUM    (255-3)                         /* ������¼������ݴ�С */
#define SREC_RECORD_STR_LEN     (4+IHEX_MAX_BINDATA_NUM*2+4)    /* ����м�¼���� */
#define IHEX_STRING_BUFF_SIZE   (IHEX_RECORD_STR_LEN+IHEX_EXTADDR_STR_LEN)


/*---------------------------------------ö�ٶ���---------------------------------------------*/
//������ö��
typedef enum
{
    SREC_RESULT_NOERROR = 0,            /* �޴��� */
    SREC_RESULT_INVALID_RECORD,         /* ���ַ���S���Ƿ��ļ�¼ */
    SREC_RESULT_INVALID_CHAR,           /* 16���ƴ��ڷǷ����ַ� */
    SREC_RESULT_INVALID_TYPE,           /* �Ƿ��ļ�¼���� */
    SREC_RESULT_ERROR_CHECKSUM,         /* У��ʹ��� */
    SREC_RESULT_ERROR_DATA_COUNT,       /* ������������ */
}SRecResult;

//��¼����
typedef enum
{
    SREC_TYPE_DATA  = 0,    /* ���ݼ�¼���� */
    SREC_TYPE_HEAD,         /* ��¼ͷ���� */
    SREC_TYPE_ENDBOOT,      /* ������ַ��¼���� */
    SREC_TYPE_COUNT,        /* ������¼ */
}SRecType;

/*---------------------------------------���Ͷ���---------------------------------------------*/
typedef struct
{
    uint8_t count;          /* �����ֽ��� */
    uint32_t address;       /* ��¼���ݵ�ַ */
    SRecType type;          /* ��¼���� */
    uint8_t *pbuff;         /* �ڴ�ָ�� */
    uint16_t reclines;      /* ���ݼ�¼������ */

    uint32_t minAddr;       /* ��С��ַ */
    uint32_t maxAddr;       /* ����ַ */
}SRecRecord;

//дSREC�����ṹ��
typedef struct
{
    uint8_t* pbuff;         /* bin�ڴ��� */
    uint32_t size;          /* bin�ļ���С */
    uint32_t offsetAddr;    /* binƫ�Ƶ�ַ */
    uint16_t linewidth;     /* ÿ������ */

    uint32_t index;         /* ��ǰ���� */
}SRecWriteDescribe;

/*---------------------------------------�ӿں���---------------------------------------------*/
//����C C++��ϱ��
#ifdef __cplusplus
extern "C" {
#endif

void SRec_InitRecord(SRecRecord* precord, uint8_t* pbuff);
void SRec_InitWriteDesc(SRecWriteDescribe* pdes);
SRecResult SRec_GetNextHeadInfo(SRecRecord* precord, const char * str);
SRecResult SRec_GetNextBindata(SRecRecord* precord, const char * str);
uint32_t SRec_GetNextlinestr(SRecWriteDescribe* pinfo, char* str);

//����C C++��ϱ��
#ifdef __cplusplus 
}
#endif

#endif

