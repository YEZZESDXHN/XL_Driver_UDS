/***********************************************************************************************
_____________________________________���ݴ��ʿƼ����޹�˾_______________________________________
@�� �� ��: IntelHex.c
@��	��: 2016.09.02
@��	��: ��ʫ��

@�ļ�˵��:

IntelHex�����ڴ���Intel��HEX��¼�ļ��Ĳ�����

@�޶�˵��:

2016.09.02	��ʼ�汾
2016.09.10	�Ժ�����ʵ�ֽ����ع�

***********************************************************************************************/

/*----------------------------------------ͷ�ļ�----------------------------------------------*/
#include <assert.h>
#include "IntelHex.h"

/*----------------------------------------��������----------------------------------------------*/

//�ֶ�ƫ��������
#define OFFSET_BYTE_COUNT		(1) 	/* �ֽ�����ƫ�� 1�ֽ� */
#define OFFSET_ADDRESS			(3) 	/* ��ַ�ֶ�ƫ�� 2�ֽ� */
#define OFFSET_RECORD_TYPE		(7) 	/* ��¼�����ֶ�ƫ�� 1�ֽ� */
#define OFFSET_BIN_DATA 		(9) 	/* �����ֶ�ƫ�� 1�ֽ� */

/*----------------------------------------�ڲ�����----------------------------------------------*/

//�ⲿ��������
uint32_t Bcv_Hexstring2Uint32(const char* pchr, uint8_t len, uint8_t* presult);
void Bcv_Byte2Hexstring(char pdes[], uint8_t adata);
char* Bcv_strcpy(char* dest, const char* source);
char* Bcv_ConvertHexAndSum(char* deststr, void* binbuff, uint8_t count, uint8_t* sum);

//д���¼�ṹ��
typedef struct
{
    uint16_t rcdAddr;       /* ��¼������ʼ��ַ */
    uint16_t extAddr;       /* ��չ��ַ */
    HexRecordType type;     /* ��¼���� */
    uint8_t count;          /* �����ֽ��� */
    uint8_t *pbuff;         /* �ڴ�ָ�� */
}IHEXWriteRecord;

