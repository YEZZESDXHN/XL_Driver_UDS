#include "uds_tp.h"
#include <stdint.h>
// �����״̬, ���� 3 �� ����״̬(NWL_IDLE)������״̬(NWL_XMIT)������״̬(NWL_RECV)
// �����յ���֡ʱ��״̬����Ϊ NWL_RECV��ֱ������֡������ɲ���Ϊ NWL_IDLE
// �����Ͷ�֡ʱ����״̬����Ϊ NWL_XMIT��ֱ��������ɺ����Ϊ NWL_IDLE
static network_layer_st nwl_st = NWL_IDLE;

// ����֡���ձ�־���ӽ��յ���֡ʱ�� 1��ֱ������֡��������� 0
static bool_t g_wait_cf = FALSE;

// ����֡�ȴ���־����������֡ʱ���ñ�־�� 1�����յ�����֡ʱ������Ϣ�� 0���������֡�е���״̬Ϊ�ȴ�״̬����ñ�־������ 1
static bool_t g_wait_fc = FALSE;

// TP ����ض�ʱ������
static uint32_t nt_timer[TIMER_CNT] = { 0 };

// ���յ�����֡�е� STmin ֡���ʱ�䣬�����ⷢ������֡ʱ�����ͼ������С�ڸ�ֵ
static uint8_t g_rfc_stmin = 0;

// ���յ�����֡�е� bolck size ���С��
// �����ֵ��Ϊ 0����ÿ�����յ� g_rfc_bs ������֡���践��һ֡����֡
// �����ֵΪ 0���������ͷ�����һֱ�����Ʒ�������֡��ֱ�����������������֡
static uint8_t g_rfc_bs = 0;

// ����֡���ͼ���
static uint8_t g_xcf_bc = 0;

// ����֡֡��ţ���֡����ʱ��ʹ�ã�ÿ����һ֡����֡ʱ��Ӧ�� +1������ 0xf ��λΪ 0
static uint8_t g_xcf_sn = 0;

// ��������֡������־������ >= NT_XMIT_FC_BS (ǰ��������NT_XMIT_FC_BS ��Ϊ 0) ʱ��Ӧ����һ֡����֡�������㸴λ
static uint8_t g_rcf_bc = 0;

// ����֡֡��ţ���������֡ʱ��ʹ�ã����յ�������֡�У���Ӧ��ÿ�� +1������ 0xf ��λΪ 0 
static uint8_t g_rcf_sn = 0;

// �������ݻ�����
static uint8_t remain_buf[UDS_TX_MAX];

// ʣ����Ҫ���͵���Ч���ݳ��ȣ�ÿ���ⷢ��һ֡���ģ�remain_len ����Ҫ��ȥ������Ч���ݵĳ���
static uint16_t remain_len = 0;

// ʣ����Ҫ���͵���Ч������ remain_buf �����е���ʼλ��
static uint16_t remain_pos = 0;

// �������ݻ���������֡���ݽ���ƴ�ӣ��γ�һ����������Ч���ݴ�ŵ���������
static uint8_t recv_buf[UDS_RX_MAX];

// ��������֡�������һ�������Ļ���������Ҫ��Ϊ���ڶ�֡���յĹ����У������ܹ��������ղ�����֡
static uint8_t recv_buf_sf[FRAME_SIZE];

// ��Ч�����ܳ��ȣ����ڽ��ն�֡���ģ�������֡�и�ֵ����Ӧ�õ�����֡+������������֡�е���Ч���ݳ���
static uint16_t recv_fdl = 0;

// ʵ�ʽ��յ�����Ч�����ܳ��ȣ����ڽ��ն�֡����ʱ�ۼӼ���
static uint16_t recv_len = 0;

// 0:����Ѱַ; 1:����Ѱַ
uint8_t g_tatype;






