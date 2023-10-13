/***********************************************************************************************
_____________________________________���ݴ��ʿƼ����޹�˾_______________________________________
@�� �� ��: SRecord.c
@��    ��: 2016.09.16
@��    ��: ��ʫ��

@�ļ�˵��:

SRecord�����ڴ���MOTOROL��S��¼�ļ��Ĳ�����

@�޶�˵��:

2016.09.16  ��ʼ�汾

***********************************************************************************************/

/*----------------------------------------ͷ�ļ�----------------------------------------------*/
#include <assert.h>
#include "Srecord.h"

/*---------------------------------------��������---------------------------------------------*/
//�ֶ�ƫ��������
#define OFFSET_TYPE             (1)     /* ��¼����ƫ�� 1�ַ� */
#define OFFSET_BYTE_COUNT       (2)     /* �ֽ�����ƫ�� 1�ֽ� */
#define OFFSET_ADDRESS          (4)     /* ��ַ�ֶ�ƫ�� 2��3��4�ֽ� */

/*---------------------------------------�ڲ�����---------------------------------------------*/

typedef struct
{
    uint8_t addrwidth;      // ��ַ�ֶο���ֽ���
    SRecType rectype;       // ��¼����
}SRecSTypeStruct;

static const SRecSTypeStruct typeTable[10] =
{
    { 2, SREC_TYPE_HEAD },   // S0
    { 2, SREC_TYPE_DATA },   // S1
    { 3, SREC_TYPE_DATA },   // S2
    { 4, SREC_TYPE_DATA },   // S3
    { 0, (SRecType)-1 },     // S4
    { 2, SREC_TYPE_COUNT },  // S5
    { 3, SREC_TYPE_COUNT },  // S6
    { 4, SREC_TYPE_ENDBOOT },// S7
    { 3, SREC_TYPE_ENDBOOT },// S8
    { 2, SREC_TYPE_ENDBOOT } // S9
};

/*---------------------------------------�ڲ�����---------------------------------------------*/

// �ⲿ���ú���
uint32_t Bcv_Hexstring2Uint32(const char* pchr, uint8_t len, uint8_t* presult);
void Bcv_Byte2Hexstring(char pdes[], uint8_t adata);
char* Bcv_strcpy(char* dest, const char* source);
char* Bcv_ConvertHexAndSum(char* deststr, void* binbuff, uint8_t count, uint8_t* sum);

