/*
 * @Author: zhongwei
 * @Date: 2020/3/30 10:12:41
 * @Description: FPGA��ַ����
 * @File: hd_fpga_def.h
 * 
 * �μ���FPGAͨѶ�ӿڹ�Լ���ĵ�
 * FPGA����ַ 0x40000000
 *  
 * 0x0000     
 * ...          - ģ�������� 
 * 0x0200 
 * ... 
 * 0x0400 
 * ...          - HSR���Ľ���
 * 0x0800 
 * ...          - HSR���ķ���
 * 0x0C00 
 * ... 
 * 0x1000 
 * ...          - SV���Ľ��գ�0��
 * 0x1100 
 * ... 
 * ... ...
 * ... 
 * 0x2F00 
 * ...          - SV���Ľ��գ�31��
 * 0x3000 
 * ...          - SV���ķ���
 * 0x3800 
 * ... 
 * 0x3C00 
 * ...          - ����ֵ�������ͣ�0��
 * 0x3E00 
 * ...          - ����ֵ�������ͣ�1��
 * 0x4000 
 * ...          - AXI_HPͨ�ſ�����(MMS����)
 * 0x4100 
 * ...          - AXI_HPͨ�ſ�����(����ֵ����/Goose����/�ڵ�䱨��)
 * 0x4200 
 * ... 
 * 0x4800 
 * ...          - Goose���Ľ���
 * 0x4C00 
 * ... 
 * 0x8000 
 * ...          - ͨ�üĴ���
 * 0x8400 
*/

#ifndef SRC_H_BAREMETAL_PLT_HD_FPGA_DEF_H_
#define SRC_H_BAREMETAL_PLT_HD_FPGA_DEF_H_

// todo define FPGA MACRO FPGA����ַ
#define FPGA_BASE                       0x40000000

#define FPGA_HP_MSG_LITTLE_ENDIAN   //FPGA����С��ģʽ

/*
   Analog Sampling region ģ�������� 
ģ��������ģ�����2��ADC������ÿ�����1ƬADCоƬ��16·ģ����ͨ������Ϊ2�����(AD-00��AD-01)��Ϣ���͡� 
ADCоƬ����AD7616����оƬ��16λ16ͨ����ͬ��ADC����оƬ��������ƫ���ֵ32767(+10V)�����̸�ƫ���ֵ-32768(-10V) �� 
��������2���Ʋ����ʾ���ڲ�����ֵ��������ADC��������ʱֱ�ӽ�16λADC����ֵ�����з�������չΪ32λ�з��������ͣ�������ϵ���任�� 
 
ģ��������ģ���ַ����(0x000~0x1FF)
0x000       :   �˲���ʱ�Ĵ���
0x010~0x06F :   AD-00����У׼�Ĵ���
0x070~0x0CF :   AD-01����У׼�Ĵ���
*/
#define FPGA_ANALOG_SAMPLING_OFFSET 0x0000
#define FPGA_ANALOG_SAMPLING_FILTER_DELAY_REG           (FPGA_BASE + FPGA_ANALOG_SAMPLING_OFFSET + 0)       //�˲���ʱ�Ĵ���
#define FPGA_ANALOG_SAMPLING_AD_CALIBRATION_REG         (FPGA_BASE + FPGA_ANALOG_SAMPLING_OFFSET + 0x10)    //AD ����У׼�Ĵ��� 4�ֽ�һ������32����AD0��AD1��16����
#define FPGA_ANALOG_SAMPLING_AD_CALIBRATION_INFO_LEN    0x60
/* FPGA Analog sampling filter delay reg Definitions and Masks ��������FPGA_ANALOG_SAMPLING_FILTER_DELAY_REG */
#define ANALOG_SAMPLING_FILTER_DELAY_SHIFT      0U
#define ANALOG_SAMPLING_FILTER_DELAY_MASK       0x000003FFU             //BIT0-BIT9 RW ADC����ǰ���˲���·��ʱ(��λ:1us)
/* FPGA Analog sampling Calibration reg Definitions and Masks  ��������FPGA_ANALOG_SAMPLING_AD_CALIBRATION_REG */
#define ANALOG_SAMPLE_ZERO_DRIFT_CTRL_SHIFT     0U 
#define ANALOG_SAMPLE_ZERO_DRIFT_CTRL_MASK      0x0000FFFFU             //BIT0-BIT15 RW ����ͨ��15ȥ��Ư������(�������ö����Ʋ����ʾ)
#define ANALOG_SAMPLE_PHASE_ANGLE_SHIFT         16U
#define ANALOG_SAMPLE_PHASE_ANGLE_MASK          0x00FF0000U             //BIT16-BIT23 RW ����ͨ��15�����(�Ƕȵ�λ:�֣��������ö����Ʋ����ʾ)


