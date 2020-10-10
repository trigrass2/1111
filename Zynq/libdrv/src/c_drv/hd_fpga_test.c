#include "plt_inc_c.h"
#include "hd_fpga.h"
#include "xtime_l.h"
#include "xparameters_ps.h"
#include "sleep.h"
#include "xplatform_info.h"
#include "xil_cache.h"
#include "hd_fpga.h"
#include "xil_mmu.h"
#include "stdio.h"
#include "xscugic.h"

/* Frame (336 bytes) */
unsigned char pkt1[336] = {
0x01, 0x0c, 0xcd, 0x04, 0x00, 0x0f, 0x00, 0xe1, /* ........ */
0xc2, 0xc3, 0xc4, 0xc5, 0x88, 0xba, 0x40, 0x0f, /* ......@. */
0x01, 0x42, 0x00, 0x00, 0x00, 0x00, 0x60, 0x82, /* .B....`. */
0x01, 0x36, 0x80, 0x01, 0x01, 0xa2, 0x82, 0x01, /* .6...... */
0x2f, 0x30, 0x82, 0x01, 0x2b, 0x80, 0x21, 0x4d, /* /0..+.!M */
0x4c, 0x50, 0x41, 0x43, 0x53, 0x35, 0x39, 0x31, /* LPACS591 */
0x31, 0x43, 0x41, 0x4d, 0x55, 0x53, 0x56, 0x30, /* 1CAMUSV0 */
0x31, 0x2f, 0x4c, 0x4c, 0x4e, 0x30, 0x24, 0x53, /* 1/LLN0$S */
0x56, 0x24, 0x73, 0x6d, 0x76, 0x63, 0x62, 0x30, /* V$smvcb0 */
0x82, 0x02, 0x00, 0x95, 0x83, 0x04, 0x00, 0x00, /* ........ */
0x00, 0x01, 0x85, 0x01, 0x01, 0x87, 0x81, 0xf8, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* ........ */
0xfe, 0xee, 0xd6, 0x6b, 0x00, 0x00, 0x00, 0x00, /* ...k.... */
0xfe, 0xee, 0xd6, 0x6b, 0x00, 0x00, 0x00, 0x00, /* ...k.... */
0x00, 0x9c, 0x57, 0x17, 0x00, 0x00, 0x00, 0x00, /* ..W..... */
0x00, 0x9c, 0x57, 0x17, 0x00, 0x00, 0x00, 0x00, /* ..W..... */
0x00, 0x74, 0xc9, 0xed, 0x00, 0x00, 0x00, 0x00, /* .t...... */
0x00, 0x74, 0xc9, 0xed, 0x00, 0x00, 0x00, 0x00, /* .t...... */
0xfe, 0xee, 0xd6, 0x6b, 0x00, 0x00, 0x00, 0x00, /* ...k.... */
0x00, 0x9c, 0x57, 0x17, 0x00, 0x00, 0x00, 0x00, /* ..W..... */
0x00, 0x74, 0xc9, 0xed, 0x00, 0x00, 0x00, 0x00, /* .t...... */
0xff, 0xff, 0xf7, 0x6f, 0x00, 0x00, 0x00, 0x00, /* ...o.... */
0xff, 0xff, 0xf7, 0x6f, 0x00, 0x00, 0x00, 0x00  /* ...o.... */
};

