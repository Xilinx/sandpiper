#
#Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
#SPDX-License-Identifier: MIT
#

proc generate {drv_handle} {
	xdefine_include_file $drv_handle "xparameters.h" "tc_selector" "NUM_INSTANCES" "DEVICE_ID"  "C_S00_AXI_BASEADDR" "C_S00_AXI_HIGHADDR"
}