/* 
   General config region ͨ�üĴ���
   ͨ�üĴ�������ĳЩȫ���źŵĿ��ƻ�ӦĳЩ״̬��Ϣ
ͨ�üĴ�����ַ����(0x00~0x3FF)
0x000       :   �汾�Ĵ���
0x004       :   ���ƼĴ���
0x010       :   �ⲿ��ʱ�Ĵ���0
0x014       :   �ⲿ��ʱ�Ĵ���1
0x018       :   �ⲿ��ʱ�Ĵ���2
0x020       :   1PPSx1���߼Ĵ���
0x024       :   1PPSx2���߼Ĵ���
0x028       :   1PPSx3���߼Ĵ���
0x030       :   ����������߼Ĵ���
0x070~0x0FF :   ���ڵ�ַ�Ĵ���
0x100       :   ��ʱ���Ĵ���0
0x104       :   ��ʱ���Ĵ���1
*/
#define FPGA_GEN_CONFIG_OFFSET                  0x8000
#define FPGA_VERSION_REG                        (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0)            //�汾�Ĵ��� BIT0-15 RO ����汾�ţ�BIT16-31 RO ��Լ�汾��
#define FPGA_CTRL_REG                           (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x4)          //���ƼĴ��� �����FPGAͨѶ��Լ���ĵ�
#define FPGA_GPS_TIME_0_REG                     (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x10)         //�ⲿ��ʱ�Ĵ���0
#define FPGA_GPS_TIME_1_REG                     (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x14)         //�ⲿ��ʱ�Ĵ���1
#define FPGA_GPS_TIME_2_REG                     (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x18)         //�ⲿ��ʱ�Ĵ���2
#define FPGA_1PPS_x1_REG                        (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x20)         //1PPSx1�Ĵ���
#define FPGA_1PPS_x2_REG                        (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x24)         //1PPSx2�Ĵ���
#define FPGA_1PPS_x3_REG                        (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x28)         //1PPSx3�Ĵ���
#define FPGA_BACKPLANE_BUS_STATUS_REG           (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x30)         //����������߼Ĵ��� �����FPGAͨѶ��Լ���ĵ�
#define FPGA_MAC_REPLACE_ENABLE_REG             (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x70)         //���ڵ�ַ�Ĵ��� BIT0-15 ����MAC��ַ�滻ʹ��(���Ͷ˿�)
#define FPGA_ETH_STATUS_0_REG                   (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x74)         //����״̬���ӼĴ���0
#define FPGA_ETH_STATUS_1_REG                   (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x78)         //����״̬���ӼĴ���1
#define FPGA_ETH_MAC_ADD_REG                    (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x80)         //����MAC��ַ�Ĵ��� ÿ������ռ8�ֽڣ���16������
#define FPGA_TIMER_PERIOD_REG                   (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x100)        //��ʱ���Ĵ���(BIT00-20 �趨��ʱ������ ��λ: ns��ȡֵ��Χ595000~1390000 �ɸö�ʱ���ɲ���1PPSx3���ߴ����ź�)
#define FPGA_TIMER_COUNTER_REG                  (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x104)        //��ʱ������ֵ(BIT00-20 �趨��ʱ������ ��λ: ns��ȡֵ��Χ595000~1390000 �ɸö�ʱ���ɲ���1PPSx3���ߴ����ź�)

#define FPGA_ETH_PORT_COUNT                     16      //����16������

