#ifndef _UDS_TP_H_
#define _UDS_TP_H_
#include<stdint.h>
#include"SID34_36_37TransferData.h"
typedef enum __NT_TIMER_T__
{
	TIMER_N_CR = 0,                 // N_CR ��ʱ�������շ��յ�����֡���ʱ�䲻�ܴ��� TIMEOUT_N_CR����λ: ms
	TIMER_N_BS,                     // N_BS ��ʱ�������ͷ����������֡�󵽽��յ�����֮֡���ʱ�䲻�ܴ��� TIMEOUT_N_BS����λ: ms
	TIMER_STmin,                    // STmin ��ʱ������������֡ʱ�����ʱ����СΪ g_rfc_stmin,��λ: ms
	TIMER_Response,
	TIMER_CNT                       // ��ʱ���ܸ���
}nt_timer_t;

typedef enum __NETWORK_LAYER_STATUS_
{
	NWL_IDLE = 0,                   // ����״̬
	NWL_XMIT,                       // ����״̬
	NWL_RECV,                       // ����״̬
	//NWL_DL,							// ����״̬��ˢд
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
	FS_CTS = 0,                    // ������������
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

typedef enum _N_RX_RESULT_
{
	
	N_OK = 0,
	N_TIMEOUT_Bs,                       // TIMER_N_BS ��ʱ����ʱ
	N_TIMEOUT_Cr,                       // TIMER_N_CR ��ʱ����ʱ
	N_TIMEROUT_Respone,
	N_WRONG_SN,                         // ���յ�������֡֡��Ŵ���
	N_INVALID_FS,                       // ���յ�������֡����״̬�Ƿ�
	N_UNEXP_PDU,                        // �����ڴ���֡���ͣ������ڽ�������֡��Ī���յ���֡
	N_BUFFER_OVFLW,                     // ���յ�������֡����״̬Ϊ���
	N_FF_MSG,							//���յ���֡
	/*TX*/
	N_TX_OK,
}n_result_t;



//�ϲ��� TP ��ע��ķ��ͱ��Ľӿں���
typedef int(*UDS_SEND_FRAME)(unsigned int, unsigned char*, unsigned char);

// �ϲ��� TP ��ע���һЩ�ӿں������� TP ����������괦������ͨ����Щ�ӿں��������ݽ����ϲ��������
//typedef void(*indication_func) (uint8_t* msg_buf, uint16_t msg_dlc, n_result_t n_result);


/******************************************************************************
* ��������: void uds_dataff_indication(uint8_t* msg_buf, uint16_t msg_dlc, n_result_t n_result);
* ����˵��: �ϲ�ص�����
* �������: 
* �������: 
* ��������: ��
* ����˵��: ��
******************************************************************************/
void uds_data_indication(uint8_t* msg_buf, uint16_t msg_dlc, n_result_t n_result);

extern unsigned int task_cycle;
// 0:����Ѱַ; 1:����Ѱַ
extern uint8_t g_tatype;

extern unsigned int task_cycle;//��λ΢��(us)��1000��ʾ1ms

//#define REQUEST_ID 0x724
//#define FUNCTION_ID 0x7df
//#define RESPONSE_ID 0x7a4
extern unsigned int REQUEST_ID;			// ���� ID
extern unsigned int FUNCTION_ID;			// ���� ID
extern unsigned int RESPONSE_ID;			// Ӧ�� ID

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

// ������������֡�ĸ�������Ϊ 0�����ʾ���ͷ�����һֱ�����Ʒ�������֡��ֱ������������е�����֡
// ����Ϊ 0�����ʾ�����ͷ����͵�����֡����Ϊ NT_XMIT_FC_BS ����ȴ����շ��ظ�һ֡����֡�����ͷ���������֡�����������ķ������
#define NT_XMIT_FC_BS               (0)

// ֪ͨ���ͷ���������֡��֡�����Сʱ�䣬��λ: ms
#define NT_XMIT_FC_STMIN            (0x0A*1000)

// ���շ��յ�����֡���ʱ�䲻�ܴ��� TIMEOUT_N_CR����λ: ms
#define TIMEOUT_N_CR                (1000*1000)

// ���շ��յ���Ӧʱ�䲻�ܴ��� TIMEOUT_N_Response����λ: ms
#define TIMEOUT_Response                (1000*1000)

// ���ͷ����������֡�󵽽��յ�����֮֡���ʱ�䲻�ܴ��� TIMEOUT_N_BS����λ: ms
#define TIMEOUT_N_BS                (1000*1000)

#define FRAME_SIZE      8               // ֡����



//#define REQUEST_ID      0x123           // ���� ID
//#define FUNCTION_ID     0x7DF           // ���� ID
//#define RESPONSE_ID     0x456           // Ӧ�� ID

#define UDS_RX_MAX      1024            // ���ջ��������� --UDS TP ��Э��涨���֧�� 4095 �ֽڣ����ǿ��Ը���ʵ����Ҫ�Ķ���������Դ�˷�
#define UDS_TX_MAX      0x802             // ���ͻ��������� --UDS TP ��Э��涨���֧�� 4095 �ֽڣ����ǿ��Ը���ʵ����Ҫ�Ķ���������Դ�˷�

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


/*----------------------------------------------------------------------------------------------*/
typedef enum __UDS_NRC_ENUM__
{
	NRC_GENERAL_REJECT = 0x10, // �÷�����Ӧ����Э������֧�ֵ�
	NRC_SERVICE_NOT_SUPPORTED = 0x11,	// ECU ѹ����û���������
	NRC_SUBFUNCTION_NOT_SUPPORTED = 0x12,	// ECU ��֧�ֵ�ǰ������ӹ���
	NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT = 0x13, // �����ĵĳ��Ȼ��߸�ʽ����ȷ
	NRC_CONDITIONS_NOT_CORRECT = 0x22, // �Ⱦ�����������
	NRC_REQUEST_SEQUENCE_ERROR = 0x24, // �����ĵ�˳����ȷ
	NRC_REQUEST_OUT_OF_RANGE = 0x31, // ����������Χ/���� ID ��֧��
	NRC_SECURITY_ACCESS_DENIED = 0x33, // �����㰲ȫ���ԣ����Ƚ���
	NRC_INVALID_KEY = 0x35, // ��Կ��ƥ��
	NRC_EXCEEDED_NUMBER_OF_ATTEMPTS = 0x36, // ���Խ��������Ѵ�����
	NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED = 0x37, // ��ȫ����ʧ�ܣ���ʱʱ��δ��
	NRC_UPLOAD_DOWNLOAD_NOT_ACCEPTED = 0x70, // �������ϴ�/����
	NRC_TRANSFER_DATA_SUSPENDED = 0x71, // ���ݴ�����ֹ
	NRC_GENERAL_PROGRAMMING_FAILURE = 0x72, // ��������д�ڴ�ʱ����
	NRC_WRONG_BLOCK_SEQUENCE_COUNTER = 0x73, // �����м�������
	NRC_SERVICE_BUSY = 0x78, // ����ȷ����������Ϣ��������Щ�ظ�
	NRC_SUBFUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION = 0x7E, // ��ǰ�Ự�£����ӹ��ܲ�֧��
	NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION = 0x7F, // ��ǰ�Ự�£��÷���֧��
	NRC_VOLTAGE_TOO_HIGH = 0x92, // ��ѹ����
	NRC_VOLTAGE_TOO_LOW = 0x93, // ��ѹ����
}uds_nrc_em;


// �ڶ����ֽ��е����λ bit7 ��ʾ�϶���Ӧ����λ
// ������ 1 ʱ�����ʾ����Ҫ�ظ��϶���Ӧ��ֻ���з���������
#define UDS_GET_SUB_FUNCTION_SUPPRESS_POSRSP(byte)    ((byte >> 7u)&0x01u)

// ��ȡ�ӹ��ܺ� - �ڶ����ֽ��еĵ� 7 λ��ʾ�ӹ��ܺţ���Χ��0 ~ 0x7F
#define UDS_GET_SUB_FUNCTION(byte)     (byte & 0x7fu)

// �϶���Ӧ������ ID �� +0x40
#define POSITIVE_RSP 			0x40
#define USD_GET_POSITIVE_RSP(server_id)         (POSITIVE_RSP + server_id)

// ����Ӧ
#define NEGATIVE_RSP 			0x7F

// ��ȫ���ʳ�ʱʱ�䣬��λ��ms�������ȫ��������ƥ������ﵽ����ʱ�������ö�ʱ������ TIMEOUT_FSA ʱ��������յ��������ӷ�������Ҫ�ظ� NRC 37
#define TIMEOUT_FSA          	10000

// S3server ��ʱ����ʱʱ�䣬�ڷ�Ĭ�ϻỰģʽ�£������ TIMEOUT_S3server ʱ����û���յ��κ���Ϣ�Ļ������Զ��ص�Ĭ�ϻỰ����λ��ms
#define TIMEOUT_S3server     5000


typedef enum __UDS_TIMER_T__
{
	UDS_TIMER_FSA = 0,			// FSA ��ʱ���������ȫ��������ƥ������ﵽ����ʱ�������ö�ʱ������ TIMEOUT_FSA ʱ��������յ��������ӷ�������Ҫ�ظ� NRC 37
	UDS_TIMER_S3server,			// S3server ��ʱ�����ڷ�Ĭ�ϻỰģʽ�£������ TIMEOUT_S3server ʱ����û���յ��κ���Ϣ�Ļ������Զ��ص�Ĭ�ϻỰ
	UDS_TIMER_CNT				// Ӧ�ò㶨ʱ���ܸ���
}uds_timer_t;


typedef enum __UDS_SESSION_T_
{
	UDS_SESSION_STD = 1,		// Ĭ�ϻỰ
	UDS_SESSION_PROG,			// ��̻Ự
	UDS_SESSION_EXT				// ��չ�Ự
}uds_session_t;


typedef enum __UDS_SA_LV__
{
	UDS_SA_NON = 0,				// ��ȫ���ʵȼ� - ��
	UDS_SA_LV1,					// ��ȫ���ʵȼ� - 1 ��
	UDS_SA_LV2,					// ��ȫ���ʵȼ� - 2 ��
}uds_sa_lv;


typedef struct __UDS_SERVICE_T__
{
	uint8_t uds_sid;									// ���� ID
	void(*uds_service)  (const uint8_t *, uint16_t);	// ����������
	bool_t(*check_len)  (const uint8_t *, uint16_t);	// ������ݳ����Ƿ�Ϸ�
	bool_t std_spt;   									// �Ƿ�֧��Ĭ�ϻỰ
	bool_t prog_spt;  									// �Ƿ�֧�ֱ�̻Ự
	bool_t ext_spt;   									// �Ƿ�֧����չ�Ự
	bool_t fun_spt;   									// �Ƿ�֧�ֹ���Ѱַ
	bool_t ssp_spt;  									// �Ƿ�֧�ֿ϶���Ӧ����
	uds_sa_lv uds_sa; 									// ��ȫ���ʵȼ�
}uds_service_t;
/*-------------------------------------------------------------------------------------------------------*/


/*-----------------------------------------service_cfg----------------------------------------------------------------*/
#define SID_NUM       16     // ��ǰ��֧�� 16 ������

#define SID_10        (0x10) /* SessionControl */
#define SID_11        (0x11) /* ECUReset */
#define SID_14        (0x14) /* ClearDTC */
#define SID_18        (0x18) /* KWPReadDTC */
#define SID_19        (0x19) /* ReadDTC */
#define SID_22        (0x22) /* ReadID */
#define SID_27        (0x27) /* SecurityAccess */
#define SID_2E        (0x2E) /* WriteID */
#define SID_2F        (0x2F) /* InputOutputControlID */
#define SID_28        (0x28) /* CommunicationControl */
#define SID_31        (0x31) /* RoutineControl */
#define SID_3E        (0x3E) /* TesterPresent */
#define SID_85        (0x85) /* ControlDTCSetting */
#define SID_34        (0x34) /* RequestDownload */
#define SID_36        (0x36) /* TransferData */
#define SID_37        (0x37) /* RequestTransferExit */





/*---------------------------------------------------------------------------------------------------------*/



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
	���������� value = 1����ʾ��ʱ����ʱ�ѷ�������������������ʱ�¼�
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
* ����˵��: ������֡
* �������: uint8_t*    frame_buf       --���ձ���֡�����׵�ַ
	��������uint8_t     frame_dlc       --���ձ���֡���ݳ���
* �������: ��
* ��������: 0: OK; -1: ERR
* ����˵��: ��
******************************************************************************/
int recv_singleframe(UDS_SEND_FRAME sendframefun, uint8_t* frame_buf, uint8_t frame_dlc);


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
int send_firstframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc);

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
int send_multipleframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc);


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
void uds_tp_recv_frame(UDS_SEND_FRAME sendframefun, uint8_t* frame_buf, uint8_t frame_dlc);