///******************************************************************************
//* ��������: static void nt_timer_start(nt_timer_t num)
//* ����˵��: ���� TP �㶨ʱ��
//* �������: nt_timer_t num              --��ʱ��
//* �������: ��
//* ��������: ��
//* ����˵��: ��
//******************************************************************************/
//static void nt_timer_start(nt_timer_t num)
//{
//	// �������Ϸ���
//	if (num >= TIMER_CNT) return;
//
//	// ���� N_CR ��ʱ��
//	if (num == TIMER_N_CR)
//		nt_timer[TIMER_N_CR] = TIMEOUT_N_CR + 1;
//
//	// ���� N_BS ��ʱ��
//	if (num == TIMER_N_BS)
//		nt_timer[TIMER_N_BS] = TIMEOUT_N_BS + 1;
//
//	// ���� STmin ��ʱ��
//	if (num == TIMER_STmin)
//		nt_timer[TIMER_STmin] = g_rfc_stmin;
//}
//
//
///******************************************************************************
//* ��������: static void nt_timer_start_wv(nt_timer_t num, uint32_t value)
//* ����˵��: �������ö�ʱ���ļ���ֵ
//* �������: nt_timer_t num              --��ʱ��
//	��������uint32_t value              --����ֵ
//* �������: ��
//* ��������: ��
//* ����˵��: �� value = 0����ʾ�رն�ʱ������ͬ�� nt_timer_stop() ����
//	���������� value = 1����ʾ��ʱ����ʱ�ѷ�����������������ʱ�¼�
//	���������� value Ϊ����ֵʱ����ʱ�������� value ֵ���¿�ʼ��ʱ
//******************************************************************************/
//static void nt_timer_start_wv(nt_timer_t num, uint32_t value)
//{
//	// �������Ϸ���
//	if (num >= TIMER_CNT) return;
//
//	// �������� N_CR ��ʱ���ļ���ֵ
//	if (num == TIMER_N_CR)
//		nt_timer[TIMER_N_CR] = value;
//
//	// �������� N_BS ��ʱ���ļ���ֵ
//	if (num == TIMER_N_BS)
//		nt_timer[TIMER_N_BS] = value;
//
//	// �������� STmin ��ʱ���ļ���ֵ
//	if (num == TIMER_STmin)
//		nt_timer[TIMER_STmin] = value;
//}
//
//
///******************************************************************************
//* ��������: static void nt_timer_stop (nt_timer_t num)
//* ����˵��: �رն�ʱ��
//* �������: nt_timer_t num              --��ʱ��
//* �������: ��
//* ��������: ��
//* ����˵��: ��
//******************************************************************************/
//static void nt_timer_stop(nt_timer_t num)
//{
//	// �������Ϸ���
//	if (num >= TIMER_CNT) return;
//
//	// ����ֵ�� 0����ʾ�رն�ʱ��
//	nt_timer[num] = 0;
//}
//
///******************************************************************************
//* ��������: static int nt_timer_run(nt_timer_t num)
//* ����˵��: ��ʱ����������
//* �������: nt_timer_t num              --��ʱ��
//* �������: ��
//* ��������: 0: ��ʱ���Ѿ����ر�; -1: ��ʱ����; 1: ��ʱ�����ڼ�ʱ����
//* ����˵��: �ú�����Ҫ�� 1ms ���ڵ���
//******************************************************************************/
//static int nt_timer_run(nt_timer_t num)
//{
//	// �������Ϸ���
//	if (num >= TIMER_CNT) return 0;
//
//	// �������ֵΪ 0����ʾ��ʱ���Ѿ��رգ����ٹ���
//	if (nt_timer[num] == 0)
//	{
//		return 0;                   // ���� 0����ʱ���Ѿ����ر�
//	}
//	// �������ֵΪ 1����ʾ��ʱ����ʱ�ѷ���
//	else if (nt_timer[num] == 1)
//	{
//		nt_timer[num] = 0;          // �رն�ʱ��
//		return -1;                  // ���� -1��������ʱ
//	}
//	// ����������ʾ��ʱ����������
//	else
//	{
//		nt_timer[num]--;            // ����ֵ -1
//		return 1;                   // ���� 1����ʱ�����ڼ�ʱ����
//	}
//}
//
//
///******************************************************************************
//* ��������: static int nt_timer_chk(nt_timer_t num)
//* ����˵��: ��鶨ʱ��״̬Ȼ��رն�ʱ��
//* �������: nt_timer_t num              --��ʱ��
//* �������: ��
//* ��������: 0: ��ʱ����ֹͣ����;  1: ��ʱ�����ڼ�ʱ����
//* ����˵��: �ú���ִ�к����۶�ʱ���Ƿ��������У��������ر�
//******************************************************************************/
//static int nt_timer_chk(nt_timer_t num)
//{
//	// �������Ϸ���
//	if (num >= TIMER_CNT) return 0;
//
//	// �����ʱ������ֵ > 0,��ʾ��ʱ�����ڹ���
//	if (nt_timer[num] > 0)
//	{
//		nt_timer[num] = 0;          // �رն�ʱ��
//		return 1;                   // ���� 1����ʱ�����ڼ�ʱ����
//	}
//	else
//	{
//		nt_timer[num] = 0;          // ��ʱ����ֹͣ����
//		return 0;                   // ���� 0����ʱ����ֹͣ����
//	}
//}