/* FPGA GENERAL CTRL REG Bit Definitions and Masks �������� FPGA_CTRL_REG */
#define EXT_TIME_SOURCE_SHIFT                   0U
#define EXT_TIME_SOURCE_MASK                    0x00000007U //BIT0-2 RW �ⲿ��ʱ�ź�Դ b010���ⲿ��B��-����b011���ⲿ��B��-����b100:�ⲿ��IRIG-B(����)��b101:�ⲿ��IRIG-B(����)
#define PPS_1X_OUT_SHIFT                        4U
#define PPS_1x_OUT_MASK                         0x00000010U //BIT4 RW 1PPSx1�����ź��Ƿ����
#define PPS_2X_OUT_SHIFT                        5U
#define PPS_2x_OUT_MASK                         0x00000020U //BIT5 RW 1PPSx2�����ź��Ƿ����
#define PPS_3X_OUT_SHIFT                        6U
#define PPS_3x_OUT_MASK                         0x00000040U //BIT6 RW 1PPSx3�����ź��Ƿ����
#define SV_SYNC_TYPE_SHIFT                      8U
#define SV_SYNC_TYPE_MASK                       0x00000700U //BIT8-10 RW ����ֵͬ������ b000:�������ͬ�����ƣ�b010:ֱ����ֵͬ�����ƣ�b011:��ʱ�ɲ�ͬ������
#define SV_DELAY_SHIFT                          13U
#define SV_DELAY_MASK                           0x00FFE000U //BIT13-23 RW ����ֵ��ʱ���ƣ���λ1us (������2000us������IEC61850-9-2 SV���ķ�����ʱ����)
#define BUS_ADR_SHIFT                           28U
#define BUS_ADR_MASK                            0xF0000000U //BIT28-21 RO ���߽ڵ��� ��װ�÷����Ψһ��ַ���(0~15)
/* FPGA BACK SYS BUS STATUS REG Bit Definitions and Masks �������� FPGA_BACKPLANE_BUS_STATUS_REG */
#define DATA0_CODE_ERR_SHIFT                    0U
#define DATA0_CODE_ERR_MASK                     0x00000001U //BIT0 RW ��������DATA0�����쳣��CPU��ͨ������λд1ʵ����0����
#define DATA1_CODE_ERR_SHIFT                    1U
#define DATA1_CODE_ERR_MASK                     0x00000002U //BIT1 RW ��������DATA1�����쳣��CPU��ͨ������λд1ʵ����0����
#define DATA2_CODE_ERR_SHIFT                    2U
#define DATA2_CODE_ERR_MASK                     0x00000004U //BIT2 RW ��������DATA2�����쳣��CPU��ͨ������λд1ʵ����0����
#define DATA3_CODE_ERR_SHIFT                    3U
#define DATA3_CODE_ERR_MASK                     0x00000008U //BIT3 RW ��������DATA3�����쳣��CPU��ͨ������λд1ʵ����0����
#define DATA0_LEN_OR_CRC_ERR_SHIFT              4U
#define DATA0_LEN_OR_CRC_ERR_MASK               0x00000010U //BIT4 RW ��������DATA0���Ȼ�У���쳣��CPU��ͨ������λд1ʵ����0����
#define DATA1_LEN_OR_CRC_ERR_SHIFT              5U
#define DATA1_LEN_OR_CRC_ERR_MASK               0x00000020U //BIT5 RW ��������DATA1���Ȼ�У���쳣��CPU��ͨ������λд1ʵ����0����
#define DATA2_LEN_OR_CRC_ERR_SHIFT              6U
#define DATA2_LEN_OR_CRC_ERR_MASK               0x00000040U //BIT6 RW ��������DATA2���Ȼ�У���쳣��CPU��ͨ������λд1ʵ����0����
#define DATA3_LEN_OR_CRC_ERR_SHIFT              7U
#define DATA3_LEN_OR_CRC_ERR_MASK               0x00000080U //BIT7 RW ��������DATA3���Ȼ�У���쳣��CPU��ͨ������λд1ʵ����0����
#define GET_BUS_CONTROL_FAIL_SHIFT              8U
#define GET_BUS_CONTROL_FAIL_MASK               0x00000100U //BIT8 RW ��γ���δ��ȡ���߿���Ȩ������16���������ٲ�ʧ����δ��ȡ���߿���Ȩ(���߾���ʧ��)�������޷������ķ��ͳ�ȥ��CPU��ͨ������λд1ʵ����0������
#define TRANS_SEQ_ERR_SHIFT                     9U
#define TRANS_SEQ_ERR_MASK                      0x00000200U //BIT9 RW ���Ĵ���ʱ������ڱ��ĵ��첽���ݶμ�⵽ͬ�����ݶ���Ϣ�����Ĵ���ʱ�����CPU��ͨ������λд1ʵ����0������
#define MSG_BIT_ERR_SHIFT                       10U
#define MSG_BIT_ERR_MASK                        0x00000400U //BIT10 RW ����λ�����ڱ����첽���ݶμ�⵽λ����(������Ե�ƽ�ջ����Ե�ƽ)��CPU��ͨ������λд1ʵ����0������
#define BUF_OVERFLOW_ERR_SHIFT                  11U
#define BUF_OVERFLOW_ERR_MASK                   0x00000800U //BIT11 RW �������������λ��1˵�����ݻ����������CPU��ͨ������λд1ʵ����0������

#define FPGA_TIMER_MASK                         0x001FFFFFU

