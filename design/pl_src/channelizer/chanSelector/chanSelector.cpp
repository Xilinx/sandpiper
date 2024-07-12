//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Jaemin Park

#include <iostream>
#include "chanSelector.h"
using namespace std;

// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------

chanSelector::chanSelector( void )
{
}

// ------------------------------------------------------------
// Run
// ------------------------------------------------------------

// Sel is passed as pass by value, since it does not need read access.
// If need read access, a pointer (i.e., const sel_t *sel) should be used.
void chanSelector::run(TT_STRM_I sig_i[N_STRM_I], TT_STRM_O sig_o[N_STRM_O], const sel_t sel, TT_STRM_O sig_o_tc[N_STRM_O], int size)
{
    #pragma HLS inline
    //static cnt_t cc_cnt = 0;
    //static sel_t sel_sync = 0; // sel sync'd to CUBE BOUNDARY
    static c_din_t buffer[SSR_I] = {};  
    //#pragma HLS reset variable=cc_cnt
    //#pragma HLS reset variable=sel_sync
    #pragma HLS array_partition variable=buffer type=complete dim=0

    //sel_sync = cc_cnt == cnt_t(0) ? sel : sel_sync;
/*#ifdef DEBUG
    cout << "sel_sync: " << sel_sync << ' ';
#endif*/
	for(int i = 0; i < size; i++){
#pragma HLS pipeline II=1
	    TT_IN val_i  = sig_i[0].read();
	    for (unsigned j=0; j < SSR_I; ++j) buffer[j] = val_i[j];
	    TT_OUT val_o = buffer[sel];
	    sig_o[0].write(val_o);
		sig_o_tc[0].write(val_o);
	    //cc_cnt = cc_cnt == cnt_t(CUBE_BNDRY_CC-1) ? cnt_t(0) : cnt_t(cc_cnt+1);
	}
} // run

// ------------------------------------------------------------
// Wrapper
// ------------------------------------------------------------

// Sel is passed as pass by value, since it does not need read access.
// If need read access, a pointer (i.e., const sel_t *sel) should be used.
void chanSelector_wrapper(TT_STRM_I sig_i[N_STRM_I], TT_STRM_O sig_o[N_STRM_O], const sel_t sel, TT_STRM_O sig_o_tc[N_STRM_O], int size)
{
#pragma HLS INTERFACE mode=s_axilite port=sel bundle=control
#pragma HLS INTERFACE mode=s_axilite port=size bundle=control
#pragma HLS INTERFACE mode=s_axilite port=return bundle=control

    static chanSelector dut;
    dut.run(sig_i, sig_o, sel, sig_o_tc, size);
}
