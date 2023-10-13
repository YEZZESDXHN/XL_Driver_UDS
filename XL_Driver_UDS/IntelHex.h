/***********************************************************************************************
_____________________________________���ݴ��ʿƼ����޹�˾_______________________________________
@�� �� ��: IntelHex.c
@��    ��: 2016.09.02
@��    ��: ��ʫ��

@�ļ�˵��:

IntelHex�����ڴ���Intel��HEX��¼�ļ��Ĳ�����

@�޶�˵��:

2016.09.02  ��ʼ�汾
2016.09.10  �Ժ�����ʵ�ֽ����ع�

***********************************************************************************************/

#ifndef _INTEL_HEX_H_
#define _INTEL_HEX_H_

/*----------------------------------------ͷ�ļ�----------------------------------------------*/
#include <stdint.h>

/*---------------------------------------��������---------------------------------------------*/

#define IHEX_MAX_BINDATA_NUM    (255)       /* ������¼������ݴ�С */
#define IHEX_RECORD_STR_LEN     (9+IHEX_MAX_BINDATA_NUM*2+2+4)  /* ����м�¼���� */
#define IHEX_EXTADDR_STR_LEN    (9+4+4)     /* ��չ���Ե�ַ����ַ������� */
/* binת��Ϊhexʱ����ַ���������С */
#define IHEX_STRING_BUFF_SIZE   (IHEX_RECORD_STR_LEN+IHEX_EXTADDR_STR_LEN)

/*---------------------------------------ö�ٶ���---------------------------------------------*/
//������ö��
typedef enum eErrorEnum
{
    IHEX_RESULT_NOERROR = 0,            /* �޴��� */
    IHEX_RESULT_INVALID_RECORD,         /* ���ַ���`:`���Ƿ��ļ�¼ */
    IHEX_RESULT_INVALID_CHAR,           /* 16���ƴ��ڷǷ����ַ� */
    IHEX_RESULT_INVALID_TYPE,           /* �Ƿ��ļ�¼���� */
    IHEX_RESULT_ERROR_CHECKSUM,         /* У��ʹ��� */
    IHEX_RESULT_ERROR_RECORD_COUNT,     /* ������������ */
    IHEX_RESULT_RECORD_END,             /* ���ݼ�¼���� */
}IHexResult;

//HEX��¼����ö��
typedef enum eRecordTypeEnum
{
    IHEX_TYPE_DATARECORD = 0,       /* ���ݼ�¼���� */
    IHEX_TYPE_ENDFILE    = 1,       /* �ļ����� λ���ļ����һ�� */
    IHEX_TYPE_EXTSEG     = 2,       /* ��չ�� ��ַ��Ϊ0 ���ݶ�*16�ۼ��ں������е�ַ�� */
    IHEX_TYPE_STARTSEG   = 3,       /* ��ʼ����չ */
    IHEX_TYPE_EXTADDRESS = 4,       /* ��չ���Ե�ַ ��ַ��Ϊ0   ���ݶ�Ϊ�������е�ַ�ĸ�16λ */
    IHEX_TYPE_STARTADDRESS  = 5,    /* ��ʼ���Ե�ַ��¼ */
}HexRecordType;

/*---------------------------------------���Ͷ���---------------------------------------------*/

//HEX��¼�ṹ
typedef struct eHexRecord
{
    uint8_t count;          /* ��ǰ��¼�������ֽ��� */
    uint16_t rcdAddr;       /* ��ǰ��¼������ʼ��ַ */
    HexRecordType type;     /* ��ǰ��¼���� */
    uint32_t fileAddr;      /* �ļ���ַ=��ǰ��¼��ַ+��չ��ַ */
    uint32_t extAddr;       /* ���ڱ����¼�е���չ��ַ */
    uint8_t *pbuff;         /* ת�����ݻ����ڴ�ָ�� */

    uint32_t minAddr;       /* ��¼����С��ַ */
    uint32_t maxAddr;       /* ��¼������ַ */
}IHexRecord;

//дHEX�����ṹ��
typedef struct eHexWirteInfo
{
    uint8_t* pbuff;         /* bin�ڴ��� */
    uint32_t size;          /* bin�ļ���С */
    uint32_t startAddr;     /* bin��ʼƫ�Ƶ�ַ */
    uint16_t linewidth;     /* ÿ�������ֽ����� */

    uint32_t offset;        /* ��ǰ���� */
}IHexWriteDescribe;

/*---------------------------------------�ӿں���---------------------------------------------*/

//����C C++��ϱ��
#ifdef __cplusplus
extern "C" {
#endif

void IHex_InitRecord(IHexRecord* precord, uint8_t* pbuff);
IHexResult IHex_GetNextHeadInfo(IHexRecord* precord, const char * linestr);
IHexResult IHex_GetNextBindata(IHexRecord* precord, const char * str);
void IHex_InitWriteDesc(IHexWriteDescribe* pinfo);
uint32_t IHex_GetNextHexlinestr(IHexWriteDescribe* pinfo, char* str);

    //����C C++��ϱ��
#ifdef __cplusplus 
}
#endif


#endif
