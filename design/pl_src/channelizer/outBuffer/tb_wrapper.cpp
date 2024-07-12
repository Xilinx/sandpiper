//
// Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Author: Jaemin Park

#include <iostream>
#include <fstream>
#include <string>
#include <cctype> // toupper()
#include <cstdio> // sprintf()
#include "outBuffer.h"

using namespace std;

// Use this to convert samples from I/O files:
typedef c_din_t TT; 

extern void outBuffer_wrapper(TT_STRM_I sig_i[N_STRM_I], TT_STRM_O sig_o[N_STRM_O]);

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------
int main(int argc, char* argv[])
{
    // sim config parameters:
    const string inDataFile ("tileOutput.txt");
    const string inGen      ("tileIndex.txt");
    const string outDataFile  = "sigOut.txt";  // output data 
    const string goldDataFile = "FftInput.txt"; // expected data file
    unsigned NSAMP = 4000; // lines of input files

    // command-line options:
    char data_mode = 'f'; // file data unless defined otherwise by command-line

    switch (argc) {
        case 1: data_mode = 'f'; break;
        case 2: data_mode = *argv[1]; break;
        default: cerr << "sorry: illegal command-line format: " << argc << endl; exit(-1);
    } // switch
    cout << "data_mode: " << data_mode << endl;
    char data_mode_str[2];
    sprintf(data_mode_str,"%c",toupper(data_mode)); // to_string(char) unsupported

    // open data input files:
    ifstream sig_ifile[N_STRM_I]; 

    if (data_mode == 'f'){
        cout << "Input files:";
        for (int t=0; t < N_STRM_I; ++t){ // t: tile
            string fname = inDataFile; fname.insert(4/*pos*/,to_string(t));
            cout << ' ' << fname;
            sig_ifile[t].open(fname);
            if (sig_ifile[t].is_open() == 0) cerr << "can't open file " << fname << endl;
        } // for i
        cout << endl;
    } // if data_mode == 'f'

    // open output files:
    ofstream sig_ofile [N_STRM_O];
    string fname; // this will be used again in result comparison below
    for (int f=0; f < N_STRM_O; ++f){ // f: fft
        fname = outDataFile; fname.insert(6/*pos*/,data_mode_str);
        cout << fname << endl;
        sig_ofile[f].open(fname);
        if (sig_ofile[f].is_open() == 0) cerr << "can't open file " << fname << endl;
    } // for i

    // I/O streams:
    TT_STRM_I sig_i[N_STRM_I];
    TT_STRM_O sig_o[N_STRM_O];
  
    // data gen init part for data mode 'c':
    TT_IN val_i[N_STRM_I] = {};
    din_t tg_base[N_TG][2]; // tg_base[TG][even|odd]
    if (data_mode == 'c') {
        NSAMP = 8*2*2;
        // fix Q to represent tile id
        for (int tg=0; tg < N_TG; tg++) // tg: tile group
            for (int t=0; t < TG_SIZE; ++t) { // t: tile within a tg
                const int t_idx = TG_SIZE * tg + t; // tile index
                val_i[t_idx][0].imag(t_idx); 
                val_i[t_idx][1].imag(t_idx); 
            } // i
        tg_base[0][0] =  0; tg_base[0][1] = 64;
        tg_base[1][0] =  0; tg_base[1][1] = 48;
        tg_base[2][0] = 16; tg_base[2][1] = 80;
        tg_base[3][0] =  0; tg_base[3][1] = 64;
    } // if data mode 'c'

    //////////////////////////////////////////////////////////////////////////
    // main test loop
    //////////////////////////////////////////////////////////////////////////
 for(int loop=0; loop<2; loop++) {
    for (int cc=0; cc < NSAMP; cc++) { // clock cycle cc
        cout << endl << "*** cc: " << dec << cc << " ***" << endl;
        /////////////////////////////////////
        // input data gen
        /////////////////////////////////////
        if (data_mode == 'c') { // count data: real(sequential pattern), imag(tile id)
            const uint1 cc_div2 = cc % 2;
            // tile group 0 (buff0):
            val_i[3][0].real(tg_base[0][cc_div2]++); val_i[3][1].real(val_i[3][0].real()+32);
            val_i[2][0].real(val_i[3][0].real()+4 ); val_i[2][1].real(val_i[3][1].real()+ 4);
            val_i[1][0].real(val_i[2][0].real()+4 ); val_i[1][1].real(val_i[2][1].real()+ 4);
            val_i[0][0].real(val_i[1][0].real()+4 ); val_i[0][1].real(val_i[1][1].real()+ 4);
            // tg1 (buff1):
            din_t adden;
            if (cc_div2 == 0) { // even tick
                val_i[7][0].real(0); val_i[7][1].real(tg_base[1][0] +16); tg_base[1][0]++;
                adden = 0; // keep this column all 0
            } else { // odd tick
                val_i[7][0].real(tg_base[1][1]++); val_i[7][1].real(val_i[7][0].real()+32);
                adden = 4; // inc this column as other colunms
            }
            val_i[6][0].real(val_i[7][0].real()+adden); val_i[6][1].real(val_i[7][1].real()+ 4);
            val_i[5][0].real(val_i[6][0].real()+adden); val_i[5][1].real(val_i[6][1].real()+ 4);
            val_i[4][0].real(val_i[5][0].real()+adden); val_i[4][1].real(val_i[5][1].real()+ 4);
            // tg2 (buff2):
            val_i[11][0].real(tg_base[2][cc_div2]++); val_i[11][1].real(val_i[11][0].real()+32);
            val_i[10][0].real(val_i[11][0].real()+4); val_i[10][1].real(val_i[11][1].real()+ 4);
            val_i[ 9][0].real(val_i[10][0].real()+4); val_i[ 9][1].real(val_i[10][1].real()+ 4);
            val_i[ 8][0].real(val_i[ 9][0].real()+4); val_i[ 8][1].real(val_i[ 9][1].real()+ 4);
            // tg3 (buff3):
            val_i[15][0].real(tg_base[3][cc_div2]++); val_i[15][1].real(val_i[15][0].real()+32);
            val_i[14][0].real(val_i[15][0].real()+4); val_i[14][1].real(val_i[15][1].real()+ 4);
            val_i[13][0].real(val_i[14][0].real()+4); val_i[13][1].real(val_i[14][1].real()+ 4);
            val_i[12][0].real(val_i[13][0].real()+4); val_i[12][1].real(val_i[13][1].real()+ 4);
        } else { // from input data file(s):
            for (int t=0; t < N_STRM_I; ++t) { // t: tile
                for (int i=SSR_I-1; 0 <= i; --i ) { // i
                    din_t val_I, val_Q; 
                    sig_ifile[t] >> val_I >> val_Q; 
                    val_i[t][i] = c_din_t(val_I,val_Q);
                } // for i
            } // for t
        } // else

        for (int t=0; t < N_STRM_I; ++t) { // tile t
            sig_i[t].write(val_i[t]);
        } // for t
#ifdef DEBUG
        for (int t=0; t < N_STRM_I; ++t) { // tile t
            cout << "val_i[" << t << "]=";
            for (int s=SSR_I-1; 0 <= s; --s ) {
                cout << val_i[t][s];
            } // for s
            cout << endl;
        } // for t
#endif
  }

  for (int t=0; t < N_STRM_I; ++t) { // t: tile
    sig_ifile[t].clear();
    sig_ifile[t].seekg( 0, std::ios_base::beg );
  }
        /////////////////////////////////////
        // run DUT:
        /////////////////////////////////////
        outBuffer_wrapper( sig_i, sig_o );
  
    for (int cc=0; cc < NSAMP; cc++) { // clock cycle cc
        /////////////////////////////////////
        // store result:
        /////////////////////////////////////
        //if ( 8 <= cc) { // dump first 8 zero output cycles
            for (int f = 0; f < N_STRM_O; ++f) { // f: fft
                TT_OUT val_o = sig_o[f].read();
#ifdef DEBUG
                cout << "val_o " << cc << " =";
                for (int s=SSR_O-1; 0 <= s; --s )  // s: sample pos
                    cout << val_o[s];
                cout << endl;
#endif
                for (int s=SSR_O-1; 0 <= s; --s ) {
                    sig_ofile[f] << val_o[s].real() << ' ' << val_o[s].imag() << ' '; 
                }
                sig_ofile[f] << endl;
            } // for f
        //} // if 8 <= cc
    } // for cc
 }   
    // close opened files
    for (int t=0; t < N_STRM_I; ++t){ // t: tile
        sig_ifile[t].close();
    }
    for (int f=0; f < N_STRM_O; ++f){ // f: fft
        sig_ofile[f].close(); 
    }

    /////////////////////////////////////
    // compare the results
    /////////////////////////////////////
    int retval = 0;
    for (int f=0; f < N_STRM_O; ++f) { // f: fft
        string fname2 = goldDataFile; // dafault gold data file for data_mode 'f'
        if (data_mode == 'c') fname2 = "sigGoldC.txt";
        const string cmd_str = "diff --brief -w "+fname+" "+fname2; // -w: ignore wspaces
        const char* cmd = cmd_str.c_str(); // required arg type for system()
        cout << cmd << endl;
        retval += system(cmd);
    } // for i

    if (retval == 0) cout << "--- PASSED ---" << endl;
    else             cout << "*** FAILED ***" << endl;
    return(retval);
} // main
