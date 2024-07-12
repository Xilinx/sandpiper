//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Jaemin Park

#include <iostream>
#include "outBuffer.h"

using namespace std;

// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------

outBuffer::outBuffer( void )
{
}

// ------------------------------------------------------------
// store the input samples 
// ------------------------------------------------------------

// note: usually array arguments should be passed as pointers, but Vitis-HLS requires the size,
// which is ignored by C++ compiler but is used by Vitis-HLS (p82 UG1399(v2023.1))
void outBuffer::store_samples( 
    c_din_t        pp_buf[2][N_TG/*tiles*/][4/*columns*/][16/*samples*/], 
    TT_STRM_I      sig_i[N_STRM_I],
    const TT_STATE state, // input/output beat id indicator (cycling by 8)
    const uint1    ping)  // "ping-pong" buffer selector (pp_buf[0], pp_buf[1]
{
#pragma HLS INLINE
    // read next data block from input streams:
    TG_L: 
    for (unsigned tg=0; tg < N_TG; ++tg ) // tg: tile group
        for (unsigned t=0; t < TG_SIZE; ++t ) { // t: tile within a tg
            const uint4 t_idx = TG_SIZE * tg + t; // tile index
            const TT_IN val_i = sig_i[t_idx].read(); 
            const uint4 s_idx = TG_SIZE * (TG_SIZE-1 - t) + state/2; // sample index
            for (unsigned j=0; j < SSR_I; ++j ) {
                #pragma HLS unroll // full unrolling
                const uint2 c_idx = 2 * (state % 2) + j;   // column base index
                pp_buf[ping][tg][c_idx][s_idx] = val_i[j];
            } // j
        } // for t
} // store_samples

// ------------------------------------------------------------
// Format Streams
// ------------------------------------------------------------
// note: usually array arguments should be passed as pointers, but Vitis-HLS requires the size,
// which is ignored by C++ compiler but is used by Vitis-HLS (p82 UG1399(v2023.1))
void outBuffer::format_stream(
          TT_OUT    strm_o[N_STRM_O], 
    const c_din_t   pp_buf[2][N_TG/*tile groups(buff)*/][4/*columns*/][16/*samples*/], 
    const TT_STATE  state, // input/output cycle id indicator (cycling by 8)
    const uint1     pong) // "ping-pong" buffer selector (pp_buf[0], pp_buf[1]
{
#pragma HLS INLINE
  
    const uint2 col = state / 2; // column index
    const uint1 bufB = state % 2; // bufB flag
    #pragma HLS reset variable=col
    #pragma HLS reset variable=bufB
#ifdef DEBUG
    cout << "format_stream:pong = " << pong << " A(0)/B(1) = " << bufB << ", col = " << col << endl;
#endif
    // Send out stream:
    OUT_L: for (unsigned f=0; f < N_STRM_O; ++f){ // f: fft
        switch (bufB) {
            case 0: // bufA
                // from tg(buff) 0:
                strm_o[f][1] = pp_buf[pong][0][col][15]; strm_o[f][0] = pp_buf[pong][0][col][14];
                strm_o[f][3] = pp_buf[pong][0][col][13]; strm_o[f][2] = pp_buf[pong][0][col][12];
                strm_o[f][5] = pp_buf[pong][0][col][11]; strm_o[f][4] = pp_buf[pong][0][col][10];
                strm_o[f][7] = pp_buf[pong][0][col][ 9]; strm_o[f][6] = pp_buf[pong][0][col][ 8];
                strm_o[f][9] = pp_buf[pong][0][col][ 7]; strm_o[f][8] = pp_buf[pong][0][col][ 6];
                strm_o[f][11] = pp_buf[pong][0][col][ 5]; strm_o[f][10] = pp_buf[pong][0][col][ 4];
                strm_o[f][13] = pp_buf[pong][0][col][ 3]; strm_o[f][12] = pp_buf[pong][0][col][ 2];
                strm_o[f][15] = pp_buf[pong][0][col][ 1]; strm_o[f][14] = pp_buf[pong][0][col][ 0];
                // from tg(buff) 1:
                strm_o[f][17] = pp_buf[pong][1][col][15]; strm_o[f][16] = pp_buf[pong][1][col][14];
                strm_o[f][19] = pp_buf[pong][1][col][13]; strm_o[f][18] = pp_buf[pong][1][col][12];
                strm_o[f][21] = pp_buf[pong][1][col][11]; strm_o[f][20] = pp_buf[pong][1][col][10];
                strm_o[f][23] = pp_buf[pong][1][col][ 9]; strm_o[f][22] = pp_buf[pong][1][col][ 8];
                strm_o[f][25] = pp_buf[pong][1][col][ 7]; strm_o[f][24] = pp_buf[pong][1][col][ 6];
                strm_o[f][27] = pp_buf[pong][1][col][ 5]; strm_o[f][26] = pp_buf[pong][1][col][ 4];
                strm_o[f][29] = pp_buf[pong][1][col][ 3]; strm_o[f][28] = pp_buf[pong][1][col][ 2];
                strm_o[f][31] = pp_buf[pong][1][col][ 1]; strm_o[f][30] = pp_buf[pong][1][col][ 0];
                break;
            case 1: // bufB - crossing among buff 2 and 3
                // from tg(buff) 3:
                 strm_o[f][1] = pp_buf[pong][3][col][15]; strm_o[f][0] = pp_buf[pong][3][col][14];
                 strm_o[f][3] = pp_buf[pong][3][col][13]; strm_o[f][2] = pp_buf[pong][3][col][12];
                 strm_o[f][5] = pp_buf[pong][3][col][11]; strm_o[f][4] = pp_buf[pong][3][col][10];
                 strm_o[f][7] = pp_buf[pong][3][col][ 9]; strm_o[f][6] = pp_buf[pong][3][col][ 8];
                 strm_o[f][9] = pp_buf[pong][3][col][ 7]; strm_o[f][8] = pp_buf[pong][3][col][ 6];
                 strm_o[f][11] = pp_buf[pong][3][col][ 5]; strm_o[f][10] = pp_buf[pong][3][col][ 4];
                 strm_o[f][13] = pp_buf[pong][3][col][ 3]; strm_o[f][12] = pp_buf[pong][3][col][ 2];
                 strm_o[f][15] = pp_buf[pong][3][col][ 1]; strm_o[f][14] = pp_buf[pong][3][col][ 0];
                // from tg(buff) 2:
                 strm_o[f][17] = pp_buf[pong][2][col][15]; strm_o[f][16] = pp_buf[pong][2][col][14];
                 strm_o[f][19] = pp_buf[pong][2][col][13]; strm_o[f][18] = pp_buf[pong][2][col][12];
                 strm_o[f][21] = pp_buf[pong][2][col][11]; strm_o[f][20] = pp_buf[pong][2][col][10];
                 strm_o[f][23] = pp_buf[pong][2][col][ 9]; strm_o[f][22] = pp_buf[pong][2][col][ 8];
                 strm_o[f][25] = pp_buf[pong][2][col][ 7]; strm_o[f][24] = pp_buf[pong][2][col][ 6];
                 strm_o[f][27] = pp_buf[pong][2][col][ 5]; strm_o[f][26] = pp_buf[pong][2][col][ 4];
                 strm_o[f][29] = pp_buf[pong][2][col][ 3]; strm_o[f][28] = pp_buf[pong][2][col][ 2];
                 strm_o[f][31] = pp_buf[pong][2][col][ 1]; strm_o[f][30] = pp_buf[pong][2][col][ 0];
                break;
        } // switch bufB
    } // for f
} // format_stream

