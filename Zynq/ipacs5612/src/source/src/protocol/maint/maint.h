/*------------------------------------------------------------------------
 $Rev: 27 $
------------------------------------------------------------------------*/

#ifndef MAINT_H
#define MAINT_H

#include "syscfg.h"

#include "pslave.h"

#define MAINT_YCCHAGE_SEND

#define ACK					0x8001	//��ȷӦ��
#define NACK				0x8002	//����Ӧ��

#define RESET				0x0103	//��λ

#define GET_CLK				0x0a04	//��ȡʱ��
#define SET_CLK				0x0405	//����ʱ��
#define GET_DIFFCLK			0x0a05	//��ȡDIFFʱ��
#define SET_DIFFCLK			0x0406	//����DIFFʱ��


#define GET_YC				0x0c10	//��ң��
#define GET_YX_SINGLE		0x0c11	//������ң��
#define GET_SOE_SINGLE		0x0c12	//������SOE
#define GET_DD				0x0c13	//�����
#define GET_COS_SINGLE		0x0c14	//������COS
#define GET_UI              	      0x0c15  //����ѹ����
#define GET_HIS_YC              	      0x0c16  //����ʷң������
#define GET_HIS_DD              	      0x0c17  //����ʷ�������
#define GET_EVENT              	      0x0c18  //���¼�����
#define GET_RUNINFO             	      0x0c19  //��������Ϣ
#define GET_YB              	      0x0c1a  //���¼�����
#define GET_CHG_YC          0x0c1b  //���仯ң������           
#define GET_DYX          0x0c1c  //dyx          
#define GET_DSOE          0x0c1d  //dsoe
#define GET_maxvalue          0x0c1e  //��ֵ           
#define GET_ycfloat          0x0c1f  //ycfloat           
#define GET_accurve          0x0c20  //��������          
#define GET_wubc          0x0c21  //�޹�����       
#define GET_dotdef          0x0c22  //�������     
#define GET_ver          0x0c23  //�汾��Ϣ
#define GET_addr          0x0c24  //��ַ��Ϣ
#define GET_YBVal          0x0c25  //��ַ��Ϣ
#define GET_COMM          0x0c26  //��ַ��Ϣ
#define GET_BH          0x0c27  //��ַ��Ϣ
#define GET_YXZ          0x0c28  //��Чֵ
#define GET_EQPNAME          0x0c29  //��װ���б�
#define GET_GPRSADDR          0x0c2a  //��GPRS��ַ
#define GET_YC1				0x0c2b	//��ң��
#define GET_YC2				0x0c2c	//��ң��
#define GET_STATE           0x0c2d  //��װ��״̬ hll add
#define GET_SELFRUNPARA 0x0c2e  //��ȡװ�ù�����Ϣ�Ͳ���������Ϣ
#define SET_SELFRUNPARA  0x0c2f  //дװ�ù�����Ϣ�Ͳ���������Ϣ
#define GET_ICMD                        0x0c30 //��i����
#define GET_SoeCosDotDef                        0x0c31 //SOE��COS������Ϣ
#define Set_GainCommand 	0x0c32 //gain.sys�ļ���������
#define SetDevIDCmd            0x0c33 //����װ��24λID ��
#define Get_JIAMI                 0x0c34 //��ȡ����оƬ״̬
#define GET_Lubo                  0x0c35 //�ֶ�����¼��
#define SET_DEV_TYPE        0x0c36 //�����ն˳����ͺ�
#define SET_DEV_MFR          0x0c37 //�����ն�������
#define GET_PB                        0x0c38  //��ȡPB_RELATE
#define GET_ADType              0x0c39  //��ȡ���������ͺ�

#define SET_IP_BLT              0x0c3a  //��������ip
#define SET_GJ_Time             0x0c3b  //�澯����ʱ������
#define SET_DEV_SV              0x0c3c  //��������汾��
#define GET_LL_ID               0x0c3d  //��ȡ��������ģ��ID

