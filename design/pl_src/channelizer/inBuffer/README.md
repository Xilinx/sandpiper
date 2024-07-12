
[//]: # (Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved)
[//]: # (SPDX-License-Identifier: MIT)

# Description

C++ source code for input buffer for 4F16T PFB .

# Directories and Files

data : input stimulus and golden expected data files  
&emsp;cubeSamples.txt: input data for data_mode 'f' (file data)  
&emsp;sig_goldC_\*.txt: golden expected data for each tile for data_mode 'c' (count data)  
&emsp;sig_goldF_\*.txt: golden expected data for each tile for data_mode 'f'  

tb_wrapper.cpp : C++ testbench  
inBuffer.cpp : top C++ function  
inBuffer.h : header file for the design  
hls_config.cfg : configuration file for Vitis run and Make  
MakeFile : compiles and generates xo  
README : this readme file

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