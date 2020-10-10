
 PARAMETER VERSION = 2.2.0


BEGIN OS
 PARAMETER OS_NAME = device_tree
 PARAMETER PROC_INSTANCE = ps7_cortexa9_0
 PARAMETER console_device = axi_uart16550_0
 PARAMETER main_memory = ps7_ddr_0
END


BEGIN PROCESSOR
 PARAMETER DRIVER_NAME = cpu_cortexa9
 PARAMETER HW_INSTANCE = ps7_cortexa9_0
END


BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER HW_INSTANCE = axi_bram_ctrl_0
 PARAMETER clock-names = s_axi_aclk
 PARAMETER clocks = misc_clk_0
 PARAMETER compatible = xlnx,axi-bram-ctrl-4.1
 PARAMETER reg = 0x40000000 0x10000
 PARAMETER xlnx,bram-addr-width = 14
 PARAMETER xlnx,bram-inst-mode = EXTERNAL
 PARAMETER xlnx,ecc = 0
 PARAMETER xlnx,ecc-onoff-reset-value = 0
 PARAMETER xlnx,ecc-type = 0
 PARAMETER xlnx,fault-inject = 0
 PARAMETER xlnx,memory-depth = 16384
 PARAMETER xlnx,rd-cmd-optimization = 0
 PARAMETER xlnx,read-latency = 1
 PARAMETER xlnx,s-axi-ctrl-addr-width = 32
 PARAMETER xlnx,s-axi-ctrl-data-width = 32
 PARAMETER xlnx,s-axi-id-width = 12
 PARAMETER xlnx,s-axi-supports-narrow-burst = 0
 PARAMETER xlnx,select-xpm = 0
 PARAMETER xlnx,single-port-bram = 1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = axi_iic
 PARAMETER HW_INSTANCE = axi_iic_0
 PARAMETER clock-names = s_axi_aclk
 PARAMETER clocks = misc_clk_0
 PARAMETER compatible = xlnx,axi-iic-2.0 xlnx,xps-iic-2.00.a
 PARAMETER interrupt-names = iic2intc_irpt
 PARAMETER interrupt-parent = intc
 PARAMETER interrupts = 0 56 4
 PARAMETER reg = 0x81650000 0x10000
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = axi_iic
 PARAMETER HW_INSTANCE = axi_iic_1
 PARAMETER clock-names = s_axi_aclk
 PARAMETER clocks = misc_clk_0
 PARAMETER compatible = xlnx,axi-iic-2.0 xlnx,xps-iic-2.00.a
 PARAMETER interrupt-names = iic2intc_irpt
 PARAMETER interrupt-parent = intc
 PARAMETER interrupts = 0 28 4
 PARAMETER reg = 0x81660000 0x10000
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = axi_iic
 PARAMETER HW_INSTANCE = axi_iic_2
 PARAMETER clock-names = s_axi_aclk
 PARAMETER clocks = misc_clk_0
 PARAMETER compatible = xlnx,axi-iic-2.0 xlnx,xps-iic-2.00.a
 PARAMETER interrupt-names = iic2intc_irpt
 PARAMETER interrupt-parent = intc
 PARAMETER interrupts = 0 31 4
 PARAMETER reg = 0x81670000 0x10000
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartns
 PARAMETER HW_INSTANCE = axi_uart16550_0
 PARAMETER clock-frequency = 125000000
 PARAMETER clock-names = s_axi_aclk
 PARAMETER clocks = misc_clk_0
 PARAMETER interrupt-names = ip2intc_irpt
 PARAMETER interrupt-parent = intc
 PARAMETER interrupts = 0 30 4
 PARAMETER reg = 0x43c00000 0x10000
 PARAMETER xlnx,external-xin-clk-hz = 25000000
 PARAMETER xlnx,external-xin-clk-hz-d = 25
 PARAMETER xlnx,has-external-rclk = 0
 PARAMETER xlnx,has-external-xin = 0
 PARAMETER xlnx,is-a-16550 = 1
 PARAMETER xlnx,s-axi-aclk-freq-hz-d = 125.0
 PARAMETER xlnx,use-modem-ports = 1
 PARAMETER xlnx,use-user-ports = 1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartns
 PARAMETER HW_INSTANCE = axi_uart16550_1
 PARAMETER clock-frequency = 125000000
 PARAMETER clock-names = s_axi_aclk
 PARAMETER clocks = misc_clk_0
 PARAMETER interrupt-names = ip2intc_irpt
 PARAMETER interrupt-parent = intc
 PARAMETER interrupts = 0 31 4
 PARAMETER port-number = 2
 PARAMETER reg = 0x43c10000 0x10000
 PARAMETER xlnx,external-xin-clk-hz = 25000000
 PARAMETER xlnx,external-xin-clk-hz-d = 25
 PARAMETER xlnx,has-external-rclk = 0
 PARAMETER xlnx,has-external-xin = 0
 PARAMETER xlnx,is-a-16550 = 1
 PARAMETER xlnx,s-axi-aclk-freq-hz-d = 125.0
 PARAMETER xlnx,use-modem-ports = 1
 PARAMETER xlnx,use-user-ports = 1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartns
 PARAMETER HW_INSTANCE = axi_uart16550_10
 PARAMETER clock-frequency = 125000000
 PARAMETER clock-names = s_axi_aclk
 PARAMETER clocks = misc_clk_0
 PARAMETER interrupt-names = ip2intc_irpt
 PARAMETER interrupt-parent = intc
 PARAMETER interrupts = 0 55 4
 PARAMETER port-number = 3
 PARAMETER reg = 0x81630000 0x10000
 PARAMETER xlnx,external-xin-clk-hz = 25000000
 PARAMETER xlnx,external-xin-clk-hz-d = 25
 PARAMETER xlnx,has-external-rclk = 0
 PARAMETER xlnx,has-external-xin = 0
 PARAMETER xlnx,is-a-16550 = 1
 PARAMETER xlnx,s-axi-aclk-freq-hz-d = 125.0
 PARAMETER xlnx,use-modem-ports = 1
 PARAMETER xlnx,use-user-ports = 1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartns
 PARAMETER HW_INSTANCE = axi_uart16550_11
 PARAMETER clock-frequency = 125000000
 PARAMETER clock-names = s_axi_aclk
 PARAMETER clocks = misc_clk_0
 PARAMETER interrupt-names = ip2intc_irpt
 PARAMETER interrupt-parent = intc
 PARAMETER interrupts = 0 28 4
 PARAMETER port-number = 4
 PARAMETER reg = 0x81640000 0x10000
 PARAMETER xlnx,external-xin-clk-hz = 25000000
 PARAMETER xlnx,external-xin-clk-hz-d = 25
 PARAMETER xlnx,has-external-rclk = 0
 PARAMETER xlnx,has-external-xin = 0
 PARAMETER xlnx,is-a-16550 = 1
 PARAMETER xlnx,s-axi-aclk-freq-hz-d = 125.0
 PARAMETER xlnx,use-modem-ports = 1
 PARAMETER xlnx,use-user-ports = 1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartns
 PARAMETER HW_INSTANCE = axi_uart16550_2
 PARAMETER clock-frequency = 125000000
 PARAMETER clock-names = s_axi_aclk
 PARAMETER clocks = misc_clk_0
 PARAMETER interrupt-names = ip2intc_irpt
 PARAMETER interrupt-parent = intc
 PARAMETER interrupts = 0 32 4
 PARAMETER port-number = 5
 PARAMETER reg = 0x43c20000 0x10000
 PARAMETER xlnx,external-xin-clk-hz = 25000000
 PARAMETER xlnx,external-xin-clk-hz-d = 25
 PARAMETER xlnx,has-external-rclk = 0
 PARAMETER xlnx,has-external-xin = 0
 PARAMETER xlnx,is-a-16550 = 1
 PARAMETER xlnx,s-axi-aclk-freq-hz-d = 125.0
 PARAMETER xlnx,use-modem-ports = 1
 PARAMETER xlnx,use-user-ports = 1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartns
 PARAMETER HW_INSTANCE = axi_uart16550_3
 PARAMETER clock-frequency = 125000000
 PARAMETER clock-names = s_axi_aclk
 PARAMETER clocks = misc_clk_0
 PARAMETER interrupt-names = ip2intc_irpt
 PARAMETER interrupt-parent = intc
 PARAMETER interrupts = 0 33 4
 PARAMETER port-number = 6
 PARAMETER reg = 0x43c30000 0x10000
 PARAMETER xlnx,external-xin-clk-hz = 25000000
 PARAMETER xlnx,external-xin-clk-hz-d = 25
 PARAMETER xlnx,has-external-rclk = 0
 PARAMETER xlnx,has-external-xin = 0
 PARAMETER xlnx,is-a-16550 = 1
 PARAMETER xlnx,s-axi-aclk-freq-hz-d = 125.0
 PARAMETER xlnx,use-modem-ports = 1
 PARAMETER xlnx,use-user-ports = 1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartns
 PARAMETER HW_INSTANCE = axi_uart16550_4
 PARAMETER clock-frequency = 125000000
 PARAMETER clock-names = s_axi_aclk
 PARAMETER clocks = misc_clk_0
 PARAMETER interrupt-names = ip2intc_irpt
 PARAMETER interrupt-parent = intc
 PARAMETER interrupts = 0 34 4
 PARAMETER port-number = 7
 PARAMETER reg = 0x43c40000 0x10000
 PARAMETER xlnx,external-xin-clk-hz = 25000000
 PARAMETER xlnx,external-xin-clk-hz-d = 25
 PARAMETER xlnx,has-external-rclk = 0
 PARAMETER xlnx,has-external-xin = 0
 PARAMETER xlnx,is-a-16550 = 1
 PARAMETER xlnx,s-axi-aclk-freq-hz-d = 125.0
 PARAMETER xlnx,use-modem-ports = 1
 PARAMETER xlnx,use-user-ports = 1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartns
 PARAMETER HW_INSTANCE = axi_uart16550_5
 PARAMETER clock-frequency = 125000000
 PARAMETER clock-names = s_axi_aclk
 PARAMETER clocks = misc_clk_0
 PARAMETER interrupt-names = ip2intc_irpt
 PARAMETER interrupt-parent = intc
 PARAMETER interrupts = 0 35 4
 PARAMETER port-number = 8
 PARAMETER reg = 0x43c50000 0x10000
 PARAMETER xlnx,external-xin-clk-hz = 25000000
 PARAMETER xlnx,external-xin-clk-hz-d = 25
 PARAMETER xlnx,has-external-rclk = 0
 PARAMETER xlnx,has-external-xin = 0
 PARAMETER xlnx,is-a-16550 = 1
 PARAMETER xlnx,s-axi-aclk-freq-hz-d = 125.0
 PARAMETER xlnx,use-modem-ports = 1
 PARAMETER xlnx,use-user-ports = 1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartns
 PARAMETER HW_INSTANCE = axi_uart16550_6
 PARAMETER clock-frequency = 125000000
 PARAMETER clock-names = s_axi_aclk
 PARAMETER clocks = misc_clk_0
 PARAMETER interrupt-names = ip2intc_irpt
 PARAMETER interrupt-parent = intc
 PARAMETER interrupts = 0 36 4
 PARAMETER port-number = 9
 PARAMETER reg = 0x43c60000 0x10000
 PARAMETER xlnx,external-xin-clk-hz = 25000000
 PARAMETER xlnx,external-xin-clk-hz-d = 25
 PARAMETER xlnx,has-external-rclk = 0
 PARAMETER xlnx,has-external-xin = 0
 PARAMETER xlnx,is-a-16550 = 1
 PARAMETER xlnx,s-axi-aclk-freq-hz-d = 125.0
 PARAMETER xlnx,use-modem-ports = 1
 PARAMETER xlnx,use-user-ports = 1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartns
 PARAMETER HW_INSTANCE = axi_uart16550_7
 PARAMETER clock-frequency = 125000000
 PARAMETER clock-names = s_axi_aclk
 PARAMETER clocks = misc_clk_0
 PARAMETER interrupt-names = ip2intc_irpt
 PARAMETER interrupt-parent = intc
 PARAMETER interrupts = 0 52 4
 PARAMETER port-number = 10
 PARAMETER reg = 0x81600000 0x10000
 PARAMETER xlnx,external-xin-clk-hz = 25000000
 PARAMETER xlnx,external-xin-clk-hz-d = 25
 PARAMETER xlnx,has-external-rclk = 0
 PARAMETER xlnx,has-external-xin = 0
 PARAMETER xlnx,is-a-16550 = 1
 PARAMETER xlnx,s-axi-aclk-freq-hz-d = 125.0
 PARAMETER xlnx,use-modem-ports = 1
 PARAMETER xlnx,use-user-ports = 1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartns
 PARAMETER HW_INSTANCE = axi_uart16550_8
 PARAMETER clock-frequency = 125000000
 PARAMETER clock-names = s_axi_aclk
 PARAMETER clocks = misc_clk_0
 PARAMETER interrupt-names = ip2intc_irpt
 PARAMETER interrupt-parent = intc
 PARAMETER interrupts = 0 53 4
 PARAMETER port-number = 11
 PARAMETER reg = 0x81610000 0x10000
 PARAMETER xlnx,external-xin-clk-hz = 25000000
 PARAMETER xlnx,external-xin-clk-hz-d = 25
 PARAMETER xlnx,has-external-rclk = 0
 PARAMETER xlnx,has-external-xin = 0
 PARAMETER xlnx,is-a-16550 = 1
 PARAMETER xlnx,s-axi-aclk-freq-hz-d = 125.0
 PARAMETER xlnx,use-modem-ports = 1
 PARAMETER xlnx,use-user-ports = 1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartns
 PARAMETER HW_INSTANCE = axi_uart16550_9
 PARAMETER clock-frequency = 125000000
 PARAMETER clock-names = s_axi_aclk
 PARAMETER clocks = misc_clk_0
 PARAMETER interrupt-names = ip2intc_irpt
 PARAMETER interrupt-parent = intc
 PARAMETER interrupts = 0 54 4
 PARAMETER port-number = 12
 PARAMETER reg = 0x81620000 0x10000
 PARAMETER xlnx,external-xin-clk-hz = 25000000
 PARAMETER xlnx,external-xin-clk-hz-d = 25
 PARAMETER xlnx,has-external-rclk = 0
 PARAMETER xlnx,has-external-xin = 0
 PARAMETER xlnx,is-a-16550 = 1
 PARAMETER xlnx,s-axi-aclk-freq-hz-d = 125.0
 PARAMETER xlnx,use-modem-ports = 1
 PARAMETER xlnx,use-user-ports = 1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER HW_INSTANCE = ps7_afi_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER HW_INSTANCE = ps7_afi_1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER HW_INSTANCE = ps7_afi_2
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER HW_INSTANCE = ps7_afi_3
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = canps
 PARAMETER HW_INSTANCE = ps7_can_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = canps
 PARAMETER HW_INSTANCE = ps7_can_1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER HW_INSTANCE = ps7_coresight_comp_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = ddrps
 PARAMETER HW_INSTANCE = ps7_ddr_0
 PARAMETER reg = 0x0 0x40000000
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = ddrcps
 PARAMETER HW_INSTANCE = ps7_ddrc_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = devcfg
 PARAMETER HW_INSTANCE = ps7_dev_cfg_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = none
 PARAMETER HW_INSTANCE = ps7_dma_ns
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = dmaps
 PARAMETER HW_INSTANCE = ps7_dma_s
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = globaltimerps
 PARAMETER HW_INSTANCE = ps7_globaltimer_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = gpiops
 PARAMETER HW_INSTANCE = ps7_gpio_0
 PARAMETER emio-gpio-width = 16
 PARAMETER gpio-mask-high = 0
 PARAMETER gpio-mask-low = 22016
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER HW_INSTANCE = ps7_gpv_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = iicps
 PARAMETER HW_INSTANCE = ps7_i2c_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER HW_INSTANCE = ps7_intc_dist_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER HW_INSTANCE = ps7_iop_bus_config_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER HW_INSTANCE = ps7_l2cachec_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = ocmcps
 PARAMETER HW_INSTANCE = ps7_ocmc_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = pl310ps
 PARAMETER HW_INSTANCE = ps7_pl310_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = pmups
 PARAMETER HW_INSTANCE = ps7_pmu_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = qspips
 PARAMETER HW_INSTANCE = ps7_qspi_0
 PARAMETER is-dual = 0
 PARAMETER spi-rx-bus-width = 4
 PARAMETER spi-tx-bus-width = 4
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER HW_INSTANCE = ps7_qspi_linear_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = ramps
 PARAMETER HW_INSTANCE = ps7_ram_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = none
 PARAMETER HW_INSTANCE = ps7_ram_1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER HW_INSTANCE = ps7_scuc_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = scugic
 PARAMETER HW_INSTANCE = ps7_scugic_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = scutimer
 PARAMETER HW_INSTANCE = ps7_scutimer_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = scuwdt
 PARAMETER HW_INSTANCE = ps7_scuwdt_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER HW_INSTANCE = ps7_sd_0
 PARAMETER xlnx,has-cd = 0
 PARAMETER xlnx,has-power = 0
 PARAMETER xlnx,has-wp = 0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = generic
 PARAMETER HW_INSTANCE = ps7_sd_1
 PARAMETER xlnx,has-cd = 0
 PARAMETER xlnx,has-power = 0
 PARAMETER xlnx,has-wp = 0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = slcrps
 PARAMETER HW_INSTANCE = ps7_slcr_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = spips
 PARAMETER HW_INSTANCE = ps7_spi_0
 PARAMETER num-cs = 3
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = spips
 PARAMETER HW_INSTANCE = ps7_spi_1
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = ttcps
 PARAMETER HW_INSTANCE = ps7_ttc_0
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartps
 PARAMETER HW_INSTANCE = ps7_uart_0
 PARAMETER port-number = 13
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = uartps
 PARAMETER HW_INSTANCE = ps7_uart_1
 PARAMETER port-number = 14
END

BEGIN DRIVER
 PARAMETER DRIVER_NAME = xadcps
 PARAMETER HW_INSTANCE = ps7_xadc_0
END


