//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Jaemin Park

#include <iostream>
#include "inBuffer.h"
using namespace std;

// ------------------------------------------------------------
// Constructor
// ------------------------------------------------------------

inBuffer::inBuffer( void )
{
}

// ------------------------------------------------------------
// store the input samples 
// ------------------------------------------------------------

// note: usually array arguments should be passed as pointers, but Vitis-HLS requires the size,
// which is ignored by C++ compiler but is used by Vitis-HLS (p82 UG1399(v2023.1))
void inBuffer::store_samples( 
    c_din_t        pp_buf[2][N_TG-1][4][SSR_I], 
    TT_STRM_I      sig_i[N_STRM_I],
    const TT_STATE state, // input/output cycle id indicator (cycling by 8)
    const uint1    ping)  // "ping-pong" buffer selector (pp_buf[0], pp_buf[1]
{
#pragma HLS INLINE
    TT_IN val_i = sig_i[0].read(); // read next SSR_I cint samples from input stream
    // cout << "val_i[" << dec << state << "] = " << val_i << endl; // debug-print
    switch (state) {
        case 0: { // input samples 0 ~ 15
            const uint1 pong = ping ? 0 : 1; // "toggling"
            for (unsigned j=0; j < SSR_I; ++j ) {
                #pragma HLS unroll   // full unrolling
                pp_buf[ping][0][0][j] = val_i[j]; // TG 0, col 0
                pp_buf[ping][1][0][j] = pp_buf[pong][2][3][j]; // TG 1, col 0 from TG 2, col 3 of "pong" 
            } // for j
            break;
        }
        case 1: // input samples 16 ~ 31
            for (unsigned j=0; j < SSR_I; ++j ) {
                #pragma HLS unroll   // full unrolling
                pp_buf[ping][1][1][j] = val_i[j]; // TG 1, col 1
                pp_buf[ping][2][0][j] = val_i[j]; // TG 2, col 0
            } // for j
            break;
        case 2: // input samples 32 ~ 47
            for (unsigned j=0; j < SSR_I; ++j ) {
                #pragma HLS unroll   // full unrolling
                pp_buf[ping][0][1][j] = val_i[j]; // TG 0, col 1
            } // for j
            break;
        case 3: // input samples 48 ~ 63
            for (unsigned j=0; j < SSR_I; ++j ) {
                #pragma HLS unroll   // full unrolling
                pp_buf[ping][1][2][j] = val_i[j]; // TG 1, col 2
                pp_buf[ping][2][1][j] = val_i[j]; // TG 2, col 1
            } // for j
            break;
        case 4: // input samples 64 ~ 79
            for (unsigned j=0; j < SSR_I; ++j ) {
                #pragma HLS unroll   // full unrolling
                pp_buf[ping][0][2][j] = val_i[j]; // TG 0, col 2
            } // for j
            break;
        case 5: // input samples 80 ~ 95
            for (unsigned j=0; j < SSR_I; ++j ) {
                #pragma HLS unroll   // full unrolling
                pp_buf[ping][1][3][j] = val_i[j]; // TG 1, col 3
                pp_buf[ping][2][2][j] = val_i[j]; // TG 2, col 2
            } // for j
            break;
        case 6: // input samples 96 ~ 111
            for (unsigned j=0; j < SSR_I; ++j ) {
                #pragma HLS unroll   // full unrolling
                pp_buf[ping][0][3][j] = val_i[j]; // TG 0, col 3
            } // for j
            break;
        case 7: // input samples 112 ~ 127
            for (unsigned j=0; j < SSR_I; ++j ) {
                #pragma HLS unroll   // full unrolling
                pp_buf[ping][2][3][j] = val_i[j]; // TG 2, col 3
            } // for j
            break;
        // default: 
    } // switch i
} // store_samples

///////////////////////////////////////
// Format Streams
///////////////////////////////////////
// note: usually array arguments should be passed as pointers, but Vitis-HLS requires the size,
// which is ignored by C++ compiler but is used by Vitis-HLS (p82 UG1399(v2023.1))
void inBuffer::format_strm(
          TT_OUT    strm_o[N_STRM_O-4], 
    const c_din_t   pp_buf[2][N_TG-1][4/*columns*/][SSR_I/*samples*/], 
    const TT_STATE  state, // input/output cycle id indicator (cycling by NSTATE)
    const uint1     pong) // "ping-pong" buffer selector (pp_buf[0], pp_buf[1])
{
#pragma HLS INLINE
  
    // Send streams:
    TG_L: 
    for (unsigned tg=0; tg < N_TG-1; ++tg ) // tg: tile group
        for (unsigned t=0; t < TG_SIZE; ++t ) // t: tile within a tg
            for (unsigned j=0; j < SSR_O; ++j ) {
                #pragma HLS unroll   // full unrolling
                const uint4 t_idx = TG_SIZE * tg + t; // tile index
                const uint2 c_idx = 2 * (state % 2) + j; // column base index 
                const uint4 s_idx = TG_SIZE * (TG_SIZE-1 - t) + state / 2; // sample index
                strm_o[t_idx][j] = pp_buf[pong][tg][c_idx][s_idx];
            } // j
} // format_strm