/* 
   SV msg receive region SV���Ľ���
SV���Ľ���ģ�����ڵ�Ե��������ʽ�½��պϲ���Ԫ���͵�4000Hz�����ʵ�IEC61850-9-2����ֵ���ģ� 
���������öԱ��Ľ��й��˺Ͳ���ֵͬ������

SV���Ľ���ģ���ַ����(0x00~0xFF)
0x00        :   ���Ŀ��ƼĴ���
0x04        :   ���Ĺ��˼Ĵ���0
0x08        :   ���Ĺ��˼Ĵ���1
0x0C        :   ���Ĺ��˼Ĵ���2
0x10        :   ���Ĺ��˼Ĵ���3
...
*/
#define FPGA_SV_RECEIVE_OFFSET                  0x1000
#define FPGA_SV_RECEIVE_CONFIG_REG              (FPGA_BASE + FPGA_SV_RECEIVE_OFFSET + 0x00)     //���Ŀ��ƼĴ���
#define FPGA_SV_FILTER_CONFIG0_REG              (FPGA_BASE + FPGA_SV_RECEIVE_OFFSET + 0x04)     //���Ĺ��˼Ĵ���0
#define FPGA_SV_FILTER_CONFIG1_REG              (FPGA_BASE + FPGA_SV_RECEIVE_OFFSET + 0x08)     //���Ĺ��˼Ĵ���1
#define FPGA_SV_FILTER_CONFIG2_REG              (FPGA_BASE + FPGA_SV_RECEIVE_OFFSET + 0x0C)     //���Ĺ��˼Ĵ���2
#define FPGA_SV_FILTER_CONFIG3_REG              (FPGA_BASE + FPGA_SV_RECEIVE_OFFSET + 0x10)     //���Ĺ��˼Ĵ���3
#define FPGA_SV_RECEIVE_REGION_LEN              0x14        //ÿ������ģ������ռ��0x14
#define FPGA_SV_RECEIVE_REGION_MAX_CNT          6           //SV����ģ�����������������ڣ�0~5��

/* SV msg receive config REG Bit Definitions and Masks ��������FPGA_SV_RECEIVE_CONFIG_REG */
#define SV_RECEIVE_PHASE_ANGLE_SHIFT            0U
#define SV_RECEIVE_PHASE_ANGLE_MASK             0x00001FFFU //BIT0-12 RW SV���ձ��ĵ����(�Ƕȵ�λ:�֣��������ö����Ʋ����ʾ)
#define SV_RECEIVE_RATED_DELAY_VALID_SHIFT      21U
#define SV_RECEIVE_RATED_DELAY_VALID_MASK       0x00200000U //BIT21 RW SV���ձ��Ķ��ʱ�Ƿ���Ч 0:��Ч 1:��Ч ��Ĭ�ϵ�һͨ��Ϊ���ʱͨ����
#define SV_RECEIVE_NET_PORT_SHIFT               27U
#define SV_RECEIVE_NET_PORT_MASK                0x78000000U //BIT27-30 RW SV��������(0~15) ������ ����3~8
#define SV_RECEIVE_ENABLE_SHIFT                 31U
#define SV_RECEIVE_ENABLE_MASK                  0x80000000U //BIT31 RW SV����ʹ��
/* SV msg receive fitler0 REG Bit Definitions and Masks �������� FPGA_SV_FILTER_CONFIG0_REG*/
#define SV_RECEIVE_DES_MAC_BYTE6_SHIFT          0U
#define SV_RECEIVE_DES_MAC_BYTE6_MASK           0x000000FFU //BIT0-7 RW SV���ձ���Ŀ��MAC��ַ��6�ֽ�����(Byte6)
#define SV_RECEIVE_DES_MAC_BYTE5_SHIFT          8U
#define SV_RECEIVE_DES_MAC_BYTE5_MASK           0x0000FF00U //BIT8-15 RW SV���ձ���Ŀ��MAC��ַ��5�ֽ�����(Byte5)
#define SV_RECEIVE_APPID_SHIFT                  16U
#define SV_RECEIVE_APPID_MASK                   0xFFFF0000U //BIT16-31 RW SV���ձ���APPID
/* SV msg receive fitler1 REG Bit Definitions and Masks �������� FPGA_SV_FILTER_CONFIG1_REG*/
#define SV_RECEIVE_SVID_CRC32_SHIFT             0U
#define SV_RECEIVE_SVID_CRC32_MASK              0xFFFFFFFFU //BIT0-31 RW SV���ձ���svID�ַ���CRC-32����У����
/* SV msg receive fitler2 REG Bit Definitions and Masks �������� FPGA_SV_FILTER_CONFIG2_REG*/
#define SV_RECEIVE_CONFREV_SHIFT                0U
#define SV_RECEIVE_CONFREV_MASK                 0xFFFFFFFFU //BIT0-31 RW SV���ձ���confRev
/* SV msg receive fitler3 REG Bit Definitions and Masks �������� FPGA_SV_FILTER_CONFIG3_REG*/
#define SV_RECEIVE_ASDU_CHAN_CNT_SHIFT          0U
#define SV_RECEIVE_ASDU_CHAN_CNT_MASK           0x0000003FU //BIT0-5 SV���ձ���ÿ��ASDU����ͨ����Ŀ

