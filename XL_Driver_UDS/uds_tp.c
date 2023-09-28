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
uint8_t g_tatype=0;

// �ϲ��� TP ��ע���һЩ�ӿں��������¼�� N_USData �У��� TP ����������괦�����ͨ����Щ�ӿں��������ݽ����ϲ��������
static nt_usdata_t N_USData = { NULL, NULL, NULL };




/******************************************************************************
* ��������: static void nt_timer_start(nt_timer_t num)
* ����˵��: ���� TP �㶨ʱ��
* �������: nt_timer_t num              --��ʱ��
* �������: ��
* ��������: ��
* ����˵��: ��
******************************************************************************/
static void nt_timer_start(nt_timer_t num)
{
	// �������Ϸ���
	if (num >= TIMER_CNT) return;

	// ���� N_CR ��ʱ��
	if (num == TIMER_N_CR)
		nt_timer[TIMER_N_CR] = TIMEOUT_N_CR + 1;

	// ���� N_BS ��ʱ��
	if (num == TIMER_N_BS)
		nt_timer[TIMER_N_BS] = TIMEOUT_N_BS + 1;

	// ���� STmin ��ʱ��
	if (num == TIMER_STmin)
		nt_timer[TIMER_STmin] = g_rfc_stmin;
}


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
static void nt_timer_start_wv(nt_timer_t num, uint32_t value)
{
	// �������Ϸ���
	if (num >= TIMER_CNT) return;

	// �������� N_CR ��ʱ���ļ���ֵ
	if (num == TIMER_N_CR)
		nt_timer[TIMER_N_CR] = value;

	// �������� N_BS ��ʱ���ļ���ֵ
	if (num == TIMER_N_BS)
		nt_timer[TIMER_N_BS] = value;

	// �������� STmin ��ʱ���ļ���ֵ
	if (num == TIMER_STmin)
		nt_timer[TIMER_STmin] = value;
}


/******************************************************************************
* ��������: static void nt_timer_stop (nt_timer_t num)
* ����˵��: �رն�ʱ��
* �������: nt_timer_t num              --��ʱ��
* �������: ��
* ��������: ��
* ����˵��: ��
******************************************************************************/
static void nt_timer_stop(nt_timer_t num)
{
	// �������Ϸ���
	if (num >= TIMER_CNT) return;

	// ����ֵ�� 0����ʾ�رն�ʱ��
	nt_timer[num] = 0;
}

/******************************************************************************
* ��������: static int nt_timer_run(nt_timer_t num)
* ����˵��: ��ʱ����������
* �������: nt_timer_t num              --��ʱ��
* �������: ��
* ��������: 0: ��ʱ���Ѿ����ر�; -1: ��ʱ����; 1: ��ʱ�����ڼ�ʱ����
* ����˵��: �ú�����Ҫ�� 1ms ���ڵ���
******************************************************************************/
static int nt_timer_run(nt_timer_t num)
{
	// �������Ϸ���
	if (num >= TIMER_CNT) return 0;

	// �������ֵΪ 0����ʾ��ʱ���Ѿ��رգ����ٹ���
	if (nt_timer[num] == 0)
	{
		return 0;                   // ���� 0����ʱ���Ѿ����ر�
	}
	// �������ֵΪ 1����ʾ��ʱ����ʱ�ѷ���
	else if (nt_timer[num] == 1)
	{
		nt_timer[num] = 0;          // �رն�ʱ��
		return -1;                  // ���� -1��������ʱ
	}
	// ����������ʾ��ʱ����������
	else
	{
		nt_timer[num]--;            // ����ֵ -1
		return 1;                   // ���� 1����ʱ�����ڼ�ʱ����
	}
}