/******************************************************************************************
˵��:			������¼ͷ����[:][�ֽ���][��ַ][��¼����][����][CRC]
                ���ڿ��ٵ������¼�ĵ�ַ�ͼ�¼���͵���Ϣ
����:
// @precord		HEX��¼ָ��
// @pchr		�ַ�����
����ֵ:	IHEXResult	���
******************************************************************************************/
static IHexResult ParseHead(IHexRecord* precord, const char pchr[])
{
    uint8_t res = 0;

	//`��`������ʼ��־
	if (':' != pchr[0])
		return IHEX_RESULT_INVALID_RECORD;
	//��ȡ�ֽ���
    precord->count = (uint8_t)Bcv_Hexstring2Uint32(&pchr[OFFSET_BYTE_COUNT], 2, &res);
    if (!res)
        return IHEX_RESULT_INVALID_CHAR;
    //��ȡ��ַ
    precord->rcdAddr = (uint16_t)Bcv_Hexstring2Uint32(&pchr[OFFSET_ADDRESS], 4, &res);
    if (!res)
        return IHEX_RESULT_INVALID_CHAR;
	//��ȡ��¼����
    precord->type = (HexRecordType)Bcv_Hexstring2Uint32(&pchr[OFFSET_RECORD_TYPE], 2, &res);
    if (!res)
        return IHEX_RESULT_INVALID_CHAR;

	//���������¼����
	switch (precord->type)
	{
		/* ���ݼ�¼���� */
	case IHEX_TYPE_DATARECORD:
        if (!precord->count)
            return IHEX_RESULT_ERROR_RECORD_COUNT;

		//�����ļ�HEX��ַ
		precord->fileAddr = precord->extAddr + precord->rcdAddr;
		break;
		/* ��¼���� */
	case IHEX_TYPE_ENDFILE:
		if (precord->count)
			return IHEX_RESULT_ERROR_RECORD_COUNT;
		break;
		/* ��չ��¼���� */
	case IHEX_TYPE_EXTADDRESS:
		if (2 != precord->count)
			return IHEX_RESULT_ERROR_RECORD_COUNT;

		//��ȡ��ַ
        precord->extAddr = Bcv_Hexstring2Uint32(&pchr[OFFSET_BIN_DATA], 4, &res);
        precord->extAddr <<= 16;
        if (!res)
            return IHEX_RESULT_INVALID_CHAR;
		break;
		/* ��չ�μ�¼���� */
	case IHEX_TYPE_EXTSEG:
		if (2 != precord->count)
			return IHEX_RESULT_ERROR_RECORD_COUNT;

		//��ȡ��ַ
        precord->extAddr = Bcv_Hexstring2Uint32(&pchr[OFFSET_BIN_DATA], 4, &res);
        precord->extAddr <<= 4;
        if (!res)
            return IHEX_RESULT_INVALID_CHAR;
		break;
	case IHEX_TYPE_STARTSEG:
	case IHEX_TYPE_STARTADDRESS:
		if (4 != precord->count)
			return IHEX_RESULT_ERROR_RECORD_COUNT;

		//��ȡ��ַ
        precord->fileAddr = Bcv_Hexstring2Uint32(&pchr[OFFSET_BIN_DATA], 8, &res);
        if (!res)
            return IHEX_RESULT_INVALID_CHAR;
		break;
	default:
		return IHEX_RESULT_INVALID_TYPE;
	}

	//ת���ɹ�
	return IHEX_RESULT_NOERROR;
}
/******************************************************************************************
˵��:			ת����¼BIN����
����:
// @precord		HEX��¼ָ��
// @pchr		�ַ�����
����ֵ:	uint8_t	������
******************************************************************************************/
static IHexResult ConvertData(const IHexRecord* precord, const char pchr[])
{
	uint8_t res = 0, temp, i;
	uint8_t sum;

	//�����ֽ�������ַ����¼����У���
	sum = precord->count;						/* �ֽ��� */
	sum += (uint8_t)precord->type;				/* �������� */
	sum += (uint8_t)(precord->rcdAddr >> 8);	/* ��ַ���ֽ� */
	sum += (uint8_t)precord->rcdAddr;			/* ��ַ���ֽ� */

	pchr = &pchr[OFFSET_BIN_DATA];	/* �����ֶ�ƫ�� */
	//��������������
	for (i = 0; i < precord->count; i++, pchr += 2)
	{
        temp = (uint8_t)Bcv_Hexstring2Uint32(pchr, 2, &res);
		if (!res) 
            return IHEX_RESULT_INVALID_CHAR;

		precord->pbuff[i] = temp;	  /* ����ת�������ֵ */
		sum += temp;
	}

	//���У���
    temp = (uint8_t)Bcv_Hexstring2Uint32(pchr, 2, &res);
    if (!res)
        return IHEX_RESULT_INVALID_CHAR;
	if ((uint8_t)(temp + sum))
		return IHEX_RESULT_ERROR_CHECKSUM;

	return IHEX_RESULT_NOERROR;
}
/******************************************************************************************
˵��:			ת����¼ΪHEX�ļ��ַ���
����:
// @str			�ַ���
// @precord		��¼ָ��
����ֵ:	char*	�����ַ���ĩβָ��
******************************************************************************************/
static char* ConvertHexline(char pchr[], IHEXWriteRecord* precord)
{
	uint8_t sum = 0, temp;
	static const char endline[] = ":00000001FF";

	//��������
	assert(pchr);
	assert(precord);

	//������չ�κ���չ��ַ ǿ��ָ�������ֶ�
	if (IHEX_TYPE_EXTADDRESS == precord->type)
	{
		precord->count = 2;
        precord->rcdAddr = 0;
	}

	//��ʼת��Ϊ��¼�ַ���
	if (IHEX_TYPE_ENDFILE == precord->type)
	{	/* ��¼���� */
		pchr = Bcv_strcpy(pchr, endline);
	}
	else
	{
        // :��־
        pchr[0] = ':';
		// ת����¼ͷ
		Bcv_Byte2Hexstring(&pchr[OFFSET_BYTE_COUNT], precord->count);					/* �ֽ����� */
		Bcv_Byte2Hexstring(&pchr[OFFSET_ADDRESS], (uint8_t)(precord->rcdAddr >> 8));	/* ��ַ���ֽ� */
		Bcv_Byte2Hexstring(&pchr[OFFSET_ADDRESS + 2], (uint8_t)precord->rcdAddr);		/* ��ַ���ֽ� */
		Bcv_Byte2Hexstring(&pchr[OFFSET_RECORD_TYPE], (uint8_t)precord->type);			/* ��¼���� */
		// �����ֽ�������ַ����¼����У���
		sum += precord->count;						/* �ֽ��� */
		sum += (uint8_t)(precord->rcdAddr >> 8);	/* ��ַ���ֽ� */
		sum += (uint8_t)precord->rcdAddr;			/* ��ַ���ֽ� */
        sum += (uint8_t)precord->type;				/* �������� */
		
		// ת������
		switch (precord->type)
		{
			/* ���ݼ�¼���� */
		case IHEX_TYPE_DATARECORD:
            pchr = Bcv_ConvertHexAndSum(&pchr[OFFSET_BIN_DATA], precord->pbuff, 
                precord->count, &sum);
			break;
			/* ��չ��¼���� */
		case IHEX_TYPE_EXTADDRESS:
			pchr = &pchr[OFFSET_BIN_DATA];
			sum += (temp = (uint8_t)(precord->extAddr >> 8));
			Bcv_Byte2Hexstring(pchr, temp);
			sum += (temp = (uint8_t)(precord->extAddr));
			Bcv_Byte2Hexstring(&pchr[2], temp);
			pchr += 4;
			break;
		default:
			assert(0);
			break;
		}

		//����У���
		Bcv_Byte2Hexstring(pchr, (uint8_t)(-sum));
		pchr += 2;
	}

	//��ӻ��к�\0�ַ�
	pchr[0] = '\n';
	pchr[1] = '\0';
	return &pchr[1];
}

