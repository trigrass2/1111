#!/bin/sh

ifconfig eth0 192.168.0.30
ifconfig eth1 192.168.0.31

scp wz@192.168.0.102:/work/xilinx/1g_prj/1g_image/sd_image.ub /mnt/tf/image.ub
sync


