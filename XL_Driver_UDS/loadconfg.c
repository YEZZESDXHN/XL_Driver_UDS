/*ini.c*/
#include <stdio.h>  
#include <string.h>       
#include"loadconfg.h"

//struct DID_list
//{
//	char did_name[128];
//	char did[4096];
//};
//struct ECU_DID
//{
//	int DID_num;
//	struct DID_list DID_list[128];
//};
////struct DID_list DID_info;
//struct ECU_list
//{
//	char ECU_name[128];
//	char DIAG_REQ_PHY_ID[16];
//	char DIAG_RESP_ID[16];
//	char DIAG_REQ_FUNC_ID[16];
//};
//
//struct DIAG_info
//{
//	int ECU_num;
//
//	struct ECU_list ECU_list[128];
//	struct ECU_DID ECU_DID[128];
//
//};
//struct DIAG_info gDiag_info;





/*
	* ��������         GetIniKeyString
	* ��ڲ�����        title
	*                      �����ļ���һ�����ݵı�ʶ
	*                  key
	*                      ����������Ҫ������ֵ�ı�ʶ
	*                  filename
	*                      Ҫ��ȡ���ļ�·��
	* ����ֵ��         �ҵ���Ҫ���ֵ�򷵻���ȷ��� 0
	*                  ���򷵻�-1
	*/
int GetIniKeyString(char *title, char *key, char *filename, char *buf)
{
	FILE *fp;
	int  flag = 0;
	char sTitle[64], *wTmp;
	char sLine[1024];
	sprintf(sTitle, "[%s]", title);

	if (NULL == (fp = fopen(filename, "r"))) {
		perror("fopen");
		return -1;
	}
	while (NULL != fgets(sLine, 1024, fp)) {
		// ����ע����  

		if (0 == strncmp("//", sLine, 2)) continue;
		if ('#' == sLine[0])              continue;
		wTmp = strchr(sLine, '=');
		if ((NULL != wTmp) && (1 == flag)) {
			if (0 == strncmp(key, sLine, strlen(key))) { // �������ļ���ȡ��Ϊ׼  
				sLine[strlen(sLine) - 1] = '\0';
				fclose(fp);
				while (*(wTmp + 1) == ' ') {
					wTmp++;
				}
				strcpy(buf, wTmp + 1);
				return 0;
			}
		}
		else {
			if (0 == strncmp(sTitle, sLine, strlen(sTitle))) { // �������ļ���ȡ��Ϊ׼  
				flag = 1; // �ҵ�����λ��  
			}
		}
	}
	fclose(fp);
	return -1;
}

/*
	* ��������         PutIniKeyString
	* ��ڲ�����        title
	*                      �����ļ���һ�����ݵı�ʶ
	*                  key
	*                      ����������Ҫ������ֵ�ı�ʶ
	*                  val
	*                      ���ĺ��ֵ
	*                  filename
	*                      Ҫ��ȡ���ļ�·��
	* ����ֵ��         �ɹ�����  0
	*                  ���򷵻� -1
	*/
int PutIniKeyString(char *title, char *key, char *val, char *filename)
{
	FILE *fpr, *fpw;
	int  flag = 0;
	char sLine[1024], sTitle[32], *wTmp;
	sprintf(sTitle, "[%s]", title);

	if (NULL == (fpr = fopen(filename, "r")))
		return -1;// ��ȡԭ�ļ�  
	sprintf(sLine, "%s.tmp", filename);
	if (NULL == (fpw = fopen(sLine, "w")))
		return -1;// д����ʱ�ļ�        
	while (NULL != fgets(sLine, 1024, fpr)) {
		if (2 != flag) { // ����ҵ�Ҫ�޸ĵ���һ�У��򲻻�ִ���ڲ��Ĳ���  
			wTmp = strchr(sLine, '=');
			if ((NULL != wTmp) && (1 == flag)) {
				if (0 == strncmp(key, sLine, strlen(key))) { // �������ļ���ȡ��Ϊ׼ 
					flag = 2;// ����ֵ������д���ļ�  
					sprintf(wTmp + 1, " %s\n", val);
					//sprintf_s(wTmp + 1, strlen(val) + 1, " %s\n", val);

				}
			}
			else {
				if (0 == strncmp(sTitle, sLine, strlen(sTitle))) { // �������ļ���ȡ��Ϊ׼
					flag = 1; // �ҵ�����λ��  
				}
			}
		}
		fputs(sLine, fpw); // д����ʱ�ļ� 
	}
	fclose(fpr);
	fclose(fpw);
	sprintf(sLine, "%s.tmp", filename);
	//sprintf_s(sLine + 1, strlen(filename) + 1, " %s\n", filename);
	return rename(sLine, filename);// ����ʱ�ļ����µ�ԭ�ļ�  
}

