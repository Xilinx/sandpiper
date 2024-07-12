//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Jaemin Park

#include <iostream>  // cout
#include <cmath>    // complex math
#include <cassert>  // assert()
#include <fstream>
#include <string>
#include "fft_top.h"

TT_TWID genTwidFactors(const int N)
{
    assert(N % 2 == 0);
    TT_TWID twid;
// #ifdef DEBUG // for sim only, not "v++compiler synthsizable"
//     const string twidFileName = "twiddleF.txt";
//     ifstream twidFile;
//     twidFile.open(twidFileName);
//     if (twidFile.is_open() == 0) cerr << "can't open file " << twidFileName << endl;
// 
//     for (int i=0; i < N/2; ++i){
//         int twid_re, twid_im;
//         twidFile >> twid_re >> twid_im;
//         twid[i] = TT_TWID_D(twid_t( twid_re * pow(2,-fftTwidFracLen)),
//                             twid_t(-twid_im * pow(2,-fftTwidFracLen)));
//         cout << "twid[" << i << "] = " << twid[i] << endl;
//     } // for i
//     twidFile.close();
// #else
    for (int i=0; i < N/2; ++i){
        // twid[i] = cos(2*M_PI/N*i) - sin(2*M_PI/N*i)*1i;
        twid[i] = cos(2*M_PI/N*i) + sin(2*M_PI/N*i)*1i;
#ifdef DEBUG
        cout << "twid[" << i << "] = " << twid[i] << endl;
#endif
    } // for i
// #endif
    return twid;
} // genTwidFactors