#define SET_ESN                 0x0c41  //���û�Ϊ����esn������
#define GET_ESN                 0x0c3f  //��ȡ��Ϊ����esn
#define GET_LTE                 0x0c40  //��ȡ��ΪLTE��Ϣ
#define SET_DIFFPara_Init       0x0c3e  //�������ʼ��
#define SET_Maint_Confirm       0x0c42  //���ü�����ά��֤

#define SET_DEV_MFR_UTF8        0x0c43  //�����ն�������utf8��ʽ

#define SET_YK				0x0520	//ң��Ԥ��
#define DO_YK				0x0521	//ң��ִ��
#define UNDO_YK				0x0522	//ң�س���
#define RESET_PROTECT				0x0523	//��������
#define RGZS				0x0524	//�˹�����
#define SHOWBUFATA				0x0525	//�������������
#define SET_DCHH				0x0527	//��ػ
#define SET_remotequit				0x0528	//Զ���˳�
#define SET_IP				0x0529	//IP����
#define SET_com				0x052a	//��������
#define CLR_records			0x052b	//��¼���
#define GPRS_POWER			0x052c	//gprs��Դ����
#define SET_BH			0x052d	//gprs��Դ����
#define SET_GPRSADDR			0x052e	//gprs��ַ����
#define SET_HZGZ			0x052f	//��Ƶ����
#define SET_PB                 0x0530  //����PB_RELATE
#define SET_JIAMICER   0x0531  //����˫����ܵ��ն�֤��
#define SET_ADType      0x0532  //���ò���оƬ�ͺ�
#define SET_DIFF        0x0533  //д�����
#define GET_DIFF        0x0534  //�������

#define YT_SET				0x8404	//ң�����

#define READ_FILE			0x0f30	//���ļ�
#define WRITE_FILE			0x0f31	//д�ļ�
#define READ_DIR			      0x0f32	//��Ŀ¼
#define BUILD_DIR			0x0f33	//����Ŀ¼
#define RENAME_FILE			0x0f34	//�ļ�����
#define RENAME_DIR			0x0f35	//Ŀ¼����
#define SET_FILE_ATTR		0x0f36	//�޸��ļ�����
#define GET_FILE_ATTR		0x0f37	//��ȡ�ļ�����
#define DEL_FILE			      0x0f38	//ɾ���ļ�
#define DEL_DIR				0x0f39	//ɾ��Ŀ¼

#define INTO_SHELL			0x0540	//����shellģʽ
#define YC_PARASET			0x0542	//ң���������
#define YC_PARAGET          0x0541  
#define YC_PARAVALGET       0x0543
#define YC_PARAVALSET       0x0544
#define YC_PARAZERO           0x0545
#define YC_COM_TEST            0x0546  //���ڲ���

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


#define END_MAINTAIN		0x054f	//����ά��

#define MIN_RECEIVE_SIZE  7    //��С����֡��
#define MAX_FRAME_SIZE  1280  
#define FILE_NAME_LEN	32
#define m25p_file_sector_star 52
#define m25p_file_addr_star 0x340000

#define PROG_FILE_MAX    1024
struct VFrame
{
    BYTE StartCode1;  	//�����ַ�1
    BYTE Len1Low;
    BYTE Len1High;
    BYTE Len2Low;
    BYTE Len2High;
    BYTE StartCode2;  	//�����ַ�2
    BYTE Address;      	//�����ַ
    BYTE CMD;
    BYTE afn;
    BYTE Data[40];     //�������ݳ��ȵ��ֽ�
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
	    BOOL Init(WORD wTaskID);		//��ʼ��
    	VFrame *m_pReceive; 		//����֡ͷָ��
    	VFrame *m_pSend;			//����֡ͷָ��
   	 	BYTE   *m_pFrameData;		//����֡������λָ��
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
