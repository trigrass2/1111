/*
 * @Author: zhongwei
 * @Date: 2020/3/31 17:44:09
 * @Description: FPGA����ӿ�
 * @File: hd_fpga_deal.h
 *
*/

#ifndef SRC_H_BAREMETAL_PLT_HD_FPGA_DEAL_H_
#define SRC_H_BAREMETAL_PLT_HD_FPGA_DEAL_H_

typedef struct{
    uint8 year;     //��00-99
    uint16 day;     //�գ�һ���еĵڼ��죩
    uint8 hour;     //ʱ
    uint8 minute;   //��
    uint8 second;   //��
    uint8 leap_second_forecast; //����Ԥ��
    uint8 leap_second_flag;     //�����־
    uint8 summer_forecast;      //����ʱԤ��
    uint8 summer_flag;          //����ʱ��־
    uint8 t_offset_sign;        //ʱ��ƫ�Ʒ���λ
    uint8 t_offset;             //ʱ��ƫ��Сʱ
    uint8 t_offset_half;        //ʱ��ƫ��0.5Сʱ
    uint8 quality;              //ʱ��Ʒ��
}TFPGA_GPS_TIME;

//IRIG-B��ʱ�� ��У����
BOOL fpga_gps_time_odd_chk(uint32 data[3]);

//FPGAʱ��Ĵ������� -> TFPGA_GPS_TIME
void fpga_gps_time_decode(uint32 data[2], TFPGA_GPS_TIME * pTime);

//ns���������� 
void fpga_gps_time_ns_decode(uint32 data2, uint8 * state, uint32 * ns);

//����GOOSE���ձ��ġ�Ŀ��MAC+APPID+GoCBRef+DatSet+GoID�����ݶ�CRC-32����У���� ������Goose���չ���
uint32 fpga_calc_goose_rcv_crc32(const uint8 * dest_mac, 
                                                  uint16 app_id, 
                                                  const char * gocbRef,
                                                  const char * datSet,
                                                  const char * goID);

//���һ��goose���չ�����
STATUS fpga_add_one_goose_rcv(const uint8 * dest_mac, 
                                                  uint16 app_id, 
                                                  const char * gocbRef,
                                                  const char * datSet,
                                                  const char * goID,
                                                  uint32 in_port_bitwise);

#endif /* SRC_H_BAREMETAL_PLT_HD_FPGA_DEAL_H_ */
