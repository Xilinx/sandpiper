#
# Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
# SPDX-License-Identifier: MIT
#
# Author: Jaemin Park

ECHO            := @echo
KERNEL          := chanSelector_wrapper
KERNEL_SRC      := chanSelector.cpp
XO              := ${KERNEL}.xo
PART            := xcvc1902-vsva2197-2MP-e-S
HLS_FREQ        := 125000000

.PHONY: clean all help

help::
	$(ECHO) "Makefile Usage:"
	$(ECHO) "  make all"
	$(ECHO) "      Command to generate the hls kernel used in this design"
	$(ECHO) ""
	$(ECHO) "  make clean"
	$(ECHO) "      Command to remove all the generated files."

all: ${XO}

${XO}: ${KERNEL_SRC}
	v++ --compile \
	    --mode hls \
	    --part ${PART} \
	    --config ./hls_config.cfg \
	    ${KERNEL_SRC}

clean:
	rm -rf chanselector_wrapper .Xil
