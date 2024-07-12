#
# Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
#

%.PHONY: help
help::
	@echo  ""
	@echo  " Makefile Usage:"
	@echo  " Default TARGET is hw_emu"
	@echo  " make all"
	@echo  "  make kernels"
	@echo  "  Command to generate kernels"
	@echo  "  make xsa"
	@echo  "  Command to generate xsa for specified target"
	@echo  "  make application"
	@echo  "  Command to generate the A72 application"
	@echo  "  make package"
	@echo  "  Command to package design into an sd_card for specified target"
	@echo  "  make clean_target"
	@echo  "  Command to remove the generated files for a specific target"
	@echo  "  make cleanall"
	@echo  "  Command to remove the generated files and reports for all variations"
	@echo  ""
	@echo  " OPTIONS:"
	@echo  " Use the make recipes with required values for options mentioned below-"
	@echo  "    TARGET     -specifies target: Hardware or Emulation. Values: hw_emu(default), hw"
	@echo  "    PL_FREQ    -specifies HLS kernel frequencies. Values: 250Mhz(default)"
	@echo  ""

# Print all options passed to Makefile
print-%  : ; @echo $* = $($*)

# =========================================================
# TARGET can be set as:
#   hw_emu: Hardware Emulation
#   hw    : Hardware Run
# =========================================================
TARGET := hw_emu

# =========================================================
# PL Frequency in Mhz:
#   250MHz(default)
# =========================================================
PL_FREQ := 125
VPP_CLOCK_FREQ := $(PL_FREQ)000000

ENABLE_MATLAB_TEST := 1
# =========================================================
# Source directories
# =========================================================
RELATIVE_PROJECT_DIR := ./
CUR_DIR := ./

PROJECT_REPO      := $(shell readlink -f $(RELATIVE_PROJECT_DIR))
DEVICE_NAME       := xcvc1902-vsva2197-2MP-e-S

## Setting other folders relative to PROJECT_REPO
DESIGN_REPO       := $(PROJECT_REPO)/design
PL_SRC_REPO       := $(DESIGN_REPO)/pl_src
AIE_SRC_REPO      := $(DESIGN_REPO)/aie_src
HOST_APP_SRC      := $(DESIGN_REPO)/host_app_src
EXEC_SCRIPTS_REPO := $(DESIGN_REPO)/exec_scripts
MM2S_SRC          := $(PL_SRC_REPO)/mm2s.cpp
S2MM_SRC          := $(PL_SRC_REPO)/s2mm.cpp
WDMA_SRC          := $(PL_SRC_REPO)/wdma_rdma/wdma.cpp
RDMA_SRC          := $(PL_SRC_REPO)/wdma_rdma/rdma.cpp
TC_SELECTOR_SRC   := $(PL_SRC_REPO)/TC_Selector/xo/tc_selector.xo
CHAN_SRC_REPO     := $(PL_SRC_REPO)/channelizer
INTERP_SRC_REPO	  := $(PL_SRC_REPO)/interp_and_mix
UPSAMPLER_SRC     := $(INTERP_SRC_REPO)/xo/interp_and_mix.xo
IN_BUFFER_SRC     := $(CHAN_SRC_REPO)/inBuffer/inBuffer.cpp
OUT_BUFFER_SRC    := $(CHAN_SRC_REPO)/outBuffer/outBuffer.cpp
FFT_TOP_SRC       := $(CHAN_SRC_REPO)/fft/fft_top.cpp
CH_SELECTOR_SRC   := $(CHAN_SRC_REPO)/chanSelector/chanSelector.cpp

BASE_BLD_DIR     := $(PROJECT_REPO)/build
BUILD_TARGET_DIR := $(BASE_BLD_DIR)/gen_outputs/$(TARGET)


EMBEDDED_PACKAGE_OUT := $(BUILD_TARGET_DIR)/package
EMBEDDED_EXEC_SCRIPT := run_script.sh
PLATFORM := $(XILINX_VITIS)/base_platforms/xilinx_vck190_base_202320_1/xilinx_vck190_base_202320_1.xpfm 

MM2S_XO                 := mm2s.$(TARGET)
S2MM_XO                 := s2mm.$(TARGET)
WDMA_XO                 := wdma.$(TARGET)
RDMA_XO                 := rdma.$(TARGET)

# ==========================================================
# Below are the names for Application executable,
# kernel executables, and xclbin
# ==========================================================
APP_ELF := jesd_platform_xrt.elf
XSA  := vck190_v1_0.$(TARGET).xsa

# =========================================================
# Application Source Files repository
# =========================================================
APP_SRC_CPP := $(HOST_APP_SRC)/main.cpp