SECTION_PLT_CODE uint32 axi_hp_sv_receive_test_init()
{
    TSV_RECEIVE_FILTER Filter_config;
    uint8 svID[100] = {
            0x4D, 0x4C, 0x50, 0x41, 0x43, 0x53, 0x35, 0x39,
            0x31, 0x31, 0x43, 0x41, 0x4d, 0x55, 0x53, 0x56,
            0x30, 0x31, 0x2F, 0x4C, 0x4C, 0x4E, 0x30, 0x24,
            0x53, 0x56, 0x24, 0x73, 0x6D, 0x76, 0x63, 0x62,
            0x30};
    Filter_config.APPID = 0x400F;
    Filter_config.des_mac_addr[4] = 0x00;
    Filter_config.des_mac_addr[5] = 0x0F;
    Filter_config.svid_crc32 = fpga_calc_crc32(svID, 33);
    Filter_config.confRev = 1;
    Filter_config.chan_cnt = 31;
    axi_hp_set_sv_receive_filter(0, &Filter_config);

    /* init sv msg receive */
    TSV_RECEIVE_CONTROL SvRcvConfig;
    SvRcvConfig.sv_rcv_enable = 1;
    SvRcvConfig.sv_rcv_net_port = 3;
    SvRcvConfig.sv_rcv_rated_delay_enable = 0;
    SvRcvConfig.sv_rcv_phase_angle = 0;
    axi_hp_set_sv_receive_control(0, &SvRcvConfig);

    /* init sampling msg receive */
    axi_hp_set_sampling_msg0_interval_config(0, 0xFFFF, 0, SAMPLING_SRC_TYPE_AD, SAMPLING_SRC_AD_OR_SV_0);
    axi_hp_set_sampling_msg0_interval_config(1, 0xFFFF, 0, SAMPLING_SRC_TYPE_AD, SAMPLING_SRC_AD_OR_SV_1);
	
    axi_hp_set_sampling_msg0_interval_config(2, 0xFFFF, 0, SAMPLING_SRC_TYPE_AD, SAMPLING_SRC_AD_OR_SV_1+1);
	
    axi_hp_set_sampling_msg0_control(SAMPLING_RATE_120SPC, 3, 0xFFFF);

    return 0;
}