/******************************************************************************
* ��������: static void clear_network(void)
* ����˵��: ��λ�����״̬
* �������: ��
* �������: ��
* ��������: ��
* ����˵��: ��Ҫ�Ǹ�λ����״̬���������ҹر�������ض�ʱ��
******************************************************************************/
static void clear_network(void)
{
	nt_timer_t num;
	nwl_st = NWL_IDLE;                      // �������״̬Ϊ����״̬
	g_wait_cf = FALSE;                      // ������֡���ձ�־
	g_wait_fc = FALSE;                      // ������֡�ȴ���־
	g_xcf_bc = 0;                           // ������֡���ͼ���
	g_xcf_sn = 0;                           // ������֡����֡���
	g_rcf_bc = 0;                           // ���������֡������־
	g_rcf_sn = 0;                           // ���յ�����֡֡��� 

	// �ر������������ض�ʱ��
	for (num = 0; num < TIMER_CNT; num++)
		nt_timer_stop(num);
}



/******************************************************************************
* ��������: static int recv_singleframe (uint8_t* frame_buf, uint8_t frame_dlc)
* ����˵��: ����֡
* �������: uint8_t*    frame_buf       --���ձ���֡�����׵�ַ
	��������uint8_t     frame_dlc       --���ձ���֡���ݳ���
* �������: ��
* ��������: 0: OK; -1: ERR
* ����˵��: ��
******************************************************************************/
int recv_singleframe(uint8_t* frame_buf, uint8_t frame_dlc)
{
	uint16_t i, uds_dlc;

	// ��֡��һ���ֽڵĵ� 4 λΪ��Ч���ݳ���
	uds_dlc = NT_GET_SF_DL(frame_buf[0]);

	// ��Ч���ݳ��ȺϷ��Լ��
	if (uds_dlc > FRAME_SIZE - 1 || 0 == uds_dlc)
		return -1;

	// ����Ч���ݿ����� recv_buf_sf ��
	for (i = 0; i < uds_dlc; i++)
		recv_buf_sf[i] = frame_buf[1 + i];

	// TP �㵥֡���ݴ�����ɣ������ݽ����ϲ��������
	//N_USData.indication(recv_buf_sf, uds_dlc, N_OK);

	return 0;
}


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
int recv_firstframe(UDS_SEND_FRAME sendframefun, uint16_t REQUEST_ID, uint8_t* frame_buf, uint8_t frame_dlc)
{
	uint16_t i;
	uint16_t uds_dlc;

	// ��֡ byte0 �ĵ� 4 λ�Լ� byte1 ��ƴ�� 12 λ����Ч���ݳ��ȣ����ݳ������Ϊ 4095
	uds_dlc = ((uint16_t)(frame_buf[0] & 0x0f)) << 8;
	uds_dlc |= frame_buf[1];

	// ��֡��Ч���ݳ��Ȳ�Ӧ��С�� FRAME_SIZE
	if (uds_dlc < FRAME_SIZE)
		return -1;

	// �����Ч���ݳ��ȴ��� UDS_RX_MAX�� �򷵻�����֡(���)
	if (uds_dlc > UDS_RX_MAX)
	{
		send_flowcontrol(sendframefun, REQUEST_ID,FS_OVFLW);
		return -2;
	}

	// ��Ч�����ܳ���
	recv_fdl = uds_dlc;

	// ����֡�е���Ч���ݿ����� recv_buf ������
	for (i = 0; i < frame_dlc - 2; i++)
		recv_buf[i] = frame_buf[2 + i];

	// recv_len: ʵ�ʽ��յ�����Ч���ݳ��ȣ����ں���������֡���ۼ�
	recv_len = frame_dlc - 2;

	// ��������֡(������������Ʒ���)��������С���Ϊ NT_XMIT_FC_STMIN
	send_flowcontrol(sendframefun, REQUEST_ID,FS_CTS);

	// ���������֡������־
	g_rcf_bc = 0;

	// ���յ���֡����ζ�Ž�������Ҫ�յ�����֡������������֡���ձ�־
	g_wait_cf = TRUE;

	// ���� N_CR ��ʱ�������շ��յ�����֡���ʱ�䲻Ӧ���� TIMEOUT_N_CR
	//nt_timer_start(TIMER_N_CR);

	// ������֡֡��� 
	g_rcf_sn = 0;

	// TP ����֡���ݴ�����ɣ�֪ͨ�ϲ�
	//N_USData.ffindication(N_OK);

	return 0;
}


