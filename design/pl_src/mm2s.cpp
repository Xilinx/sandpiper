/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

#include <ap_int.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>


extern "C" {

void mm2s(ap_int<64>* mem, hls::stream<qdma_axis<64, 0, 0, 0>  >& s, int size) {
#pragma HLS INTERFACE m_axi port=mem offset=slave bundle=gmem max_read_burst_length=256

#pragma HLS interface axis port=s
//#pragma HLS interface axis port=s_tc

#pragma HLS INTERFACE s_axilite port=mem bundle=control
#pragma HLS INTERFACE s_axilite port=size bundle=control
#pragma HLS interface s_axilite port=return bundle=control

	for(int i = 0; i < size; i++) {
#pragma HLS PIPELINE II=1
ap_int<64> read_data;
		read_data = mem[i];
		qdma_axis<64, 0, 0, 0> x;
		x.data = read_data; 
		x.keep_all();
		s.write(x);
		//qdma_axis<64, 0, 0, 0> y;
		//y.data = read_data; 
		//y.keep_all();
		//s_tc.write(y);
	}

}

}
