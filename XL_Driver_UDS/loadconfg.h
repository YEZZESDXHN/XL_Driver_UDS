#pragma once
/*ini.h*/
#ifndef INI_H
#define INI_H

#include <stdio.h>  
//#include <string.h>

struct DID_list
{
	char did_name[128];
	char did[4096];
};
struct ECU_DID
{
	int DID_num;
	struct DID_list DID_list[128];
};
//struct DID_list DID_info;
struct ECU_list
{
	char ECU_name[128];
	char DIAG_REQ_PHY_ID[16];
	char DIAG_RESP_ID[16];
	char DIAG_REQ_FUNC_ID[16];
	char SecurityAccessDLL[128];
};

struct DIAG_info
{
	int ECU_num;
	int LoadECU[128];
	struct ECU_list ECU_list[128];
	struct ECU_DID ECU_DID[128];

};
struct DIAG_info gDiag_info;



int GetIniKeyString(char *title, char *key, char *filename, char *buf);

int PutIniKeyString(char *title, char *key, char *val, char *filename);

int GetIniAllKeyString(char *title, int ecu_num, char *filename, struct DIAG_info *DIAG);

int GetIniECUlist(char *filename, struct DIAG_info *DIAG);

int initDIAG(char *filename, struct DIAG_info *DIAG);

int GetIniDIAGKeyString(char *title, char *key, char *filename, char *buf);








#endif /*INI_H*/