/* 
   SV msg send region SV���ķ���
SV���ķ���ģ�����ڽ�ģ��������ֵ����Ե�SV���Ľ��ղ���ֵ������ֵͬ���������֯Ϊ 
��̫��IEC61850-9-2����ֵ����(���49������ͨ��)�����͸�����װ�á�

SV���ķ���ģ���ַ����(0x000~0x7FF)
0x000       :   ���Ϳ��ƼĴ���
0x010       :   ����Ѱַ�Ĵ���
0x040~0x2FF :   ͨ�����üĴ���
0x300~0x7FF :   ���ͱ��Ļ�����
 
*/
#define FPGA_SV_SEND_OFFSET                     0x3000
#define FPGA_SV_SEND_CONFIG_REG                 (FPGA_BASE + FPGA_SV_SEND_OFFSET + 0x00)    //���Ϳ��ƼĴ���
#define FPGA_SV_SEND_ADDRESSING_REG             (FPGA_BASE + FPGA_SV_SEND_OFFSET + 0x10)    //����Ѱַ�Ĵ���
#define FPGA_SV_SEND_CHAN_CONFIG_REG            (FPGA_BASE + FPGA_SV_SEND_OFFSET + 0x40)    //ͨ�����üĴ��� (48��ͨ����ÿ��ͨ��ռ��3��DWORD�Ĵ���)
#define FPGA_SV_SEND_BUF_REG                    (FPGA_BASE + FPGA_SV_SEND_OFFSET + 0x300)   //���ͱ��Ļ����� (ADDR = 0x300~0x7FF)
#define FPGA_SV_SEND_CHAN_MAX_CNT               49      //���֧��49��ͨ�� 00-48
#define FPGA_SV_SEND_CHAN_CONFIG_REG_MAX_CNT    3       //ÿ��ͨ��ռ��3��REG DWORD
#define FPGA_SV_SEND_CHAN_CONFIG_REGS_LENS      0x0C    //ÿ��ͨ��ռ���ֽ��� 3*4 = 12 = 0x0C

/* SV send config REG Bit Definitions and Masks �������� FPGA_SV_SEND_CONFIG_REG*/
#define SV_SEND_MSG_LEN_SHIFT                   0U
#define SV_SEND_MSG_LEN_MASK                    0x000003FFU //BIT0-9 RW ���ĳ���(1~1023�ֽ�)
#define SV_SEND_CHAN_CNT_SHIFT                  10U
#define SV_SEND_CHAN_CNT_MASK                   0x0000FC00U //BIT10-15 RW ͨ����Ŀ(1~49)
#define SV_SEND_FORCE_TEST_SHIFT                16U
#define SV_SEND_FORCE_TEST_MASK                 0x00010000U //BIT16 RW ǿ�Ʋ��� 0:�� 1��
#define SV_SEND_FORCE_SYN_SHIFT                 17U
#define SV_SEND_FORCE_SYN_MASK                  0x00020000U //BIT17 RW ǿ��ͬ�� 0:�� 1��
#define SV_SEND_NET_PORT_SHIFT                  18U
#define SV_SEND_NET_PORT_MASK                   0x7FFC0000U //BIT18-30 RW ����ʹ�� ÿλ��Ӧһ���ⲿ���ڣ���0~12����(Ŀǰ����3~8)
#define SV_SEND_CONFIG_UPDATE_SHIFT             31U
#define SV_SEND_CONFIG_UPDATE_MASK              0x80000000U //BIT31 RW ���ø���
/* 
    SV_SEND_CONFIG_UPDATE_MASK ���ø��� ����˵����
0:����Ը�ģ�����е����üĴ����ͷ��ͱ��Ļ������д�����������������д������󽫸ñ�־λ��1�� 
  ��֪���ķ��Ϳ�������Ҫ�������á����ķ��Ϳ�����������һ������ֵ���ķ���ǰ���µ����ð��Ƶ�
  ����ִ�����������µ�������֯����ֵ���ģ��ҽ��ñ�־λ��0��
1:��ֹ�Ը�ģ�����е����üĴ����ͷ��ͱ��Ļ������д�������
ע:Ϊ����PT�л����ܣ���������üĴ����ͷ��ͱ��Ļ������ʵʱд���޸ģ��ұ�֤����������(���޸���) 
����һ������ֵ����ͬʱ��Ч��������������������ֱ�Ϊ���ö�д������������ִ���������ö�д�������� 
����ж���д������������ִ������ʵ����Ч�������������������д�����������ø��±�־��1���д���� 
���������ݰ���������ִ������ʵ���������ݸ��¡�
*/ 


