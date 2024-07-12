//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Jaemin Park

#include <iostream>
#include <fstream>
#include <string>
#include "fft_top.h"

int main(int argc, char* argv[])
{
    /////////////////////////////////////
    // parameters for sim
    /////////////////////////////////////
    const int SIM_FRAMES_C = 1;    // number of data frame for data_mode 'c'
    const int SIM_FRAMES_F = 4016; // = number of lines in inDataFile for data mode 'f'
    const string inDataFile        = "FftInput.txt";
    const string outDataFile       = "sig_o_16_15.txt";     // output data in scale-down as TT_ODATA
    const string goldDataFileF     = "fftOutput_16_15.txt"; // expected data for data_mode 'f' in TT_DATA
    const string outDataFileFull   = "sig_o_27_25.txt";     // output data in full scaled as TT_DATA
    const string goldDataFileFFull = "fftOutput_27_25.txt"; // expected data for data_mode 'f' in TT_DATA
    const string goldDataFileC     = "fftGoldC.txt";        // expected data for data_mode 'c' in TT_DATA
    // command-line options
    char data_mode = 'f'; // file data unless defined otherwise by command-line

    switch (argc) {
        case 1: data_mode = 'f'; break;
        case 2: data_mode = *argv[1]; break;
        default:
            cerr << "Illegal command-line format: " << argc << endl;
            exit(-1);
    } // switch

    /////////////////////////////////////
    // data input/output files
    /////////////////////////////////////
    ifstream inFile; 
    int sim_frames = SIM_FRAMES_C;

    if (data_mode == 'f'){
        sim_frames = SIM_FRAMES_F;
        inFile.open(inDataFile);
        if (inFile.is_open() == 0) cerr << "can't open file " << inDataFile << endl;
    } // if data_mode == 'f'

    // output file
    ofstream outFile; // scale down output file
    outFile.open(outDataFile);
    if (outFile.is_open() == 0) cerr << "can't open file " << outDataFile << endl;

    ofstream outFileFull; // full scale output file
    outFileFull.open(outDataFileFull);
    if (outFileFull.is_open() == 0) cerr << "can't open file " << outDataFileFull << endl;

    /////////////////////////////////////
    // 
    /////////////////////////////////////
    cout << "data_mode: " << data_mode << endl;
    TT_IN_STRM  fft_in_strm, fft_out_full_strm;
    TT_OUT_STRM fft_out_strm;

    // know what the current AP data type resolutions are:
    cout << "2^-" << fftStageFracLen << " = " << dec << din_t (pow(2,-fftStageFracLen)) << endl;
    cout << "2^-" << fftTwidFracLen  << " = " << dec << twid_t(pow(2,-fftTwidFracLen))  << endl;
    cout << "2^-" << fftOutFracLen   << " = " << dec << dout_t(pow(2,-fftOutFracLen))   << endl;
    cout << "din_t(1)  = " << dec << din_t(1)  << endl;
    cout << "twid_t(1) = " << dec << twid_t(1) << endl;
    cout << "dout_t(1) = " << dec << dout_t(1) << endl;

    ////////////////////////////////////////////////////////////////////
    // main loop
    ////////////////////////////////////////////////////////////////////
    for (int frame = 0; frame < sim_frames; ++frame) {
        cout << endl << "*** cc: " << dec << frame << " ***" << endl;
        /////////////////////////////////////
        // input data gen
        /////////////////////////////////////
        TT_IN vect_in;

        if (data_mode == 'c'){ // count data
            for (int j=0; j<FFT_LEN; ++j){ // aggregating the data into a vector
                vect_in[j] = TT_DATA(din_t((frame*FFT_LEN+j) * pow(2,-fftStageFracLen)),
                                     din_t((frame*FFT_LEN+j) * pow(2,-fftStageFracLen))); 
            } // for j
        } else {// if data_mode == 'f'
            for (int j=0; j<FFT_LEN; ++j){ // aggregating the data into a vector
                int in_re, in_im;
                inFile >> in_re >> in_im;
                vect_in[j] = TT_DATA(din_t(in_re * pow(2,-fftStageFracLen)),
                                     din_t(in_im * pow(2,-fftStageFracLen)));
            } // for j
        }
#ifdef DEBUG
        cout << "vect_in="; 
        for(int j=0; j < FFT_LEN; ++j){
            cout << vect_in[j];
        } // for j
        cout << endl;
#endif
        fft_in_strm.write(vect_in);    

        /////////////////////////////////////
        // run DUT:
        /////////////////////////////////////
        fft_top(fft_in_strm, fft_out_strm );
  
        /////////////////////////////////////
        // store result:
        /////////////////////////////////////
        //TT_IN  vect_out_full = fft_out_full_strm.read();

//        for (int j=0; j < FFT_LEN; ++j){ // full-scale output
//            TT_CONV_FULL re_full, im_full;
//            re_full = vect_out_full[j].real() * TT_CONV_FULL(pow(2,fftStageFracLen));
//            im_full = vect_out_full[j].imag() * TT_CONV_FULL(pow(2,fftStageFracLen));
//            outFileFull << dec << re_full << ' ' << im_full << ' ';
//        } // for j
//        outFileFull << endl; // new line
//#ifdef DEBUG
//        cout << "out_full[" << frame << "]="; 
//        for (int j=0; j < FFT_LEN; ++j){ // full-scale output
//            TT_CONV_FULL re_full, im_full;
//            re_full = vect_out_full[j].real() * TT_CONV_FULL(pow(2,fftStageFracLen));
//            im_full = vect_out_full[j].imag() * TT_CONV_FULL(pow(2,fftStageFracLen));
//            cout << dec << re_full << ' ' << im_full << ' ';
//        } // for j
//        cout << endl;
//#endif
        TT_OUT vect_out = fft_out_strm.read();
        for (int j=0; j < FFT_LEN; ++j){ // scaled-down output
            TT_CONV re, im;
            re = vect_out[j].real() * TT_CONV(pow(2,fftOutFracLen));
            im = vect_out[j].imag() * TT_CONV(pow(2,fftOutFracLen));
            outFile << dec << re << ' ' << im << ' ';
        } // for j
        outFile << endl; // new line
#ifdef DEBUG
        cout << "out[" << frame << "]="; 
        for (int j=0; j < FFT_LEN; ++j){ // scaled-down output
            TT_CONV re, im;
            re = vect_out[j].real() * TT_CONV(pow(2,fftOutFracLen));
            im = vect_out[j].imag() * TT_CONV(pow(2,fftOutFracLen));
            cout << dec << re << ' ' << im << ' ';
        } // for j
        cout << endl;
#endif
    } // for frame
    
    // close opened files
    inFile.close();
    outFile.close(); 
    outFileFull.close();

    /////////////////////////////////////
    // compare the results
    /////////////////////////////////////
    int retval = 0;

    const string goldDataFile = data_mode == 'f' ? goldDataFileF : goldDataFileC; // file selected by data_mode
    const string str = "diff --brief -w "+outDataFile+" "+goldDataFile; // -w: ignore wspaces
    const char* cmd = str.c_str(); // required arg type for system()
    cout << cmd << endl;
    retval += system(cmd);

    if (data_mode == 'f') {
        const string str = "diff --brief -w "+outDataFileFull+" "+goldDataFileFFull; // -w: ignore wspaces
        const char* cmd = str.c_str(); // required arg type for system()
        cout << cmd << endl;
        retval += system(cmd);
    }

    if (retval == 0) cout << "--- PASSED ---" << endl;
    else             cout << "*** FAILED ***" << endl;
    return(retval);
} // main

