#!/bin/bash

#
#Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
#SPDX-License-Identifier: MIT
#

ifconfig eth0 192.168.1.11
cd /run/media/mmcblk0p1/
./jesd_platform_xrt.elf -p 5000 -f a.xclbin