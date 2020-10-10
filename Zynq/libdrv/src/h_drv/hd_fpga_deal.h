/*
 * @Author: zhongwei
 * @Date: 2020/3/31 17:44:09
 * @Description: FPGA处理接口
 * @File: hd_fpga_deal.h
 *
*/

#ifndef SRC_H_BAREMETAL_PLT_HD_FPGA_DEAL_H_
#define SRC_H_BAREMETAL_PLT_HD_FPGA_DEAL_H_

typedef struct{
    uint8 year;     //年00-99
    uint16 day;     //日（一年中的第几天）
    uint8 hour;     //时
    uint8 minute;   //分
    uint8 second;   //秒
    uint8 leap_second_forecast; //闰秒预告
    uint8 leap_second_flag;     //闰秒标志
    uint8 summer_forecast;      //夏令时预告
    uint8 summer_flag;          //夏令时标志
    uint8 t_offset_sign;        //时间偏移符号位
    uint8 t_offset;             //时间偏移小时
    uint8 t_offset_half;        //时间偏移0.5小时
    uint8 quality;              //时间品质
}TFPGA_GPS_TIME;

//IRIG-B码时间 奇校验检查
BOOL fpga_gps_time_odd_chk(uint32 data[3]);

//FPGA时间寄存器解码 -> TFPGA_GPS_TIME
void fpga_gps_time_decode(uint32 data[2], TFPGA_GPS_TIME * pTime);

//ns计数器解码 
void fpga_gps_time_ns_decode(uint32 data2, uint8 * state, uint32 * ns);

//计算GOOSE接收报文”目的MAC+APPID+GoCBRef+DatSet+GoID”数据段CRC-32生成校验码 ，用于Goose接收过滤
uint32 fpga_calc_goose_rcv_crc32(const uint8 * dest_mac, 
                                                  uint16 app_id, 
                                                  const char * gocbRef,
                                                  const char * datSet,
                                                  const char * goID);

//添加一个goose接收过滤器
STATUS fpga_add_one_goose_rcv(const uint8 * dest_mac, 
                                                  uint16 app_id, 
                                                  const char * gocbRef,
                                                  const char * datSet,
                                                  const char * goID,
                                                  uint32 in_port_bitwise);

#endif /* SRC_H_BAREMETAL_PLT_HD_FPGA_DEAL_H_ */