# =========================================================
# Application Compiler and Linker Flags
# =========================================================
GCC_FLAGS := -O
GCC_FLAGS += -c
GCC_FLAGS += -DENABLE_MATLAB_TEST=$(ENABLE_MATLAB_TEST)
#GCC_FLAGS += -std=c++14
#GCC_FLAGS += -D__linux__

CXX=aarch64-linux-gnu-g++  -mcpu=cortex-a72.cortex-a53 -march=armv8-a+crc -fstack-protector-strong  -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security --sysroot=$(SDKTARGETSYSROOT)
CC=aarch64-linux-gnu-gcc  -mcpu=cortex-a72.cortex-a53 -march=armv8-a+crc -fstack-protector-strong  -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security --sysroot=$(SDKTARGETSYSROOT)

GCC_INC_FLAGS := -I$(SDKTARGETSYSROOT)/usr/include/xrt
GCC_INC_FLAGS += -I$(SDKTARGETSYSROOT)/usr/include
GCC_INC_FLAGS += -I$(SDKTARGETSYSROOT)/usr/lib
GCC_INC_FLAGS += -I$(XILINX_VITIS)/aietools/include

GCC_INC_LIB += -L$(SDKTARGETSYSROOT)/usr/lib

GCC_LIB := -lxrt_coreutil
GCC_LIB += -lxilinxopencl
GCC_LIB += -lstdc++

# =========================================================
# Kernel Compiler and Linker Flags
# ========================================================
VPP_FLAGS := --platform $(PLATFORM)
VPP_FLAGS += --save-temps
VPP_FLAGS += --temp_dir $(BUILD_TARGET_DIR)/_x
VPP_FLAGS += -g
VPP_FLAGS +=   --config $(PL_SRC_REPO)/wdma_rdma/hls_pre.cfg 

#VPP_LINK_FLAGS += --vivado.impl.strategies "ALL"
#VPP_LINK_FLAGS += --advanced.param "compiler.enableMultiStrategies=1"
#VPP_LINK_FLAGS += --advanced.param "compiler.multiStrategiesWaitOnAllRuns=1"
VPP_LINK_FLAGS += --vivado.synth.jobs 8
VPP_LINK_FLAGS += --vivado.impl.jobs 33

VPP_LINK_FLAGS += --clock.defaultFreqHz $(VPP_CLOCK_FREQ)
VPP_LINK_FLAGS += --clock.defaultTolerance 0.001
VPP_LINK_FLAGS += --config $(PROJECT_REPO)/system.cfg
VPP_LINK_FLAGS += --vivado.prop fileset.sim_1.xsim.simulate.log_all_signals=true
VPP_LINK_FLAGS += --vivado.prop run.synth_1.{STEPS.SYNTH_DESIGN.ARGS.CONTROL_SET_OPT_THRESHOLD}={16}
VPP_LINK_FLAGS += --vivado.prop run.synth_1.{STEPS.SYNTH_DESIGN.ARGS.KEEP_EQUIVALENT_REGISTERS}={true}
VPP_LINK_FLAGS += --xp param:compiler.worstNegativeSlack=-0.100
#VPP_LINK_FLAGS += --to_step vpl.synth
#VPP_LINK_FLAGS += --from_step vpl.impl

# =========================================================
# Packaging Flags
# ========================================================
PKG_FLAGS := -t $(TARGET)
PKG_FLAGS += --save-temps
PKG_FLAGS += --temp_dir $(BUILD_TARGET_DIR)/_x
PKG_FLAGS += -f $(PLATFORM)
PKG_FLAGS += --package.rootfs $(XLNX_VERSAL)/rootfs.ext4
PKG_FLAGS += --package.kernel_image $(XLNX_VERSAL)/Image
PKG_FLAGS += --package.boot_mode=sd
PKG_FLAGS += --package.out_dir $(EMBEDDED_PACKAGE_OUT)
PKG_FLAGS += --package.image_format=ext4
PKG_FLAGS += --package.sd_file $(BUILD_TARGET_DIR)/$(APP_ELF)
PKG_FLAGS += --package.sd_file $(EXEC_SCRIPTS_REPO)/$(EMBEDDED_EXEC_SCRIPT)
PKG_FLAGS += --package.defer_aie_run
PKG_FLAGS += $(BUILD_TARGET_DIR)/$(XSA)

ifdef XRT_ROOT
   PKG_FLAGS += --package.sd_dir $(XRT_ROOT)
endif

platform:
	$(MAKE) -C build DEVICE_NAME=$(DEVICE_NAME)

# =========================================================
# Step 1. Kernel XO File Generation
# ========================================================

