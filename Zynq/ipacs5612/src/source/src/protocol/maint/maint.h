/*------------------------------------------------------------------------
 $Rev: 27 $
------------------------------------------------------------------------*/

#ifndef MAINT_H
#define MAINT_H

#include "syscfg.h"

#include "pslave.h"

#define MAINT_YCCHAGE_SEND

#define ACK					0x8001	//正确应答
#define NACK				0x8002	//错误应答

#define RESET				0x0103	//复位

#define GET_CLK				0x0a04	//获取时钟
#define SET_CLK				0x0405	//设置时钟
#define GET_DIFFCLK			0x0a05	//获取DIFF时钟
#define SET_DIFFCLK			0x0406	//设置DIFF时钟


#define GET_YC				0x0c10	//读遥测
#define GET_YX_SINGLE		0x0c11	//读单点遥信
#define GET_SOE_SINGLE		0x0c12	//读单点SOE
#define GET_DD				0x0c13	//读电度
#define GET_COS_SINGLE		0x0c14	//读单点COS
#define GET_UI              	      0x0c15  //读电压电流
#define GET_HIS_YC              	      0x0c16  //读历史遥测数据
#define GET_HIS_DD              	      0x0c17  //读历史电度数据
#define GET_EVENT              	      0x0c18  //读事件数据
#define GET_RUNINFO             	      0x0c19  //读运行信息
#define GET_YB              	      0x0c1a  //读事件数据
#define GET_CHG_YC          0x0c1b  //读变化遥测数据           
#define GET_DYX          0x0c1c  //dyx          
#define GET_DSOE          0x0c1d  //dsoe
#define GET_maxvalue          0x0c1e  //极值           
#define GET_ycfloat          0x0c1f  //ycfloat           
#define GET_accurve          0x0c20  //采样波形          
#define GET_wubc          0x0c21  //无功补偿       
#define GET_dotdef          0x0c22  //点号描述     
#define GET_ver          0x0c23  //版本信息
#define GET_addr          0x0c24  //地址信息
#define GET_YBVal          0x0c25  //地址信息
#define GET_COMM          0x0c26  //地址信息
#define GET_BH          0x0c27  //地址信息
#define GET_YXZ          0x0c28  //有效值
#define GET_EQPNAME          0x0c29  //读装置列表
#define GET_GPRSADDR          0x0c2a  //读GPRS地址
#define GET_YC1				0x0c2b	//读遥测
#define GET_YC2				0x0c2c	//读遥测
#define GET_STATE           0x0c2d  //读装置状态 hll add
#define GET_SELFRUNPARA 0x0c2e  //读取装置固有信息和部分运行信息
#define SET_SELFRUNPARA  0x0c2f  //写装置固有信息和部分运行信息
#define GET_ICMD                        0x0c30 //读i命令
#define GET_SoeCosDotDef                        0x0c31 //SOE、COS描述信息
#define Set_GainCommand 	0x0c32 //gain.sys文件操作命令
#define SetDevIDCmd            0x0c33 //设置装置24位ID 号
#define Get_JIAMI                 0x0c34 //获取加密芯片状态
#define GET_Lubo                  0x0c35 //手动触发录波
#define SET_DEV_TYPE        0x0c36 //设置终端出厂型号
#define SET_DEV_MFR          0x0c37 //设置终端制造商
#define GET_PB                        0x0c38  //获取PB_RELATE
#define GET_ADType              0x0c39  //获取采样类型型号

#define SET_IP_BLT              0x0c3a  //蓝牙设置ip
#define SET_GJ_Time             0x0c3b  //告警复归时间设置
#define SET_DEV_SV              0x0c3c  //设置软件版本号
#define GET_LL_ID               0x0c3d  //读取华彩线损模块ID

#define SET_ESN                 0x0c41  //设置华为主板esn到铁电
#define GET_ESN                 0x0c3f  //获取华为主板esn
#define GET_LTE                 0x0c40  //获取华为LTE信息
#define SET_DIFFPara_Init       0x0c3e  //差动参数初始化
#define SET_Maint_Confirm       0x0c42  //设置加密运维认证

#define SET_DEV_MFR_UTF8        0x0c43  //设置终端制造商utf8格式