void fft_top(TT_IN_STRM& in_strm, TT_OUT_STRM& out_strm)
{
#pragma HLS interface mode=ap_ctrl_none port=return // no block-level I/O protocol, 
                                                    // may blocking C/RTL cosim verification
#pragma HLS pipeline II=1 style=flp

    // assert(fftOutFracLen == fftTwidFracLen && fftOutWrdLen == fftTwidWrdLen);
    // Need to declare as 'static' in Vitis HLS to allow resets to be applied:
    static TT_TWID twid = genTwidFactors(FFT_LEN);

    ////////////////////////////////////////////////////////////////////////////
    // load data
    ////////////////////////////////////////////////////////////////////////////
    TT_IN vecIn = in_strm.read();

    ////////////////////////////////////////////////////////////////////////////
    // FFT core: direct implmentation of the signal flow graph (butterfly diagram) for 32-pt FFT.
    // Shuffling is inherently implemented in the butterfly diagram.
    // The final output is in natural order.
    ////////////////////////////////////////////////////////////////////////////
    // stage 0:
    TT_IN vecA;
    #pragma HLS BIND_OP variable=vecA op=mul impl=dsp
    Stage0: {
    #pragma HLS latency max=4
    // vecA[ 0] = vecIn[ 0] + vecIn[16] * TT_DATA(twid[0]); // *: complex multiplier
    // vecA[ 1] = vecIn[ 0] - vecIn[16] * TT_DATA(twid[0]); // *: complex multiplier
    vecA[ 0] = vecIn[ 0] + TT_DATA(vecIn[16].real()*twid[0].real()-vecIn[16].imag()*twid[0].imag(),vecIn[16].imag()*twid[0].real()+vecIn[16].real()*twid[0].imag());
    vecA[ 1] = vecIn[ 0] - TT_DATA(vecIn[16].real()*twid[0].real()-vecIn[16].imag()*twid[0].imag(),vecIn[16].imag()*twid[0].real()+vecIn[16].real()*twid[0].imag());
    vecA[ 2] = vecIn[ 8] + TT_DATA(vecIn[24].real()*twid[0].real()-vecIn[24].imag()*twid[0].imag(),vecIn[24].imag()*twid[0].real()+vecIn[24].real()*twid[0].imag());
    vecA[ 3] = vecIn[ 8] - TT_DATA(vecIn[24].real()*twid[0].real()-vecIn[24].imag()*twid[0].imag(),vecIn[24].imag()*twid[0].real()+vecIn[24].real()*twid[0].imag());
    vecA[ 4] = vecIn[ 4] + TT_DATA(vecIn[20].real()*twid[0].real()-vecIn[20].imag()*twid[0].imag(),vecIn[20].imag()*twid[0].real()+vecIn[20].real()*twid[0].imag());
    vecA[ 5] = vecIn[ 4] - TT_DATA(vecIn[20].real()*twid[0].real()-vecIn[20].imag()*twid[0].imag(),vecIn[20].imag()*twid[0].real()+vecIn[20].real()*twid[0].imag());
    vecA[ 6] = vecIn[12] + TT_DATA(vecIn[28].real()*twid[0].real()-vecIn[28].imag()*twid[0].imag(),vecIn[28].imag()*twid[0].real()+vecIn[28].real()*twid[0].imag());
    vecA[ 7] = vecIn[12] - TT_DATA(vecIn[28].real()*twid[0].real()-vecIn[28].imag()*twid[0].imag(),vecIn[28].imag()*twid[0].real()+vecIn[28].real()*twid[0].imag());
    vecA[ 8] = vecIn[ 2] + TT_DATA(vecIn[18].real()*twid[0].real()-vecIn[18].imag()*twid[0].imag(),vecIn[18].imag()*twid[0].real()+vecIn[18].real()*twid[0].imag());
    vecA[ 9] = vecIn[ 2] - TT_DATA(vecIn[18].real()*twid[0].real()-vecIn[18].imag()*twid[0].imag(),vecIn[18].imag()*twid[0].real()+vecIn[18].real()*twid[0].imag());
    vecA[10] = vecIn[10] + TT_DATA(vecIn[26].real()*twid[0].real()-vecIn[26].imag()*twid[0].imag(),vecIn[26].imag()*twid[0].real()+vecIn[26].real()*twid[0].imag());
    vecA[11] = vecIn[10] - TT_DATA(vecIn[26].real()*twid[0].real()-vecIn[26].imag()*twid[0].imag(),vecIn[26].imag()*twid[0].real()+vecIn[26].real()*twid[0].imag());
    vecA[12] = vecIn[ 6] + TT_DATA(vecIn[22].real()*twid[0].real()-vecIn[22].imag()*twid[0].imag(),vecIn[22].imag()*twid[0].real()+vecIn[22].real()*twid[0].imag());
    vecA[13] = vecIn[ 6] - TT_DATA(vecIn[22].real()*twid[0].real()-vecIn[22].imag()*twid[0].imag(),vecIn[22].imag()*twid[0].real()+vecIn[22].real()*twid[0].imag());
    vecA[14] = vecIn[14] + TT_DATA(vecIn[30].real()*twid[0].real()-vecIn[30].imag()*twid[0].imag(),vecIn[30].imag()*twid[0].real()+vecIn[30].real()*twid[0].imag());
    vecA[15] = vecIn[14] - TT_DATA(vecIn[30].real()*twid[0].real()-vecIn[30].imag()*twid[0].imag(),vecIn[30].imag()*twid[0].real()+vecIn[30].real()*twid[0].imag());

    vecA[16] = vecIn[ 1] + TT_DATA(vecIn[17].real()*twid[0].real()-vecIn[17].imag()*twid[0].imag(),vecIn[17].imag()*twid[0].real()+vecIn[17].real()*twid[0].imag());
    vecA[17] = vecIn[ 1] - TT_DATA(vecIn[17].real()*twid[0].real()-vecIn[17].imag()*twid[0].imag(),vecIn[17].imag()*twid[0].real()+vecIn[17].real()*twid[0].imag());
    vecA[18] = vecIn[ 9] + TT_DATA(vecIn[25].real()*twid[0].real()-vecIn[25].imag()*twid[0].imag(),vecIn[25].imag()*twid[0].real()+vecIn[25].real()*twid[0].imag());
    vecA[19] = vecIn[ 9] - TT_DATA(vecIn[25].real()*twid[0].real()-vecIn[25].imag()*twid[0].imag(),vecIn[25].imag()*twid[0].real()+vecIn[25].real()*twid[0].imag());
    vecA[20] = vecIn[ 5] + TT_DATA(vecIn[21].real()*twid[0].real()-vecIn[21].imag()*twid[0].imag(),vecIn[21].imag()*twid[0].real()+vecIn[21].real()*twid[0].imag());
    vecA[21] = vecIn[ 5] - TT_DATA(vecIn[21].real()*twid[0].real()-vecIn[21].imag()*twid[0].imag(),vecIn[21].imag()*twid[0].real()+vecIn[21].real()*twid[0].imag());
    vecA[22] = vecIn[13] + TT_DATA(vecIn[29].real()*twid[0].real()-vecIn[29].imag()*twid[0].imag(),vecIn[29].imag()*twid[0].real()+vecIn[29].real()*twid[0].imag());
    vecA[23] = vecIn[13] - TT_DATA(vecIn[29].real()*twid[0].real()-vecIn[29].imag()*twid[0].imag(),vecIn[29].imag()*twid[0].real()+vecIn[29].real()*twid[0].imag());
    vecA[24] = vecIn[ 3] + TT_DATA(vecIn[19].real()*twid[0].real()-vecIn[19].imag()*twid[0].imag(),vecIn[19].imag()*twid[0].real()+vecIn[19].real()*twid[0].imag());
    vecA[25] = vecIn[ 3] - TT_DATA(vecIn[19].real()*twid[0].real()-vecIn[19].imag()*twid[0].imag(),vecIn[19].imag()*twid[0].real()+vecIn[19].real()*twid[0].imag());
    vecA[26] = vecIn[11] + TT_DATA(vecIn[27].real()*twid[0].real()-vecIn[27].imag()*twid[0].imag(),vecIn[27].imag()*twid[0].real()+vecIn[27].real()*twid[0].imag());
    vecA[27] = vecIn[11] - TT_DATA(vecIn[27].real()*twid[0].real()-vecIn[27].imag()*twid[0].imag(),vecIn[27].imag()*twid[0].real()+vecIn[27].real()*twid[0].imag());
    vecA[28] = vecIn[ 7] + TT_DATA(vecIn[23].real()*twid[0].real()-vecIn[23].imag()*twid[0].imag(),vecIn[23].imag()*twid[0].real()+vecIn[23].real()*twid[0].imag());
    vecA[29] = vecIn[ 7] - TT_DATA(vecIn[23].real()*twid[0].real()-vecIn[23].imag()*twid[0].imag(),vecIn[23].imag()*twid[0].real()+vecIn[23].real()*twid[0].imag());
    vecA[30] = vecIn[15] + TT_DATA(vecIn[31].real()*twid[0].real()-vecIn[31].imag()*twid[0].imag(),vecIn[31].imag()*twid[0].real()+vecIn[31].real()*twid[0].imag());
    vecA[31] = vecIn[15] - TT_DATA(vecIn[31].real()*twid[0].real()-vecIn[31].imag()*twid[0].imag(),vecIn[31].imag()*twid[0].real()+vecIn[31].real()*twid[0].imag());
    } // Stage0

    // stage 1:
    TT_IN vecB;
    #pragma HLS BIND_OP variable=vecB op=mul impl=dsp
    Stage1: {
    #pragma HLS latency max=4
    vecB[ 0] = vecA[ 0] + TT_DATA(vecA[ 2].real()*twid[0].real()-vecA[ 2].imag()*twid[0].imag(),vecA[ 2].imag()*twid[0].real()+vecA[ 2].real()*twid[0].imag());
    vecB[ 2] = vecA[ 0] - TT_DATA(vecA[ 2].real()*twid[0].real()-vecA[ 2].imag()*twid[0].imag(),vecA[ 2].imag()*twid[0].real()+vecA[ 2].real()*twid[0].imag());
    vecB[ 1] = vecA[ 1] + TT_DATA(vecA[ 3].real()*twid[8].real()-vecA[ 3].imag()*twid[8].imag(),vecA[ 3].imag()*twid[8].real()+vecA[ 3].real()*twid[8].imag());
    vecB[ 3] = vecA[ 1] - TT_DATA(vecA[ 3].real()*twid[8].real()-vecA[ 3].imag()*twid[8].imag(),vecA[ 3].imag()*twid[8].real()+vecA[ 3].real()*twid[8].imag());
    vecB[ 4] = vecA[ 4] + TT_DATA(vecA[ 6].real()*twid[0].real()-vecA[ 6].imag()*twid[0].imag(),vecA[ 6].imag()*twid[0].real()+vecA[ 6].real()*twid[0].imag());
    vecB[ 6] = vecA[ 4] - TT_DATA(vecA[ 6].real()*twid[0].real()-vecA[ 6].imag()*twid[0].imag(),vecA[ 6].imag()*twid[0].real()+vecA[ 6].real()*twid[0].imag());
    vecB[ 5] = vecA[ 5] + TT_DATA(vecA[ 7].real()*twid[8].real()-vecA[ 7].imag()*twid[8].imag(),vecA[ 7].imag()*twid[8].real()+vecA[ 7].real()*twid[8].imag());
    vecB[ 7] = vecA[ 5] - TT_DATA(vecA[ 7].real()*twid[8].real()-vecA[ 7].imag()*twid[8].imag(),vecA[ 7].imag()*twid[8].real()+vecA[ 7].real()*twid[8].imag());
    vecB[ 8] = vecA[ 8] + TT_DATA(vecA[10].real()*twid[0].real()-vecA[10].imag()*twid[0].imag(),vecA[10].imag()*twid[0].real()+vecA[10].real()*twid[0].imag());
    vecB[10] = vecA[ 8] - TT_DATA(vecA[10].real()*twid[0].real()-vecA[10].imag()*twid[0].imag(),vecA[10].imag()*twid[0].real()+vecA[10].real()*twid[0].imag());
    vecB[ 9] = vecA[ 9] + TT_DATA(vecA[11].real()*twid[8].real()-vecA[11].imag()*twid[8].imag(),vecA[11].imag()*twid[8].real()+vecA[11].real()*twid[8].imag());
    vecB[11] = vecA[ 9] - TT_DATA(vecA[11].real()*twid[8].real()-vecA[11].imag()*twid[8].imag(),vecA[11].imag()*twid[8].real()+vecA[11].real()*twid[8].imag());
    vecB[12] = vecA[12] + TT_DATA(vecA[14].real()*twid[0].real()-vecA[14].imag()*twid[0].imag(),vecA[14].imag()*twid[0].real()+vecA[14].real()*twid[0].imag());
    vecB[14] = vecA[12] - TT_DATA(vecA[14].real()*twid[0].real()-vecA[14].imag()*twid[0].imag(),vecA[14].imag()*twid[0].real()+vecA[14].real()*twid[0].imag());
    vecB[13] = vecA[13] + TT_DATA(vecA[15].real()*twid[8].real()-vecA[15].imag()*twid[8].imag(),vecA[15].imag()*twid[8].real()+vecA[15].real()*twid[8].imag());
    vecB[15] = vecA[13] - TT_DATA(vecA[15].real()*twid[8].real()-vecA[15].imag()*twid[8].imag(),vecA[15].imag()*twid[8].real()+vecA[15].real()*twid[8].imag());

    vecB[16] = vecA[16] + TT_DATA(vecA[18].real()*twid[0].real()-vecA[18].imag()*twid[0].imag(),vecA[18].imag()*twid[0].real()+vecA[18].real()*twid[0].imag());
    vecB[18] = vecA[16] - TT_DATA(vecA[18].real()*twid[0].real()-vecA[18].imag()*twid[0].imag(),vecA[18].imag()*twid[0].real()+vecA[18].real()*twid[0].imag());
    vecB[17] = vecA[17] + TT_DATA(vecA[19].real()*twid[8].real()-vecA[19].imag()*twid[8].imag(),vecA[19].imag()*twid[8].real()+vecA[19].real()*twid[8].imag());
    vecB[19] = vecA[17] - TT_DATA(vecA[19].real()*twid[8].real()-vecA[19].imag()*twid[8].imag(),vecA[19].imag()*twid[8].real()+vecA[19].real()*twid[8].imag());
    vecB[20] = vecA[20] + TT_DATA(vecA[22].real()*twid[0].real()-vecA[22].imag()*twid[0].imag(),vecA[22].imag()*twid[0].real()+vecA[22].real()*twid[0].imag());
    vecB[22] = vecA[20] - TT_DATA(vecA[22].real()*twid[0].real()-vecA[22].imag()*twid[0].imag(),vecA[22].imag()*twid[0].real()+vecA[22].real()*twid[0].imag());
    vecB[21] = vecA[21] + TT_DATA(vecA[23].real()*twid[8].real()-vecA[23].imag()*twid[8].imag(),vecA[23].imag()*twid[8].real()+vecA[23].real()*twid[8].imag());
    vecB[23] = vecA[21] - TT_DATA(vecA[23].real()*twid[8].real()-vecA[23].imag()*twid[8].imag(),vecA[23].imag()*twid[8].real()+vecA[23].real()*twid[8].imag());
    vecB[24] = vecA[24] + TT_DATA(vecA[26].real()*twid[0].real()-vecA[26].imag()*twid[0].imag(),vecA[26].imag()*twid[0].real()+vecA[26].real()*twid[0].imag());
    vecB[26] = vecA[24] - TT_DATA(vecA[26].real()*twid[0].real()-vecA[26].imag()*twid[0].imag(),vecA[26].imag()*twid[0].real()+vecA[26].real()*twid[0].imag());
    vecB[25] = vecA[25] + TT_DATA(vecA[27].real()*twid[8].real()-vecA[27].imag()*twid[8].imag(),vecA[27].imag()*twid[8].real()+vecA[27].real()*twid[8].imag());
    vecB[27] = vecA[25] - TT_DATA(vecA[27].real()*twid[8].real()-vecA[27].imag()*twid[8].imag(),vecA[27].imag()*twid[8].real()+vecA[27].real()*twid[8].imag());
    vecB[28] = vecA[28] + TT_DATA(vecA[30].real()*twid[0].real()-vecA[30].imag()*twid[0].imag(),vecA[30].imag()*twid[0].real()+vecA[30].real()*twid[0].imag());
    vecB[30] = vecA[28] - TT_DATA(vecA[30].real()*twid[0].real()-vecA[30].imag()*twid[0].imag(),vecA[30].imag()*twid[0].real()+vecA[30].real()*twid[0].imag());
    vecB[29] = vecA[29] + TT_DATA(vecA[31].real()*twid[8].real()-vecA[31].imag()*twid[8].imag(),vecA[31].imag()*twid[8].real()+vecA[31].real()*twid[8].imag());
    vecB[31] = vecA[29] - TT_DATA(vecA[31].real()*twid[8].real()-vecA[31].imag()*twid[8].imag(),vecA[31].imag()*twid[8].real()+vecA[31].real()*twid[8].imag());
    } // Stage1

    // stage 2
    TT_IN vecC;
    #pragma HLS BIND_OP variable=vecC op=mul impl=dsp
    Stage2: {
    #pragma HLS latency max=4
    vecC[ 0] = vecB[ 0] + TT_DATA(vecB[ 4].real()*twid[ 0].real()-vecB[ 4].imag()*twid[ 0].imag(),vecB[ 4].imag()*twid[ 0].real()+vecB[ 4].real()*twid[ 0].imag());
    vecC[ 4] = vecB[ 0] - TT_DATA(vecB[ 4].real()*twid[ 0].real()-vecB[ 4].imag()*twid[ 0].imag(),vecB[ 4].imag()*twid[ 0].real()+vecB[ 4].real()*twid[ 0].imag());
    vecC[ 1] = vecB[ 1] + TT_DATA(vecB[ 5].real()*twid[ 4].real()-vecB[ 5].imag()*twid[ 4].imag(),vecB[ 5].imag()*twid[ 4].real()+vecB[ 5].real()*twid[ 4].imag());
    vecC[ 5] = vecB[ 1] - TT_DATA(vecB[ 5].real()*twid[ 4].real()-vecB[ 5].imag()*twid[ 4].imag(),vecB[ 5].imag()*twid[ 4].real()+vecB[ 5].real()*twid[ 4].imag());
    vecC[ 2] = vecB[ 2] + TT_DATA(vecB[ 6].real()*twid[ 8].real()-vecB[ 6].imag()*twid[ 8].imag(),vecB[ 6].imag()*twid[ 8].real()+vecB[ 6].real()*twid[ 8].imag());
    vecC[ 6] = vecB[ 2] - TT_DATA(vecB[ 6].real()*twid[ 8].real()-vecB[ 6].imag()*twid[ 8].imag(),vecB[ 6].imag()*twid[ 8].real()+vecB[ 6].real()*twid[ 8].imag());
    vecC[ 3] = vecB[ 3] + TT_DATA(vecB[ 7].real()*twid[12].real()-vecB[ 7].imag()*twid[12].imag(),vecB[ 7].imag()*twid[12].real()+vecB[ 7].real()*twid[12].imag());
    vecC[ 7] = vecB[ 3] - TT_DATA(vecB[ 7].real()*twid[12].real()-vecB[ 7].imag()*twid[12].imag(),vecB[ 7].imag()*twid[12].real()+vecB[ 7].real()*twid[12].imag());
    vecC[ 8] = vecB[ 8] + TT_DATA(vecB[12].real()*twid[ 0].real()-vecB[12].imag()*twid[ 0].imag(),vecB[12].imag()*twid[ 0].real()+vecB[12].real()*twid[ 0].imag());
    vecC[12] = vecB[ 8] - TT_DATA(vecB[12].real()*twid[ 0].real()-vecB[12].imag()*twid[ 0].imag(),vecB[12].imag()*twid[ 0].real()+vecB[12].real()*twid[ 0].imag());
    vecC[ 9] = vecB[ 9] + TT_DATA(vecB[13].real()*twid[ 4].real()-vecB[13].imag()*twid[ 4].imag(),vecB[13].imag()*twid[ 4].real()+vecB[13].real()*twid[ 4].imag());
    vecC[13] = vecB[ 9] - TT_DATA(vecB[13].real()*twid[ 4].real()-vecB[13].imag()*twid[ 4].imag(),vecB[13].imag()*twid[ 4].real()+vecB[13].real()*twid[ 4].imag());
    vecC[10] = vecB[10] + TT_DATA(vecB[14].real()*twid[ 8].real()-vecB[14].imag()*twid[ 8].imag(),vecB[14].imag()*twid[ 8].real()+vecB[14].real()*twid[ 8].imag());
    vecC[14] = vecB[10] - TT_DATA(vecB[14].real()*twid[ 8].real()-vecB[14].imag()*twid[ 8].imag(),vecB[14].imag()*twid[ 8].real()+vecB[14].real()*twid[ 8].imag());
    vecC[11] = vecB[11] + TT_DATA(vecB[15].real()*twid[12].real()-vecB[15].imag()*twid[12].imag(),vecB[15].imag()*twid[12].real()+vecB[15].real()*twid[12].imag());
    vecC[15] = vecB[11] - TT_DATA(vecB[15].real()*twid[12].real()-vecB[15].imag()*twid[12].imag(),vecB[15].imag()*twid[12].real()+vecB[15].real()*twid[12].imag());
    vecC[16] = vecB[16] + TT_DATA(vecB[20].real()*twid[ 0].real()-vecB[20].imag()*twid[ 0].imag(),vecB[20].imag()*twid[ 0].real()+vecB[20].real()*twid[ 0].imag());
    vecC[20] = vecB[16] - TT_DATA(vecB[20].real()*twid[ 0].real()-vecB[20].imag()*twid[ 0].imag(),vecB[20].imag()*twid[ 0].real()+vecB[20].real()*twid[ 0].imag());
    vecC[17] = vecB[17] + TT_DATA(vecB[21].real()*twid[ 4].real()-vecB[21].imag()*twid[ 4].imag(),vecB[21].imag()*twid[ 4].real()+vecB[21].real()*twid[ 4].imag());
    vecC[21] = vecB[17] - TT_DATA(vecB[21].real()*twid[ 4].real()-vecB[21].imag()*twid[ 4].imag(),vecB[21].imag()*twid[ 4].real()+vecB[21].real()*twid[ 4].imag());
    vecC[18] = vecB[18] + TT_DATA(vecB[22].real()*twid[ 8].real()-vecB[22].imag()*twid[ 8].imag(),vecB[22].imag()*twid[ 8].real()+vecB[22].real()*twid[ 8].imag());
    vecC[22] = vecB[18] - TT_DATA(vecB[22].real()*twid[ 8].real()-vecB[22].imag()*twid[ 8].imag(),vecB[22].imag()*twid[ 8].real()+vecB[22].real()*twid[ 8].imag());
    vecC[19] = vecB[19] + TT_DATA(vecB[23].real()*twid[12].real()-vecB[23].imag()*twid[12].imag(),vecB[23].imag()*twid[12].real()+vecB[23].real()*twid[12].imag());
    vecC[23] = vecB[19] - TT_DATA(vecB[23].real()*twid[12].real()-vecB[23].imag()*twid[12].imag(),vecB[23].imag()*twid[12].real()+vecB[23].real()*twid[12].imag());
    vecC[24] = vecB[24] + TT_DATA(vecB[28].real()*twid[ 0].real()-vecB[28].imag()*twid[ 0].imag(),vecB[28].imag()*twid[ 0].real()+vecB[28].real()*twid[ 0].imag());
    vecC[28] = vecB[24] - TT_DATA(vecB[28].real()*twid[ 0].real()-vecB[28].imag()*twid[ 0].imag(),vecB[28].imag()*twid[ 0].real()+vecB[28].real()*twid[ 0].imag());
    vecC[25] = vecB[25] + TT_DATA(vecB[29].real()*twid[ 4].real()-vecB[29].imag()*twid[ 4].imag(),vecB[29].imag()*twid[ 4].real()+vecB[29].real()*twid[ 4].imag());
    vecC[29] = vecB[25] - TT_DATA(vecB[29].real()*twid[ 4].real()-vecB[29].imag()*twid[ 4].imag(),vecB[29].imag()*twid[ 4].real()+vecB[29].real()*twid[ 4].imag());
    vecC[26] = vecB[26] + TT_DATA(vecB[30].real()*twid[ 8].real()-vecB[30].imag()*twid[ 8].imag(),vecB[30].imag()*twid[ 8].real()+vecB[30].real()*twid[ 8].imag());
    vecC[30] = vecB[26] - TT_DATA(vecB[30].real()*twid[ 8].real()-vecB[30].imag()*twid[ 8].imag(),vecB[30].imag()*twid[ 8].real()+vecB[30].real()*twid[ 8].imag());
    vecC[27] = vecB[27] + TT_DATA(vecB[31].real()*twid[12].real()-vecB[31].imag()*twid[12].imag(),vecB[31].imag()*twid[12].real()+vecB[31].real()*twid[12].imag());
    vecC[31] = vecB[27] - TT_DATA(vecB[31].real()*twid[12].real()-vecB[31].imag()*twid[12].imag(),vecB[31].imag()*twid[12].real()+vecB[31].real()*twid[12].imag());
    } // Stage2

    // stage 3:
    TT_IN vecD;
    #pragma HLS BIND_OP variable=vecD op=mul impl=dsp
    Stage3: {
    #pragma HLS latency max=4
    vecD[ 0] = vecC[ 0] + TT_DATA(vecC[ 8].real()*twid[ 0].real()-vecC[ 8].imag()*twid[ 0].imag(),vecC[ 8].imag()*twid[ 0].real()+vecC[ 8].real()*twid[ 0].imag());
    vecD[ 8] = vecC[ 0] - TT_DATA(vecC[ 8].real()*twid[ 0].real()-vecC[ 8].imag()*twid[ 0].imag(),vecC[ 8].imag()*twid[ 0].real()+vecC[ 8].real()*twid[ 0].imag());
    vecD[ 1] = vecC[ 1] + TT_DATA(vecC[ 9].real()*twid[ 2].real()-vecC[ 9].imag()*twid[ 2].imag(),vecC[ 9].imag()*twid[ 2].real()+vecC[ 9].real()*twid[ 2].imag());
    vecD[ 9] = vecC[ 1] - TT_DATA(vecC[ 9].real()*twid[ 2].real()-vecC[ 9].imag()*twid[ 2].imag(),vecC[ 9].imag()*twid[ 2].real()+vecC[ 9].real()*twid[ 2].imag());
    vecD[ 2] = vecC[ 2] + TT_DATA(vecC[10].real()*twid[ 4].real()-vecC[10].imag()*twid[ 4].imag(),vecC[10].imag()*twid[ 4].real()+vecC[10].real()*twid[ 4].imag());
    vecD[10] = vecC[ 2] - TT_DATA(vecC[10].real()*twid[ 4].real()-vecC[10].imag()*twid[ 4].imag(),vecC[10].imag()*twid[ 4].real()+vecC[10].real()*twid[ 4].imag());
    vecD[ 3] = vecC[ 3] + TT_DATA(vecC[11].real()*twid[ 6].real()-vecC[11].imag()*twid[ 6].imag(),vecC[11].imag()*twid[ 6].real()+vecC[11].real()*twid[ 6].imag());
    vecD[11] = vecC[ 3] - TT_DATA(vecC[11].real()*twid[ 6].real()-vecC[11].imag()*twid[ 6].imag(),vecC[11].imag()*twid[ 6].real()+vecC[11].real()*twid[ 6].imag());
    vecD[ 4] = vecC[ 4] + TT_DATA(vecC[12].real()*twid[ 8].real()-vecC[12].imag()*twid[ 8].imag(),vecC[12].imag()*twid[ 8].real()+vecC[12].real()*twid[ 8].imag());
    vecD[12] = vecC[ 4] - TT_DATA(vecC[12].real()*twid[ 8].real()-vecC[12].imag()*twid[ 8].imag(),vecC[12].imag()*twid[ 8].real()+vecC[12].real()*twid[ 8].imag());
    vecD[ 5] = vecC[ 5] + TT_DATA(vecC[13].real()*twid[10].real()-vecC[13].imag()*twid[10].imag(),vecC[13].imag()*twid[10].real()+vecC[13].real()*twid[10].imag());
    vecD[13] = vecC[ 5] - TT_DATA(vecC[13].real()*twid[10].real()-vecC[13].imag()*twid[10].imag(),vecC[13].imag()*twid[10].real()+vecC[13].real()*twid[10].imag());
    vecD[ 6] = vecC[ 6] + TT_DATA(vecC[14].real()*twid[12].real()-vecC[14].imag()*twid[12].imag(),vecC[14].imag()*twid[12].real()+vecC[14].real()*twid[12].imag());
    vecD[14] = vecC[ 6] - TT_DATA(vecC[14].real()*twid[12].real()-vecC[14].imag()*twid[12].imag(),vecC[14].imag()*twid[12].real()+vecC[14].real()*twid[12].imag());
    vecD[ 7] = vecC[ 7] + TT_DATA(vecC[15].real()*twid[14].real()-vecC[15].imag()*twid[14].imag(),vecC[15].imag()*twid[14].real()+vecC[15].real()*twid[14].imag());
    vecD[15] = vecC[ 7] - TT_DATA(vecC[15].real()*twid[14].real()-vecC[15].imag()*twid[14].imag(),vecC[15].imag()*twid[14].real()+vecC[15].real()*twid[14].imag());

    vecD[16] = vecC[16] + TT_DATA(vecC[24].real()*twid[ 0].real()-vecC[24].imag()*twid[ 0].imag(),vecC[24].imag()*twid[ 0].real()+vecC[24].real()*twid[ 0].imag());
    vecD[24] = vecC[16] - TT_DATA(vecC[24].real()*twid[ 0].real()-vecC[24].imag()*twid[ 0].imag(),vecC[24].imag()*twid[ 0].real()+vecC[24].real()*twid[ 0].imag());
    vecD[17] = vecC[17] + TT_DATA(vecC[25].real()*twid[ 2].real()-vecC[25].imag()*twid[ 2].imag(),vecC[25].imag()*twid[ 2].real()+vecC[25].real()*twid[ 2].imag());
    vecD[25] = vecC[17] - TT_DATA(vecC[25].real()*twid[ 2].real()-vecC[25].imag()*twid[ 2].imag(),vecC[25].imag()*twid[ 2].real()+vecC[25].real()*twid[ 2].imag());
    vecD[18] = vecC[18] + TT_DATA(vecC[26].real()*twid[ 4].real()-vecC[26].imag()*twid[ 4].imag(),vecC[26].imag()*twid[ 4].real()+vecC[26].real()*twid[ 4].imag());
    vecD[26] = vecC[18] - TT_DATA(vecC[26].real()*twid[ 4].real()-vecC[26].imag()*twid[ 4].imag(),vecC[26].imag()*twid[ 4].real()+vecC[26].real()*twid[ 4].imag());
    vecD[19] = vecC[19] + TT_DATA(vecC[27].real()*twid[ 6].real()-vecC[27].imag()*twid[ 6].imag(),vecC[27].imag()*twid[ 6].real()+vecC[27].real()*twid[ 6].imag());
    vecD[27] = vecC[19] - TT_DATA(vecC[27].real()*twid[ 6].real()-vecC[27].imag()*twid[ 6].imag(),vecC[27].imag()*twid[ 6].real()+vecC[27].real()*twid[ 6].imag());
    vecD[20] = vecC[20] + TT_DATA(vecC[28].real()*twid[ 8].real()-vecC[28].imag()*twid[ 8].imag(),vecC[28].imag()*twid[ 8].real()+vecC[28].real()*twid[ 8].imag());
    vecD[28] = vecC[20] - TT_DATA(vecC[28].real()*twid[ 8].real()-vecC[28].imag()*twid[ 8].imag(),vecC[28].imag()*twid[ 8].real()+vecC[28].real()*twid[ 8].imag());
    vecD[21] = vecC[21] + TT_DATA(vecC[29].real()*twid[10].real()-vecC[29].imag()*twid[10].imag(),vecC[29].imag()*twid[10].real()+vecC[29].real()*twid[10].imag());
    vecD[29] = vecC[21] - TT_DATA(vecC[29].real()*twid[10].real()-vecC[29].imag()*twid[10].imag(),vecC[29].imag()*twid[10].real()+vecC[29].real()*twid[10].imag());
    vecD[22] = vecC[22] + TT_DATA(vecC[30].real()*twid[12].real()-vecC[30].imag()*twid[12].imag(),vecC[30].imag()*twid[12].real()+vecC[30].real()*twid[12].imag());
    vecD[30] = vecC[22] - TT_DATA(vecC[30].real()*twid[12].real()-vecC[30].imag()*twid[12].imag(),vecC[30].imag()*twid[12].real()+vecC[30].real()*twid[12].imag());
    vecD[23] = vecC[23] + TT_DATA(vecC[31].real()*twid[14].real()-vecC[31].imag()*twid[14].imag(),vecC[31].imag()*twid[14].real()+vecC[31].real()*twid[14].imag());
    vecD[31] = vecC[23] - TT_DATA(vecC[31].real()*twid[14].real()-vecC[31].imag()*twid[14].imag(),vecC[31].imag()*twid[14].real()+vecC[31].real()*twid[14].imag());
    } // Stage3

    // stage 4: (final)
    TT_IN vecOut;
    #pragma HLS BIND_OP variable=vecOut op=mul impl=dsp
    Stage4: {
    #pragma HLS latency max=4
    vecOut[ 0] = vecD[ 0] + TT_DATA(vecD[16].real()*twid[ 0].real()-vecD[16].imag()*twid[ 0].imag(),vecD[16].imag()*twid[ 0].real()+vecD[16].real()*twid[ 0].imag());
    vecOut[16] = vecD[ 0] - TT_DATA(vecD[16].real()*twid[ 0].real()-vecD[16].imag()*twid[ 0].imag(),vecD[16].imag()*twid[ 0].real()+vecD[16].real()*twid[ 0].imag());
    vecOut[ 1] = vecD[ 1] + TT_DATA(vecD[17].real()*twid[ 1].real()-vecD[17].imag()*twid[ 1].imag(),vecD[17].imag()*twid[ 1].real()+vecD[17].real()*twid[ 1].imag());
    vecOut[17] = vecD[ 1] - TT_DATA(vecD[17].real()*twid[ 1].real()-vecD[17].imag()*twid[ 1].imag(),vecD[17].imag()*twid[ 1].real()+vecD[17].real()*twid[ 1].imag());
    vecOut[ 2] = vecD[ 2] + TT_DATA(vecD[18].real()*twid[ 2].real()-vecD[18].imag()*twid[ 2].imag(),vecD[18].imag()*twid[ 2].real()+vecD[18].real()*twid[ 2].imag());
    vecOut[18] = vecD[ 2] - TT_DATA(vecD[18].real()*twid[ 2].real()-vecD[18].imag()*twid[ 2].imag(),vecD[18].imag()*twid[ 2].real()+vecD[18].real()*twid[ 2].imag());
    vecOut[ 3] = vecD[ 3] + TT_DATA(vecD[19].real()*twid[ 3].real()-vecD[19].imag()*twid[ 3].imag(),vecD[19].imag()*twid[ 3].real()+vecD[19].real()*twid[ 3].imag());
    vecOut[19] = vecD[ 3] - TT_DATA(vecD[19].real()*twid[ 3].real()-vecD[19].imag()*twid[ 3].imag(),vecD[19].imag()*twid[ 3].real()+vecD[19].real()*twid[ 3].imag());
    vecOut[ 4] = vecD[ 4] + TT_DATA(vecD[20].real()*twid[ 4].real()-vecD[20].imag()*twid[ 4].imag(),vecD[20].imag()*twid[ 4].real()+vecD[20].real()*twid[ 4].imag());
    vecOut[20] = vecD[ 4] - TT_DATA(vecD[20].real()*twid[ 4].real()-vecD[20].imag()*twid[ 4].imag(),vecD[20].imag()*twid[ 4].real()+vecD[20].real()*twid[ 4].imag());
    vecOut[ 5] = vecD[ 5] + TT_DATA(vecD[21].real()*twid[ 5].real()-vecD[21].imag()*twid[ 5].imag(),vecD[21].imag()*twid[ 5].real()+vecD[21].real()*twid[ 5].imag());
    vecOut[21] = vecD[ 5] - TT_DATA(vecD[21].real()*twid[ 5].real()-vecD[21].imag()*twid[ 5].imag(),vecD[21].imag()*twid[ 5].real()+vecD[21].real()*twid[ 5].imag());
    vecOut[ 6] = vecD[ 6] + TT_DATA(vecD[22].real()*twid[ 6].real()-vecD[22].imag()*twid[ 6].imag(),vecD[22].imag()*twid[ 6].real()+vecD[22].real()*twid[ 6].imag());
    vecOut[22] = vecD[ 6] - TT_DATA(vecD[22].real()*twid[ 6].real()-vecD[22].imag()*twid[ 6].imag(),vecD[22].imag()*twid[ 6].real()+vecD[22].real()*twid[ 6].imag());
    vecOut[ 7] = vecD[ 7] + TT_DATA(vecD[23].real()*twid[ 7].real()-vecD[23].imag()*twid[ 7].imag(),vecD[23].imag()*twid[ 7].real()+vecD[23].real()*twid[ 7].imag());
    vecOut[23] = vecD[ 7] - TT_DATA(vecD[23].real()*twid[ 7].real()-vecD[23].imag()*twid[ 7].imag(),vecD[23].imag()*twid[ 7].real()+vecD[23].real()*twid[ 7].imag());

    vecOut[ 8] = vecD[ 8] + TT_DATA(vecD[24].real()*twid[ 8].real()-vecD[24].imag()*twid[ 8].imag(),vecD[24].imag()*twid[ 8].real()+vecD[24].real()*twid[ 8].imag());
    vecOut[24] = vecD[ 8] - TT_DATA(vecD[24].real()*twid[ 8].real()-vecD[24].imag()*twid[ 8].imag(),vecD[24].imag()*twid[ 8].real()+vecD[24].real()*twid[ 8].imag());
    vecOut[ 9] = vecD[ 9] + TT_DATA(vecD[25].real()*twid[ 9].real()-vecD[25].imag()*twid[ 9].imag(),vecD[25].imag()*twid[ 9].real()+vecD[25].real()*twid[ 9].imag());
    vecOut[25] = vecD[ 9] - TT_DATA(vecD[25].real()*twid[ 9].real()-vecD[25].imag()*twid[ 9].imag(),vecD[25].imag()*twid[ 9].real()+vecD[25].real()*twid[ 9].imag());
    vecOut[10] = vecD[10] + TT_DATA(vecD[26].real()*twid[10].real()-vecD[26].imag()*twid[10].imag(),vecD[26].imag()*twid[10].real()+vecD[26].real()*twid[10].imag());
    vecOut[26] = vecD[10] - TT_DATA(vecD[26].real()*twid[10].real()-vecD[26].imag()*twid[10].imag(),vecD[26].imag()*twid[10].real()+vecD[26].real()*twid[10].imag());
    vecOut[11] = vecD[11] + TT_DATA(vecD[27].real()*twid[11].real()-vecD[27].imag()*twid[11].imag(),vecD[27].imag()*twid[11].real()+vecD[27].real()*twid[11].imag());
    vecOut[27] = vecD[11] - TT_DATA(vecD[27].real()*twid[11].real()-vecD[27].imag()*twid[11].imag(),vecD[27].imag()*twid[11].real()+vecD[27].real()*twid[11].imag());
    vecOut[12] = vecD[12] + TT_DATA(vecD[28].real()*twid[12].real()-vecD[28].imag()*twid[12].imag(),vecD[28].imag()*twid[12].real()+vecD[28].real()*twid[12].imag());
    vecOut[28] = vecD[12] - TT_DATA(vecD[28].real()*twid[12].real()-vecD[28].imag()*twid[12].imag(),vecD[28].imag()*twid[12].real()+vecD[28].real()*twid[12].imag());
    vecOut[13] = vecD[13] + TT_DATA(vecD[29].real()*twid[13].real()-vecD[29].imag()*twid[13].imag(),vecD[29].imag()*twid[13].real()+vecD[29].real()*twid[13].imag());
    vecOut[29] = vecD[13] - TT_DATA(vecD[29].real()*twid[13].real()-vecD[29].imag()*twid[13].imag(),vecD[29].imag()*twid[13].real()+vecD[29].real()*twid[13].imag());
    vecOut[14] = vecD[14] + TT_DATA(vecD[30].real()*twid[14].real()-vecD[30].imag()*twid[14].imag(),vecD[30].imag()*twid[14].real()+vecD[30].real()*twid[14].imag());
    vecOut[30] = vecD[14] - TT_DATA(vecD[30].real()*twid[14].real()-vecD[30].imag()*twid[14].imag(),vecD[30].imag()*twid[14].real()+vecD[30].real()*twid[14].imag());
    vecOut[15] = vecD[15] + TT_DATA(vecD[31].real()*twid[15].real()-vecD[31].imag()*twid[15].imag(),vecD[31].imag()*twid[15].real()+vecD[31].real()*twid[15].imag());
    vecOut[31] = vecD[15] - TT_DATA(vecD[31].real()*twid[15].real()-vecD[31].imag()*twid[15].imag(),vecD[31].imag()*twid[15].real()+vecD[31].real()*twid[15].imag());
    } // Stage4

    ////////////////////////////////////////////////////////////////////////////
    // store result
    ////////////////////////////////////////////////////////////////////////////
    TT_OUT vecOut_strm; 
    //TT_IN vecOutFull_strm; 
    for (int i=0; i<FFT_LEN; ++i ) {
        vecOut_strm[i]     = (TT_ODATA)vecOut[i]; // scale down by TT_ODATA
        //vecOutFull_strm[i] = vecOut[i]; // full scale
    } // for i
#ifdef DEBUG
     cout << "vecA = ";
     for (int i=0; i<FFT_LEN; ++i) cout << vecA[i];
     cout << endl << "vecB = ";
     for (int i=0; i<FFT_LEN; ++i) cout << vecB[i];
     cout << endl << "vecC = ";
     for (int i=0; i<FFT_LEN; ++i) cout << vecC[i];
     cout << endl << "vecD = ";
     for (int i=0; i<FFT_LEN; ++i) cout << vecD[i];
     cout << endl << "vecOut = ";
     for (int i=0; i<FFT_LEN; ++i) cout << vecOut[i];
     cout << endl << "vecOut_strm = ";
     for (int i=0; i<FFT_LEN; ++i) cout << vecOut_strm[i];
     cout << endl;
#endif

    out_strm.write(vecOut_strm); // scale down
    //full_out_strm.write(vecOutFull_strm); // full scale
} // fft_top
