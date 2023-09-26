#ifndef _UDS_TP_H_
#define _UDS_TP_H_
#include<stdint.h>
typedef enum __NT_TIMER_T__
{
	TIMER_N_CR = 0,                 // N_CR ��ʱ�������շ��յ�����֡���ʱ�䲻�ܴ��� TIMEOUT_N_CR����λ: ms
	TIMER_N_BS,                     // N_BS ��ʱ�������ͷ����������֡�󵽽��յ�����֮֡���ʱ�䲻�ܴ��� TIMEOUT_N_BS����λ: ms
	TIMER_STmin,                    // STmin ��ʱ������������֡ʱ�����ʱ����СΪ g_rfc_stmin,��λ: ms
	TIMER_CNT                       // ��ʱ���ܸ���
}nt_timer_t;

typedef enum __NETWORK_LAYER_STATUS_
{
	NWL_IDLE = 0,                   // ����״̬
	NWL_XMIT,                       // ����״̬
	NWL_RECV,                       // ����״̬
	NWL_CNT                         // ״̬����
}network_layer_st;


typedef enum __NETWORK_PCI_TYPE_
{
	PCI_SF = 0,                     // ��֡
	PCI_FF,                         // ��֡
	PCI_CF,                         // ����֡
	PCI_FC                          // ����֡
}network_pci_type_t;


typedef enum __NETWORK_FLOW_STATUS__
{
	FS_CTS = 0,                    // �����������
	FS_WT,                         // �ȴ�
	FS_OVFLW,                      // ���
	FS_RESERVED                    // �Ƿ�
}network_flow_status_t;
//typedef int(*DLL_FUNCTION_GenerateKeyEx) (const unsigned char*, unsigned int, const unsigned int, const char*, unsigned char*, unsigned int, unsigned int*);
typedef void(*UDS_SEND_FRAME)(unsigned short, unsigned char*, unsigned short);


//uint16_t REQUEST_ID = 0x724;			// ���� ID
//uint16_t FUNCTION_ID = 0x7DF;			// ���� ID
//uint16_t RESPONSE_ID = 0x7A4;			// Ӧ�� ID

// // ���ֵ��������͵���Ч���ݲ���һ֡�����ø�ֵ���
// #define PADDING_VAL                 (0x55)

// ����֡����Ϊ��֡ 
#define NT_SET_PCI_TYPE_SF(low)     (0x00 | (low & 0x0f))

// ����֡����Ϊ��֡
#define NT_SET_PCI_TYPE_FF(low)     (0x10 | (low & 0x0f))

// ����֡����Ϊ����֡
#define NT_SET_PCI_TYPE_CF(low)     (0x20 | (low & 0x0f))

// ����֡����Ϊ����֡
#define NT_SET_PCI_TYPE_FC(low)     (0x30 | (low & 0x0f))

// ��ȡ֡����
#define NT_GET_PCI_TYPE(n_pci)      (n_pci >> 4)

// ��ȡ��֡����
#define NT_GET_SF_DL(n_pci)         (0x0f & n_pci)

// ��ȡ����֡֡���
#define NT_GET_CF_SN(n_pci)         (0x0f & n_pci)

// ��ȡ��״̬
#define NT_GET_FC_FS(n_pci)         (0x0f & n_pci)

// ����������֡�ĸ�������Ϊ 0�����ʾ���ͷ�����һֱ�����Ʒ�������֡��ֱ������������е�����֡
// ����Ϊ 0�����ʾ�����ͷ����͵�����֡����Ϊ NT_XMIT_FC_BS ����ȴ����շ��ظ�һ֡����֡�����ͷ���������֡�����������ķ������
#define NT_XMIT_FC_BS               (0)

// ֪ͨ���ͷ���������֡��֡�����Сʱ�䣬��λ: ms
#define NT_XMIT_FC_STMIN            (0x0A)

// ���շ��յ�����֡���ʱ�䲻�ܴ��� TIMEOUT_N_CR����λ: ms
#define TIMEOUT_N_CR                (1000)

// ���ͷ����������֡�󵽽��յ�����֮֡���ʱ�䲻�ܴ��� TIMEOUT_N_BS����λ: ms
#define TIMEOUT_N_BS                (1000)

#define FRAME_SIZE      8               // ֡����



#define REQUEST_ID      0x123           // ���� ID
#define FUNCTION_ID     0x7DF           // ���� ID
#define RESPONSE_ID     0x456           // Ӧ�� ID

#define UDS_RX_MAX      1024            // ���ջ��������� --UDS TP ��Э��涨���֧�� 4095 �ֽڣ����ǿ��Ը���ʵ����Ҫ�Ķ���������Դ�˷�
#define UDS_TX_MAX      128             // ���ͻ��������� --UDS TP ��Э��涨���֧�� 4095 �ֽڣ����ǿ��Ը���ʵ����Ҫ�Ķ���������Դ�˷�

#define PADDING_VAL     0xAA            // ���ֵ��������͵���Ч���ݲ���һ֡�����ø�ֵ���

#ifndef TRUE
#define	TRUE	(1)
#endif


#ifndef FALSE
#define FALSE	(0)
#endif


#ifndef NULL
#define NULL  ((void *) 0)
#endif

typedef unsigned char		bool_t;







/******************************************************************************
* ��������: static int recv_singleframe (uint8_t* frame_buf, uint8_t frame_dlc)
* ����˵��: ����֡
* �������: uint8_t*    frame_buf       --���ձ���֡�����׵�ַ
	��������uint8_t     frame_dlc       --���ձ���֡���ݳ���
* �������: ��
* ��������: 0: OK; -1: ERR
* ����˵��: ��
******************************************************************************/
int recv_singleframe(uint8_t* frame_buf, uint8_t frame_dlc);



/******************************************************************************
* ��������: static int recv_firstframe(uint8_t* frame_buf, uint8_t frame_dlc)
* ����˵��: ������֡
* �������: uint8_t*    frame_buf       --���ձ���֡�����׵�ַ
	��������uint8_t     frame_dlc       --���ձ���֡���ݳ���
	��������UDS_SEND_FRAME sendframefun --���ͱ��ĺ���ָ�룬Ӳ�������ӿں���
* �������: ��
* ��������: 0: OK; other: -ERR
* ����˵��: ��
******************************************************************************/
int recv_firstframe(UDS_SEND_FRAME sendframefun, uint8_t* frame_buf, uint8_t frame_dlc);



/******************************************************************************
* ��������: static void send_flowcontrol(network_flow_status_t flow_st)
* ����˵��: ��������֡
* �������: network_flow_status_t   flow_st       --��״̬
* �������: ��
* ��������: 0: ��������֡�������; 1: ��������; other: -ERR
* ����˵��: ��
******************************************************************************/
void send_flowcontrol(UDS_SEND_FRAME sendframefun, network_flow_status_t flow_st);


/******************************************************************************
* ��������: static int send_singleframe (uint8_t* msg_buf, uint16_t msg_dlc)
* ����˵��: ���͵�֡
* �������: uint8_t*    msg_buf         --���������׵�ַ
	��������uint8_t     msg_dlc         --�������ݳ���
* �������: ��
* ��������: 0: OK; -1: ERR
* ����˵��: ��
******************************************************************************/
int send_singleframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint8_t msg_dlc);






#endif