#define SET_YK				0x0520	//遥控预置
#define DO_YK				0x0521	//遥控执行
#define UNDO_YK				0x0522	//遥控撤销
#define RESET_PROTECT				0x0523	//复归命令
#define RGZS				0x0524	//人工置数
#define SHOWBUFATA				0x0525	//浏览缓冲区数据
#define SET_DCHH				0x0527	//电池活化
#define SET_remotequit				0x0528	//远程退出
#define SET_IP				0x0529	//IP设置
#define SET_com				0x052a	//串口设置
#define CLR_records			0x052b	//记录清除
#define GPRS_POWER			0x052c	//gprs电源控制
#define SET_BH			0x052d	//gprs电源控制
#define SET_GPRSADDR			0x052e	//gprs地址设置
#define SET_HZGZ			0x052f	//变频采样
#define SET_PB                 0x0530  //设置PB_RELATE
#define SET_JIAMICER   0x0531  //设置双向加密的终端证书
#define SET_ADType      0x0532  //设置采样芯片型号
#define SET_DIFF        0x0533  //写差动参数
#define GET_DIFF        0x0534  //读差动参数

#define YT_SET				0x8404	//遥调输出

#define READ_FILE			0x0f30	//读文件
#define WRITE_FILE			0x0f31	//写文件
#define READ_DIR			      0x0f32	//读目录
#define BUILD_DIR			0x0f33	//创建目录
#define RENAME_FILE			0x0f34	//文件改名
#define RENAME_DIR			0x0f35	//目录改名
#define SET_FILE_ATTR		0x0f36	//修改文件属性
#define GET_FILE_ATTR		0x0f37	//获取文件属性
#define DEL_FILE			      0x0f38	//删除文件
#define DEL_DIR				0x0f39	//删除目录

#define INTO_SHELL			0x0540	//进入shell模式
#define YC_PARASET			0x0542	//遥测参数整定
#define YC_PARAGET          0x0541  
#define YC_PARAVALGET       0x0543
#define YC_PARAVALSET       0x0544
#define YC_PARAZERO           0x0545
#define YC_COM_TEST            0x0546  //串口测试

#define PROG_READ           0x0930
#define PROG_WRITE          0x0931
#define BOOT_WRITE          0x0932
#define MMI_PROG_WRITE      0x0933
#define BOOTSTRAP_WRITE      0x0934

#define FORMAT_DISK     0x0906

#define MAINT_FILE_NAMEERR     0x12
#define MAINT_FILE_ERR         0x10
#define MAINT_DISK_ERR         0x20
#define MAINT_BOOT_ERR         0x30
#define MAINT_SEC_ERR          0x40


#define END_MAINTAIN		0x054f	//结束维护

#define MIN_RECEIVE_SIZE  7    //最小接收帧长
#define MAX_FRAME_SIZE  1280  
#define FILE_NAME_LEN	32
#define m25p_file_sector_star 52
#define m25p_file_addr_star 0x340000

#define PROG_FILE_MAX    1024
struct VFrame
{
    BYTE StartCode1;  	//启动字符1
    BYTE Len1Low;
    BYTE Len1High;
    BYTE Len2Low;
    BYTE Len2High;
    BYTE StartCode2;  	//启动字符2
    BYTE Address;      	//对象地址
    BYTE CMD;
    BYTE afn;
    BYTE Data[40];     //数据内容长度低字节
} ; 
typedef struct{
	char name[16];
	BYTE* pval;
} sval;

class CMaint : public CPSecondary
{
	public:
    
    	DWORD m_dwEqpName;
    	WORD m_wYKCode;
    	WORD m_wYTCode;
		DWORD fileoffset;
    	CMaint();
    	BYTE m_sendcount;
		BYTE m_wirecommflag;
	    BOOL Init(WORD wTaskID);		//初始化
    	VFrame *m_pReceive; 		//接收帧头指针
    	VFrame *m_pSend;			//发送帧头指针
   	 	BYTE   *m_pFrameData;		//接收帧数据域定位指针
    	int prosetnum;
		int gainnum;
    	virtual BOOL DoReceive(void);

    	virtual DWORD SearchOneFrame(BYTE *Buf, WORD Len);
		
    
    	void SendFrameHead(WORD dwCode);
    	void SendFrameHead1(WORD dwCode);
    	void SendFrameTail(void);
    	void SendFrameTail1(void);
    	void SendBaseFrame(WORD dwCode);
		
