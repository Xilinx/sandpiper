//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Jaemin Park

#include <iostream>
#include <fstream>
#include <string>
#include "chanSelector.h"

using namespace std;

extern void chanSelector_wrapper(TT_STRM_I sig_i[N_STRM_I], TT_STRM_O sig_o[N_STRM_O], const sel_t sel);

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------
int main(int argc, char* argv[])
{
    // sanity check before running:
    assert(N_STRM_I == 1 && N_STRM_O == 1);

    // sim config parameters
    const int NSAMP = 4016; // num of lines in fftOut.txt
    const string inDataFile  = "fftOut.txt";
    // note: mutable string object below;
    const string outDataFile  ("sig_o.txt");    // output data
    const string goldDataFile ("sig_gold.txt"); // expected data

    // open data input file
    ifstream sig_ifile; 
    sig_ifile.open(inDataFile);
    if (!sig_ifile.is_open()) cerr << "can't open file " << inDataFile << endl;

    // open output files
    ofstream sig_ofile;
    sig_ofile.open(outDataFile);
    if (!sig_ofile.is_open()) cerr << "can't open file " << outDataFile << endl;

    TT_STRM_I sig_i[N_STRM_I];
    TT_STRM_O sig_o[N_STRM_O];
    sel_t  sel = 0; // default for selector
  
    //////////////////////////////////////////////////////////////////////////
    // main test loop
    //////////////////////////////////////////////////////////////////////////

    for (int cc=0; cc < NSAMP; ++cc ) { // cc: clock cycle
#ifdef DEBUG 
        cout << endl << "*** cc: " << dec << cc << " ***" << endl;
        if (cc % CUBE_BNDRY_CC == 0)
           cout << "new cube boundary!" << endl;
#endif
        if (cc % CUBE_BNDRY_CC == CUBE_BNDRY_CC - 15) { // to emulate channel section chane
           // note: latency of sel is 12 cc
           sel += 1; 
#ifdef DEBUG 
           cout << "new sel: " << sel << endl;
#endif
        }

        /////////////////////////////////////
        // input data gen
        /////////////////////////////////////
        TT_IN val_i;
        for (int j=0; j < SSR_I; ++j ) {
            din_t val_re, val_im; // I/Q part of cint sample
            sig_ifile >> val_re >> val_im;
            val_i[j] = c_din_t(val_re,val_im);
        } // for j
#ifdef DEBUG 
        cout << "val_i[" << cc << "]=";
        for (int j=SSR_I-1; 0 <= j; --j ) cout << val_i[j];
        cout << endl;
#endif
        // send thru the stream:
        sig_i[0].write(val_i);
  
        /////////////////////////////////////
        // run DUT:
        /////////////////////////////////////
        chanSelector_wrapper( sig_i, sig_o, sel );
  
        /////////////////////////////////////
        // store result:
        /////////////////////////////////////
        TT_OUT val_o = sig_o[0].read();
        for (int s=SSR_O-1; 0 <= s; --s ) { // s: sample pos
            sig_ofile << val_o[s].real() << ' ' << val_o[s].imag() << ' ';
        } // for s
        sig_ofile << endl;
#ifdef DEBUG 
        cout << "sel=" << sel << " val_o=";
        for (int s=SSR_O-1; 0 <= s; --s ) { // s: sample pos
            cout << val_o[s]; 
        } // for s
        cout << endl;
#endif
    } // for cc
    
    // close opened files
    sig_ifile.close();
    sig_ofile.close(); 

    /////////////////////////////////////
    // compare the results
    /////////////////////////////////////
    const string cmd_str = "diff --brief -w "+outDataFile+" "+goldDataFile; // -w: ignore wspaces
    const char* cmd = cmd_str.c_str(); // required arg type for system()
    cout << cmd << endl;

    int retval = 0;
    retval += system(cmd);

    if (retval == 0) cout << "--- PASSED ---" << endl;
    else             cout << "*** FAILED ***" << endl;
    return(retval);
} // main
