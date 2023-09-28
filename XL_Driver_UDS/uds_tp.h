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


typedef enum _N_TATYPE_T_
{
	N_TATYPE_NONE = 0,                  // none
	N_TATYPE_PHYSICAL,                  // ����Ѱַ
	N_TATYPE_FUNCTIONAL                 // ����Ѱַ
}n_tatype_t;

typedef enum _N_RESULT_
{
	N_OK = 0,
	N_TIMEOUT_Bs,                       // TIMER_N_BS ��ʱ����ʱ
	N_TIMEOUT_Cr,                       // TIMER_N_CR ��ʱ����ʱ
	N_WRONG_SN,                         // ���յ�������֡֡��Ŵ���
	N_INVALID_FS,                       // ���յ�������֡����״̬�Ƿ�
	N_UNEXP_PDU,                        // �����ڴ���֡���ͣ������ڽ�������֡��Ī���յ���֡
	N_BUFFER_OVFLW,                     // ���յ�������֡����״̬Ϊ���
}n_result_t;


//�ϲ��� TP ��ע��ķ��ͱ��Ľӿں���
typedef int(*UDS_SEND_FRAME)(unsigned short, unsigned char*, unsigned short);

// �ϲ��� TP ��ע���һЩ�ӿں������� TP ����������괦�����ͨ����Щ�ӿں��������ݽ����ϲ��������
typedef void(*ffindication_func) (n_result_t n_result);
typedef void(*indication_func) (uint8_t* msg_buf, uint16_t msg_dlc, n_result_t n_result);
typedef void(*confirm_func) (n_result_t n_result);

typedef struct _NETWORK_USER_DATA_T_
{
	ffindication_func   ffindication;
	indication_func     indication;
	confirm_func        confirm;
}nt_usdata_t;

// 0:����Ѱַ; 1:����Ѱַ
extern uint8_t g_tatype;




extern uint16_t REQUEST_ID;			// ���� ID
extern uint16_t FUNCTION_ID;			// ���� ID
extern uint16_t RESPONSE_ID;			// Ӧ�� ID

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



//#define REQUEST_ID      0x123           // ���� ID
//#define FUNCTION_ID     0x7DF           // ���� ID
//#define RESPONSE_ID     0x456           // Ӧ�� ID

#define UDS_RX_MAX      1024            // ���ջ��������� --UDS TP ��Э��涨���֧�� 4095 �ֽڣ����ǿ��Ը���ʵ����Ҫ�Ķ���������Դ�˷�
#define UDS_TX_MAX      128             // ���ͻ��������� --UDS TP ��Э��涨���֧�� 4095 �ֽڣ����ǿ��Ը���ʵ����Ҫ�Ķ���������Դ�˷�

#define PADDING_VAL     0x55            // ���ֵ��������͵���Ч���ݲ���һ֡�����ø�ֵ���

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
* ��������: static void nt_timer_start(nt_timer_t num)
* ����˵��: ���� TP �㶨ʱ��
* �������: nt_timer_t num              --��ʱ��
* �������: ��
* ��������: ��
* ����˵��: ��
******************************************************************************/
static void nt_timer_start(nt_timer_t num);


/******************************************************************************
* ��������: static void nt_timer_start_wv(nt_timer_t num, uint32_t value)
* ����˵��: �������ö�ʱ���ļ���ֵ
* �������: nt_timer_t num              --��ʱ��
	��������uint32_t value              --����ֵ
* �������: ��
* ��������: ��
* ����˵��: �� value = 0����ʾ�رն�ʱ������ͬ�� nt_timer_stop() ����
	���������� value = 1����ʾ��ʱ����ʱ�ѷ�����������������ʱ�¼�
	���������� value Ϊ����ֵʱ����ʱ�������� value ֵ���¿�ʼ��ʱ
******************************************************************************/
static void nt_timer_start_wv(nt_timer_t num, uint32_t value);


/******************************************************************************
* ��������: static void nt_timer_stop (nt_timer_t num)
* ����˵��: �رն�ʱ��
* �������: nt_timer_t num              --��ʱ��
* �������: ��
* ��������: ��
* ����˵��: ��
******************************************************************************/
static void nt_timer_stop(nt_timer_t num);

