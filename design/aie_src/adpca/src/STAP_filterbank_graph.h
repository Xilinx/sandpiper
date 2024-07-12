/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

// Author: Dhimiter Qendri

#pragma once

#include <adf.h>
#include <filterbank_taps_init.h>
#include "kernels/polyphase_fir_graph.h"

using namespace adf;

// ------------------------------------------------------------
// Top Level Graph
// ------------------------------------------------------------



class STAP_filterbank_graph : public graph {
public:
  static constexpr unsigned NSAMP = 2048;
  // PLIO:
  std::array<port<input>,16>  sig_i;
  std::array<port<output>,16> sig_o;

  // Declare sub-graphs:
  polyphase_fir_graph<0,NSAMP> dut_graph0;
  polyphase_fir_graph<1,NSAMP> dut_graph1;
  polyphase_fir_graph<2,NSAMP> dut_graph2;
  polyphase_fir_graph<3,NSAMP> dut_graph3;
  polyphase_fir_graph<4,NSAMP> dut_graph4;
  polyphase_fir_graph<5,NSAMP> dut_graph5;
  polyphase_fir_graph<6,NSAMP> dut_graph6;
  polyphase_fir_graph<7,NSAMP> dut_graph7;
  polyphase_fir_graph<0,NSAMP> dut_graph8;
  polyphase_fir_graph<1,NSAMP> dut_graph9;
  polyphase_fir_graph<2,NSAMP> dut_graph10;
  polyphase_fir_graph<3,NSAMP> dut_graph11;
  polyphase_fir_graph<4,NSAMP> dut_graph12;
  polyphase_fir_graph<5,NSAMP> dut_graph13;
  polyphase_fir_graph<6,NSAMP> dut_graph14;
  polyphase_fir_graph<7,NSAMP> dut_graph15;




  // Constructor:
  STAP_filterbank_graph( void )
    : dut_graph0( tile0_fir_0, tile0_fir_1 , tile0_fir_2, tile0_fir_3),
      dut_graph1( tile1_fir_0, tile1_fir_1 , tile1_fir_2, tile1_fir_3),
      dut_graph2( tile2_fir_0, tile2_fir_1,  tile2_fir_2, tile2_fir_3),
      dut_graph3( tile3_fir_0, tile3_fir_1,  tile3_fir_2, tile3_fir_3),
      dut_graph4( tile4_fir_0, tile4_fir_1 , tile4_fir_2, tile4_fir_3),
      dut_graph5( tile5_fir_0, tile5_fir_1 , tile5_fir_2, tile5_fir_3),
      dut_graph6( tile6_fir_0, tile6_fir_1 , tile6_fir_2, tile6_fir_3),
      dut_graph7( tile7_fir_0, tile7_fir_1 , tile7_fir_2, tile7_fir_3),
	  dut_graph8( tile8_fir_0, tile8_fir_1 , tile8_fir_2, tile8_fir_3),
	  dut_graph9( tile9_fir_0, tile9_fir_1 , tile9_fir_2, tile9_fir_3),
	  dut_graph10( tile10_fir_0, tile10_fir_1 , tile10_fir_2, tile10_fir_3),
	  dut_graph11( tile11_fir_0, tile11_fir_1 , tile11_fir_2, tile11_fir_3),
	  dut_graph12( tile12_fir_0, tile12_fir_1,  tile12_fir_2, tile12_fir_3),
	  dut_graph13( tile13_fir_0, tile13_fir_1,  tile13_fir_2, tile13_fir_3),
	  dut_graph14( tile14_fir_0, tile14_fir_1 , tile14_fir_2, tile14_fir_3),
	  dut_graph15( tile15_fir_0, tile15_fir_1 , tile15_fir_2, tile15_fir_3)
  {
    // Connections:
    connect<stream>( sig_i[0], dut_graph0.sig_i[0] );
    connect<stream>( sig_i[1], dut_graph1.sig_i[0] );
    connect<stream>( sig_i[2], dut_graph2.sig_i[0] );
    connect<stream>( sig_i[3], dut_graph3.sig_i[0] );
    connect<stream>( sig_i[4], dut_graph4.sig_i[0] );
    connect<stream>( sig_i[5], dut_graph5.sig_i[0] );
    connect<stream>( sig_i[6], dut_graph6.sig_i[0] );
    connect<stream>( sig_i[7], dut_graph7.sig_i[0] );
    connect<stream>( sig_i[8], dut_graph8.sig_i[0] );
    connect<stream>( sig_i[9], dut_graph9.sig_i[0] );
    connect<stream>( sig_i[10], dut_graph10.sig_i[0] );
    connect<stream>( sig_i[11], dut_graph11.sig_i[0] );
    connect<stream>( sig_i[12], dut_graph12.sig_i[0] );
    connect<stream>( sig_i[13], dut_graph13.sig_i[0] );
    connect<stream>( sig_i[14], dut_graph14.sig_i[0] );
    connect<stream>( sig_i[15], dut_graph15.sig_i[0] );


    connect<stream>( dut_graph0.sig_o[0], sig_o[0] );
    connect<stream>( dut_graph1.sig_o[0], sig_o[1] );
    connect<stream>( dut_graph2.sig_o[0], sig_o[2] );
    connect<stream>( dut_graph3.sig_o[0], sig_o[3] );
    connect<stream>( dut_graph4.sig_o[0], sig_o[4] );
    connect<stream>( dut_graph5.sig_o[0], sig_o[5] );
    connect<stream>( dut_graph6.sig_o[0], sig_o[6] );
    connect<stream>( dut_graph7.sig_o[0], sig_o[7] );
    connect<stream>( dut_graph8.sig_o[0], sig_o[8] );
    connect<stream>( dut_graph9.sig_o[0], sig_o[9] );
    connect<stream>( dut_graph10.sig_o[0], sig_o[10] );
    connect<stream>( dut_graph11.sig_o[0], sig_o[11] );
    connect<stream>( dut_graph12.sig_o[0], sig_o[12] );
    connect<stream>( dut_graph13.sig_o[0], sig_o[13] );
    connect<stream>( dut_graph14.sig_o[0], sig_o[14] );
    connect<stream>( dut_graph15.sig_o[0], sig_o[15] );
  }
};




