//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Jaemin Park

#include <complex>
#include <vector>
#include "ap_fixed.h"
#include "hls_vector.h"
#include <hls_stream.h>

// configurable params
const unsigned char FFT_NFFT     = 5; // stages
const unsigned char FFT_LEN      = 1 << FFT_NFFT;
const unsigned char FFT_IN_WID   = 27; // input element data width
const unsigned char FFT_TWID_WID = 18; // twiddle factor data width
const unsigned char FFT_OUT_WID  = 16; // output element data width

const int fftStageWrdLen  = FFT_IN_WID;     // fi word length for input/intermediate signal
const int fftStageFracLen = FFT_IN_WID-2;   // = 25: fi fraction length for input/intermediate signal 
const int fftTwidWrdLen   = FFT_TWID_WID;   // fi word length for FFT twiddle factors
const int fftTwidFracLen  = FFT_TWID_WID-2; // = 16; fi fraction length for FFT twiddle factors 
const int fftOutWrdLen    = FFT_OUT_WID;    // fi word length for output signal
const int fftOutFracLen   = FFT_OUT_WID-1;  // = 15;fi fraction length for output signal 

using namespace std;

// ap_[u]fixed<W,I,Q,O,N>: Word len, Integer, Quantization/Round, Overflow/saturation, N saturation bits

typedef ap_fixed<fftStageWrdLen,fftStageWrdLen-fftStageFracLen,AP_RND,AP_SAT,fftStageWrdLen> din_t;  // <27,25>
typedef ap_fixed<fftTwidWrdLen, fftTwidWrdLen-fftTwidFracLen,  AP_RND,AP_SAT,fftTwidWrdLen > twid_t; // <18,2>
typedef ap_fixed<fftOutWrdLen,  fftOutWrdLen-fftOutFracLen,    AP_RND,AP_SAT,fftOutWrdLen  > dout_t; // <16,1>
typedef ap_fixed<fftStageFracLen+2,fftStageFracLen+2> TT_CONV_FULL; // ap_fixed<27,27>
typedef ap_fixed<fftOutFracLen  +2,fftOutFracLen  +2> TT_CONV;      // ap_fixed<17,17>

typedef complex<din_t>  c_din_t;
typedef complex<twid_t> c_twid_t;
typedef complex<dout_t> c_dout_t;

typedef c_din_t  TT_DATA;   // input and internal data type
typedef c_twid_t TT_TWID_D; // twiddle data type
typedef c_dout_t TT_ODATA;  // output data type

typedef hls::vector<TT_DATA,  FFT_LEN>   TT_IN;   // input data aggregated in vector
typedef hls::vector<TT_TWID_D,FFT_LEN/2> TT_TWID; // twiddle factors aggregated in vector
typedef hls::vector<TT_ODATA, FFT_LEN>   TT_OUT;  // output data aggregated in vector

typedef hls::stream<TT_IN>  TT_IN_STRM;  // input stream or output in full precision
typedef hls::stream<TT_OUT> TT_OUT_STRM; // output stream

void fft_top(TT_IN_STRM& in, TT_OUT_STRM& out);