/* SV send msg addressing config REG Bit Definitions and Masks ��������FPGA_SV_SEND_ADDRESSING_REG */
#define SV_SEND_CHAN0_VALUE_POS_SHIFT           0U
#define SV_SEND_CHAN0_VALUE_POS_MASK            0x000003FFU //BIT0-9 RW smpSynch value���ֽ����
#define SV_SEND_SMPSYNCH_VALUE_POS_SHIFT        10U
#define SV_SEND_SMPSYNCH_VALUE_POS_MASK         0x000FFC00U //BIT10-19 RW channel0 value���ֽ����
#define SV_SEND_SMPCNT_VALUE_POS_SHIFT          20U
#define SV_SEND_SMPCNT_VALUE_POS_MASK           0x3FF00000U //BIT20-29 RW smpCnt value���ֽ����
/* 
    ���ĵ� 
    SV_SEND_CHAN0_VALUE_POS_MASK
    SV_SEND_SMPSYNCH_VALUE_POS_MASK
    SV_SEND_SMPCNT_VALUE_POS_MASK
    �ڱ��ķ��ͻ����� �ж��ж�Ӧ��λ�ã����˵���ĵ�
*/

/* SV send channel config REG Bit Definitions and Masks */
#define SV_SEND_SRC_FACTOR_SHIFT                0U
#define SV_SEND_SRC_FACTOR_MASK                 0x0000FFFFU //BIT0-15 RW ����Դϵ��(�з����������������Դ���š���־Ϊ1����ϵ����Ŵ�32��)
#define SV_SEND_SRC_SCALE_SHIFT                 16U
#define SV_SEND_SRC_SCALE_MASK                  0x00010000U //BIT16 RW ����Դ���� 0:ԭֵ 1:�Ŵ�
#define SV_SEND_SRC_CHAN_SHIFT                  18U
#define SV_SEND_SRC_CHAN_MASK                   0x00FC0000U //BIT18-23 RW ����Դͨ�� 00-48
#define SV_SEND_SRC_CHOOSE_SHIFT                24U
#define SV_SEND_SRC_CHOOSE_MASK                 0x1F000000U //BIT24-28 RW ����Դѡ�� 00-...
/* 
     SV_SEND_SRC_CHOOSE_MASK����˵��
     ����AD����ѡ0/1�ֱ��ʶAD0��AD1
     ����SV����ѡ0-5��ָ������ģ������(0 ~ FPGA_SV_RECEIVE_REGION_MAX_CNT-1)
*/

#define SV_SEND_SRC_TYPE_SHIFT                  29U
#define SV_SEND_SRC_TYPE_MASK                   0x60000000U //BIT29 RW ����Դ���� b00:AD b01:SV b11:���ʱ
#define SV_SEND_SRC_ENABLE_SHIFT                31U
#define SV_SEND_SRC_ENABLE_MASK                 0x80000000U //BIT31 RW ����Դʹ�� 0:��ֹ 1:ʹ��


