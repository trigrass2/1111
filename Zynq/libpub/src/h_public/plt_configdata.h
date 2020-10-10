/*
 * @Author: zhongwei
 * @Date: 2020/04/07 17:32:02
 * @Description: BM<->Linux之间传送初始配置信息定义
 * @File: plt_configdata.h
 *
*/

#ifndef SRC_H_PUBLIC_PLT_CONFIGDATA_H_
#define SRC_H_PUBLIC_PLT_CONFIGDATA_H_

#define STR_FOLDER_CONFIG_CUSTOM            "[custom]"      //自定义配置项

/*
    MMS_LIB库解析ccd文件得到的Goose接收发送配置数据和SV接收发送配置数据
    这里定义的是TMC_CONFIG_DATA中存储数据的标示头
    参照 app_bm_config_data.cc 中 _add_one_item 进行存储
    存储的数据类型为VARREG_TYPE_char_p，数据长度为实际内容长度
*/
#define STR_FOLDER_PROCESS_CFG              "ProcessCfg"    //对应POINT_TYPE
#define STR_PROCESS_GOOSE_RCV_CFG_FLAG      "GooseRcv"      //对应goose接收的POINT_ID
#define STR_PROCESS_GOOSE_SEND_CFG_FLAG     "GooseSend"
#define STR_PROCESS_SV_RCV_CFG_FLAG         "SVRcv"
#define STR_PROCESS_SV_SEND_CFG_FLAG        "SVSend"


#endif /* SRC_H_PUBLIC_PLT_CONFIGDATA_H_ */
