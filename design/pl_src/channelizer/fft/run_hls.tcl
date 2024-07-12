#
# Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
# Copyright 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
#
#

# Project settings
# Create a project
open_project -reset fft


# Add the file for synthsis
add_files fft_top.cpp

# Add testbench files for co-simulation
add_files -tb fft_tb.cpp
add_files -tb fftInput.txt
add_files -tb fftGold.txt
add_files -tb fftGold2.txt

# Set top module of the design
set_top fft_top

# Solution settings
open_solution -reset solution1 -flow_target vitis

# Define technology 
# set_part  {xcvu9p-flga2104-2-i}
set_part  {xcvc1902-vsva2197-2MP-e-S}
# Set the target clock period
create_clock -period 8

# Set to 0: to run setup
# Set to 1: to run setup and synthesis
# Set to 2: to run setup, synthesis and RTL simulation
# Set to 3: to run setup, synthesis, RTL simulation and RTL synthesis
# Any other value will run setup only
set hls_exec 2

# Run C simulation
csim_design

# Set any optimization directives

# End of directives

if {$hls_exec == 1} {
	# Run Synthesis and Exit
	csynth_design
	
} elseif {$hls_exec == 2} {
	# Run Synthesis, RTL Simulation and Exit
	csynth_design
	
    cosim_design -rtl verilog
} elseif {$hls_exec == 3} { 
	# Run Synthesis, RTL Simulation, RTL implementation and Exit
	csynth_design
	
    cosim_design -rtl verilog -trace_level all

	export_design
} else {
	# Default is to exit after setup
	csynth_design
}
exit