/* 
   Goose Receive region GOOSE���Ľ���
   GOOSE���Ľ���ģ�����ڽ��ⲿ��̫���ڽ��յ���GOOSE���ľ�����Ҫ�ı��Ľ������������ƴ����ת����CPU���к�������
GOOSE���Ľ���ģ���ַ����(0x000~0x3FF)
0x000       :   ���ƼĴ���
0x004       :   ���ܼĴ���
0x020       :   GOOSE���ձ�������ʹ�ܼĴ���0
0x024       :   GOOSE���ձ�������ʹ�ܼĴ���1
0x028       :   GOOSE���ձ�������ʹ�ܼĴ���2
0x040       :   GOOSE���ձ�������״̬�Ĵ���0
0x044       :   GOOSE���ձ�������״̬�Ĵ���1
0x048       :   GOOSE���ձ�������״̬�Ĵ���2
0x080~37F   :   GOOSE���ձ��Ĺ��˼Ĵ���

*/
#define FPGA_GOOSE_RECEIVE_OFFSET               0x4800
#define FPGA_GOOSE_CTRL_REG                     (FPGA_BASE + FPGA_GOOSE_RECEIVE_OFFSET + 0x00)  //���ƼĴ���
#define FPGA_GOOSE_FUN_REG                      (FPGA_BASE + FPGA_GOOSE_RECEIVE_OFFSET + 0x04)  //���ܼĴ���
#define FPGA_GOOSE_STORM_CONTROL_REG            (FPGA_BASE + FPGA_GOOSE_RECEIVE_OFFSET + 0x20)  //GOOSE���ձ�������ʹ�ܼĴ���
#define FPGA_GOOSE_STORM_CONTROL_STATUS_REG     (FPGA_BASE + FPGA_GOOSE_RECEIVE_OFFSET + 0x40)  //GOOSE���ձ�������״̬�Ĵ���
#define FPGA_GOOSE_FILTER_CRC_REG               (FPGA_BASE + FPGA_GOOSE_RECEIVE_OFFSET + 0x80)  //GOOSE���ձ��Ĺ��˼Ĵ���
#define FPGA_GOOSE_FILTER_CTRL_REG              (FPGA_BASE + FPGA_GOOSE_RECEIVE_OFFSET + 0x84)  //GOOSE���ձ��Ĺ��˿��ƼĴ���
#define FPGA_GOOSE_RECIEVE_FILETER_CONST        96                                              //GOOSE���ձ��Ĺ���������

/* 
   AXI HP communication control region (SV GOOSE) AXI_HPͨ�ſ�����
AXI_HPͨ�ſ�����ģ�����ڿ���ZYNQоƬFPGA��CPU(ARM)֮��ĸ���ͨ�Žӿ�AXI_HP��AXI_HP�˿���FPGA������ 
��������FPGA��CPU֮����а��ƣ�CPU��ҪԤ�ȷ���һ�������ڴ�ռ��FPGA��Ϊ���İ��ƻ���ռ䡣FPGA���� 
CPU�ڴ�ռ����BDģʽ���շ������������֧��128������ռ�(0~127)��ÿ�黺��ռ��СΪ2K�ֽڣ�ǰ4�ֽ� 
���ڱ�ʶ�������ݳ��ȣ������ռ����ڴ�ű������ݡ�

AXI_HPͨ�ſ�����ģ���ַ����(0x00~0xFF)
0x20            : ���ͱ��Ķ�Ӧ�����ڴ����ʼ��ַ
0x40~0x4F   : ���ͱ��Ļ���������
0x60            : ���ձ��Ķ�Ӧ�����ڴ����ʼ��ַ
0x80~0x8F   : ���ձ��Ļ���������

*/
#define FPGA_SV_AXI_HP_CONTROL_OFFSET           0x4100
#define FGPA_SV_AXI_HP_TX_BUF_ADDR_REG          (FPGA_BASE + FPGA_SV_AXI_HP_CONTROL_OFFSET + 0x20)  //CPU->FPGA���ͱ��Ķ�Ӧ�����ڴ����ʼ��ַ
#define FGPA_SV_AXI_HP_TX_BD_REG                (FPGA_BASE + FPGA_SV_AXI_HP_CONTROL_OFFSET + 0x40)  //CPU->FPGA���ͱ��Ļ����������׵�ַ
#define FGPA_SV_AXI_HP_RX_BUF_ADDR_REG          (FPGA_BASE + FPGA_SV_AXI_HP_CONTROL_OFFSET + 0x60)  //FPGA->CPU���ձ��Ķ�Ӧ�����ڴ����ʼ��ַ
#define FGPA_SV_AXI_HP_RX_BD_REG                (FPGA_BASE + FPGA_SV_AXI_HP_CONTROL_OFFSET + 0x80)  //FPGA->CPU���ձ��Ļ����������׵�ַ

/* 
   Sampling msg receive region ����ֵ��������
   0x3C00 ����ֵ��������0
   0x3E00 ����ֵ��������1
 
����ֵ��������ģ�����ڶ������;�������ֵͬ�������Ĳ���ֵ���ݣ���������״̬��Ϣ���ո�ʽ 
Ҫ����ϳ��ڲ�����ֵ���ģ�ͨ������������ߴ��͸�������ָ���ڵ�CPU����

����ֵ��������ģ���ַ����(0x000~0x1FF)
0x000       :   �������ƼĴ���
0x040~0x13F :   ������üĴ���

*/
#define FPGA_SAMPING_MSG_INTERVAL_LEN           8
#define FPGA_SAMPING_MSG_INTERVAL_CNT           32          //�������
#define FPGA_SAMPING_MSG0_RECEIVE_OFFSET        0x3C00      //����ֵ��������0
#define FPGA_SAMPING_MSG0_CONTROL_REG           (FPGA_BASE + FPGA_SAMPING_MSG0_RECEIVE_OFFSET + 0x00)   //�������ƼĴ���
#define FPGA_SAMPING_MSG0_INTERVAL_CONFIG0_REG  (FPGA_BASE + FPGA_SAMPING_MSG0_RECEIVE_OFFSET + 0x40)   //���0���üĴ���0
#define FPGA_SAMPING_MSG0_INTERVAL_CONFIG1_REG  (FPGA_BASE + FPGA_SAMPING_MSG0_RECEIVE_OFFSET + 0x44)   //���0���üĴ���1
/*
    ������üĴ��� �ܹ�0~31 ��32��������üĴ�����ÿ�����ռ��2��REG DWORD
*/ 

