//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Jaemin Park

#pragma once

// #include <cmath>     // log2
#include <complex>
#include <vector>
#include "ap_int.h"
#include "hls_vector.h"
#include "hls_stream.h"

// constexpr must be initialized at compile time:
constexpr unsigned char M          = 32; // channels in filterbank
constexpr unsigned char DATA_W     = 27; // 32; // data width
constexpr unsigned char NSTATE     =  8; // number of states
constexpr unsigned char NSTATE_W   =  3; // log2(NSTATE); // bits for width for number of states
constexpr unsigned char SSR_I      =  2; // 8; // input SSR
constexpr unsigned char SSR_O      = 32; // output SSR
constexpr unsigned char TG_SIZE    =  4; // TileGroup size (4 tiles/group)
constexpr unsigned char N_TG       =  4; // num of Tile Groups (4 groups)
constexpr unsigned char N_STRM_I   = TG_SIZE * N_TG; // 16; // num of in streams (from 16 tiles)
constexpr unsigned char N_STRM_O   =  1; // num of out streams (to 1 fft)

using namespace std;

typedef ap_uint<NSTATE_W>          TT_STATE;
typedef ap_uint<4>                 uint4;
typedef ap_uint<2>                 uint2;
typedef ap_uint<1>                 uint1;
typedef ap_int<DATA_W>             din_t;     // real or imagery part of cint sample
typedef complex<din_t>             c_din_t;   // cint sample
typedef hls::vector<c_din_t,SSR_I> TT_IN;     // SSR_I cint samples in aggregation
typedef hls::vector<c_din_t,SSR_O> TT_OUT;    // SSR_O cint samples in aggregation
typedef hls::stream<TT_IN>         TT_STRM_I; // input stream
typedef hls::stream<TT_OUT>        TT_STRM_O; // output stream
  
class outBuffer {
private:
    void store_samples(c_din_t pp_buf[2][N_TG][4][16],TT_STRM_I sig_i[SSR_I],const TT_STATE state,const uint1 ping);
    void format_stream(TT_OUT strm_o[SSR_O],const c_din_t pp_buf[2][N_TG][4][16],const TT_STATE state,const uint1 pong);
    void write_stream(TT_STRM_O sig_o[SSR_O], const TT_OUT strm_o[SSR_O]);

public:
    // Constructor:
    outBuffer( void );
    // Run: assume 125 MHz clock.
    // This routine needs to be II=1
    void run(TT_STRM_I sig_i[SSR_I], TT_STRM_O sig_o[SSR_O], int i);

}; // class outBuffer

void outBuffer_wrapper(TT_STRM_I sig_i[SSR_I], TT_STRM_O sig_o[SSR_O]);
