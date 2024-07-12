/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

#include <adf.h>
#include "kernels.h"
#include <STAP_filterbank_graph.h>

#define __AIESIM__

#define INLCUDE_COV        // Include covariance kernels.
#define INLCUDE_ECI_COMP   // Include the ECI matrix computation kernels.
#define INLCUDE_WEIGHT_APP // Include weight computation and application kernels.
#define ENABLE_CH_INV	   // Include Cholesky decomposition for Inverse computation.
#define ENABLE_COVACC
#define INCLUDE_CHANNELIZER

using namespace adf;


 class adpcaGraph : public adf::graph
{
private:

#ifdef INLCUDE_COV
  // Matrix Covariance Kernels

  kernel covariance[16];
  kernel matrixadder[3];
#ifndef ENABLE_COVACC
  kernel matrixaccumulator;
#endif

  // Matrix Invert
#ifdef ENABLE_CH_INV
	kernel chol_lib[MAT_SIZE];
	kernel cholInput;
	kernel matInvL;
#else
  kernel matInv1;
  kernel matInv2;
#endif

#endif

#ifdef INLCUDE_ECI_COMP
  // Matrix Row Adder
	kernel optECIMatrix;
	kernel ECIMatTranspose;
#endif

#ifdef INLCUDE_WEIGHT_APP
  // Weight computation and application
  kernel ksteer;
  kernel kweightcalc;
  kernel kweightapp[16];
#endif

public:

#ifdef INLCUDE_WEIGHT_APP
  // RTP parameters
  port<direction::in> normDop;
  port<direction::in> normSpa;
#endif

#ifdef INLCUDE_COV
  input_plio covcurr[16];
  input_plio covprev[16];

  // Temporary output for the Covariance accumulator
  // output_plio accOut = output_plio::create(plio_64_bits, "data/L_" + std::to_string(L) + "/covMtxTransposeRow.txt", 250);