#define FPGA_SAMPING_MSG1_RECEIVE_OFFSET        0x3E00      //����ֵ��������1
#define FPGA_SAMPING_MSG1_CONTROL_REG           (FPGA_BASE + FPGA_SAMPING_MSG1_RECEIVE_OFFSET + 0x00)
#define FPGA_SAMPING_MSG1_INTERVAL_CONFIG0_REG  (FPGA_BASE + FPGA_SAMPING_MSG1_RECEIVE_OFFSET + 0x40)
#define FPGA_SAMPING_MSG1_INTERVAL_CONFIG1_REG  (FPGA_BASE + FPGA_SAMPING_MSG1_RECEIVE_OFFSET + 0x44)

/* FPGA sampling ctrl reg Definitions and Masks  �������ò������ƼĴ��� */
#define SAMPLING_MSG_BUS_DES_ADR_SHIFT          0U
#define SAMPLING_MSG_BUS_DES_ADR_MASK           0x0000FFFFU //BIT0-15 RW ����Ŀ��ڵ�ӳ�� ÿλ��Ӧһ���ڵ㣬��00-15�ڵ�
#define SAMPLING_MSG_INTERVAL_CNT_SHIFT         16U
#define SAMPLING_MSG_INTERVAL_CNT_MASK          0x003F0000U //BIT16-21 RW ���ͼ����Ŀ(1~32) �����������ͱ��Ķ��ĵļ����Ŀ������AD��SV�������
#define SAMPLING_MSG_SAMPLE_RATE_SHIFT          22U
#define SAMPLING_MSG_SAMPLE_RATE_MASK           0x00C00000U //BIT22-23 RW ���ݲ����� b00:24�� b01:48�� b10:80��
/* FPGA sampling interval config reg 0 Definitions and Masks  �������ü�����üĴ���0*/
#define SAMPLING_INTERVAL_SRC_CHOOSE_SHIFT      0U
#define SAMPLING_INTERVAL_SRC_CHOOSE_MASK       0x0000001FU //BIT0-4 ����Դѡ��
/* 
   SAMPLING_INTERVAL_SRC_CHOOSE_MASK ͬ SV_SEND_SRC_CHOOSE_MASK
     ����AD����ѡ0/1�ֱ��ʶAD0��AD1
     ����SV����ѡ0-5��ָ������ģ������(0 ~ FPGA_SV_RECEIVE_REGION_MAX_CNT-1)
*/

#define SAMPLING_INTERVAL_SRC_TYPE_SHIFT        8U
#define SAMPLING_INTERVAL_SRC_TYPE_MASK         0x00000300U //BIT8 RW ����Դ���� b00:AD b01:SV
#define SAMPLING_INTERVAL_SRC_CHAN_MAPPING2_SHIFT   15U
#define SAMPLING_INTERVAL_SRC_CHAN_MAPPING2_MASK    0xFF800000U //BIT15-31 RW ����Դͨ��ӳ��2(������Դͨ��ӳ��1����ʹ��)
/* FPGA sampling interval config reg 1 Definitions and Masks */
#define SAMPLING_INTERVAL_SRC_CHAN_MAPPING1_SHIFT   0U
#define SAMPLING_INTERVAL_SRC_CHAN_MAPPING1_MASK    0xFFFFFFFFU //BIT0-31 RW ����Դͨ��ӳ��1(������Դͨ��ӳ��2����ʹ��)
/*
    ����Դͨ��ӳ��
    BIT[00]-ͨ��00
    ...
    BIT[48]-ͨ��48  --->  ����Դͨ��ӳ��2 ��BIT[31]
*/ 


/*FPGA��չCPLD */
#define FPGA_CPLD_OFFSET                  0x5000
#define FPGA_CPLD_BASE                     (FPGA_BASE + FPGA_CPLD_OFFSET)




#endif /* SRC_H_BAREMETAL_PLT_HD_FPGA_DEF_H_ */
