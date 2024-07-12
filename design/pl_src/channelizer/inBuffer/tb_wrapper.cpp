//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Jaemin Park

#include <iostream>
#include <fstream>
#include <string>
#include "inBuffer.h"

using namespace std;

extern void inBuffer_wrapper(TT_STRM_I sig_i[N_STRM_I], TT_STRM_O sig_o[N_STRM_O]);

///////////////////////////////////////////////////////////////
// Main
///////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    // sim config parameters
    const string inDataFile  = "cubeSamples.txt";
    const string outDataFile   ("sig_o.txt");      // output data
    const string goldDataFile  ("sig_goldC_.txt"); // count-based expected data
    const string goldDataFile2 ("sig_goldF_.txt"); // model-based expected data

    // command-line options
    char data_mode = 'f'; // file data unless defined otherwise by command-line
    int NSAMP = 128*4/SSR_I; // default for 'c' mode
    constexpr int SZ_inDataFile = 64000; // size of inDataFile in lines

    switch (argc) {
        case 1: data_mode = 'f'; NSAMP *= 2; break;
        case 2: data_mode = *argv[1]; if (data_mode == 'f') NSAMP = SZ_inDataFile/SSR_I; break;
        default: cerr << "sorry: illegal command-line format: " << argc << endl; exit(-1);
    } // switch
    cout << "data_mode: " << data_mode << endl;

    // open data input file
    ifstream sig_ifile; 

    if (data_mode == 'f'){
        sig_ifile.open(inDataFile);
        if (!sig_ifile.is_open()) cerr << "can't open file " << inDataFile << endl;
    } // if data_mode == 'f'

    // open output files
    ofstream sig_ofile [N_STRM_O];

    for (int t=0; t < N_STRM_O; ++t ) { // t: tile
        string fname = outDataFile;
        fname.insert(5/*pos*/,to_string(t));
        cout << fname << endl;
        sig_ofile[t].open(fname);
        if (!sig_ofile[t].is_open()) cerr << "can't open file " << fname << endl;
    } // for t

    TT_STRM_I sig_i[N_STRM_I];
    TT_STRM_O sig_o[N_STRM_O];
   
    //////////////////////////////////////////////////////////////////////////
    // main test loop
    //////////////////////////////////////////////////////////////////////////
    cout << "NSAMP: " << NSAMP << endl;
 for(int loop=0; loop<2; loop++) {
    for (int cc=0; cc < NSAMP; ++cc ) { // cc: clock cycle
#ifdef DEBUG
        cout << endl << "*** cc: " << dec << cc << " ***" << endl;
#endif
        /////////////////////////////////////
        // input data gen
        /////////////////////////////////////
        TT_IN val_i;
        switch (data_mode) {
            case 'c': // count data
                for (int i=0; i < SSR_I; ++i ) val_i[i] = c_din_t(SSR_I*cc+i,0); // I (sequential data), Q (0)
                break;
            case 'f': // from input data file
                for (int j=0; j < SSR_I; ++j ) {
                    din_t val_re, val_im; // I/Q part of the cint sample
                    sig_ifile >> val_re >> val_im;
                    val_i[j] = c_din_t(val_re,val_im);
                } // for j
                break;
            default: cerr << "unrecognizable data_mode " << data_mode << endl;
        } // switch data_mode
#ifdef DEBUG
        cout << "val_i[" << cc << "]=";
        for (int j=SSR_I-1; 0 <= j; --j ) cout << val_i[j];
        cout << endl;
#endif
        
        // send thru the stream:
        sig_i[0].write(val_i);
    }
        /////////////////////////////////////
        // run DUT:
        /////////////////////////////////////
        inBuffer_wrapper( sig_i, sig_o );
  

    for (int cc=0; cc < NSAMP; ++cc ) { // cc: clock cycle
        /////////////////////////////////////
        // store result:
        /////////////////////////////////////
        //if ( 8 <= cc) { // dump first 8 zero output cycles
            for (int t = 0; t < N_STRM_O; ++t) { // t: tile
                TT_OUT val_o = sig_o[t].read();
      
                for (int s=SSR_O-1; 0 <= s; --s ) { // s: sample pos
                    sig_ofile[t] << val_o[s].real() << ' ' << val_o[s].imag() << ' ';
                } // for s
                sig_ofile[t] << endl;
#ifdef DEBUG
                cout << "val_o[" << t << "]=";
                for (int s=SSR_O-1; 0 <= s; --s ) { // s: sample pos
                    cout << val_o[s]; 
                } // for s
                cout << endl;
#endif
            } // for t
        //} // if 8 <= cc
    } // for cc
 }   
    // close opened files
    sig_ifile.close();
    for (int t=0; t < N_STRM_O; ++t ){ // t: tile
        sig_ofile[t].close(); 
    }

    /////////////////////////////////////
    // compare the results
    /////////////////////////////////////
    int retval = 0;
    for (int t=0; t < N_STRM_O; ++t ) { // t tile
        const string t_str = to_string(t);
        string fname = outDataFile; fname.insert(5/*pos*/,t_str);
        string fname2;
        switch (data_mode) {
            case 'c': fname2 = goldDataFile;  break;
            case 'f': fname2 = goldDataFile2; break;
            default: ; 
        } // switch
        fname2.insert(10/*pos*/,t_str);
        const string cmd_str = "diff -U 4000 --brief -w "+fname+" "+fname2; // -w: ignore wspaces
        const char* cmd = cmd_str.c_str(); // required arg type for system()
        cout << cmd << endl;
        retval += system(cmd);
    } // for t

    if (retval == 0) cout << "--- PASSED ---" << endl;
    else             cout << "*** FAILED ***" << endl;
    return(retval);
} // main