/******************************************************************************
* ��������: static void send_flowcontrol(network_flow_status_t flow_st)
* ����˵��: ��������֡
* �������: network_flow_status_t   flow_st       --��״̬
* �������: ��
* ��������: 0: ��������֡�������; 1: ��������; other: -ERR
* ����˵��: ��
******************************************************************************/
void send_flowcontrol(UDS_SEND_FRAME sendframefun, uint16_t REQUEST_ID,network_flow_status_t flow_st)
{
	uint16_t i;
	uint8_t send_buf[FRAME_SIZE] = { 0 };

	// ���Ĭ��ֵ
	for (i = 0; i < FRAME_SIZE; i++)
		send_buf[i] = PADDING_VAL;

	// �� 4 λ��ʾ֡���ͣ�����֡; �� 4 λ��ʾ��״̬��
	send_buf[0] = NT_SET_PCI_TYPE_FC(flow_st);

	// NT_XMIT_FC_BS: ����������֡�ĸ�������Ϊ 0�����ʾ���ͷ�����һֱ�����Ʒ�������֡
	// ����Ϊ 0�����ʾ�����ͷ����͵�����֡����Ϊ NT_XMIT_FC_BS ����ȴ����շ��ظ�һ֡����֡�����ͷ���������֡�����������ķ������
	send_buf[1] = NT_XMIT_FC_BS;

	// ֡��������ͷ���������֡ʱ��������Ҫ��� NT_XMIT_FC_STMIN ʱ�� 
	send_buf[2] = NT_XMIT_FC_STMIN;

	// ����
	sendframefun(REQUEST_ID, send_buf, FRAME_SIZE);
}

/******************************************************************************
* ��������: static int send_singleframe (uint8_t* msg_buf, uint16_t msg_dlc)
* ����˵��: ���͵�֡
* �������: uint8_t*    msg_buf         --���������׵�ַ
	��������uint8_t     msg_dlc         --�������ݳ���
* �������: ��
* ��������: 0: OK; -1: ERR
* ����˵��: ��
******************************************************************************/
int send_singleframe(UDS_SEND_FRAME sendframefun, uint16_t REQUEST_ID,uint8_t* msg_buf, uint8_t msg_dlc)
{
	uint16_t i;
	uint8_t send_buf[FRAME_SIZE] = { 0 };
	
	// ���Ĭ��ֵ
	for (i = 0; i < FRAME_SIZE; i++)
		send_buf[i] = PADDING_VAL;

	// �������Ϸ���
	if (0 == msg_dlc || msg_dlc > (FRAME_SIZE - 1)) return -1;

	// ��֡��һ���ֽڸ� 4 λ��ʾ֡���ͣ��� 4 λ��ʾ֡��Ч���ݳ��� 
	send_buf[0] = NT_SET_PCI_TYPE_SF((uint8_t)msg_dlc);

	// ������Ч���ݵ� send_buf ��
	for (i = 0; i < msg_dlc; i++)
		send_buf[1 + i] = msg_buf[i];

	// ����
	sendframefun(REQUEST_ID, send_buf, FRAME_SIZE);

	return 0;

}