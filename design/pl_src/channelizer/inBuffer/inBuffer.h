//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Jaemin Park

#pragma once

#include <complex>
#include <vector>
#include <ap_int.h>
#include <hls_vector.h>
#include <hls_stream.h>

constexpr unsigned char M         = 32; // channels in filterbank
constexpr unsigned char DATA_W    = 16; // data width
constexpr unsigned char NSTATE    =  8; // number of states
constexpr unsigned char NSTATE_W  =  3; // bits for width for number of states
constexpr unsigned char SSR_I     = 16; // input SSR 
constexpr unsigned char SSR_O     =  2; // output SSR
constexpr unsigned char TG_SIZE   =  4; // TileGroup size (4 tiles/group)
constexpr unsigned char N_TG      =  4; // num of Tile Groups (4 groups)
constexpr unsigned char N_STRM_I  =  1; // num of in streams (from 1 superCube)
constexpr unsigned char N_STRM_O  = TG_SIZE * N_TG; // num of out streams (to 16 tiles)

using namespace std;

typedef ap_uint<NSTATE_W>           TT_STATE;
typedef ap_uint<1>                  uint1;
typedef ap_uint<2>                  uint2;
typedef ap_uint<4>                  uint4;
typedef ap_int<DATA_W>              din_t;     // I or Q part of cint sample
typedef complex<din_t>              c_din_t;   // cint data type
typedef hls::vector<c_din_t,SSR_I>  TT_IN;     // vector of SSR_I cint samples
typedef hls::vector<c_din_t,SSR_O>  TT_OUT;    // vector of SSR_O cint samples
typedef hls::stream<TT_IN>          TT_STRM_I; // in stream
typedef hls::stream<TT_OUT>         TT_STRM_O; // out streams 

class inBuffer {
public:
  
    // Constructor:
    inBuffer( void );

private:
    void store_samples(c_din_t pp_buf[2][N_TG-1][4][SSR_I], TT_STRM_I sig_i[N_STRM_I], 
                       const TT_STATE state, const uint1 ping);
    void format_strm(TT_OUT str_o[N_STRM_O], const c_din_t pp_buf[2][N_TG-1][4][SSR_I], 
                    const TT_STATE state, const uint1 pong);
    void write_strm(TT_STRM_O sig_o[N_STRM_O], const TT_OUT strm_o[N_STRM_O]);

public:
    // Run: assume 125 MHz clock.
    // This routine needs to be II=1
    void run(TT_STRM_I sig_i[N_STRM_I], TT_STRM_O sig_o[N_STRM_O], int i);

}; // class inBuffer {

void inBuffer_wrapper(TT_STRM_I sig_i[N_STRM_I], TT_STRM_O sig_o[N_STRM_O]);