//////////////////////////////////////////////////////////////////////////
// Write Output Streams
// sig_o[TG3] copy (from sig_o[TG0]) will take place in this function.
//////////////////////////////////////////////////////////////////////////

void inBuffer::write_strm( TT_STRM_O sig_o[N_STRM_O], const TT_OUT strm_o[N_STRM_O-4])
{
#pragma HLS INLINE
    T_L: for (unsigned tg=0; tg < N_TG-1; ++tg ){
        sig_o[TG_SIZE*tg+0].write(strm_o[TG_SIZE*tg+0]);
        sig_o[TG_SIZE*tg+1].write(strm_o[TG_SIZE*tg+1]);
        sig_o[TG_SIZE*tg+2].write(strm_o[TG_SIZE*tg+2]);
        sig_o[TG_SIZE*tg+3].write(strm_o[TG_SIZE*tg+3]);
    } // tg 
    // duplicate for TG3 streams with TG0
    sig_o[12].write(strm_o[0]);
    sig_o[13].write(strm_o[1]);
    sig_o[14].write(strm_o[2]);
    sig_o[15].write(strm_o[3]);
} // write_strm

//////////////////////////////////////////////////////////////////////////
// Run
//////////////////////////////////////////////////////////////////////////

void inBuffer::run(TT_STRM_I sig_i[N_STRM_I], TT_STRM_O sig_o[N_STRM_O], int i)
{
    #pragma HLS inline

    // Need to declare as 'static' in Vitis HLS to allow resets to be applied:
    static TT_STATE state = 0; // to trace input beat (cycling by NSTATE)
    static uint1 ping_pong = 0; // ping-pong buffer selector (pp_buf[0], pp_buf[1])
    #pragma HLS reset variable=state
    #pragma HLS reset variable=ping_pong
  
    /////////////////////////////////////
    // store the input samples 
    /////////////////////////////////////
    // use static qualifier to prevent initialization of the memory again
    static c_din_t pp_buf[2][N_TG-1][4][SSR_I] = {}; // initialize all elements with 0
            // ping-pong(2) input buffer 3 TGs(tile group) (TG 3 is copy of TG 0), 
            // 4 columns, SSR_I samples / column (or 125MHz cc)
    #pragma HLS array_partition variable=pp_buf type=complete dim=0

  if(i<40000)
    store_samples(pp_buf, sig_i, state, ping_pong);
  
    /////////////////////////////////////
    // Format streams call:
    /////////////////////////////////////
    // TG 3 is a duplicate of TG 0, so no resource needed for TG 3
    static TT_OUT strm_o[N_STRM_O-4] = {c_din_t(0,0)}; 
    #pragma HLS array_partition variable=strm_o

    uint1 pong = ping_pong ? 0 : 1;

  if(i>=8) {
    format_strm(strm_o, pp_buf, state, pong); // with correct pp_buf
#ifdef DEBUG 
    for (int tg=0; tg < N_TG-1; ++tg ) // tg: tile group
        for (int t=0; t < TG_SIZE; ++t ) { // t: tile within a tg`
            const int tile = TG_SIZE * tg + t;
            cout << "state " << dec << state << ":strm_o[" << tile << "]=";
            for (int j=SSR_O-1; 0 <= j; --j ) cout << strm_o[tile][j];
            cout << endl;
        } // for t
#endif

    /////////////////////////////////////
    // Write outputs to streams:
    // sig_o[TG3] copy (from sig_o[TG0]) will take place in this function.
    /////////////////////////////////////
    //static bool first8 = true; // first 8-cycle flag to shut down the first 8 zeros output
    //#pragma HLS reset variable=first8

        write_strm(sig_o, strm_o);
    }

    ++state; // continue for next input beat
    if (state == 0) {
        //first8 = false;
        ping_pong = ping_pong ? 0 : 1; // point to next buf to store next input data block (of 128 samples)
#ifdef DEBUG 
        cout << "ping_pong toggled! " << ping_pong << endl;
#endif
    }
} // run

// ------------------------------------------------------------
// Wrapper
// ------------------------------------------------------------

// using TT_DUT = inBuffer;

void inBuffer_wrapper(TT_STRM_I sig_i[N_STRM_I], TT_STRM_O sig_o[N_STRM_O])
{
#pragma HLS interface mode=ap_ctrl_none port=return // no block-level I/O protocol, 
    static inBuffer dut;

  for(int i=0; i<40008; i++) {
#pragma HLS pipeline II=1
    dut.run( sig_i, sig_o, i );
  }
}