		virtual BOOL DoYKRet(void);
		virtual BOOL DoYTRet(void);
		virtual void DoTimeOut(void);
		virtual DWORD DoCommState(void);
		void DoTSDataMsg(void);
	protected:
		WORD Crc(BYTE *ptr,WORD count);
		
		void SendClock(void);
		void SetClock(void);

		void SendYC(void);
		void SendYC1(void);
		void SendYC2(void);
		void SendDD(void);
		void SendSingleYX(void);
		void SendSingleSOE(void);
		void SendSingleCOS(void);
		void SendDoubleYX(void);
		void SendDoubleSOE(void);
		void SendDoubleCOS(void);
		void SendEVYX(void);
		void SendEVSOE(void);
		void SendEVCOS(void);
		void SendHisYC(void);
		void SendYB(void) ;
		WORD read_ytdef(WORD eqpID,WORD off,WORD num);
		void reset_protect(void);
		void rgzs();
		void showdatabuf();
		void Senddotdef(void);
		void  SendAC(void) ;

#ifdef MAINT_YCCHAGE_SEND
        WORD *m_wYCNum;
		WORD SearchChangeYC(WORD wEqpID, WORD wNum, WORD wBufLen, VDBYCF_L *pBuf);
		void SendChageYC(void);            	
#endif

		void SetYKCommand(BYTE Command);
	 	void SetYTCommand(BYTE Command);
		void DoYTSet(void);
		
		void SendFile(void);
		void ReceiveFile(void);
		void SendDir(void);

		void BuildDir(void);
		void RenameFile(void);
		void set_ip(void) ;           	
		void clear_records(void)   ;
 		void gprs_power(void)    ;        	
		void SendFileStatus(void);

		void readClock(void);
		void DeleteFile(void);
		void DeleteDir(void);
		void Sendruninfo(void);
		
		void SendDSOE(void);
		void SenddYX(void) ;
		
		void SendACK(void);
		void SendNACK(BYTE ErrorCode);
		void SendEvent(void);
		void dchh(void) ;
		void Sendmaxvalue(void);
		void SendYCfloat(void);
		void SendHisdd(void) ;
		WORD  read_ycdef(WORD eqpID,WORD off,WORD num);
		WORD  read_yxdef(WORD eqpID,WORD off,WORD num);
		WORD  read_dyxdef(WORD eqpID,WORD off,WORD num);
		WORD  read_yxSOEdef(WORD eqpID,WORD off,WORD num);
		WORD  read_dyxSOEdef(WORD eqpID,WORD off,WORD num);
		WORD  read_ykdef(WORD eqpID,WORD off,WORD num);
		WORD read_dddef(WORD eqpID,WORD off,WORD num);
		WORD read_dzdef(WORD off,WORD num);
		WORD read_ybdef(WORD off,WORD num);
		WORD read_hxdef(WORD off,WORD num);
		WORD read_selfparadef(WORD off,WORD num);
		WORD read_runparadef(WORD off,WORD num);
		void read_verinfo(void);
		void read_addr(void);
		void read_comm(void);
		void read_EQPname(void);	
		void set_baohudingzhi(void) ;
		void Get_YB(void)            	;
		void set_gprsip(void);		
		void set_HZGZ(void);		
		void Get_baohudingzhi(void)     ;
		void read_youxiaozhi(void);	
		void ProcFormat(void);
		void SendSelfRunPara(void);
		void WriteSelfRunPara(void);
		void SendICmd(void);
		void SendSoeCosdotdef(void);

		void setGainCommand(void);

		void setDevID(void);
		void  setDevType(void);
		 void setDevMFR(void);
		 void setDevMFR_Utf8(void);
		 void setDevSV(void);
		void SendJiami(void);
		void Do_Lubo(void);
	#ifdef YC_GAIN_NEW
	    void read_ycgainval(void);
	    void read_ycgain(void);
		void write_ycgainval(void);
		void write_ycgain(void);
	#endif
		void Get_Pb(void);
		void Set_Pb(void);
		void GetADType(void);
		void SetADType(void);
		void set_ip_blt(void);
		void Get_Esn();
		void Set_Esn();
		void Get_LTE();
};	


#endif