// ------------------------------------------------------------
// Write Output Streams
// sig_o[3] copy (from sig_o[0]) will take place in this function.
// ------------------------------------------------------------

void outBuffer::write_stream(TT_STRM_O sig_o[N_STRM_O], const TT_OUT strm_o[N_STRM_O])
{
#pragma HLS INLINE
    OUT_L: for (unsigned t=0; t < N_STRM_O; ++t ){
        sig_o[t].write(strm_o[t]);
    }
}

// ------------------------------------------------------------
// Run
// ------------------------------------------------------------

void outBuffer::run(TT_STRM_I sig_i[N_STRM_I], TT_STRM_O sig_o[N_STRM_O], int i)
{
    #pragma HLS inline

    // Need to declare as 'static' in Vitis HLS to allow resets to be applied:
    static TT_STATE state = 0; // to trace input beat (cycling by 8)
    static uint1 ping_pong = 0; // ping-pong buffer selector (pp_buf[0], pp_buf[1])
    #pragma HLS reset variable=state
    #pragma HLS reset variable=ping_pong

    /////////////////////////////////////
    // store the input samples 
    /////////////////////////////////////
    // use static qualifier to prevent initialization of the memory again
    static c_din_t pp_buf[2][N_TG][4][16] = {}; // initialize all elements with 0
            // ping-pong(2), 4 tile groups, 4 columns, 16 samples/column (or 125MHz cc)
    #pragma HLS array_partition variable=pp_buf type=complete dim=0
  
  if(i<40000)
    store_samples(pp_buf, sig_i, state, ping_pong);
  
    /////////////////////////////////////
    // Format streams call:
    /////////////////////////////////////
    static TT_OUT strm_o[N_STRM_O] = {};
    #pragma HLS array_partition variable=strm_o

    uint1 pong = ping_pong ? 0 : 1; // pong(opposite) buf pointer; 

  if(i>=8) {
    format_stream(strm_o, pp_buf, state, pong);

    /////////////////////////////////////
    // Write output to stream:
    /////////////////////////////////////
        write_stream(sig_o, strm_o);
    } // if not first8

    ++state; // continue for next input beat
    if (state == 0) {
        ping_pong = ping_pong ? 0 : 1; // point to next buf to store next input data block
#ifdef DEBUG
        cout << "ping_pong toggled! " << ping_pong << endl;
#endif
    }
} // run

// ------------------------------------------------------------
// Wrapper
// ------------------------------------------------------------

void outBuffer_wrapper(TT_STRM_I sig_i[N_STRM_I], TT_STRM_O sig_o[N_STRM_O])
{
#pragma HLS interface mode=ap_ctrl_none port=return // no block-level I/O protocol, 
                                                    // may blocking C/RTL cosim verification
  static outBuffer dut;

  for(int i=0; i<40008; i++) {
#pragma HLS pipeline II=1
     dut.run( sig_i, sig_o, i );
  }
}
