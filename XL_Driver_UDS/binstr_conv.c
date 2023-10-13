/*----------------------------------------ͷ�ļ�----------------------------------------------*/
#include <stdint.h>
#include <assert.h>

/*---------------------------------------�ӿں���---------------------------------------------*/

/******************************************************************************************
˵��:         ������16�����ַ�ת��Ϊһ���ֽ�����
����:
// @pchr        �ַ�����
// @len         ת�����ַ�����
// @presult     ת�����ָ�� 0 ʧ�� 1�ɹ�
����ֵ:  void    0Ϊ��Ƿ��ַ���ʧ�� ����Ϊ�ɹ�
******************************************************************************************/
uint32_t Bcv_Hexstring2Uint32(const char* pchr, uint8_t len, uint8_t* presult)
{
    register char ch;
    uint32_t temp = 0;

    for (register uint8_t i = len; i ; --i)
    {
        ch = *pchr++;

        if (ch <= '9') ch -= '0';           /* ���� */
        else if (ch >= 'a') ch -= 'a' - 10; /* Сд�ַ� */
        else if (ch >= 'A') ch -= 'A' - 10; /* ��д�ַ� */
        else ch = 0xff;

        //�Ƿ�Ϊ�Ϸ���16�����ַ�
        if ((uint8_t)ch >= 16)
        {
            *presult = 0;
            return 0;
        }
        //�������
        temp = temp << 4 | (uint8_t)ch;
    }
    *presult = 1;
    return temp;
}

/******************************************************************************************
˵��:         ��һ���ֽ�����ת��Ϊ2��HEX�ַ�
����:
// @pdes        �ַ�����
// @adata       ��ת������
����ֵ:  uint8_t ������
******************************************************************************************/
void Bcv_Byte2Hexstring(char pdes[], uint8_t adata)
{
    static const char ByteHexCodeTable[] = "0123456789ABCDEF";

    pdes[0] = ByteHexCodeTable[adata >> 4];
    pdes[1] = ByteHexCodeTable[adata & 0x0f];
}

/******************************************************************************************
˵��:         �����ַ�����ָ�����ڴ�����
����:
// @pdes        Ŀ��洢��
// @source      Դ�ַ���
����ֵ:  char* ����Ŀ���ڴ����ַ�������λ��
******************************************************************************************/
char* Bcv_strcpy(char* dest, const char* source)
{
    //��������
    assert(dest);
    assert(source);

    if (dest != source)
    {
        for (; '\0' != (*dest = *source); ++dest, ++source);
    }
    return dest;
}
/******************************************************************************************
˵��:         ��ָ����bin����ת��Ϊhex�ַ���������У���
����:
// @strdeststr  Ŀ���ַ�������
// @binbuff     bin����ָ��
// @count       ��������
// @sum         У��ͱ���ָ��
����ֵ:  char* ����Ŀ���ڴ����ַ������һ������λ��
******************************************************************************************/
char* Bcv_ConvertHexAndSum(char* deststr, void* binbuff, uint8_t count, uint8_t* sum)
{
    uint8_t* bytebuff = (uint8_t*)binbuff;

    for (uint8_t i = count; i; --i, deststr+=2, ++bytebuff)
    {
        Bcv_Byte2Hexstring(deststr, *bytebuff);
        *sum += *bytebuff;
    }
    return deststr;
}

