#!/bin/sh

ifconfig eth0 192.168.0.30
ifconfig eth1 192.168.0.31

scp wz@192.168.0.102:/work/xilinx/sd_workspace/zynq7015_20200416_sd/test-app/Debug/test-app.elf ./
