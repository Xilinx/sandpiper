/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

#pragma once

#include <adf.h>
#include "polyphase_fir.h"

using namespace adf;


template<unsigned PATTERN,unsigned NSAMP>
class polyphase_fir_graph : public graph {
private:
  kernel k_fir;

public:
  std::array<port<input>,1>  sig_i;
  std::array<port<output>,1> sig_o;

  using TT_KERNEL = polyphase_fir<PATTERN,NSAMP>;

  polyphase_fir_graph(
		  std::vector<typename TT_KERNEL::TT_COEFF> taps0_i, std::vector<typename TT_KERNEL::TT_COEFF> taps1_i ,
		  std::vector<typename TT_KERNEL::TT_COEFF> taps2_i, std::vector<typename TT_KERNEL::TT_COEFF> taps3_i)
  {
    k_fir = kernel::create_object<TT_KERNEL>( taps0_i, taps1_i , taps2_i, taps3_i);

    source(k_fir) = "polyphase_fir.cpp";
    runtime<ratio>(k_fir) = 0.9;
    //repetition_count(k_fir) = 2500;

    connect<stream,stream>( sig_i[0],     k_fir.in[0] );
    connect<stream,stream>( k_fir.out[0], sig_o[0]    );
  }
};

