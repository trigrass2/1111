connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~"APU*" && jtag_cable_name =~ "Digilent JTAG-SMT2 210251A08870"} -index 0
loadhw -hw E:/xilinx/Zynq/sd_hw_platform/system.hdf -mem-ranges [list {0x40000000 0xbfffffff}]
configparams force-mem-access 1
targets -set -nocase -filter {name =~ "ARM*#1" && jtag_cable_name =~ "Digilent JTAG-SMT2 210251A08870"} -index 0
rst -processor
targets -set -nocase -filter {name =~ "ARM*#1" && jtag_cable_name =~ "Digilent JTAG-SMT2 210251A08870"} -index 0
dow E:/xilinx/Zynq/zynqBM/Debug/zynqBM.elf
configparams force-mem-access 0
bpadd -addr &main
