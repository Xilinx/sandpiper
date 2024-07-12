/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

#ifndef FUNCTION_INCLUDES_H
#define FUNCTION_INCLUDES_H

#include <adf.h>

// Global parameters

#define NCUBOID                 1                         // Number of Cuboids
#define NPULSE                  10                        // Number of pulses per cuboid
#define NCOVMAT                 (NPULSE-1)                // Number of covariance matrices
#define NITER                   NCUBOID                   // Number of iteration

#define MAT_SIZE                8                         // Square Matrix Dimension
#define WLEN                    (MAT_SIZE * MAT_SIZE)     // Input Window Length (New Data)
#define N_CFLOAT_BYTES          8                         // Complex values 4 bytes for Q, 4 bytes for I
#define N_CINT16_BYTES          4                         // Complex values 2 bytes for Q, 2 bytes for I

//
#define L                       4000                      // Number of samples per frame (48 or 4000)
#define NCOV                    16                        // Number of covariance kernels
#define TC_PER_COV             (TC/NCOV)                  // Number of training cells per covariance kernel
#define COV_ACC_N_ITR           TC_PER_COV                // Number of covariance accumulator iteration
#define NWEIGHTAPP              4                        // Number of weight application kernels
#define NBATCH_WEIGHTAPP		40
#define CPI_PER_WEIGHTAPP      (L/(NWEIGHTAPP*NBATCH_WEIGHTAPP)) // Number of CPI per weight application kernel

#if L == 48
  #define TC                    32                        // Number of training cells
  constexpr float               NORMALIZED_DOPPLER = 0.200665161;
  constexpr float               NORMALIZED_SPATIAL = 0.000000000;
#else
  #define TC                    2000                      // Number of training cells
  constexpr float               NORMALIZED_DOPPLER = 0.1639799327;
  constexpr float               NORMALIZED_SPATIAL = 0.000000000;
#endif

// Debug stuff

#define DEBUG_PRINT
#define COV_DEBUG_ID            17 // Bigger than 16 means no debug prints

// Covariance defines

#define COV_TRANSPOSE_OUTPUT

#define NANTENNA                4                         // Number of antennas
#define COV_IN_WIN_SIZE         (NANTENNA*N_CINT16_BYTES) // Covariance input window size in bytes
#define COV_OUT_WIN_SIZE        (WLEN*N_CFLOAT_BYTES)     // Covariance input window size in bytes

// Matrix inverse defines

#define CHOL_HEADER						2
#define CHOL_TRIANGULAR_MATRIX_SIZE		36
#define CHOL_DATA_SIZE					CHOL_TRIANGULAR_MATRIX_SIZE+CHOL_HEADER
#define CHOL_N_COLUMNS					8

// ECI Matrix
#define ECI_MULT_P_DIM_A           20      // Number of elements along the unique dimension (rows) of Matrix A.
#define ECI_MULT_P_DIM_B           20      // Number of elements along the unique dimension (columns) of Matrix B.

// Channelizer
#define CHAN_INST					4		//Number of instances of channelizer (Polyphase FIR)
#define FIR_Y						0

#endif
