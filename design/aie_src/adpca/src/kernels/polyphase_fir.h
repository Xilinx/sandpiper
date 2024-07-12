/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

// Author: Dhimiter Qendri

#pragma once

#include <adf.h>
#include <aie_api/aie.hpp>

template<unsigned pattern,unsigned NSAMP>
class polyphase_fir {
public:
  typedef cint16 TT_DATA;
  typedef cint32 TT_DATA_OUT;
  typedef int16  TT_COEFF;
  static constexpr unsigned ShiftAcc = 5;
  static_assert( (NSAMP % 4) == 0, "NSAMP must be a multiple of 4");

private:
  alignas(16) TT_COEFF (&taps0)[16];
  alignas(16) TT_COEFF (&taps1)[16];
  alignas(16) TT_COEFF (&taps2)[16];
  alignas(16) TT_COEFF (&taps3)[16];


public:
  polyphase_fir( TT_COEFF (&taps0_i)[16], TT_COEFF (&taps1_i)[16], TT_COEFF (&taps2_i)[16], TT_COEFF (&taps3_i)[16]);

  void run_i( input_stream<TT_DATA>* __restrict sig_i,  output_stream<TT_DATA_OUT>* __restrict sig_o );

  static void registerKernelClass( void )
  {
    REGISTER_FUNCTION( polyphase_fir::run_i );
    REGISTER_PARAMETER( taps0 );
    REGISTER_PARAMETER( taps1 );
    REGISTER_PARAMETER( taps2 );
    REGISTER_PARAMETER( taps3 );
  }
};


#define RESET_COEFF\
  ptr0 = (v16int16*)taps0;\
  ptr1 = (v16int16*)taps1;\
  ptr2 = (v16int16*)taps2;\
  ptr3 = (v16int16*)taps3;


#define S1_MULMAC(data,taps,acc,pk,ShiftAcc)\
        acc = mul4(data,1,0x3210,1,taps,0,0x0000,1);\
		acc = mac4(acc,data,5,0x3210,1,taps,4,0x0000,1);\
		acc = mac4(acc,data,9,0x3210,1,taps,8,0x0000,1);\
		data = upd_v(data,0,readincr_v4(sig_i));\
		acc = mac4(acc,data,13,0x3210,1,taps,12,0x0000,1);\
		pk = lsrs(acc, ShiftAcc);\
		put_wms(0,ext_v (pk,0));\
		put_wms(0,ext_v (pk,1));\


#define S2_MULMAC(data,taps,acc,pk,ShiftAcc)\
		acc = mul4(data,5,0x3210,1,taps,0,0x0000,1);\
		acc = mac4(acc,data,9,0x3210,1,taps,4,0x0000,1);\
		acc = mac4(acc,data,13,0x3210,1,taps,8,0x0000,1);\
		data = upd_v(data,1,readincr_v4(sig_i));\
		acc = mac4(acc,data,1,0x3210,1,taps,12,0x0000,1);\
		pk = lsrs(acc, ShiftAcc);\
		put_wms(0,ext_v (pk,0));\
		put_wms(0,ext_v (pk,1));\

#define S3_MULMAC(data,taps,acc,pk,ShiftAcc)\
		acc = mul4(data,9,0x3210,1,taps,0,0x0000,1);\
		acc = mac4(acc,data,13,0x3210,1,taps,4,0x0000,1);\
		acc = mac4(acc,data,1,0x3210,1,taps,8,0x0000,1);\
		data = upd_v(data,2,readincr_v4(sig_i));\
		acc = mac4(acc,data,5,0x3210,1,taps,12,0x0000,1);\
		pk = lsrs(acc, ShiftAcc);\
		put_wms(0,ext_v (pk,0));\
		put_wms(0,ext_v (pk,1));\

#define S4_MULMAC(data,taps,acc,pk,ShiftAcc)\
		acc = mul4(data,13,0x3210,1,taps,0,0x0000,1);\
		acc = mac4(acc,data,1,0x3210,1,taps,4,0x0000,1);\
		acc = mac4(acc,data,5,0x3210,1,taps,8,0x0000,1);\
		data = upd_v(data,3,readincr_v4(sig_i));\
		acc = mac4(acc,data,9,0x3210,1,taps,12,0x0000,1);\
		pk = lsrs(acc, ShiftAcc);\
		put_wms(0,ext_v (pk,0));\
		put_wms(0,ext_v (pk,1));
