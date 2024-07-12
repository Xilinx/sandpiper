/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

#include <adf.h>
#include <aie_api/aie.hpp>

#include "polyphase_fir.h"


/**
 * @brief This constructor takes 4 taps arrays and initializes the 4 FIR per tile
 *
 */
template<unsigned PATTERN,unsigned NSAMP>
polyphase_fir<PATTERN,NSAMP>::polyphase_fir( TT_COEFF (&taps0_i)[16],
		TT_COEFF (&taps1_i)[16],
		TT_COEFF (&taps2_i)[16],
		TT_COEFF (&taps3_i)[16]
		)
: taps0(taps0_i), taps1(taps1_i), taps2(taps2_i), taps3(taps3_i)
 {
  aie::set_rounding(aie::rounding_mode::positive_inf);
  aie::set_saturation(aie::saturation_mode::saturate);
}

/**
 * @brief This function calculates one iteration of 4 FIR per tile
 */
template<unsigned PATTERN,unsigned NSAMP>
void polyphase_fir<PATTERN,NSAMP>::run_i( input_stream<TT_DATA>* __restrict sig_i,   output_stream<TT_DATA_OUT>* __restrict sig_o )
{
  // Declare buffers:
  v16cint16 buff0  = null_v16cint16();
  v16cint16 buff1  = null_v16cint16();
  v16cint16 buff2  = null_v16cint16();
  v16cint16 buff3  = null_v16cint16();

  v4cacc48  acc0   = null_v4cacc48();
  v4cacc48  acc1   = null_v4cacc48();
  v4cacc48  acc2   = null_v4cacc48();
  v4cacc48  acc3   = null_v4cacc48();

  v16int16 taps0a = undef_v16int16();
  v16int16 taps1a = undef_v16int16();
  v16int16 taps2a = undef_v16int16();
  v16int16 taps3a = undef_v16int16();

  // Pointers to tap weights:
  v16int16 *coeff0 =  (v16int16*) taps0;
  v16int16 *coeff1 =  (v16int16*) taps1;
  v16int16 *coeff2 =  (v16int16*) taps2;
  v16int16 *coeff3 =  (v16int16*) taps3;

  taps0a =  *coeff0++;   //
  taps1a =  *coeff1++;   //
  taps2a =  *coeff2++;   //
  taps3a =  *coeff3++;   //

  v4cint32 pk = undef_v4cint32();

  for ( unsigned ii=0; ii < 1250; ii++)		//Batch processing of multiple samples per loop
  //    chess_loop_range(1,)
  //chess_prepare_for_pipelining
  {
	S1_MULMAC(buff0,taps0a,acc0,pk,ShiftAcc);
	S1_MULMAC(buff1,taps1a,acc1,pk,ShiftAcc);
	S1_MULMAC(buff2,taps2a,acc2,pk,ShiftAcc);
	S1_MULMAC(buff3,taps3a,acc3,pk,ShiftAcc);

	S2_MULMAC(buff0,taps0a,acc0,pk,ShiftAcc);
	S2_MULMAC(buff1,taps1a,acc1,pk,ShiftAcc);
	S2_MULMAC(buff2,taps2a,acc2,pk,ShiftAcc);
	S2_MULMAC(buff3,taps3a,acc3,pk,ShiftAcc);

	S3_MULMAC(buff0,taps0a,acc0,pk,ShiftAcc);
	S3_MULMAC(buff1,taps1a,acc1,pk,ShiftAcc);
	S3_MULMAC(buff2,taps2a,acc2,pk,ShiftAcc);
	S3_MULMAC(buff3,taps3a,acc3,pk,ShiftAcc);

	S4_MULMAC(buff0,taps0a,acc0,pk,ShiftAcc);
	S4_MULMAC(buff1,taps1a,acc1,pk,ShiftAcc);
	S4_MULMAC(buff2,taps2a,acc2,pk,ShiftAcc);
	S4_MULMAC(buff3,taps3a,acc3,pk,ShiftAcc);
	}

}