kernels: $(BUILD_TARGET_DIR)/$(WDMA_XO).xo \
		$(BUILD_TARGET_DIR)/$(RDMA_XO).xo \
		$(BUILD_TARGET_DIR)/$(MM2S_XO).xo \
		$(BUILD_TARGET_DIR)/$(S2MM_XO).xo \
		$(BUILD_TARGET_DIR)/tc_selector.xo \
		$(BUILD_TARGET_DIR)/interp_and_mix.xo \
		$(BUILD_TARGET_DIR)/inBuffer_wrapper.xo \
		$(BUILD_TARGET_DIR)/outBuffer_wrapper.xo \
		$(BUILD_TARGET_DIR)/fft_top.xo \
		$(BUILD_TARGET_DIR)/chanSelector_wrapper.xo 

$(BUILD_TARGET_DIR)/tc_selector.xo:
	mkdir -p $(BUILD_TARGET_DIR); \
	cp $(TC_SELECTOR_SRC) $@

$(BUILD_TARGET_DIR)/interp_and_mix.xo:
	mkdir -p $(BUILD_TARGET_DIR); \
	cp $(UPSAMPLER_SRC) $@


$(BUILD_TARGET_DIR)/inBuffer_wrapper.xo: $(IN_BUFFER_SRC)
	mkdir -p $(BUILD_TARGET_DIR); \
	v++ --target $(TARGET) $(VPP_FLAGS) --hls.clock 125000000:inBuffer_wrapper -c -k inBuffer_wrapper $(IN_BUFFER_SRC) -o $@   
	mv v++_inBuffer_wrapper.log $(BUILD_TARGET_DIR)/.

$(BUILD_TARGET_DIR)/outBuffer_wrapper.xo: $(OUT_BUFFER_SRC)
	mkdir -p $(BUILD_TARGET_DIR); \
	v++ --target $(TARGET) $(VPP_FLAGS) --hls.clock 125000000:outBuffer_wrapper -c -k outBuffer_wrapper $(OUT_BUFFER_SRC) -o $@   
	mv v++_outBuffer_wrapper.log $(BUILD_TARGET_DIR)/.
	
$(BUILD_TARGET_DIR)/fft_top.xo: $(FFT_TOP_SRC)
	mkdir -p $(BUILD_TARGET_DIR); \
	v++ --target $(TARGET) $(VPP_FLAGS) --hls.clock 125000000:fft_top -c -k fft_top $(FFT_TOP_SRC) -o $@   
	mv v++_fft_top.log $(BUILD_TARGET_DIR)/.

$(BUILD_TARGET_DIR)/chanSelector_wrapper.xo: $(CH_SELECTOR_SRC)
	mkdir -p $(BUILD_TARGET_DIR); \
	v++ --target $(TARGET) $(VPP_FLAGS) --hls.clock 125000000:chanSelector_wrapper -c -k chanSelector_wrapper $(CH_SELECTOR_SRC) -o $@   
	mv v++_chanSelector_wrapper.log $(BUILD_TARGET_DIR)/.

$(BUILD_TARGET_DIR)/$(WDMA_XO).xo: $(WDMA_SRC)
	mkdir -p $(BUILD_TARGET_DIR); \
	v++ --target $(TARGET) $(VPP_FLAGS) -c -k wdma $(WDMA_SRC) -o $@   
	mv v++_wdma.$(TARGET).log $(BUILD_TARGET_DIR)/.

$(BUILD_TARGET_DIR)/$(RDMA_XO).xo: $(RDMA_SRC)
	mkdir -p $(BUILD_TARGET_DIR); \
	v++ --target $(TARGET) $(VPP_FLAGS) -c -k rdma $(RDMA_SRC) -o $@   
	mv v++_rdma.$(TARGET).log $(BUILD_TARGET_DIR)/.

$(BUILD_TARGET_DIR)/$(MM2S_XO).xo: $(MM2S_SRC)
	mkdir -p $(BUILD_TARGET_DIR); \
	v++ --target $(TARGET) $(VPP_FLAGS) -c -k mm2s $(MM2S_SRC) -o $@   
	mv v++_mm2s.$(TARGET).log $(BUILD_TARGET_DIR)/.

$(BUILD_TARGET_DIR)/$(S2MM_XO).xo: $(S2MM_SRC)
	mkdir -p $(BUILD_TARGET_DIR); \
	v++ --target $(TARGET) $(VPP_FLAGS) -c -k s2mm $(S2MM_SRC) -o $@   
	mv v++_s2mm.$(TARGET).log $(BUILD_TARGET_DIR)/.


# =========================================================
# Step 2. XSA File Generation
# ========================================================
xsa: $(BUILD_TARGET_DIR)/$(XSA)