/******************************************************************************
* ��������: static int nt_timer_run(nt_timer_t num)
* ����˵��: ��ʱ����������
* �������: nt_timer_t num              --��ʱ��
* �������: ��
* ��������: 0: ��ʱ���Ѿ����ر�; -1: ��ʱ����; 1: ��ʱ�����ڼ�ʱ����
* ����˵��: �ú�����Ҫ�� 1ms ���ڵ���
******************************************************************************/
static int nt_timer_run(nt_timer_t num);


/******************************************************************************
* ��������: static int nt_timer_chk(nt_timer_t num)
* ����˵��: ��鶨ʱ��״̬Ȼ��رն�ʱ��
* �������: nt_timer_t num              --��ʱ��
* �������: ��
* ��������: 0: ��ʱ����ֹͣ����;  1: ��ʱ�����ڼ�ʱ����
* ����˵��: �ú���ִ�к����۶�ʱ���Ƿ��������У��������ر�
******************************************************************************/
static int nt_timer_chk(nt_timer_t num);


/******************************************************************************
* ��������: static void clear_network(void)
* ����˵��: ��λ�����״̬
* �������: ��
* �������: ��
* ��������: ��
* ����˵��: ��Ҫ�Ǹ�λ����״̬���������ҹر�������ض�ʱ��
******************************************************************************/
static void clear_network(void);



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
* ��������: recv_firstframe(UDS_SEND_FRAME sendframefun, uint8_t* frame_buf, uint8_t frame_dlc)
* ����˵��: ������֡
* �������: UDS_SEND_FRAME sendframefun --���ͱ��ĺ���ָ�룬Ӳ�������ӿں���
			uint8_t*    frame_buf       --���ձ���֡�����׵�ַ
	��������uint8_t     frame_dlc       --���ձ���֡���ݳ���
	��������
* �������: ��
* ��������: 0: OK; other: -ERR
* ����˵��: ��
******************************************************************************/
int recv_firstframe(UDS_SEND_FRAME sendframefun, uint8_t* frame_buf, uint8_t frame_dlc);

/******************************************************************************
* ��������: int recv_consecutiveframe(UDS_SEND_FRAME sendframefun, uint8_t* frame_buf, uint8_t frame_dlc)
* ����˵��: ��������֡
* �������: UDS_SEND_FRAME sendframefun --���ͱ��ĺ���ָ�룬Ӳ�������ӿں���
	��������uint8_t*    frame_buf       --���ձ���֡�����׵�ַ
	��������uint8_t     frame_dlc       --���ձ���֡���ݳ���
* �������: ��
* ��������: 0: ��������֡�������; 1: ��������; other: -ERR
* ����˵��: ��
******************************************************************************/
static int recv_consecutiveframe(UDS_SEND_FRAME sendframefun, uint8_t* frame_buf, uint8_t frame_dlc);

/******************************************************************************
* ��������: static int recv_flowcontrolframe(uint8_t* frame_buf, uint8_t frame_dlc)
* ����˵��: ��������֡
* �������: uint8_t*    frame_buf       --���ձ���֡�����׵�ַ
	��������uint8_t     frame_dlc       --���ձ���֡���ݳ���
* �������: ��
* ��������: 0: ��������֡�������; 1: ��������; other: -ERR
* ����˵��: ��
******************************************************************************/
static int recv_flowcontrolframe(uint8_t* frame_buf, uint8_t frame_dlc);


/******************************************************************************
* ��������: void send_flowcontrol(UDS_SEND_FRAME sendframefun, network_flow_status_t flow_st)
* ����˵��: ��������֡
* �������: UDS_SEND_FRAME sendframefun --���ͱ��ĺ���ָ�룬Ӳ�������ӿں���
	��������network_flow_status_t   flow_st       --��״̬
* �������: ��
* ��������: 0: ��������֡�������; 1: ��������; other: -ERR
* ����˵��: ��
******************************************************************************/
void send_flowcontrol(UDS_SEND_FRAME sendframefun, network_flow_status_t flow_st);

/******************************************************************************
* ��������: int send_singleframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint8_t msg_dlc)
* ����˵��: ���͵�֡
* �������: UDS_SEND_FRAME sendframefun --���ͱ��ĺ���ָ�룬Ӳ�������ӿں���
	��������uint8_t*    msg_buf         --���������׵�ַ
	��������uint8_t     msg_dlc         --�������ݳ���
* �������: ��
* ��������: 0: OK; -1: ERR
* ����˵��: ��
******************************************************************************/
int send_singleframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint8_t msg_dlc);


