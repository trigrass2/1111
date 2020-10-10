/*
 * @Author: zhongwei
 * @Date: 2019-12-12 11:07:40
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-12-12 11:09:18
 * @Description: 管理单元配置代码
 * @FilePath: \ZynqBM\src\config\ptl_MU_Config.h
 */
#ifndef SRC_INCLUDE_PTL_MU_CONFIG_H_
#define SRC_INCLUDE_PTL_MU_CONFIG_H_

//定义最多支持的CPU个数 暂时定义为4个
#define _MAX_CPU_COUNT_ 4

enum{
    CPU_NUM_MU      = -1,           //MU核
    CPU_NUM_BM_0    = 0,
    CPU_NUM_BM_1    = 1,
    CPU_NUM_BM_2    = 2,
    CPU_NUM_BM_3    = 3,
    CPU_NUM_BM_4    = 4,
};

#define MU_MAX_PATH_NAME_LEN        128     //路径名的最大长度
#define MU_MAX_FILE_NAME_LEN        64      //文件名的最大长度
#define ARM_MAX_FILE_PATH_NAME_LEN  MU_MAX_PATH_NAME_LEN

/************************************************************************/
//  定义文件夹
//
/************************************************************************/
#define MU_PATH_DIR_LINUX_TMP               "/tmp"                           //系统临时文件夹    /tmp
#define MU_PATH_DIR_TEMP                    MU_PATH_DIR_LINUX_TMP "/temp"    //软件临时文件夹    /tmp/temp
#define MU_PATH_DIR_EMCC_APP                "/mnt/emmc_app"                  //emmc设备的app分区
#define MU_PATH_DIR_EMCC_DATA               "/mnt/emmc_data"                 //emmc设备的data分区
#define MU_PATH_DIR_BM_EXTRACT_PATH         MU_PATH_DIR_LINUX_TMP "/bmz"     //bm解压后路径      /tmp/bmz
#define MU_PATH_DIR_BM_EXTRACT_CPU_PATH     MU_PATH_DIR_BM_EXTRACT_PATH "/cpu%d"     //bm解压后路径，分CPU存放 /tmp/bmz/cpu0-1
#define MU_PATH_DIR_SAVE_PATH               MU_PATH_DIR_EMCC_DATA "/save"    //存储路径（掉电保存数据的存储路径) /mnt/emcc_data/save
#define MU_PATH_DIR_LOG_PATH                MU_PATH_DIR_EMCC_DATA "/log"     ///mnt/emcc_data/log 
#define MU_PATH_DIR_PROCESS_PATH            MU_PATH_DIR_EMCC_DATA "/process_data"   //存储过程层配置文件的目录（由MMS库解析ccd后生成的文件）

/************************************************************************/
//  定义几个文件名
//
/************************************************************************/
#define MU_FILE_NAME_MAINAPP                "mainApp.elf"               //linux主应用文件名
#define MU_FILE_NAME_BMRUN                  "bmrun.elf"                 //启动core0应用文件名
#define MU_FILE_NAME_EPIDE_IDE              "EPIDE.ide"                 //ide文件
#define MU_FILE_NAME_BMZ_N_CODE             "zynqBM%d.bmz"              //打包的BM应用
#define MU_FILE_NAME_BMZ_CODE               "zynqBM.bmz"                //打包的BM应用
#define MU_FILE_NAME_BM_CODE                "zynqBM.bm"                 //BM执行程序名称
#define MU_FILE_NAME_BM_CFPX                "zynqBM.cfpx"               //.cfpx配置文件
#define MU_FILE_NAME_SAVE_SETTING           "setting%d.yaml"            //设定值存储文件名称
#define MU_FILE_NAME_SAVE_STR_SETTING       "str_setting%d.yaml"        //字符串定值存储文件名称
#define MU_FILE_NAME_SAVE_PROT_SETTING      "prot_setting%d.yaml"       //保护定值存储文件名称
#define MU_FILE_NAME_SAVE_MU_SETTING        "mu_setting.yaml"           //APP内置定值存储文件名称
#define MU_FILE_NAME_CONFIG_YAML            "config.yaml"               //全局配置yaml文件
#define MU_FILE_NAME_BM_CONFIG_DATA         "config.data"               //BM初始化config data文件
#define MU_FILE_NAME_REALTIME_DB            "realtime%d.db"             //实时数据保存文件
#define MU_FILE_NAME_PROCESS_GOOSE_RCV_FILE     "ProcessGooseRcv.dat"   //过程层Goose接收配置文件
#define MU_FILE_NAME_PROCESS_GOOSE_SEND_FILE    "ProcessGooseSend.dat"  //过程层Goose发送配置文件
#define MU_FILE_NAME_PROCESS_SV_RCV_FILE        "ProcessSvRcv.dat"      //过程层SV接收配置文件
#define MU_FILE_NAME_PROCESS_SV_SEND_FILE       "ProcessSvSend.dat"     //过程层SV发送配置文件

/************************************************************************/
//  定义几个文件夹名称
//
/************************************************************************/
#define MU_FOLDER_NAME_SETTING_SAVE         "set"
#define MU_FOLDER_NAME_REPORT_SAVE          "report"
#define MU_FOLDER_NAME_COMTRADE_SAVE        "COMTRADE"

/************************************************************************/
//  定义带路径文件名
//
/************************************************************************/
#define MU_FILE_PATH_NAME_BM_MD5            MU_PATH_DIR_BM_EXTRACT_PATH "/bm_cpu%d.md5"   //tmp/bmz/bm_cpu0.md5


/************************************************************************/
//  Linux logfile配置 syslog
//  对应文件 /etc/syslog.conf
/*  文件内容
auth,authpriv.*		/var/log/auth.log
*.*;auth,authpriv.none	/var/log/messages
user.*			/var/log/user.log
local1.*                /var/log/bm.log
local2.*                /mnt/emmc_data/log/usr2.log
local3.*                /mnt/emmc_data/log/usr3.log 
*/ 
//  其中/var/log/ 指向/tmp
//  TRACE作为高级消息，保存于APP_LOG_HIGH和APP_LOG_TMP
//  BM传送的作为低级消息，保存于APP_LOG_BM和APP_LOG_LOW
//  其他PRINTF作为低级消息，保存于APP_LOG_LOW和APP_LOG_TMP
/************************************************************************/
#define APP_LOG_TMP_BM      LOG_LOCAL1    // /tmp/bm.log
#define APP_LOG_TMP_LOW     LOG_USER      // /tmp/user.log
#define APP_LOG_TMP_HIGH    LOG_USER      // /tmp/user.log
#define APP_LOG_BM          LOG_LOCAL4    // /mnt/emmc_data/log/bm.log，保存于EMMC，BM消息
#define APP_LOG_LOW         LOG_LOCAL2    // /mnt/emmc_data/log/usr2.log，保存于EMMC，低级消息
#define APP_LOG_HIGH        LOG_LOCAL3    // /mnt/emmc_data/log/usr3.log，保存于EMMC，高级消息


#endif /* SRC_INCLUDE_PTL_MU_CONFIG_H_ */