/******************************************************************************************
˵��:         ������¼ͷ����
����:
// @precd       ��¼ָ��
// @pchr        �ַ�����
����ֵ:  SRecResult  ���
******************************************************************************************/
static SRecResult ParseHead(SRecRecord* precd, const char pchr[])
{
    uint8_t count,res;
    uint8_t typeindex;      // ��������
    uint8_t addrwidth;      // ��ַ�ֶο��
    
    //S ������ʼ��־
    if ('S' != pchr[0])
        return SREC_RESULT_INVALID_RECORD;
    //������¼��ʽ
    typeindex = pchr[OFFSET_TYPE]-'0';   /* S0 S1 S2 S3... */
    if (typeindex > 9u || (SRecType)-1 == (precd->type = typeTable[typeindex].rectype))
        return SREC_RESULT_INVALID_TYPE;

    //�ֽ���
    count = (uint8_t)Bcv_Hexstring2Uint32(&pchr[OFFSET_BYTE_COUNT], 2, &res);
    if (!res)
        return SREC_RESULT_INVALID_CHAR;

    //��ȡ������¼�ֽ���
    addrwidth = typeTable[typeindex].addrwidth;   // n�ֽڵ�ַ+1�ֽ�crc�ֶ�
    switch (precd->type)
    {
    case SREC_TYPE_DATA:
    case SREC_TYPE_HEAD:
        if (count <= addrwidth + 1)
            return SREC_RESULT_ERROR_DATA_COUNT;
        //��ȡ��¼��ַ
        precd->address = Bcv_Hexstring2Uint32(&pchr[OFFSET_ADDRESS], addrwidth * 2, &res);
        if (!res)
            return SREC_RESULT_INVALID_CHAR;
        break;
    case SREC_TYPE_COUNT:
    case SREC_TYPE_ENDBOOT:
        if (count != addrwidth + 1)
            return SREC_RESULT_ERROR_DATA_COUNT;
        //��ȡ����ֵ
        precd->address = Bcv_Hexstring2Uint32(&pchr[OFFSET_ADDRESS], addrwidth * 2, &res);
        if (!res)
            return SREC_RESULT_INVALID_CHAR;
        break;
    default:
        assert(0);
        break;
    }

    precd->count = count - (addrwidth + 1);    /* �������������ֽ��� */
  
    //�����ɹ�
    return SREC_RESULT_NOERROR;
}
/******************************************************************************************
˵��:         ������У������
����:
// @precd       ��¼ָ��
// @str         �ַ�����
����ֵ:  SRecResult  ���
******************************************************************************************/
static SRecResult ParseData(SRecRecord* precd, const char* str)
{
    uint8_t sum, bytedata,res;
    uint32_t temp;

    //���ݼ�¼����
    if (SREC_TYPE_DATA == precd->type)
        ++precd->reclines;

    //�����ֽ�������ַ�ֶ�У���
    sum = (uint8_t)Bcv_Hexstring2Uint32(&str[OFFSET_BYTE_COUNT], 2, &res);
    if (!res)
        return SREC_RESULT_INVALID_TYPE;
    for (temp = precd->address; temp; temp >>= 8)
        sum += (uint8_t)temp;   /* ��ַ�ֶ� */

    //�����ֶ���ʼ��ַ
    bytedata = typeTable[str[OFFSET_TYPE] - '0'].addrwidth;
    str = str + OFFSET_ADDRESS + (bytedata * 2);

    //ת�����ݲ�����У���
    for (uint8_t i = 0; i < precd->count; i++, str += 2)
    {
        bytedata = (uint8_t)Bcv_Hexstring2Uint32(str, 2, &res);
        if (!res)
            return SREC_RESULT_INVALID_TYPE;
        precd->pbuff[i] = bytedata;            /* �������� */
        sum += bytedata;                       /* ����У��� */
    }
    //����У���
    bytedata = (uint8_t)Bcv_Hexstring2Uint32(str, 2, &res);
    if (!res)
        return SREC_RESULT_INVALID_TYPE;
    if (0xffu != (sum + bytedata))
        return SREC_RESULT_ERROR_CHECKSUM;

    return SREC_RESULT_NOERROR;
}
/******************************************************************************************
˵��:         ת����¼ΪHEX�ļ��ַ���[s][3][�ֽ���][��ַ][����]
����:
// @str         �ַ���
// @precord     ��¼ָ��
����ֵ:  char*   �����ַ���ĩβָ��
******************************************************************************************/
static char* ConvertHexline(char pchr[], SRecRecord* precord)
{
    uint8_t i, sum = 0;
    uint32_t temp;
    static const char endline[] = "S9030000FC";

    //��������
    assert(pchr);
    assert(precord);
    assert(SREC_TYPE_DATA == precord->type || 
        SREC_TYPE_ENDBOOT == precord->type);

    if (SREC_TYPE_DATA == precord->type)
    {   /* ���ݼ�¼���� */
        //��ʼת��Ϊ��¼�ַ���
        pchr[0] = 'S';              /* :��־ */

        pchr[OFFSET_TYPE] = '3';
        //�ֽ���
        Bcv_Byte2Hexstring(&pchr[OFFSET_BYTE_COUNT], precord->count+5); /* �ֽ����� */
        pchr = &pchr[OFFSET_ADDRESS];                   /* ��ַ�ֶ� */
        sum += precord->count+5;                        /* �����ֽ���+4�ֽڵ�ַ+1�ֽ�CRC */
        //��ַ
        for (i = 0, temp = precord->address;i < 4;++i)
        {
            Bcv_Byte2Hexstring(pchr, (uint8_t)(temp >> 24));
            sum += (uint8_t)(temp >> 24);   /* У��� */                 
            pchr += 2;                      /* �¸�����λ�� */
            temp <<= 8;
        }
        //��¼����
        pchr = Bcv_ConvertHexAndSum(pchr, precord->pbuff, precord->count, &sum);
        //����У���
        Bcv_Byte2Hexstring(pchr, 0xff - sum);
        pchr += 2;
    }
    else if (SREC_TYPE_ENDBOOT == precord->type)
    {   /* ��¼���� */
        pchr = Bcv_strcpy(pchr, endline);
    }

    //��ӻ��к�\0�ַ�
    pchr[0] = '\n';
    pchr[1] = '\0';
    return &pchr[1];
}
/*---------------------------------------�ӿں���---------------------------------------------*/