/******************************************************************************
* ��������: static int send_firstframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc)
* ����˵��: ������֡
* �������: UDS_SEND_FRAME sendframefun --���ͱ��ĺ���ָ�룬Ӳ�������ӿں���
	��������uint8_t*    msg_buf         --���������׵�ַ
	��������uint8_t     msg_dlc         --�������ݳ���
* �������: ��
* ��������: �ɹ����͵���Ч���ݳ���
* ����˵��: ��
******************************************************************************/
static int send_firstframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc);

/******************************************************************************
* ��������: static int send_consecutiveframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc, uint8_t frame_sn)
* ����˵��: ��������֡
* �������: UDS_SEND_FRAME sendframefun --���ͱ��ĺ���ָ�룬Ӳ�������ӿں���
	��������uint8_t*    msg_buf         --���������׵�ַ
	��������uint8_t     msg_dlc         --�������ݳ���
	��������uint8_t     frame_sn        --����֡֡���
* �������: ��
* ��������: �ɹ����͵���Ч���ݳ���
* ����˵��: ��
******************************************************************************/
static int send_consecutiveframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc, uint8_t frame_sn);



/******************************************************************************
* ��������: static int send_multipleframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc)
* ����˵��: ���Ͷ�֡
* �������: UDS_SEND_FRAME sendframefun --���ͱ��ĺ���ָ�룬Ӳ�������ӿں���
	��������uint8_t*    msg_buf         --���������׵�ַ
	��������uint8_t     msg_dlc         --�������ݳ���
* �������: ��
* ��������: 0: OK; -1: ERR
* ����˵��: ��
******************************************************************************/
static int send_multipleframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc);


/******************************************************************************
* ��������: int network_send_udsmsg(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc)
* ����˵��: ��������
* �������: UDS_SEND_FRAME sendframefun --���ͱ��ĺ���ָ�룬Ӳ�������ӿں���
	��������uint8_t*    msg_buf         --���������׵�ַ
	��������uint8_t     msg_dlc         --�������ݳ���
* �������: ��
* ��������: 0: OK; -1: ERR
* ����˵��: TP �����ϲ��ṩ�����ݷ��ͽӿ�
******************************************************************************/
int network_send_udsmsg(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc);


/******************************************************************************
* ��������: void network_task(UDS_SEND_FRAME sendframefun)
* ����˵��: TP ��������
* �������: UDS_SEND_FRAME sendframefun --���ͱ��ĺ���ָ�룬Ӳ�������ӿں���
* �������: ��
* ��������: ��
* ����˵��: �ú�����Ҫ�� 1ms ���ڵ���
******************************************************************************/
void network_task(UDS_SEND_FRAME sendframefun);



/******************************************************************************
* ��������: void uds_tp_recv_frame(UDS_SEND_FRAME sendframefun, uint8_t func_addr, uint8_t* frame_buf, uint8_t frame_dlc)
* ����˵��: ���յ�һ֡���Ĳ�����
* �������: UDS_SEND_FRAME sendframefun --���ͱ��ĺ���ָ�룬Ӳ�������ӿں���
	��������uint8_t     func_addr       --0:����Ѱַ; 1:����Ѱַ
	��������uint8_t*    frame_buf       --���ձ���֡�����׵�ַ
	��������uint8_t     frame_dlc       --���ձ���֡���ݳ���
* �������: ��
* ��������: ��
* ����˵��: frame_dlc ���ȱ������ FRAME_SIZE�������ж�Ϊ��Ч֡
******************************************************************************/
void uds_tp_recv_frame(UDS_SEND_FRAME sendframefun, uint8_t func_addr, uint8_t* frame_buf, uint8_t frame_dlc);



///******************************************************************************
//* ��������: int network_reg(nt_usdata_t* usdata)
//* ����˵��: �ϲ��� TP ��ע��һЩ�ӿں������� TP ����������괦�����ͨ����Щ�ӿں��������ݽ����ϲ��������
//* �������: nt_usdata_t* usdata      --�ϲ�ӿں���
//* �������: ��
//* ��������: 0: OK; -1: ERR
//* ����˵��: ָʾ����Indication������������ϲ��Ӧ�ò㴫��״̬��Ϣ�����յ�������
//	��������ȷ�Ϸ���Confirm������������ϲ��Ӧ�ò㴫��״̬��Ϣ
//	���������������Request���������ϲ�������㴫�ݿ��Ʊ�����Ϣ��Ҫ���͵�����
//******************************************************************************/
//int network_reg(nt_usdata_t* usdata);





#endif