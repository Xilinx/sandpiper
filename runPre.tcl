#
#Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
#SPDX-License-Identifier: MIT
#

config_interface -m_axi_max_widen_bitwidth 0
config_compile -name_max_length 100
set_param hls.enable_multiple_burst_maxi_ports_in_same_bundle true
