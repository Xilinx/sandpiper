
[//]: # (Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved)
[//]: # (SPDX-License-Identifier: MIT)

# Description:
Channel Selection from among 32 paths from FFT

# Directories and Files:

chanSelector.h : header file  
chanSelector.cpp : main body  
tb_wrapper.cpp : self-checking test bench  
hls_config.cfg : hls configuration file  
fftOut.txt : input data file  
sig_gold.txt : expected output file  
MakeFile : compile and generate xo  
chanSelector_wrapper.xo : Xilinx Object file  

# Run Examples

To generate xo:

    make all

Open a command prompt with vitis tools set up and then run the command below  
&emsp;vitis_hls -f run_hls.tcl  
or  
&emsp;vitis -new -w ./ws
and import hls_config.cfg at the setup stage.

Just run csim at command line:

&emsp;vitis-run --mode hls --csim --config ./hls_config.cfg --work_dir work

CSYNTH at command line:

&emsp;v++ -c --mode hls --config ./hls_config.cfg --work_dir work/

Run cosim at command line (CSYNTH should be run first):

&emsp;vitis-run --mode hls --cosim --config ./hls_config.cfg --work_dir work