  // Temporary input for the matrix inversion kernel
  // input_plio invMatIn = input_plio::create(plio_64_bits, "data/L_" + std::to_string(L) + "/Matlab_Outputs/covMtxTransposeRow.txt", 250);

#endif

#if defined(INLCUDE_COV) && !defined(INLCUDE_ECI_COMP)
  // Temporary output for the inverted covariance matrices
  output_plio invMatOut = output_plio::create(plio_128_bits, "data/L_" + std::to_string(L) + "/invTwoPulseMtxRowAll.txt", 250);
#endif

#if defined(INLCUDE_ECI_COMP) && !defined(INLCUDE_COV)
  // Temporary input / output to validate E_MatrixRowAdder
  input_plio invCovMtx = input_plio::create(plio_128_bits, "data/L_" + std::to_string(L) + "/Matlab_Outputs/invTwoPulseMtxRowAll.txt", 250);
#endif

#if defined(INLCUDE_ECI_COMP) && !defined(INLCUDE_WEIGHT_APP)
  // Complex Matrix ECI
  output_plio eciMtxColOut = output_plio::create(plio_64_bits, "data/L_" + std::to_string(L) + "/eciMtxCol.txt", 250);
#endif

#if defined(INLCUDE_WEIGHT_APP) && !defined(INLCUDE_ECI_COMP)
  // ECI Matrix generated from Matlab
  input_plio eciMtx = input_plio::create(plio_64_bits, "data/L_" + std::to_string(L) + "/Matlab_Outputs/eciMtxCol.txt", 250);
#endif

#ifdef INLCUDE_WEIGHT_APP
  input_plio weightAppIn[4];
  output_plio weightAppOut[4];
#endif

#ifdef INCLUDE_CHANNELIZER
  //static constexpr unsigned FIR_X = 6;
  //static constexpr unsigned FIR_Y = 0;
  std::array<input_plio,16>  fbank_i[CHAN_INST];
  std::array<output_plio,16> fbank_o[CHAN_INST];
  STAP_filterbank_graph dut[CHAN_INST];
#endif
  adpcaGraph()
  {
#ifdef INCLUDE_CHANNELIZER
	  std::array<int,CHAN_INST> FIR_X;
	  FIR_X[0] = 6;
	  FIR_X[1] = 14;
	  FIR_X[2] = 29;
	  FIR_X[3] = 37;
	  for (int j = 0; j < CHAN_INST; j++)
	  {
	  for (int ii=0; ii < 16; ii++) {
	        std::string plio_i = "PLIO_fbank_i_"+std::to_string(ii)+"_"+std::to_string(j);
	        std::string plio_o = "PLIO_fbank_o_"+std::to_string(ii)+"_"+std::to_string(j);
	        std::string file_i = "data/Channelizer/sig"+std::to_string(ii)+"_"+std::to_string(j)+"_i.txt";
	        std::string file_o = "data/Channelizer/sig"+std::to_string(ii)+"_"+std::to_string(j)+"_o.txt";
	        fbank_i[j][ii] = input_plio::create(plio_i,plio_64_bits,file_i, 125);
	        fbank_o[j][ii] = output_plio::create(plio_o,plio_64_bits,file_o, 250);
	        //std::string s1 = "s_"+std::to_string(ii)+"_"+std::to_string(j)+"_i";
	        connect<stream>(fbank_i[j][ii].out[0], dut[j].sig_i[ii]);
	        //fifo_depth(s1)= 64;
	        //std::string s2 = "s_"+std::to_string(ii)+"_"+std::to_string(j)+"_o";
	        connect<stream>(dut[j].sig_o[ii], fbank_o[j][ii].in[0]);
	        //fifo_depth(s2)= 64;
	      }
	      location<graph>(dut[j].dut_graph0) = area_group( {{aie_tile,FIR_X[j],FIR_Y,FIR_X[j]+3,FIR_Y+1},{shim_tile,FIR_X[j],0,FIR_X[j]+3,0}} );
	      location<graph>(dut[j].dut_graph1) = area_group( {{aie_tile,FIR_X[j],FIR_Y,FIR_X[j]+3,FIR_Y+1},{shim_tile,FIR_X[j],0,FIR_X[j]+3,0}} );
	      location<graph>(dut[j].dut_graph2) = area_group( {{aie_tile,FIR_X[j],FIR_Y,FIR_X[j]+3,FIR_Y+1},{shim_tile,FIR_X[j],0,FIR_X[j]+3,0}} );
	      location<graph>(dut[j].dut_graph3) = area_group( {{aie_tile,FIR_X[j],FIR_Y,FIR_X[j]+3,FIR_Y+1},{shim_tile,FIR_X[j],0,FIR_X[j]+3,0}} );
	      location<graph>(dut[j].dut_graph4) = area_group( {{aie_tile,FIR_X[j],FIR_Y,FIR_X[j]+3,FIR_Y+1},{shim_tile,FIR_X[j],0,FIR_X[j]+3,0}} );
	      location<graph>(dut[j].dut_graph5) = area_group( {{aie_tile,FIR_X[j],FIR_Y,FIR_X[j]+3,FIR_Y+1},{shim_tile,FIR_X[j],0,FIR_X[j]+3,0}} );
	      location<graph>(dut[j].dut_graph6) = area_group( {{aie_tile,FIR_X[j],FIR_Y,FIR_X[j]+3,FIR_Y+1},{shim_tile,FIR_X[j],0,FIR_X[j]+3,0}} );
	      location<graph>(dut[j].dut_graph7) = area_group( {{aie_tile,FIR_X[j],FIR_Y,FIR_X[j]+3,FIR_Y+1},{shim_tile,FIR_X[j],0,FIR_X[j]+3,0}} );

		  location<PLIO>(fbank_i[j][0]) = shim(FIR_X[j]+0);  location<PLIO>(fbank_o[j][0]) = shim(FIR_X[j]+0);
	      location<PLIO>(fbank_i[j][1]) = shim(FIR_X[j]+0);  location<PLIO>(fbank_o[j][1]) = shim(FIR_X[j]+0);
	      location<PLIO>(fbank_i[j][2]) = shim(FIR_X[j]+1);  location<PLIO>(fbank_o[j][2]) = shim(FIR_X[j]+1);
	      location<PLIO>(fbank_i[j][3]) = shim(FIR_X[j]+1);  location<PLIO>(fbank_o[j][3]) = shim(FIR_X[j]+1);
	      location<PLIO>(fbank_i[j][4]) = shim(FIR_X[j]+2);  location<PLIO>(fbank_o[j][4]) = shim(FIR_X[j]+2);
	      location<PLIO>(fbank_i[j][5]) = shim(FIR_X[j]+2);  location<PLIO>(fbank_o[j][5]) = shim(FIR_X[j]+2);
	      location<PLIO>(fbank_i[j][6]) = shim(FIR_X[j]+3);  location<PLIO>(fbank_o[j][6]) = shim(FIR_X[j]+3);
	      location<PLIO>(fbank_i[j][7]) = shim(FIR_X[j]+3);  location<PLIO>(fbank_o[j][7]) = shim(FIR_X[j]+3);
	      location<PLIO>(fbank_i[j][8]) = shim(FIR_X[j]+4);  location<PLIO>(fbank_o[j][8]) = shim(FIR_X[j]+4);
	      location<PLIO>(fbank_i[j][9]) = shim(FIR_X[j]+4);  location<PLIO>(fbank_o[j][9]) = shim(FIR_X[j]+4);
	      location<PLIO>(fbank_i[j][10]) = shim(FIR_X[j]+5);  location<PLIO>(fbank_o[j][10]) = shim(FIR_X[j]+5);
	      location<PLIO>(fbank_i[j][11]) = shim(FIR_X[j]+5);  location<PLIO>(fbank_o[j][11]) = shim(FIR_X[j]+5);
	      location<PLIO>(fbank_i[j][12]) = shim(FIR_X[j]+6);  location<PLIO>(fbank_o[j][12]) = shim(FIR_X[j]+6);
	      location<PLIO>(fbank_i[j][13]) = shim(FIR_X[j]+6);  location<PLIO>(fbank_o[j][13]) = shim(FIR_X[j]+6);
	      location<PLIO>(fbank_i[j][14]) = shim(FIR_X[j]+7);  location<PLIO>(fbank_o[j][14]) = shim(FIR_X[j]+7);
	      location<PLIO>(fbank_i[j][15]) = shim(FIR_X[j]+7);  location<PLIO>(fbank_o[j][15]) = shim(FIR_X[j]+7);
	  }
#endif
#ifdef INLCUDE_COV
	// Covariance PLIO interfaces
	for (int i=0; i<16; i++) {
		covcurr[i]  = input_plio::create(plio_32_bits, "data/L_" + std::to_string(L) + "/data_from_tc_selector/cov" + std::to_string(i) + "_curr_data.txt", 125);
		covprev[i]  = input_plio::create(plio_32_bits, "data/L_" + std::to_string(L) + "/data_from_tc_selector/cov" + std::to_string(i) + "_prev_data.txt", 125);
	}

    // create covariance kernels

	for(int i=0; i<16; i++) {
#ifdef ENABLE_COVACC
		covariance[i]  = kernel::create(CovarianceMatrixAcc);
#else
		covariance[i]  = kernel::create(CovarianceMatrix);
#endif
		source(covariance[i]) = "kernels/kernels.cc";
		runtime<ratio>(covariance[i]) = 0.9;
		repetition_count(covariance[i])  = NCOVMAT; //TC_PER_COV*NCOVMAT;
		location<kernel>(covariance[i]) = tile(24+i/8, i%8); // kernel location
		dimensions(covariance[i].in[0]) = {NANTENNA*TC_PER_COV};
		dimensions(covariance[i].in[1]) = {NANTENNA*TC_PER_COV};
		dimensions(covariance[i].out[0]) = {WLEN};
		stack_size(covariance[i]) = 9216;
	}

    // create covariance adder kernels
	for(int i=0; i<2; i++) {
		matrixadder[i]  = kernel::create(VectorAdd8);
		source(matrixadder[i]) = "kernels/kernels.cc";
		runtime<ratio>(matrixadder[i]) = 0.9;
		//location<kernel>(matrixadder[i]) = tile(22+i/2,i%2);
#ifdef ENABLE_COVACC
		repetition_count(matrixadder[i])  = NCOVMAT;	//TC_PER_COV*NCOVMAT;
#else
		repetition_count(matrixadder[i])  = TC_PER_COV*NCOVMAT;
#endif
		dimensions(matrixadder[i].in[0]) = {WLEN};
		dimensions(matrixadder[i].in[1]) = {WLEN};
		dimensions(matrixadder[i].in[2]) = {WLEN};
		dimensions(matrixadder[i].in[3]) = {WLEN};
		dimensions(matrixadder[i].in[4]) = {WLEN};
		dimensions(matrixadder[i].in[5]) = {WLEN};
		dimensions(matrixadder[i].in[6]) = {WLEN};
		dimensions(matrixadder[i].in[7]) = {WLEN};
		dimensions(matrixadder[i].out[0]) = {WLEN};
	}
	matrixadder[2]  = kernel::create(VectorAdd2);
	source(matrixadder[2]) = "kernels/kernels.cc";
	stack_size(matrixadder[2]) = 3000;
	runtime<ratio>(matrixadder[2]) = 0.005;
#ifdef ENABLE_COVACC
		repetition_count(matrixadder[2])  = NCOVMAT;	//TC_PER_COV*NCOVMAT;
#else
		repetition_count(matrixadder[2])  = TC_PER_COV*NCOVMAT;
#endif
	dimensions(matrixadder[2].in[0]) = {WLEN};
	dimensions(matrixadder[2].in[1]) = {WLEN};
	dimensions(matrixadder[2].out[0]) = {WLEN};
	location<kernel>(matrixadder[0]) = tile(23,3);
	location<kernel>(matrixadder[1]) = tile(26,2);
	//location<kernel>(matrixadder[0]) = location<kernel>(matrixadder[1]);
#ifndef ENABLE_COVACC
	matrixaccumulator = kernel::create(VectorAcc);
	location<kernel>(matrixaccumulator) = tile(21, 3);
	dimensions(matrixaccumulator.in[0]) = {WLEN};
	//dimensions(matrixaccumulator.in[1]) = {WLEN};
	dimensions(matrixaccumulator.out[0]) = {WLEN};
#endif



#ifdef ENABLE_CH_INV
	cholInput  = kernel::create(CholInDataPopulator);
	source(cholInput) = "kernels/kernels.cc";
	runtime<ratio>(cholInput) = 0.005;
	repetition_count(cholInput)  = NCOVMAT;
	dimensions(cholInput.in[0]) = {WLEN};
	dimensions(cholInput.out[0]) = {CHOL_DATA_SIZE};
	dimensions(cholInput.out[1]) = {CHOL_DATA_SIZE};
	stack_size(cholInput) = 3000;

	location<kernel>(cholInput) = location<kernel>(matrixadder[2]);

	for (int i = 0; i < MAT_SIZE; i++)
	{
		chol_lib[i] = kernel::create(cholesky_complex_buffer);
		source(chol_lib[i]) = "kernels/kernels.cc";
		runtime<ratio>(chol_lib[i]) = 0.01;
		repetition_count(chol_lib[i]) = NCOVMAT;
		dimensions(chol_lib[i].in[0]) = {CHOL_DATA_SIZE};
		dimensions(chol_lib[i].in[1]) = {CHOL_DATA_SIZE};
		dimensions(chol_lib[i].out[0]) = {CHOL_DATA_SIZE};
		dimensions(chol_lib[i].out[1]) = {CHOL_DATA_SIZE};
		stack_size(chol_lib[i]) = 3000;
		location<kernel>(chol_lib[i]) = location<kernel>(matrixadder[2]);
	}

	matInvL  = kernel::create(mat_inv_L);
	source(matInvL) = "kernels/kernels.cc";
	runtime<ratio>(matInvL) = 0.01;
	repetition_count(matInvL)  = NCOVMAT;
	dimensions(matInvL.in[0]) = {CHOL_DATA_SIZE};
	dimensions(matInvL.in[1]) = {CHOL_DATA_SIZE};
	dimensions(matInvL.out[0]) = {WLEN};
	stack_size(matInvL) = 3000;
	location<kernel>(matInvL) = location<kernel>(matrixadder[2]);
#else
	matInv1 = kernel::create(mat_inv_qrhhr_1);
	matInv2 = kernel::create(mat_inv_qrhhr_2);
    source(matInv1) = "kernels/kernels.cc";
    source(matInv2) = "kernels/kernels.cc";
    runtime<ratio>(matInv1) = 0.6;
    runtime<ratio>(matInv2) = 0.6;
    repetition_count(matInv1) = NCOVMAT;
    repetition_count(matInv2) = NCOVMAT;
//	async_repetition(matInv1.in[0])  = NCOVMAT;
	location<kernel>(matInv1) = tile(21, 4);
	dimensions(matInv1.in[0]) = {WLEN};
	dimensions(matInv1.out[0]) = {8*WLEN};
	dimensions(matInv2.in[0]) = {8*WLEN};
	dimensions(matInv2.out[0]) = {WLEN};
#endif

	// create nets to connect PLIO to covariance kernels

	for(int i=0; i<16; i++) {
	    connect(covprev[i].out[0], covariance[i].in[0]);
	    connect(covcurr[i].out[0], covariance[i].in[1]);
	    connect(covariance[i].out[0], matrixadder[i/8].in[i%8]);
	}

	// Create nets to connect the Covariance adder tree
	connect net48 (matrixadder[0].out[0], matrixadder[2].in[0]);
	connect net49 (matrixadder[1].out[0], matrixadder[2].in[1]);

#ifndef ENABLE_COVACC
	connect net60 (matrixadder[2].out[0], matrixaccumulator.in[0]);
#endif
        //connect net62 (matrixaccumulator.out[0], matInv1.in[0]);
        //connect net63 (matInv1.out[0], matInv2.in[0]);
	//connect net62 (matrixaccumulator.out[0], invInput.in[0]);


#ifdef ENABLE_CH_INV
#ifdef ENABLE_COVACC
	connect net62 (matrixadder[2].out[0], cholInput.in[0]);
#else
	connect net62 (matrixaccumulator.out[0], cholInput.in[0]);
#endif
	connect(cholInput.out[0], chol_lib[0].in[0]);
	connect(cholInput.out[1], chol_lib[0].in[1]);

	for (int i = 0; i < MAT_SIZE - 1; i++)
	{
		connect(chol_lib[i].out[0], chol_lib[i + 1].in[0]);
		connect(chol_lib[i].out[1], chol_lib[i + 1].in[1]);
	}
	connect(chol_lib[MAT_SIZE - 1].out[0], matInvL.in[0]);
	connect(chol_lib[MAT_SIZE - 1].out[1], matInvL.in[1]);
	//connect(matInvL.out[0], o0.in[0]);
#else
#ifdef ENABLE_COVACC
	connect net62 (matrixadder[2].out[0], matInv1.in[0]);
#else
	connect net62 (matrixaccumulator.out[0], matInv1.in[0]);
#endif
	connect net63 (matInv1.out[0], matInv2.in[0]);
#endif

    //connect( matInvQR.out[0], o0.in[0]);
	//connect net62 (matrixadder[2].out[0], invMatOut.in[0]);

  #ifndef INLCUDE_ECI_COMP
	#ifdef ENABLE_CH_INV
		connect net64 (matInvL.out[0], invMatOut.in[0]);
	#else
		connect net64 (matInv2.out[0], invMatOut.in[0]);
	#endif
  #endif

    // specify kernel sources

#ifndef ENABLE_COVACC
    source(matrixaccumulator) = "kernels/kernels.cc";
    runtime<ratio>(matrixaccumulator) = 0.9;
    repetition_count(matrixaccumulator) = TC_PER_COV*NCOVMAT;
    //	async_repetition(matrixaccumulator.out[0])  = TC_PER_COV*NCOVMAT;
#endif

#endif

#ifdef INLCUDE_ECI_COMP
    //kernel optECIMatrix;
    optECIMatrix = kernel::create(optimizedECIMatrix);
    source(optECIMatrix) = "kernels/kernels.cc";
	runtime<ratio>(optECIMatrix) = 0.8;
	//repetition_count(optECIMatrix)  = 1;
	async_repetition(optECIMatrix.in[0]) = 1;
	dimensions(optECIMatrix.in[0]) = {WLEN*9};
    dimensions(optECIMatrix.out[0]) = {2*ECI_MULT_P_DIM_A*2*ECI_MULT_P_DIM_B};
    stack_size(optECIMatrix) = 13312;
    //single_buffer(optECIMatrix.in[0]);
    //single_buffer(optECIMatrix.out[0]);

//	kernel ECIMatTranspose;
	ECIMatTranspose = kernel::create(MatTranspose<40, 40>);
	source(ECIMatTranspose) = "kernels/kernels.cc";
	runtime<ratio>(ECIMatTranspose) = 0.8;
	repetition_count(ECIMatTranspose)  = 1;
    dimensions(ECIMatTranspose.in[0]) = {2*ECI_MULT_P_DIM_A*2*ECI_MULT_P_DIM_B};
    dimensions(ECIMatTranspose.out[0]) = {2*ECI_MULT_P_DIM_A*2*ECI_MULT_P_DIM_B};
    stack_size(ECIMatTranspose) = 13312;
	connect net64 (optECIMatrix.out[0], ECIMatTranspose.in[0]);

	location<kernel>(optECIMatrix) = tile(28,4);
	location<kernel>(ECIMatTranspose) = tile(27,5);


  #ifdef INLCUDE_COV
	#ifdef ENABLE_CH_INV
		connect net65 (matInvL.out[0], optECIMatrix.in[0]);
	#else
		connect net65 (matInv2.out[0], optECIMatrix.in[0]);
	#endif
  #else
	connect net65 (invCovMtx.out[0], optECIMatrix.in[0]);
  #endif

  #ifndef INLCUDE_WEIGHT_APP
	//optimizedECI connect(ECIRowCombiner.out[0], eciMtxColOut.in[0]);
	connect net66 (ECIMatTranspose.out[0], eciMtxColOut.in[0]);
  #endif

#endif


#ifdef INLCUDE_WEIGHT_APP
    // Steering vector kernel
    ksteer = kernel::create(SteeringVectorV2);

    connect<parameter>(normDop,ksteer.in[0]);
    connect<parameter>(normSpa,ksteer.in[1]);

    source(ksteer) = "kernels/kernels.cc";
    runtime<ratio>(ksteer) = 0.3;
	repetition_count(ksteer)  = 1;
	dimensions(ksteer.out[0]) = {NANTENNA*NPULSE};
	location<kernel>(ksteer) = tile(27,4);

	// Weight PLIO interfaces

	for (int i=0; i<4; i++) {
		weightAppIn[i]  = input_plio::create(plio_64_bits, "data/L_" + std::to_string(L) + "/data_from_rdma/rdma_" + std::to_string(i+1) + ".txt", 125);
		weightAppOut[i]  = output_plio::create(plio_64_bits, "data/L_" + std::to_string(L) + "/weightAppOut" + std::to_string(i) + ".txt", 125);
	}
	location<PLIO>(weightAppIn[0]) = shim(28);
	location<PLIO>(weightAppOut[0]) = shim(28);
	location<PLIO>(weightAppIn[1]) = shim(28);
	location<PLIO>(weightAppOut[1]) = shim(28);
	location<PLIO>(weightAppIn[2]) = shim(28);
	location<PLIO>(weightAppOut[2]) = shim(28);
	location<PLIO>(weightAppIn[3]) = shim(27);
	location<PLIO>(weightAppOut[3]) = shim(27);

    // Weight kernels

	kweightcalc = kernel::create(WeightCalculation);
	async_repetition(kweightcalc.out[0])  = 1;

	for(int i=0; i<4; i++) {
		kweightapp[i]  = kernel::create(WeightApplication);
		source(kweightapp[i]) = "kernels/kernels.cc";
		runtime<ratio>(kweightapp[i]) = 0.9;
		async_repetition(kweightapp[i].in[0])  = CPI_PER_WEIGHTAPP;
		dimensions(kweightapp[i].in[0]) = {NANTENNA*NPULSE};
		dimensions(kweightapp[i].in[1]) = {NANTENNA*NPULSE*NBATCH_WEIGHTAPP};
		dimensions(kweightapp[i].out[0]) = {NBATCH_WEIGHTAPP};
		stack_size(kweightapp[i]) = 13312;

		connect(kweightcalc.out[0], kweightapp[i].in[0]);
		connect(weightAppIn[i].out[0], kweightapp[i].in[1]);
		connect(kweightapp[i].out[0], weightAppOut[i].in[0]);
	}

	connect(ksteer.out[0], kweightcalc.in[0]);

  #ifndef INLCUDE_ECI_COMP
	connect(eciMtx.out[0], kweightcalc.in[1]);
  #else
	connect(ECIMatTranspose.out[0], kweightcalc.in[1]);
  #endif

    source(kweightcalc) = "kernels/kernels.cc";
	runtime<ratio>(kweightcalc) = 0.6;
	dimensions(kweightcalc.in[0]) = {NANTENNA*NPULSE};
	dimensions(kweightcalc.in[1]) = {NANTENNA*NPULSE*NANTENNA*NPULSE};
	dimensions(kweightcalc.out[0]) = {NANTENNA*NPULSE};
	location<kernel>(kweightcalc) = location<kernel>(ksteer);
#endif
 }

};