SECTION_PLT_CODE uint32 axi_hp_sv_send_test_init()
{
    uint32 i;
    uint32 len;
    /* sv send test */
    /* step1: set sv sampling delay */
    axi_hp_set_sv_sampling_delay(1000);

    /* step 2: cp sv msg */
    {
        len = sizeof(pkt1) / 4;
        for (i = 0; i < len ; i++)
        {
            uint8 temp;
            temp = pkt1[i*4 + 0];
            pkt1[i*4 + 0] = pkt1[i*4 + 3];
            pkt1[i*4 + 3] = temp;
            temp = pkt1[i*4 + 1];
            pkt1[i*4 + 1] = pkt1[i*4 + 2];
            pkt1[i*4 + 2] = temp;

        }
        memcpy((uint32 *)FPGA_SV_SEND_BUF_REG, pkt1, sizeof(pkt1));
    }

    /* step 3: set channel config reg 31 channels */
    TSV_CHAN_CONFIG_REG ChanConfig;

    /* channel set sv delay */
    // enable channel config reg 0
    memset(&ChanConfig, 0, sizeof(TSV_CHAN_CONFIG_REG));
    ChanConfig.sv_src_enable = 1;
    ChanConfig.sv_src_type = SAMPLING_SRC_TYPE_RATED_DELAY;
    ChanConfig.sv_src_choose = SAMPLING_SRC_AD_OR_SV_0;
    ChanConfig.sv_src_chan = 0;
    ChanConfig.sv_src_scale = 0;
    ChanConfig.sv_src_factor = 1;

    axi_hp_set_sv_send_channel_config(0, 0, &ChanConfig);

    /* 1-16 AD0 */
    for (i=1; i<17; i++)
    {
        // enable channel config reg 0
        memset(&ChanConfig, 0, sizeof(TSV_CHAN_CONFIG_REG));
        ChanConfig.sv_src_enable = 1;
        ChanConfig.sv_src_type = SAMPLING_SRC_TYPE_AD;
        ChanConfig.sv_src_choose = SAMPLING_SRC_AD_OR_SV_0;
        ChanConfig.sv_src_chan = i-1;
        ChanConfig.sv_src_scale = 0;
        ChanConfig.sv_src_factor = 1;

        axi_hp_set_sv_send_channel_config(i, 0, &ChanConfig);

        // disable channel config reg 1 & 2
        memset(&ChanConfig, 0, sizeof(TSV_CHAN_CONFIG_REG));
        ChanConfig.sv_src_enable = 0;
        axi_hp_set_sv_send_channel_config(i, 1, &ChanConfig);
        axi_hp_set_sv_send_channel_config(i, 2, &ChanConfig);
    }

    /* 17-32 ad1 */
    for (i=17; i<33; i++)
    {
        // enable channel config reg 0
        memset(&ChanConfig, 0, sizeof(TSV_CHAN_CONFIG_REG));
        ChanConfig.sv_src_enable = 1;
        ChanConfig.sv_src_type = SAMPLING_SRC_TYPE_AD;
        ChanConfig.sv_src_choose = SAMPLING_SRC_AD_OR_SV_1;
        ChanConfig.sv_src_chan = i-1;
        ChanConfig.sv_src_scale = 0;
        ChanConfig.sv_src_factor = 1;
        axi_hp_set_sv_send_channel_config(i, 0, &ChanConfig);

        // disable channel config reg 1 & 2
        memset(&ChanConfig, 0, sizeof(TSV_CHAN_CONFIG_REG));
        ChanConfig.sv_src_enable = 0;
        axi_hp_set_sv_send_channel_config(i, 1, &ChanConfig);
        axi_hp_set_sv_send_channel_config(i, 2, &ChanConfig);
    }

	   /* 33 - 48 ad1 */
    for (i=33; i< 49; i++)
    {
        // enable channel config reg 0
        memset(&ChanConfig, 0, sizeof(TSV_CHAN_CONFIG_REG));
        ChanConfig.sv_src_enable = 1;
        ChanConfig.sv_src_type = SAMPLING_SRC_TYPE_AD;
        ChanConfig.sv_src_choose = SAMPLING_SRC_AD_OR_SV_1 + 1;
        ChanConfig.sv_src_chan = i-1;
        ChanConfig.sv_src_scale = 0;
        ChanConfig.sv_src_factor = 1;
        axi_hp_set_sv_send_channel_config(i, 0, &ChanConfig);

        // disable channel config reg 1 & 2
        memset(&ChanConfig, 0, sizeof(TSV_CHAN_CONFIG_REG));
        ChanConfig.sv_src_enable = 0;
        axi_hp_set_sv_send_channel_config(i, 1, &ChanConfig);
        axi_hp_set_sv_send_channel_config(i, 2, &ChanConfig);
    }
	
    /* step 4 set addressing reg */
    TSV_SEND_ADDRESSING_REG AddressingReg;
    memset(&AddressingReg, 0, sizeof(TSV_SEND_ADDRESSING_REG));
    AddressingReg.sv_send_smpCntValue_pos = 0x4a;
    AddressingReg.sv_send_smpSynchValue_pos = 0x54;
    AddressingReg.sv_send_channel0Value_pos = 0x58;
    axi_hp_set_sv_send_addressing_reg(&AddressingReg);

    /* step 5 set sv send control reg */
    TSV_SEND_CONTROL SendCtrl;
    SendCtrl.sv_send_enable_config_update = 1;
    SendCtrl.sv_send_port_bitwise = 0x1F8; // bit 3-8;
    SendCtrl.sv_send_force_sync = 0;
    SendCtrl.sv_send_force_test = 0;
    SendCtrl.sv_send_chan_cnt = 31;
    SendCtrl.sv_send_msg_len = sizeof(pkt1);

    axi_hp_set_sv_send_control(&SendCtrl);
    return 0;
}

uint32 goose_recv_test()
{
	uint8 dest_mac[6] = {0x01, 0x0c, 0xcd, 0x01, 0x00, 0x01};
	uint16 app_id = 0x2000;
	char *gocbRef = "gocbRef0";
	char *datSet = "dataSet0";
	char *goID = "goId0";

	fpga_add_one_goose_rcv(dest_mac,
	                       app_id,
	                       gocbRef,
	                       datSet,
	                       goID,
	                       0xFFFF);
	return 0;
}

SECTION_PLT_CODE STATUS axi_hp_goose_sv_test_init()
{
	goose_recv_test();
    /* sv receive test */
    //axi_hp_sv_receive_test_init();

    /* sv send test */
    //axi_hp_sv_send_test_init();

    /* irq */
    //axi_hp_fpga_irq_init();

    return 0;
}



void axi_sv_init(void)
{
	axi_hp_sv_receive_test_init();
	//axi_hp_sv_send_test_init();
}