/*----------------------------------------�ӿں���----------------------------------------------*/

/******************************************************************************************
˵��:			��ʼ����¼�ṹ��
����:
// @precord		HEX��¼�ṹ��ָ��
����ֵ:	void
******************************************************************************************/
void IHex_InitRecord(IHexRecord* precord, uint8_t* pbuff)
{
	//��������
	assert(precord);

	precord->fileAddr = 0;
	precord->rcdAddr = 0;
	precord->extAddr = 0;
	precord->count = 0;
	precord->type = IHEX_TYPE_DATARECORD;
	precord->pbuff = pbuff;
	precord->maxAddr = 0;
	precord->minAddr = (uint32_t)-1;
}

/******************************************************************************************
˵��:			��ʼ��д�ṹ��
����:
// @pinfo		HEXд�ṹ��ָ��
����ֵ:	void
******************************************************************************************/
void IHex_InitWriteDesc(IHexWriteDescribe* pdes)
{
	//��������
	assert(pdes);

	pdes->offset = 0;
}

/******************************************************************************************
˵��:			��ȡ��һ����¼ͷ
����:
// @pinfo		HEX��¼��Ϣ�ṹ��ָ��
// @precord		HEX��¼�ṹ��ָ��
// @linestr			��¼���ַ�
����ֵ:	IHEXResult ���
******************************************************************************************/
IHexResult IHex_GetNextHeadInfo(IHexRecord* precord, const char * linestr)
{
	IHexResult result;

	//��������
	assert(precord);
	assert(linestr);

	//����������¼�����Ժ������еļ�¼
	if (IHEX_TYPE_ENDFILE == precord->type)
		return IHEX_RESULT_RECORD_END;

	//ת����¼ͷ����
	if (IHEX_RESULT_NOERROR != (result = ParseHead(precord, linestr)))
		return result;

	//���ݼ�¼����
	if (IHEX_TYPE_DATARECORD == precord->type)
	{
        uint32_t end = precord->fileAddr + precord->count - 1;
		//��ȡ��ַ��Χ��Ϣ
		if (precord->maxAddr < end)
			precord->maxAddr = end;
		if (precord->minAddr > precord->fileAddr)
			precord->minAddr = precord->fileAddr;
	}
	return IHEX_RESULT_NOERROR;
}

/******************************************************************************************
˵��:			��ȡ��һ����¼����
����:
// @precord		HEX��¼�ṹ��ָ��
// @linestr			��¼���ַ�
����ֵ:	IHEXResult ���
******************************************************************************************/
IHexResult IHex_GetNextBindata(IHexRecord* precord, const char * linestr)
{
	IHexResult result;

	//��������
	assert(precord);
	assert(linestr);

	if (IHEX_RESULT_NOERROR != (result = IHex_GetNextHeadInfo(precord, linestr)))
		return result;

	//���ݼ�¼����ת��BIN����
	return ConvertData(precord, linestr);
}

/******************************************************************************************
˵��:			��ȡ��һ��HEX��¼�ַ���
����:
// @str			�ַ���������
// @pdes		д���������ṹ��ָ��
����ֵ:	uint32_t ���ط�������
******************************************************************************************/
uint32_t IHex_GetNextHexlinestr(IHexWriteDescribe* pinfo, char* str)
{
	IHEXWriteRecord wrecd;
	uint32_t rlen;

	//��������
	assert(str);
	assert(pinfo);

	//���ת���Ƿ����
	if (pinfo->offset >= pinfo->size)
	{
		wrecd.type = IHEX_TYPE_ENDFILE;
		ConvertHexline(str, &wrecd);
		return 0;
	}

	//�������跢����������
	rlen = pinfo->size - pinfo->offset;
	if (rlen > pinfo->linewidth)
		rlen = pinfo->linewidth;

	if ((pinfo->offset && (pinfo->startAddr & 0xffffu) < rlen) ||	// ��ַ���
		(!pinfo->offset && pinfo->startAddr & 0xffff0000u))		// �״ε�ַ
	{
		wrecd.type = IHEX_TYPE_EXTADDRESS; 		            /* ��չ���� */
		wrecd.extAddr = (uint16_t)(pinfo->startAddr>>16);   /* ��չ��ַ */
		str = ConvertHexline(str, &wrecd);
	}

	//�������ݼ�¼ΪHEX��¼��
	wrecd.type = IHEX_TYPE_DATARECORD;  /* ���ݼ�¼���� */
	wrecd.count = (uint8_t)rlen;        /* �������� */
	wrecd.pbuff = pinfo->pbuff;		    /* ������ */
	wrecd.rcdAddr = (uint16_t)pinfo->startAddr;	/* HEX��¼��ַ */
	ConvertHexline(str, &wrecd);

	//�����ַ������
	pinfo->offset += rlen;
	pinfo->pbuff += rlen;
	pinfo->startAddr += rlen;

	return pinfo->offset;
}

