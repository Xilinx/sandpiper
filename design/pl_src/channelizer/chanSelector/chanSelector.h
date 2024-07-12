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

constexpr unsigned M          = 32; // channels in filterbank
constexpr unsigned SEL_W      =  5; // selector data width
constexpr unsigned DATA_W     = 16; // data width
constexpr unsigned SSR_I      = 32; // input SSR 
constexpr unsigned SSR_O      =  1; // output SSR
constexpr unsigned char N_STRM_I   =  1; // num of in streams (from 1 fft)
constexpr unsigned char N_STRM_O   =  1; // num of out streams (to 1 Training Selector)
constexpr unsigned CUBE_BNDRY     = 4000 * 10; // cube boundary - should be divisable by SSR_I
constexpr unsigned CUBE_BNDRY_CC  = CUBE_BNDRY/SSR_I; // cube boundary in clock cycle = 1250
constexpr unsigned CNT_W       = 11; // count width (ceil(log2(CUBE_BNDRY_CC))

using namespace std;

typedef ap_uint<CNT_W>              cnt_t;     // count type
typedef ap_uint<SEL_W>              sel_t;     // selector type
typedef ap_int<DATA_W>              din_t;     // I or Q part of cint sample
typedef complex<din_t>              c_din_t;   // cint data type
typedef hls::vector<c_din_t,SSR_I>  TT_IN;     // 32 cint samples
typedef hls::vector<c_din_t,SSR_O>  TT_OUT;    //  1 cint samples
typedef hls::stream<TT_IN>          TT_STRM_I; // in stream
typedef hls::stream<TT_OUT>         TT_STRM_O; // out stream 

class chanSelector {
public:
  
    // Constructor:
    chanSelector( void );

    // Run: assume 125 MHz clock.
    // This routine needs to be II=1
    void run(TT_STRM_I sig_i[N_STRM_I], TT_STRM_O sig_o[N_STRM_O], const sel_t sel, TT_STRM_O sig_o_tc[N_STRM_O], int szie);

}; // class chanSelector {

void chanSelector_wrapper(TT_STRM_I sig_i[N_STRM_I], TT_STRM_O sig_o[N_STRM_O], const sel_t sel, TT_STRM_O sig_o_tc[N_STRM_O], int size);