/******************************************************************************
* ��������: static int nt_timer_chk(nt_timer_t num)
* ����˵��: ��鶨ʱ��״̬Ȼ��رն�ʱ��
* �������: nt_timer_t num              --��ʱ��
* �������: ��
* ��������: 0: ��ʱ����ֹͣ����;  1: ��ʱ�����ڼ�ʱ����
* ����˵��: �ú���ִ�к����۶�ʱ���Ƿ��������У��������ر�
******************************************************************************/
static int nt_timer_chk(nt_timer_t num)
{
	// �������Ϸ���
	if (num >= TIMER_CNT) return 0;

	// �����ʱ������ֵ > 0,��ʾ��ʱ�����ڹ���
	if (nt_timer[num] > 0)
	{
		nt_timer[num] = 0;          // �رն�ʱ��
		return 1;                   // ���� 1����ʱ�����ڼ�ʱ����
	}
	else
	{
		nt_timer[num] = 0;          // ��ʱ����ֹͣ����
		return 0;                   // ���� 0����ʱ����ֹͣ����
	}
}


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
	N_USData.indication(recv_buf_sf, uds_dlc, N_OK);

	return 0;
}


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
int recv_firstframe(UDS_SEND_FRAME sendframefun, uint8_t* frame_buf, uint8_t frame_dlc)
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
		send_flowcontrol(sendframefun, FS_OVFLW);
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
	send_flowcontrol(sendframefun, FS_CTS);

	// ���������֡������־
	g_rcf_bc = 0;

	// ���յ���֡����ζ�Ž�������Ҫ�յ�����֡������������֡���ձ�־
	g_wait_cf = TRUE;

	// ���� N_CR ��ʱ�������շ��յ�����֡���ʱ�䲻Ӧ���� TIMEOUT_N_CR
	//nt_timer_start(TIMER_N_CR);

	// ������֡֡��� 
	g_rcf_sn = 0;

	// TP ����֡���ݴ�����ɣ�֪ͨ�ϲ�
	N_USData.ffindication(N_OK);

	return 0;
}

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
static int recv_consecutiveframe(UDS_SEND_FRAME sendframefun, uint8_t* frame_buf, uint8_t frame_dlc)
{
	uint8_t cf_sn;      // ����֡��֡���
	uint16_t i;

	// ����֡���ĵ�һ���ֽڵĵ� 4 λ��ʾ֡���
	cf_sn = NT_GET_CF_SN(frame_buf[0]);

	// ��鲢ֹͣ N_CR ��ʱ�������շ��յ�����֡���ʱ�䲻�ܴ��� TIMEOUT_N_CR�������ʱ�������ٴ��������֡
	if (nt_timer_chk(TIMER_N_CR) <= 0) return -1;

	// ֡��� +1������ 0xF ��� 0 ��ʼ���¼���
	g_rcf_sn++;
	if (g_rcf_sn > 0x0f)
		g_rcf_sn = 0;

	// �յ�������֡֡���Ӧ��ÿ�� +1��������ǣ���������һ֡����֪ͨ�ϲ����쳣����
	if (g_rcf_sn != cf_sn)
	{
		N_USData.indication(recv_buf, recv_len, N_WRONG_SN);
		return -2;
	}

	// ��������֡�е���Ч���ݵ� recv_buf ������
	for (i = 0; i < (FRAME_SIZE - 1); i++)
	{
		if (recv_len + i < UDS_RX_MAX)     // ��ֹ����Խ��
			recv_buf[recv_len + i] = frame_buf[1 + i];
	}

	// recv_len��ʵ�ʽ��յ�����Ч�����ܳ��ȣ����ڽ��ն�֡����ʱ�ۼӼ���
	recv_len += (FRAME_SIZE - 1);

	// ��ʵ�ʽ��յ�����Ч�����ܳ��ȴ��ڻ������֡�л�õ���Ч�����ܳ���
	// ˵������֡������ɣ��������һ������֡�ˣ�������֡���ձ�־���������ݽ����ϲ��������
	if (recv_len >= recv_fdl)
	{
		g_wait_cf = FALSE;              // ������֡���ձ�־
		N_USData.indication(recv_buf, recv_fdl, N_OK);  // �����ݽ����ϲ��������
		return 0;
	}
	else
	{
		// �������е�����˵��������������֡��Ҫ����

		// �� NT_XMIT_FC_BS ��Ϊ 0 ʱ��ÿ�յ� NT_XMIT_FC_BS ������֡���践��һ֡����֡ 
		if (NT_XMIT_FC_BS > 0)
		{
			// ��������֡���� +1
			g_rcf_bc++;

			if (g_rcf_bc >= NT_XMIT_FC_BS)
			{
				// ����һ֡����֡
				send_flowcontrol(sendframefun, FS_CTS);

				// ��������֡������ 0
				g_rcf_bc = 0;
			}
		}

		// ����֡���ձ�־�� 1
		g_wait_cf = TRUE;

		// ���� N_CR ��ʱ�������շ��յ�����֡���ʱ�䲻�ܴ��� TIMEOUT_N_CR
		nt_timer_start(TIMER_N_CR);
		return 1;
	}
}