/******************************************************************************************
˵��:         ��ʼ����¼�ṹ��
����:
// @precd       ��¼�ṹ��ָ��
����ֵ:  void
******************************************************************************************/
void SRec_InitRecord(SRecRecord* precd, uint8_t* pbuff)
{
    //��������
    assert(precd);

    precd->count = 0;
    precd->reclines = 0;
    precd->address = 0;
    precd->pbuff = pbuff;

    precd->maxAddr = 0;
    precd->minAddr = (uint32_t)-1;
}
/******************************************************************************************
˵��:         ��ʼ��д�ṹ��
����:
// @pinfo       д�����ṹ��ָ��
����ֵ:  void
******************************************************************************************/
void SRec_InitWriteDesc(SRecWriteDescribe* pdes)
{
    //��������
    assert(pdes);

    pdes->index = 0;
}
/******************************************************************************************
˵��:         ��ȡ��һ����¼��Ϣ
����:
// @pinfo       ��¼��Ϣ�ṹ��ָ��
// @precord     ��¼�ṹ��ָ��
// @str         ��¼���ַ�
����ֵ:  SRecResult ���
******************************************************************************************/
SRecResult SRec_GetNextHeadInfo(SRecRecord* precord, const char * str)
{
    SRecResult result;
    uint32_t end;

    //��������
    assert(precord);
    assert(str);

    //ת����¼ͷ����
    if (SREC_RESULT_NOERROR != (result = ParseHead(precord, str)))
        return result;

    //���ݼ�¼����
    if (SREC_TYPE_DATA == precord->type)
    {
        end = precord->address + precord->count - 1;
        //��ȡ��ַ��Χ��Ϣ
        if (precord->maxAddr < end)
            precord->maxAddr = end;
        if (precord->minAddr > precord->address)
            precord->minAddr = precord->address;
    }
    return SREC_RESULT_NOERROR;
}
/******************************************************************************************
˵��:         ��ȡ��һ����¼����
����:
// @precd       ��¼�ṹ��ָ��
// @str         ��¼���ַ�
����ֵ:  SRecResult ���
******************************************************************************************/
SRecResult SRec_GetNextBindata(SRecRecord* precd, const char * str)
{
    SRecResult result;

    //��������
    assert(precd);
    assert(str);
    assert(precd->pbuff);

    //ת����¼ͷ����
    if (SREC_RESULT_NOERROR != (result = SRec_GetNextHeadInfo(precd, str)))
        return result;

    //���ݼ�¼����ת��BIN����
    return ParseData(precd, str);
}
/******************************************************************************************
˵��:         ��ȡ��һ��HEX��¼�ַ���
����:
// @str         �ַ���������
// @pdes        д���������ṹ��ָ��
����ֵ:  uint32_t ���ط����ڴ�����
******************************************************************************************/
uint32_t SRec_GetNextlinestr(SRecWriteDescribe* pinfo, char* str)
{
    SRecRecord record;
    uint32_t rlen;

    //��������
    assert(str);
    assert(pinfo);
    assert(pinfo->pbuff);
    
    record.pbuff = pinfo->pbuff;        /* ������ */
    record.address = pinfo->offsetAddr;    /* ��¼ƫ�Ƶ�ַ */

    //�ж��Ƿ����
    if (pinfo->index >= pinfo->size)
    {   /* HEX��¼���� */
        record.type = SREC_TYPE_ENDBOOT;
        ConvertHexline(str, &record);
        return 0;
    }

    //�������跢����������
    rlen = pinfo->size - pinfo->index;
    if (rlen > pinfo->linewidth)
        rlen = pinfo->linewidth;

    //�������ݼ�¼ΪHEX��¼��
    record.type = SREC_TYPE_DATA;       /* ���ݼ�¼���� */
    record.count = (uint8_t)rlen;       /* �������� */
    ConvertHexline(str, &record);

    //�����ַ������
    pinfo->index += rlen;
    pinfo->pbuff += rlen;
    pinfo->offsetAddr += rlen;

    return pinfo->index;
}