int GetIniAllKeyString(char *title, int ecu_num, char *filename, struct DIAG_info *DIAG)
{
	FILE *fp;
	int  flag = 0;
	char sTitle[64], *wTmp;
	int bTmp, valStart, keyStop;
	char sLine[1024];
	int did_num = 0;
	sprintf(sTitle, "[%s]", title);
	if (NULL == (fp = fopen(filename, "r"))) {
		perror("fopen");
		return -1;
	}
	while (NULL != fgets(sLine, 1024, fp)) {
		// ����ע����  
		if (0 == strncmp("//", sLine, 2)) continue;
		if ('#' == sLine[0])              continue;

		//�����Ϣ
		if ('-' == sLine[0])              continue;
		wTmp = strchr(sLine, '=');
		if (1 == flag) {
			if (NULL != wTmp)
			{
				sLine[strlen(sLine) - 1] = '\0';

				bTmp = wTmp - sLine;
				valStart = bTmp;
				keyStop = bTmp;
				while (*(wTmp + 1) == ' ') {
					wTmp++;
					valStart++;
				}
				valStart++;
				for (int i = 0; i < strlen(sLine) - valStart; i++)
				{
					(*DIAG).ECU_DID[ecu_num].DID_list[did_num].did[i] = sLine[i + valStart];
				}
				(*DIAG).ECU_DID[ecu_num].DID_list[did_num].did[strlen(sLine) - valStart] = '\0';

				wTmp = strchr(sLine, '=');

				while (*(wTmp - 1) == ' ') {
					wTmp--;
					keyStop--;
				}

				for (int i = 0; i < keyStop; i++)
				{
					(*DIAG).ECU_DID[ecu_num].DID_list[did_num].did_name[i] = sLine[i];
				}
				(*DIAG).ECU_DID[ecu_num].DID_list[did_num].did_name[keyStop] = '\0';
				//printf("ecu %d,%s=%s\n", ecu_num,(*DIAG).ECU_DID[ecu_num].DID_list[did_num].did_name, (*DIAG).ECU_DID[ecu_num].DID_list[did_num].did);
				did_num++;
				(*DIAG).ECU_DID[ecu_num].DID_num = did_num;

			}
			else if ('[' == sLine[0] && ']' == sLine[strlen(sLine) - 2])
			{

				fclose(fp);
				return 0;
			}



		}
		else {
			if (0 == strncmp(sTitle, sLine, strlen(sTitle))) { // �������ļ���ȡ��Ϊ׼  
				flag = 1; // �ҵ�����λ��  
			}
		}
	}
	fclose(fp);
	return -1;
}
int GetIniECUlist(char *filename, struct DIAG_info *DIAG)
{
	FILE *fp;
	int  flag = 0;
	char sTitle[64], *wTmp;
	char sLine[1024];
	//sprintf(sTitle, "[%s]", title);
	int ECU_num = 0;
	int structlen;
	if (NULL == (fp = fopen(filename, "r"))) {
		perror("fopen");
		return -1;
	}
	while (NULL != fgets(sLine, 1024, fp)) {
		// ����ע����  
		if (0 == strncmp("//", sLine, 2)) continue;
		if ('#' == sLine[0])              continue;

		//printf("list_%c\n", sLine[0]);
		if ('[' == sLine[0] && ']' == sLine[strlen(sLine) - 2])
		{
			for (int i = 0; i < strlen(sLine) - 3; i++)
			{
				(*DIAG).ECU_list[ECU_num].ECU_name[i] = sLine[i + 1];
			}

			(*DIAG).ECU_list[ECU_num].ECU_name[strlen(sLine) - 3] = '\0';

			ECU_num = ECU_num + 1;

			continue;
		}

	}
	(*DIAG).ECU_num = ECU_num;
	fclose(fp);
	return 1;
}
int GetIniDIAGKeyString(char *title, char *key, char *filename, char *buf)
{
	FILE *fp;
	int  flag = 0;
	char sTitle[64], *wTmp;
	char sLine[1024];
	sprintf(sTitle, "[%s]", title);

	if (NULL == (fp = fopen(filename, "r"))) {
		perror("fopen");
		return -1;
	}
	while (NULL != fgets(sLine, 1024, fp)) {
		// ����ע����  
		if (0 == strncmp("//", sLine, 2)) continue;
		if ('#' == sLine[0])              continue;

		wTmp = strchr(sLine, '=');
		if ((NULL != wTmp) && (1 == flag)) {
			if (0 == strncmp(key, sLine, strlen(key))) { // �������ļ���ȡ��Ϊ׼  
				sLine[strlen(sLine) - 1] = '\0';
				fclose(fp);
				while (*(wTmp + 1) == ' ') {
					wTmp++;
				}
				strcpy(buf, wTmp + 1);
				return 0;
			}
		}
		else {
			if (0 == strncmp(sTitle, sLine, strlen(sTitle))) { // �������ļ���ȡ��Ϊ׼  
				flag = 1; // �ҵ�����λ��  
			}
		}
	}
	fclose(fp);
	return -1;
}