/******************************************************************************
* ��������: static int recv_flowcontrolframe(uint8_t* frame_buf, uint8_t frame_dlc)
* ����˵��: ��������֡
* �������: uint8_t*    frame_buf       --���ձ���֡�����׵�ַ
	��������uint8_t     frame_dlc       --���ձ���֡���ݳ���
* �������: ��
* ��������: 0: ��������֡�������; 1: ��������; other: -ERR
* ����˵��: ��
******************************************************************************/
static int recv_flowcontrolframe(uint8_t* frame_buf, uint8_t frame_dlc)
{
	uint8_t fc_fs;

	// ��ȡ��״̬
	fc_fs = NT_GET_FC_FS(frame_buf[0]);

	// ��鲢ֹͣ N_BS ��ʱ�������ͷ����������֡�󵽽��յ�����֮֡���ʱ�䲻Ӧ���� TIMEOUT_N_BS
	// �����ʱ�������ٴ��������֡
	if (nt_timer_chk(TIMER_N_BS) <= 0) return -1;

	// ����Ѱַ�ǲ�Ӧ�÷�������֡�ģ�����յ���Ӧ�ö���
	if (g_tatype == N_TATYPE_FUNCTIONAL) return -1;

	// �Ѿ��ȵ�������֡����������֡�ȴ���־�Ϳ����� 0 ��
	g_wait_fc = FALSE;

	// ��״̬�Ƿ���֪ͨ�ϲ㲢�˳�
	if (fc_fs >= FS_RESERVED)
	{
		N_USData.confirm(N_INVALID_FS);
		return -2;
	}

	// ��״̬Ϊ���״̬��֪ͨ�ϲ㲢�˳�
	if (fc_fs == FS_OVFLW)
	{
		N_USData.confirm(N_BUFFER_OVFLW);
		return -3;
	}

	// ��״̬Ϊ�ȴ�״̬������֡�ȴ���־������ 1�����¿��� N_BS ��ʱ�����˳�
	if (fc_fs == FS_WT)
	{
		g_wait_fc = TRUE;
		nt_timer_start(TIMER_N_BS);
		return 1;
	}

	// ���յ�����֡�е� bolck size ���С
	// �����ֵ��Ϊ 0����ÿ�����յ� g_rfc_bs ������֡���践��һ֡����֡
	// �����ֵΪ 0������Ҫ�ٻظ�����֡�ˣ����ͷ�����һֱ�����Ʒ�������֡��ֱ�����������������֡
	g_rfc_bs = frame_buf[1];

	// ���յ�����֡�е� STmin ֡���ʱ�䣬�����ⷢ������֡ʱ�����ͼ������С�ڸ�ֵ
	// 0~7F ��ʾ: 0~127ms, F1~F9 ��ʾ: 100~900us������ͳһ�� 1ms ����������ֵ��Ϊ��Чֵ��ͳһ����� 127ms
	// ��Ϊ��ʱ������ֵΪ 1 ��ʱ���ʾ��ʱ�������������õļƶ�ʱ����ֵ���� +1 ��
	if (frame_buf[2] <= 0x7F)
		g_rfc_stmin = frame_buf[2] + 1;
	else if (frame_buf[2] >= 0xF0 && frame_buf[2] <= 0xF9)
		g_rfc_stmin = 1 + 1;
	else
		g_rfc_stmin = 0x7F + 1;

	// ������֡���ͼ���
	g_xcf_bc = 0;

	// �������� STmin ��ʱ������ֵΪ 1����ʾ��ʱ����ʱ�ѷ�����������������ʱ�¼�
	nt_timer_start_wv(TIMER_STmin, 1);

	return 0;
}


