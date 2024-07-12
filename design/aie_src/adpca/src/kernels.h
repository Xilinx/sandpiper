/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

#ifndef FUNCTION_KERNELS_H
#define FUNCTION_KERNELS_H


  #include "aie_api/aie.hpp"
  #include "aie_api/aie_adf.hpp"
  #include "kernels/include.h"

  // Matrix Covariance
  void CovarianceMatrixAcc(
     adf::input_buffer<cint16> & __restrict pulsem,
     adf::input_buffer<cint16> & __restrict pulsem1,
     adf::output_buffer<cfloat> & __restrict out);

  // Matrix Covariance
  void CovarianceMatrix(
  	adf::input_buffer<cint16> & __restrict pulsem,
  	adf::input_buffer<cint16> & __restrict pulsem1,
  	adf::output_buffer<cfloat> & __restrict out);

  void VectorAcc(
		adf::input_buffer<cfloat> & __restrict matA,
		adf::output_async_buffer<cfloat> & __restrict matC);

  void VectorAdd8(
  		adf::input_buffer<cfloat> & __restrict matA1,
  		adf::input_buffer<cfloat> & __restrict matB1,
  		adf::input_buffer<cfloat> & __restrict matA2,
  		adf::input_buffer<cfloat> & __restrict matB2,
  		adf::input_buffer<cfloat> & __restrict matA3,
  		adf::input_buffer<cfloat> & __restrict matB3,
  		adf::input_buffer<cfloat> & __restrict matA4,
  		adf::input_buffer<cfloat> & __restrict matB4,
  		adf::output_buffer<cfloat> & __restrict matC);
  void VectorAdd2(
     		adf::input_buffer<cfloat> & __restrict matA,
     		adf::input_buffer<cfloat> & __restrict matB,
     		adf::output_buffer<cfloat> & __restrict matC);

  // Matrix Invert
  void mat_inv_qrhhr_1(
  		adf::input_buffer<cfloat> & __restrict AT,
  		adf::output_buffer<cfloat> & __restrict out);

  void mat_inv_qrhhr_2(
  		adf::input_buffer<cfloat> & __restrict QR,
  		adf::output_buffer<cfloat> & __restrict out);

  void CholInDataPopulator(
  		adf::input_buffer<cfloat> & __restrict matA,
  		adf::output_buffer<float> & __restrict AR,
  		adf::output_buffer<float> & __restrict AI);

  void cholesky_complex_buffer(
		adf::input_buffer<float> & __restrict matA_real,
  		adf::input_buffer<float> & __restrict matA_imag,
  		adf::output_buffer<float> & __restrict matL_real,
  		adf::output_buffer<float> & __restrict matL_imag);

  void mat_inv_L(
  		adf::input_buffer<float> & __restrict LR,
  		adf::input_buffer<float> & __restrict LI,
  		adf::output_buffer<cfloat> & __restrict Inv);

  void mat_inv(input_window<cfloat> * __restrict in, output_window<cfloat> * __restrict out);

  #ifdef DEBUG
  #include <assert.h>
  #endif

  void print_matrix(cfloat *matrix_in, int matrix_row, int matrix_col, const char *s);
  void print_vector(aie::vector<cfloat, MAT_SIZE> v, int len, const char *s);

  float reciprocal(float a);

  // ECI Matrix
  void E_MatrixRowAdder(
  		adf::input_buffer<cfloat> & __restrict matRi,
  	    adf::output_buffer<cfloat> & __restrict matOut);

  void optimizedECIMatrix(
  		adf::input_async_buffer<cfloat> & __restrict matRi,
  	    adf::output_buffer<cfloat> & __restrict matOut);

  void GammaMatrix(output_window_cfloat* __restrict matOut);

  void GammaMatrixTranspose(output_window_cfloat* __restrict matOut);

  void GammaMatrixTransposeSplitLowUp(
  		adf::output_buffer<cfloat> & __restrict gammaMatLow,
  		adf::output_buffer<cfloat> & __restrict gammaMatUp);

  template <int LOWER_UPPER>
  void GammaMatrixTransposeSplit(adf::output_buffer<cfloat> & __restrict gammaMat);

  template <int NRows, int NCols>
  void Tiling_4x2_to_2x2(
  		adf::input_buffer<cfloat> & __restrict tile4x2,
  		adf::output_buffer<cfloat> & __restrict tile2x2);

  template <int NRows, int NCols>
  void Untiling_4x2_ColCombiner(input_window_cfloat* __restrict lowerColTile4x2, input_window_cfloat* __restrict upperColTile4x2, output_window_cfloat* __restrict MatOut);

  template <int NRows, int NCols>
  void Untiling_4x2_RowCombiner(
  		adf::input_buffer<cfloat> & __restrict lowerRowlTile4x2,
  		adf::input_buffer<cfloat> & __restrict upperRowlTile4x2,
  		adf::output_buffer<cfloat> & __restrict matOut);

  template <int NRows, int NCols>
  void MatTranspose(
  		adf::input_buffer<cfloat> & __restrict matIn,
  		adf::output_buffer<cfloat> & __restrict matOut);

  template <int NRows, int NCols>
  void RowCombiner(
  		adf::input_buffer<cfloat> & __restrict lowerRow,
  		adf::input_buffer<cfloat> & __restrict upperRow,
  		adf::output_buffer<cfloat> & __restrict matOut);

  template <int NRows, int NCols>
  void ColCombiner(input_window_cfloat* __restrict lowerCol, input_window_cfloat* __restrict upperCol, output_window_cfloat* __restrict matOut);

  // Weight Computation and Application

  void SteeringVector( output_window_cfloat * out, float normDop, float normSpa);
  void SteeringVectorV2(adf::output_buffer<cfloat> & __restrict out, float normDop, float normSpa);

  void WeightCalculation(
  		adf::input_buffer<cfloat> & __restrict st,
  		adf::input_buffer<cfloat> & __restrict eci,
  		adf::output_buffer<cfloat> & __restrict weight);

  void WeightApplication(
  		adf::input_async_buffer<cfloat> & __restrict weight,
  		adf::input_buffer<cint16> & __restrict matB,
  		adf::output_buffer<cfloat> & __restrict matC);

  // Complex implementations on vectors
  //
  float norm(aie::vector<cfloat, MAT_SIZE> v);

  float inv_norm(aie::vector<cfloat, MAT_SIZE> v);

  cfloat sign(cfloat z);

  void mat_mult_A_BT(const cfloat* __restrict A, const cfloat* __restrict BT, cfloat* __restrict C);

  void mat_mult_AT_BT(const cfloat* __restrict AT, const cfloat* __restrict BT, cfloat* __restrict C);

  cfloat reciprocal(cfloat z);

#endif