///******************************************************************************
//* ��������: int network_reg(nt_usdata_t* usdata)
//* ����˵��: �ϲ��� TP ��ע��һЩ�ӿں������� TP ����������괦������ͨ����Щ�ӿں��������ݽ����ϲ��������
//* �������: nt_usdata_t* usdata      --�ϲ�ӿں���
//* �������: ��
//* ��������: 0: OK; -1: ERR
//* ����˵��: ָʾ����Indication������������ϲ��Ӧ�ò㴫��״̬��Ϣ�����յ�������
//	��������ȷ�Ϸ���Confirm������������ϲ��Ӧ�ò㴫��״̬��Ϣ
//	���������������Request���������ϲ�������㴫�ݿ��Ʊ�����Ϣ��Ҫ���͵�����
//******************************************************************************/
//int network_reg(nt_usdata_t* usdata);


/******************************************************************************
* ��������: int SecurityAccessWithDLL(char *iFilename, const unsigned char iSeed[], unsigned int iSeedSize, const unsigned int iSecurityLevel, unsigned char *oKeyArray, unsigned int iKeyArrayMaxSize,unsigned int *oSize)
* ����˵��: ����Vector Dll����
* �������:  char *iFilename						Dll·��,�ļ���
			const unsigned char iSeed[]			��������
			unsigned int iSeedSize				�������ӳ���
			const unsigned int iSecurityLevel	�����ȼ�

			unsigned int iKeyArrayMaxSize		key������󳤶�

* �������:	unsigned char *oKeyArray			���key
			unsigned int *oSize					���key����
* ��������:	0������GenerateKeyEx�ɹ�����
			1������GenerateKeyExOpt�ɹ�����
			-1:����ʧ��
* ����˵��:
******************************************************************************/
int SecurityAccessWithDLL(char *iFilename, const unsigned char iSeed[], unsigned int iSeedSize, const unsigned int iSecurityLevel, unsigned char *oKeyArray, unsigned int iKeyArrayMaxSize, unsigned int *oSize);

#endif