/******************************************************************************
* ��������: void send_flowcontrol(UDS_SEND_FRAME sendframefun, network_flow_status_t flow_st)
* ����˵��: ��������֡
* �������: UDS_SEND_FRAME sendframefun --���ͱ��ĺ���ָ�룬Ӳ�������ӿں���
	��������network_flow_status_t   flow_st       --��״̬
* �������: ��
* ��������: 0: ��������֡�������; 1: ��������; other: -ERR
* ����˵��: ��
******************************************************************************/
void send_flowcontrol(UDS_SEND_FRAME sendframefun, network_flow_status_t flow_st)
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
* ��������: int send_singleframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint8_t msg_dlc)
* ����˵��: ���͵�֡
* �������: UDS_SEND_FRAME sendframefun --���ͱ��ĺ���ָ�룬Ӳ�������ӿں���
	��������uint8_t*    msg_buf         --���������׵�ַ
	��������uint8_t     msg_dlc         --�������ݳ���
* �������: ��
* ��������: 0: OK; -1: ERR
* ����˵��: ��
******************************************************************************/
int send_singleframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint8_t msg_dlc)
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
static int send_firstframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc)
{
	uint16_t i;
	uint8_t send_buf[FRAME_SIZE] = { 0 };

	// �������Ϸ���
	if (msg_dlc < FRAME_SIZE || msg_dlc > UDS_TX_MAX) return 0;

	// ��֡�ĵ�һ���ֽڸ� 4 λ��ʾ֡���ͣ��� 4 λ�͵ڶ����ֽڹ���� 12 λ����ʾ֡��Ч���ݳ��� 
	send_buf[0] = NT_SET_PCI_TYPE_FF((uint8_t)(msg_dlc >> 8));
	send_buf[1] = (uint8_t)(msg_dlc & 0x00ff);

	// ������Ч���ݵ� send_buf ��
	for (i = 0; i < FRAME_SIZE - 2; i++)
		send_buf[2 + i] = msg_buf[i];

	// ����
	sendframefun(RESPONSE_ID, send_buf, FRAME_SIZE);

	// ��������֡������֡�ȴ���־���� 1���������ȴ���������֡
	g_wait_fc = TRUE;

	// ���� N_BS ��ʱ�������ͷ����������֡�󵽽��յ�����֮֡���ʱ�䲻�ܴ��� TIMEOUT_N_BS����λ: ms
	nt_timer_start(TIMER_N_BS);

	// �����ѷ��͵���Ч���ݳ���
	return FRAME_SIZE - 2;
}

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
static int send_consecutiveframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc, uint8_t frame_sn)
{
	uint16_t i;
	uint8_t send_buf[FRAME_SIZE] = { 0 };

	// ����֡�ĵ�һ���ֽڵĸ� 4 λ��ʾ֡���ͣ��� 4 λ��ʾ֡��ţ�ÿ����һ֡����֡ʱ��Ӧ�� +1������ 0xf ��λΪ 0
	send_buf[0] = NT_SET_PCI_TYPE_CF(frame_sn);

	// ������Ч���ݵ� send_buf ��
	for (i = 0; i < msg_dlc && i < (FRAME_SIZE - 1); i++)
		send_buf[1 + i] = msg_buf[i];

	// ����֡�����һ֡�����ǲ����ģ������Ĭ��ֵ PADDING_VAL
	for (; i < (FRAME_SIZE - 1); i++)
		send_buf[1 + i] = PADDING_VAL;

	// ����
	sendframefun(RESPONSE_ID, send_buf, FRAME_SIZE);

	// ��� msg_dlc > FRAME_SIZE - 1��˵���ⲻ�����һ������֡��ʵ�ʷ��͵���Ч���ݳ���Ϊ FRAME_SIZE - 1
	// ����˵����������֡�����һ֡������ʵ�ʷ������ʣ�����Ч���ݳ���
	if (msg_dlc > (FRAME_SIZE - 1))
		return (FRAME_SIZE - 1);
	else
		return msg_dlc;
}



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
static int send_multipleframe(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc)
{
	uint16_t i;
	uint8_t send_len;

	// �������Ϸ���
	if (msg_dlc < FRAME_SIZE || msg_dlc > UDS_TX_MAX)
		return -1;

	// ��Ҫ���͵����ݿ����� remain_buf ������
	for (i = 0; i < msg_dlc; i++)
		remain_buf[i] = msg_buf[i];

	// ����֡֡����� 0����֡����ʱ��ʹ�ã�ÿ����һ֡����֡ʱ��Ӧ�� +1������ 0xf ��λΪ 0
	g_xcf_sn = 0;

	// ��֡����ʱ�Ƚ���֡���ͳ�ȥ���������� network_task �������н�ʣ�µ����ݲ�ֳ�����֡��������
	send_len = send_firstframe(sendframefun,msg_buf, msg_dlc);

	// �Ѿ����ͳ�ȥһ֡�ˣ�����ʣ����Ҫ���͵���Ч������ remain_buf �����е���ʼλ����Ҫ����ƶ� send_len
	remain_pos = send_len;

	// ʣ����Ҫ���͵���Ч���ݳ��� = ������Ҫ���͵���Ч�����ܳ���(msg_dlc) - ��֡���ѷ��ͳ�ȥ����Ч���ݳ���(send_len)
	remain_len = msg_dlc - send_len;

	// ����֡����ʱ���ڷ��������֡��û����Ƴ�һ������֡���ͱ�־���Ա������������м�����������֡
	// ��ô������֡�ķ����������Ƶ��أ�
	// ���յ�����֡ recv_flowcontrolframe ������������ nt_timer_start_wv(TIMER_STmin, 1);
	// �������� STmin ��ʱ������ֵΪ 1����ʾ STmin ��ʱ����ʱ�ѷ���
	// �� network_task �������м�� STmin ��ʱ���Ƿ�ʱ�������ʱ���������������֡��ÿ����һ֡����֡ STmin ��ʱ�������¿�ʼ����
	// ֱ��ʣ����Ҫ���͵���Ч���ݳ��� remain_len Ϊ 0(�������ݷ������)������ clear_network ������λ�����״̬

	return 0;
}


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
int network_send_udsmsg(UDS_SEND_FRAME sendframefun, uint8_t* msg_buf, uint16_t msg_dlc)
{
	// �������Ϸ���
	if (NULL == msg_buf || 0 == msg_dlc || msg_dlc > UDS_TX_MAX)
		return -1;

	// �������ݳ����ж�ӦΪ��֡���ͻ��Ƕ�֡����
	if (msg_dlc < FRAME_SIZE)
	{
		send_singleframe(sendframefun,msg_buf, msg_dlc);
	}
	else
	{
		// ����Ƕ�֡���ͣ��򽫵�ǰ�����״̬����Ϊ����״̬
		nwl_st = NWL_XMIT;
		send_multipleframe(sendframefun,msg_buf, msg_dlc);
	}

	return 0;
}