$(BUILD_TARGET_DIR)/$(XSA): 
	cd $(BUILD_TARGET_DIR);	\
	cp ../../../runPre.tcl .; \
	v++ -l $(VPP_FLAGS) $(VPP_LINK_FLAGS) -t $(TARGET) -o $@ \
	$(BUILD_TARGET_DIR)/$(WDMA_XO).xo \
	$(BUILD_TARGET_DIR)/$(RDMA_XO).xo \
	$(BUILD_TARGET_DIR)/$(MM2S_XO).xo \
	$(BUILD_TARGET_DIR)/tc_selector.xo \
	$(BUILD_TARGET_DIR)/$(S2MM_XO).xo \
	$(BUILD_TARGET_DIR)/chanSelector_wrapper.xo \
	$(BUILD_TARGET_DIR)/fft_top.xo \
	$(BUILD_TARGET_DIR)/inBuffer_wrapper.xo \
	$(BUILD_TARGET_DIR)/outBuffer_wrapper.xo \
	$(BUILD_TARGET_DIR)/interp_and_mix.xo libadf.a

# =========================================================
# Step 3. A72 Application Executable File Generation
# ========================================================
application:
ifeq ($(ENABLE_MATLAB_TEST),0)
	$(CXX) $(GCC_FLAGS) $(GCC_INC_FLAGS) $(APP_SRC_CPP) -o $(BUILD_TARGET_DIR)/jesd_platform_app.o $(GCC_INC_LIB) $(GCC_LIB)
	$(CXX) $(BUILD_TARGET_DIR)/jesd_platform_app.o $(GCC_INC_LIB) $(GCC_LIB) -lpthread -o $(BUILD_TARGET_DIR)/$(APP_ELF)
else
	$(CXX) $(GCC_FLAGS) $(GCC_INC_FLAGS) $(APP_SRC_CPP) -o $(BUILD_TARGET_DIR)/jesd_platform_app.o $(GCC_INC_LIB) $(GCC_LIB)
	$(CXX) $(GCC_FLAGS) $(GCC_INC_FLAGS) $(HOST_APP_SRC)/server.cpp -o $(BUILD_TARGET_DIR)/server.o $(GCC_INC_LIB) $(GCC_LIB)
	$(CXX) $(GCC_FLAGS) $(GCC_INC_FLAGS) $(HOST_APP_SRC)/serial_processor.cpp -o $(BUILD_TARGET_DIR)/serial_processor.o $(GCC_INC_LIB) $(GCC_LIB)
	$(CXX) $(GCC_FLAGS) $(GCC_INC_FLAGS) $(HOST_APP_SRC)/serial_frame.cpp -o $(BUILD_TARGET_DIR)/serial_frame.o $(GCC_INC_LIB) $(GCC_LIB)
	$(CXX) $(GCC_FLAGS) $(GCC_INC_FLAGS) $(HOST_APP_SRC)/circular_buffer.cpp -o $(BUILD_TARGET_DIR)/circular_buffer.o $(GCC_INC_LIB) $(GCC_LIB)
	$(CC)  $(GCC_FLAGS) $(GCC_INC_FLAGS) $(HOST_APP_SRC)/mem_dump.c -o $(BUILD_TARGET_DIR)/mem_dump.o $(GCC_INC_LIB) $(GCC_LIB)
	$(CXX) $(BUILD_TARGET_DIR)/jesd_platform_app.o \
           $(BUILD_TARGET_DIR)/server.o \
           $(BUILD_TARGET_DIR)/serial_processor.o \
           $(BUILD_TARGET_DIR)/serial_frame.o \
           $(BUILD_TARGET_DIR)/mem_dump.o \
           $(BUILD_TARGET_DIR)/circular_buffer.o $(GCC_INC_LIB) $(GCC_LIB) -lpthread -o $(BUILD_TARGET_DIR)/$(APP_ELF)
endif
# =========================================================
# Step 4. Package Generation 
# ========================================================
package:
	rm -rf $(EMBEDDED_PACKAGE_OUT)
	cd $(BUILD_TARGET_DIR);	\
	v++ -p $(PKG_FLAGS) libadf.a

# =========================================================
# Step 5. Run Hardware Emulation 
# ========================================================
# If the target is for HW_EMU, launch the emulator
# If the target is for HW, you'll have to follow the
# instructions in the README.md
run_emu:
	cd $(EMBEDDED_PACKAGE_OUT);
	./launch_hw_emu.sh -run-app $(EMBEDDED_EXEC_SCRIPT)

graph:
	mkdir -p $(BUILD_TARGET_DIR);
	cd $(BUILD_TARGET_DIR);	\
	aiecompiler --platform $(PLATFORM) $(AIE_SRC_REPO)/adpca/src/graph.cpp -include $(AIE_SRC_REPO)/adpca/src -include=src src/graph.cpp -include $(AIE_SRC_REPO)/adpca/src/kernels


all: graph kernels xsa application package

clean_target : 
	rm -rf $(BUILD_TARGET_DIR)

cleanall :
	rm -rf $(BASE_BLD_DIR)