int initDIAG(char *filename, struct DIAG_info *DIAG)
{
	//fopen("test.ini", "w");
	int ret = 0;
	ret = GetIniECUlist(filename, DIAG);
	if (ret == 1)
	{
		for (int i = 0; i < (*DIAG).ECU_num; i++)
		{
			GetIniDIAGKeyString((*DIAG).ECU_list[i].ECU_name, "-DIAG_REQ_PHY_ID", filename, (*DIAG).ECU_list[i].DIAG_REQ_PHY_ID);
			GetIniDIAGKeyString((*DIAG).ECU_list[i].ECU_name, "-DIAG_REQ_FUNC_ID", filename, (*DIAG).ECU_list[i].DIAG_REQ_FUNC_ID);
			GetIniDIAGKeyString((*DIAG).ECU_list[i].ECU_name, "-DIAG_RESP_ID", filename, (*DIAG).ECU_list[i].DIAG_RESP_ID);
			GetIniDIAGKeyString((*DIAG).ECU_list[i].ECU_name, "-SecurityAccessDLL", filename, (*DIAG).ECU_list[i].SecurityAccessDLL);
			GetIniAllKeyString((*DIAG).ECU_list[i].ECU_name, i, filename, DIAG);


		}
	}
	else
	{
		FILE *fp;
		fp = fopen(filename, "w");
		fprintf(fp, "[ECU1]\n");
		fprintf(fp, "//�����Ϣ\n");
		fprintf(fp, "-DIAG_REQ_PHY_ID =  0721\n");
		fprintf(fp, "-DIAG_RESP_ID = 07A1\n");
		fprintf(fp, "-DIAG_REQ_FUNC_ID = 07DF\n");
		fprintf(fp, "-SecurityAccessDLL = NULL\n");
		fprintf(fp, "//DID\n");
		fprintf(fp, "ReadSWVersion = 22f195\n");
		fprintf(fp, "[ECU2]\n");
		fprintf(fp, "-DIAG_REQ_PHY_ID =  0722\n");
		fprintf(fp, "-DIAG_RESP_ID = 07A2\n");
		fprintf(fp, "-DIAG_REQ_FUNC_ID = 07DF\n");
		fprintf(fp, "-SecurityAccessDLL = SeednKeyML\n");
		fprintf(fp, "��ȡ = 22f197\n");
		fclose(fp);
		return -1;

	}

	return 1;
}