/******************************************************************************
* ��������: void network_task(UDS_SEND_FRAME sendframefun)
* ����˵��: TP ��������
* �������: UDS_SEND_FRAME sendframefun --���ͱ��ĺ���ָ�룬Ӳ�������ӿں���
* �������: ��
* ��������: ��
* ����˵��: �ú�����Ҫ�� 1ms ���ڵ���
******************************************************************************/
void network_task(UDS_SEND_FRAME sendframefun)
{
	uint8_t send_len;

	// ��� N_CR ��ʱ����ʱ����λ�����״̬����֪ͨ�ϲ����쳣����
	if (nt_timer_run(TIMER_N_CR) < 0)
	{
		clear_network();
		N_USData.indication(recv_buf, recv_len, N_TIMEOUT_Cr);
	}

	// ��� N_BS ��ʱ����ʱ����λ�����״̬����֪ͨ�ϲ����쳣����
	if (nt_timer_run(TIMER_N_BS) < 0)
	{
		clear_network();
		N_USData.confirm(N_TIMEOUT_Bs);
	}

	// ��� STmin ��ʱ����ʱ����ʾ���Լ�����������֡
	if (nt_timer_run(TIMER_STmin) < 0)
	{
		// ����֡֡��� +1������ 0xf ��λΪ 0
		g_xcf_sn++;
		if (g_xcf_sn > 0x0f)
			g_xcf_sn = 0;

		// ����һ֡����֡
		send_len = send_consecutiveframe(sendframefun,&remain_buf[remain_pos], remain_len, g_xcf_sn);

		// ʣ����Ҫ���͵���Ч������ remain_buf �����е���ʼλ�� remain_pos �ƶ� send_len
		remain_pos += send_len;

		// ʣ����Ҫ���͵���Ч���ݳ������ȥ send_len
		remain_len -= send_len;

		// �ж�ʣ����Ҫ���͵���Ч���ݳ����Ƿ���� 0�������� 0����ʾ��������֡��Ҫ�������ͣ����򣬱�ʾ����֡��ȫ��������ϣ���λ�����״̬
		if (remain_len > 0)
		{
			// g_rfc_bs: ���յ�����֡�е� bolck size ���С
			// �����ֵ��Ϊ 0����ÿ�����յ� g_rfc_bs ������֡���践��һ֡����֡
			// �����ֵΪ 0������Ҫ�ٻظ�����֡�ˣ����ͷ�����һֱ�����Ʒ�������֡��ֱ�����������������֡
			if (g_rfc_bs > 0)
			{
				// ����֡���ͼ��� +1
				g_xcf_bc++;
				if (g_xcf_bc < g_rfc_bs)
				{
					nt_timer_start(TIMER_STmin);// ���� STmin ��ʱ��
				}
				else
				{
					// �������� g_rfc_bs ������֡����Ҫ�ٴεȴ�����֡ 
					g_wait_fc = TRUE;
					nt_timer_start(TIMER_N_BS); // ���� N_BS ��ʱ��
				}
			}
			else
			{
				nt_timer_start(TIMER_STmin);    // ���� STmin ��ʱ��
			}
		}
		else
		{
			clear_network();                    // ��λ�����״̬
		}
	}
}



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
void uds_tp_recv_frame(UDS_SEND_FRAME sendframefun, uint8_t func_addr, uint8_t* frame_buf, uint8_t frame_dlc)
{
	uint8_t pci_type;
	int ret = -1;

	// �������Ϸ���
	if (NULL == frame_buf || FRAME_SIZE != frame_dlc)
		return;

	if (0 == func_addr)
		g_tatype = N_TATYPE_PHYSICAL;           // ����Ѱַ
	else
		g_tatype = N_TATYPE_FUNCTIONAL;         // ����Ѱַ

	// ÿ֡���ĵ�һ���ֽڵĸ� 4 λ��ʾ֡���ͣ��� 4 �֣���֡(SF)����֡(SF)������֡(CF)������֡(FC)
	pci_type = NT_GET_PCI_TYPE(frame_buf[0]);

	// ����������֡���ͷֱ���
	switch (pci_type)
	{
	case PCI_SF:                            // ��֡
			// ����֡�����ﲻ�����ƣ�����ζ�ż������ڶ�֡�շ��Ĺ����У�����Ҳ���Դ���֡
			// Ϊ�˲�Ӱ���֡���ݵĽ��գ�����Ϊ��֡���ݵ������һ�������Ļ����� recv_buf_sf
		recv_singleframe(frame_buf, frame_dlc);
		break;
	case PCI_FF:                            // ��֡
		// �������״̬Ϊ���ջ��߿���״̬ʱ
		if (NWL_RECV == nwl_st || NWL_IDLE == nwl_st)
		{
			// ��λ�����״̬����Ҫ�Ǹ�λ����״̬���������ҹر�������ض�ʱ��
			clear_network();

			// ����ڽ�������֡ʱ����һ����֡�����ǲ������ģ�֪ͨ�ϲ����쳣����
			// �������� recv_firstframe �����н��յ�������֡�����ƻ������´�����֡����
			if (NWL_RECV == nwl_st)
				N_USData.indication(recv_buf, recv_len, N_UNEXP_PDU);

			// ������֡
			ret = recv_firstframe(sendframefun,frame_buf, frame_dlc);

			// ����֡�������ʱ���������״̬Ϊ����״̬����֡��ʽ�������������״̬Ϊ����״̬
			if (0 == ret)
				nwl_st = NWL_RECV;
			else
				nwl_st = NWL_IDLE;
		}
		break;
	case PCI_CF:                            // ����֡
		// �������״̬Ϊ����״̬���Ҵ�������֡����״̬ʱ
		if (NWL_RECV == nwl_st && TRUE == g_wait_cf)
		{
			// ��������֡
			ret = recv_consecutiveframe(sendframefun,frame_buf, frame_dlc);

			// ������֡������ɻ���֡��ʽ����ʱ
			if (ret <= 0)
			{
				// ��λ�����״̬����Ҫ�Ǹ�λ����״̬���������ҹر�������ض�ʱ��
				clear_network();
				nwl_st = NWL_IDLE;          // �������״̬Ϊ����״̬
			}
		}
		break;
	case PCI_FC:                            // ����֡
		// �������״̬Ϊ����״̬���Ҵ��ڵȴ�����֡״̬ʱ
		if (NWL_XMIT == nwl_st && TRUE == g_wait_fc)
		{
			// ��������֡
			ret = recv_flowcontrolframe(frame_buf, frame_dlc);

			// ������֡������ɻ���֡��ʽ����ʱ
			if (ret < 0)
			{
				// ��λ�����״̬����Ҫ�Ǹ�λ����״̬���������ҹر�������ض�ʱ��
				clear_network();
				nwl_st = NWL_IDLE;          // �������״̬Ϊ����״̬
			}
		}
		break;
	default:
		// �Ƿ���Ч֡
		break;
	}
}



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
//int network_reg(nt_usdata_t* usdata)
//{
//	// �������Ϸ���
//	if (NULL == usdata || NULL == usdata->ffindication || NULL == usdata->indication || NULL == usdata->confirm)
//		return -1;
//
//	// �ϲ��� TP ��ע���һЩ�ӿں��������¼�� N_USData �У��� TP ����������괦�����ͨ����Щ�ӿں��������ݽ����ϲ��������
//	N_USData.confirm = usdata->confirm;
//	N_USData.ffindication = usdata->ffindication;
//	N_USData.indication = usdata->indication;
//
//	return 0;
//}