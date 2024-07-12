/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

#include <adf.h>
#include "include.h"
#include "kernels.h"

#include "aie_api/aie.hpp"
#include "aie_api/aie_adf.hpp"
#include <aie_api/utils.hpp>

/**
 * @brief This kernel calculates the co-variance matrix for a single sheet of the cube.
 * @input concatenated vector data in cint16 format from PL side
 * @output co-variance matrix output single complex floating point format
 */
float get_magic2(int shift)
{
    int magic = (127 + 23 + shift << 23) + (1 << 22);
    return as_float(magic);
};

void fix2float_vectorized2(v8float &out0, v8float &out1, v16int16 in, int shift)
{
    v16float magic = upd_elem(undef_v16float(),0,get_magic2(-shift));
    magic = upd_elem(magic,1,-0.00000);
    v16acc48 acc = msc16(ups(in,0),as_v32int16(magic),0,0,0,4369,0,0,0,13107);
    v8float chess_storage(wd1) acc0 = as_v8float(lsrs(ext_lo(acc),0));
    v8float chess_storage(wd1) acc1 = as_v8float(lsrs(ext_hi(acc),0));
    v8float chess_storage(wr2) out0_l = fpsub(acc0,magic,0,0);
    v8float chess_storage(wr3) out1_l = fpsub(acc1,magic,0,0);
    out0 = out0_l;
    out1 = out1_l;
};

void CovarianceMatrixAcc(
   adf::input_buffer<cint16> & __restrict pulsem,
   adf::input_buffer<cint16> & __restrict pulsem1,
   adf::output_buffer<cfloat> & __restrict out)
{

#ifndef __X86SIM__
  alignas(aie::vector_decl_align) cfloat transpose[8*128];
#else
  thread_local cfloat transpose[8*128];
#endif
    cfloat* __restrict transposePtr = transpose;
    auto transposeIter1 = aie::begin_vector<4>(transposePtr);

    aie::vector<cint16,2*4> data;
    //Step 1 read NANTENNA elem complex cint16 vector from the previous and current pulses
    auto InIter0 = aie::begin_vector<4>(pulsem);
    auto InIter1 = aie::begin_vector<4>(pulsem1);

   for(int k=0; k<125; k++)
   chess_prepare_for_pipelining
   {
	   data.insert(0, *InIter0++);
	   data.insert(1, *InIter1++);
	   //Step 2 convert cint16 to complex float vector
	   v16int16 *ptr3 = (v16int16*)&data;
	   v8float out0, out1;
	   fix2float_vectorized2(out0, out1, *ptr3, 0);
	   cfloat *ptr1 = (cfloat*)&out0;
	   cfloat *ptr2 = (cfloat*)&out1;
	   aie::vector<cfloat,8> cflvec;
	   auto iter0 = aie::begin_vector<4>(ptr1);
	   auto iter1 = aie::begin_vector<4>(ptr2);

	   auto v4In0 = *iter0;
	   auto v4In1 = *iter1;
	   for (int j=0; j<4; j++) {
		   transpose[128*j+k] = v4In0.get(j);
		   transpose[128*(4+j)+k] = v4In1.get(j);
		}
   }
   for(int k=125; k<128; k++)
   chess_prepare_for_pipelining
   {
	   aie::vector<cfloat,4> v4In0 = null_v4cfloat();
	   aie::vector<cfloat,4> v4In1 = null_v4cfloat();
	   for (int j=0; j<4; j++) {
		   transpose[128*j+k] = v4In0.get(j);
		   transpose[128*(4+j)+k] = v4In1.get(j);
		}
   }

	auto OutIter = aie::begin(out);
	for(int j=0; j<8*8; j+=2)
	chess_prepare_for_pipelining
	{
			auto bufIter3 = aie::begin_vector<4>(transposePtr+int(j/8)*128);
			auto bufIter4 = aie::begin_vector<4>(transposePtr+(j%8)*128);
			auto bufIter5 = bufIter4 + 128/4;
			aie::accum<caccfloat,4> acc1=aie::zeros<caccfloat,4>();
			aie::accum<caccfloat,4> acc2=aie::zeros<caccfloat,4>();

			for(int k=0; k<128/4; k++)
			chess_prepare_for_pipelining
			{
				acc1 = aie::mac(acc1,*bufIter3,aie::op_conj(*bufIter4++));
				acc2 = aie::mac(acc2,*bufIter3++,aie::op_conj(*bufIter5++));
			}

			auto dotp0 = aie::reduce_add(acc1.to_vector<cfloat>(0));
			*OutIter++ = dotp0;
			auto dotp1 = aie::reduce_add(acc2.to_vector<cfloat>(0));
			*OutIter++ = dotp1;
   }
 }


void CovarianceMatrix(
	adf::input_buffer<cint16> & __restrict pulsem,
	adf::input_buffer<cint16> & __restrict pulsem1,
	adf::output_buffer<cfloat> & __restrict out)
{
//printf("Enter CovarianceMatrix \n");
	aie::vector<cint16,2*NANTENNA> data;

	//Step 1 read NANTENNA elem complex cint16 vector from the previous and current pulses
	auto InIter0 = aie::begin_vector<4>(pulsem);
	auto InIter1 = aie::begin_vector<4>(pulsem1);
	auto OutIter = aie::begin_vector<8> (out);

	data.insert(0, *InIter0++);
	data.insert(1, *InIter1++);

	//Step 2 convert cint16 to complex float vector
	//aie::vector<cfloat,8> cflvec = aie::to_float(data,0);
	v16int16 *ptr3 = (v16int16*)&data;
	v8float out0, out1;
	fix2float_vectorized2(out0, out1, *ptr3, 0);

	cfloat *ptr1 = (cfloat*)&out0;
	cfloat *ptr2 = (cfloat*)&out1;

	aie::vector<cfloat,8> cflvec;
	auto iter0 = aie::begin_vector<4>(ptr1);
	auto iter1 = aie::begin_vector<4>(ptr2);

	cflvec.insert(0, *iter0);
	cflvec.insert(1, *iter1);

	//Step 3 covariance calc
	aie::vector<cfloat,8> va=aie::conj(cflvec);
	aie::vector<cfloat,8> vb=cflvec;

	for(int j=0; j<8; j++)
	chess_prepare_for_pipelining
	{
#ifdef COV_TRANSPOSE_OUTPUT
		// Swapping this way produces a transpose of the product, which is required by the matrix inversion implementation.
		auto vt1=aie::mul(vb[j],va);
#else
		auto vt1=aie::mul(va[j],vb);
#endif

		*OutIter++ = vt1.to_vector<cfloat>(0);
	}
	//printf("Exit CovarianceMatrix \n");

}

/**
 * @brief This kernel adds the covariance outputs
 * @input matA
 * @input matB
 * @output matC
 */
void VectorAdd2(
		adf::input_buffer<cfloat> & __restrict matA,
		adf::input_buffer<cfloat> & __restrict matB,
		adf::output_buffer<cfloat> & __restrict matC)
{
	aie::vector<cfloat, 4> va;
	aie::vector<cfloat, 4> vb;
	aie::vector<cfloat, 4> vc;

	auto InIter0 = aie::begin_vector<4>(matA);
	auto InIter1 = aie::begin_vector<4>(matB);
	auto OutIter = aie::begin_vector<4>(matC);


	for(unsigned i=0;i<16;i++)
	chess_prepare_for_pipelining
	{
		va = *InIter0++;
		vb = *InIter1++;

		vc = aie::add(va, vb);
		*OutIter++ = vc;
	}
}

void VectorAdd8(
		adf::input_buffer<cfloat> & __restrict matA1,
		adf::input_buffer<cfloat> & __restrict matB1,
		adf::input_buffer<cfloat> & __restrict matA2,
		adf::input_buffer<cfloat> & __restrict matB2,
		adf::input_buffer<cfloat> & __restrict matA3,
		adf::input_buffer<cfloat> & __restrict matB3,
		adf::input_buffer<cfloat> & __restrict matA4,
		adf::input_buffer<cfloat> & __restrict matB4,
		adf::output_buffer<cfloat> & __restrict matC)
{
	aie::vector<cfloat, 4> va;
	aie::vector<cfloat, 4> vb;
	aie::vector<cfloat, 4> vc1;
	aie::vector<cfloat, 4> vc2;
	aie::vector<cfloat, 4> vc3;
	aie::vector<cfloat, 4> vc4;
	aie::vector<cfloat, 4> vc5;
	aie::vector<cfloat, 4> vc6;
	aie::vector<cfloat, 4> vc;

	auto InIter0 = aie::begin_vector<4>(matA1);
	auto InIter1 = aie::begin_vector<4>(matB1);
	auto InIter2 = aie::begin_vector<4>(matA2);
	auto InIter3 = aie::begin_vector<4>(matB2);
	auto InIter4 = aie::begin_vector<4>(matA3);
	auto InIter5 = aie::begin_vector<4>(matB3);
	auto InIter6 = aie::begin_vector<4>(matA4);
	auto InIter7 = aie::begin_vector<4>(matB4);
	auto OutIter = aie::begin_vector<4>(matC);

	for(unsigned i=0;i<16;i++)
	chess_prepare_for_pipelining
	{
		va = *InIter0++;
		vb = *InIter1++;
		vc1 = aie::add(va, vb);
		va = *InIter2++;
		vb = *InIter3++;
		vc2 = aie::add(va, vb);
		va = *InIter4++;
		vb = *InIter5++;
		vc3 = aie::add(va, vb);
		va = *InIter6++;
		vb = *InIter7++;
		vc4 = aie::add(va, vb);
		vc5 = aie::add(vc1,vc2);
		vc6 = aie::add(vc3,vc4);
		vc = aie::add(vc5, vc6);
		*OutIter++ = vc;
	}
}

/**
 * @brief This kernel accumulates the covariance outputs
 * @input matA
 * @input matB
 * @output matC
 */

#ifndef ENABLE_COVACC
void VectorAcc(
		adf::input_buffer<cfloat> & __restrict matA,
		adf::output_async_buffer<cfloat> & __restrict matC)
{
	//printf("Enter VectorAcc \n");

#ifndef __X86SIM__
  static int itr = 0;
	static aie::vector<cfloat, 4> acc1[8];
	static aie::vector<cfloat, 4> acc2[8];
#else
  static thread_local int itr = 0;
	static thread_local aie::vector<cfloat, 4> acc1[8];
	static thread_local aie::vector<cfloat, 4> acc2[8];
#endif

	aie::vector<cfloat, 4> va;
	aie::vector<cfloat, 4> vb;

	auto InIter0 = aie::begin_vector<4>(matA);

	if (itr == 0) {
		for(int i=0; i<8; i++)
		chess_prepare_for_pipelining
		{
			// Clear the accumulator
			acc1[i] = null_v4cfloat();
			acc2[i] = null_v4cfloat();
		}
	}

	for (int i=0; i<8; i++)
	chess_prepare_for_pipelining
	{
		va = *InIter0++;
		acc1[i] = aie::add(acc1[i], va);
		vb = *InIter0++;
		acc2[i] = aie::add(acc2[i], vb);
	}

	if (itr == COV_ACC_N_ITR-1) {
		// Last matrix to add, acquire output window to output the result
		matC.acquire();
		auto OutIter = aie::begin_vector<4>(matC);

		for(int i=0; i<8; i++)
		chess_prepare_for_pipelining
		{
			// Last matrix to add, output the result
			*OutIter++ = acc1[i];
			*OutIter++ = acc2[i];
		}
		matC.release();
		itr = 0;
	}
	else {
		itr++;
	}
	//printf("Exit VectorAcc \n");

}
#endif

void CholInDataPopulator(
		adf::input_buffer<cfloat> & __restrict matA,
		adf::output_buffer<float> & __restrict AR,
		adf::output_buffer<float> & __restrict AI)
{
	cfloat a = {0.0,0.0};
	cfloat dim = {CHOL_N_COLUMNS, 0};
	cfloat pid = {0, 0};

	auto outIter1=aie::begin(AR);
	auto outIter2=aie::begin(AI);

	//Update header with dimension and pid data
	*outIter1++ = dim.real; //writeincr(AR, dim.real);
	*outIter2++ = dim.imag; //writeincr(AI, dim.imag);
	*outIter1++ = pid.real; //writeincr(AR, pid.real);
	*outIter2++ = pid.imag; //writeincr(AI, pid.imag);
	cfloat* __restrict matA_t = matA.data();

	for(int row=0; row < 8; row++)
	{
		for (int col=row; col < 8; col++)
		{
			a = *(matA_t+(row*8)+col);
			*outIter1++ = a.real; //writeincr(AR, a.real);
			*outIter2++ = a.imag; //writeincr(AI, a.imag);
		}
	}
}

void cholesky_complex_buffer(
		adf::input_buffer<float> & __restrict matA_real,
		adf::input_buffer<float> & __restrict matA_imag,
		adf::output_buffer<float> & __restrict matL_real,
		adf::output_buffer<float> & __restrict matL_imag)
{
    aie::vector<float, 4> L_ij_real;
    aie::vector<float, 4> L_ij_imag;
    aie::vector<float, 4> A_ik_real;
    aie::vector<float, 4> A_ik_imag;
    aie::accum<accfloat, 4> Acc_ik_real;
    aie::accum<accfloat, 4> Acc_ik_imag;

    float info0_real, info0_imag;
    float info1_real, info1_imag;
    float val_real, val_imag;
    float lij_real, lij_imag;
    float lkj_real, lkj_imag;
    float ajj_real, ajj_imag;
    float aik_real, aik_imag;
    float diag;

    float outCols_real[64];
    float outCols_imag[64];

    auto pIn1  = aie::begin(matA_real);
    auto pIn2  = aie::begin(matA_imag);
    auto pOut1  = aie::begin(matL_real);
    auto pOut2  = aie::begin(matL_imag);

    int dim, pid;
    int num, cnt;
    info0_real = *pIn1++;//READINCR(SS_rsrc1, matA_real);
    info0_imag = *pIn2++;//READINCR(SS_rsrc2, matA_imag);
    dim = info0_real;
    num = info0_imag;
    info1_real = *pIn1++;//READINCR(SS_rsrc1, matA_real);
    info1_imag = *pIn2++;//READINCR(SS_rsrc2, matA_imag);
    pid = info1_real;
    cnt = info1_imag;
    info1_real = pid + 1;
    info0_imag = num + dim - pid;
    *pOut1++ = info0_real; //WRITEINCR(MS_rsrc1, matL_real, info0_real.i); // dim
    *pOut2++ = info0_imag; //WRITEINCR(MS_rsrc2, matL_imag, info0_imag.i); // num = num + dim -pid
    *pOut1++ = info1_real; //WRITEINCR(MS_rsrc1, matL_real, info1_real.i); // pid = pid + 1
    *pOut2++ = info1_imag; //WRITEINCR(MS_rsrc2, matL_imag, info1_imag.i); // cnt


    for (int i = 0; i < num; i++) chess_prepare_for_pipelining {
            val_real = *pIn1++; //READINCR(SS_rsrc1, matA_real);
            val_imag = *pIn2++; //READINCR(SS_rsrc2, matA_imag);
            *pOut1++ = val_real; //WRITEINCR(MS_rsrc1, matL_real, val_real.i);
            *pOut2++ = val_imag; //WRITEINCR(MS_rsrc2, matL_imag, val_imag.i);
        }
    // calculate diag element
    ajj_real = *pIn1++;//READINCR(SS_rsrc1, matA_real);
    ajj_imag = *pIn2++;//READINCR(SS_rsrc2, matA_imag);
    diag = aie::sqrt(ajj_real);
    float invDiag = aie::inv(diag);
    ajj_real = diag;
    ajj_imag = 0;
    *pOut1++ = diag; //WRITEINCR(MS_rsrc1, matL_real, diag.i);
    *pOut2++ = 0; //WRITEINCR(MS_rsrc2, matL_imag, 0);
    outCols_real[pid] = diag;
    outCols_imag[pid] = 0;

    // calculate other column elements
    for (int i = pid + 1; i < dim; i++) chess_prepare_for_pipelining {
            lij_real  = *pIn1++; //READINCR(SS_rsrc1, matA_real);
            lij_imag = *pIn2++; //READINCR(SS_rsrc2, matA_imag);
            lij_real = lij_real * invDiag;
            lij_imag = lij_imag * invDiag;
            *pOut1++ = lij_real;//WRITEINCR(MS_rsrc1, matL_real, lij_real.i);
            *pOut2++ = lij_imag; //WRITEINCR(MS_rsrc2, matL_imag, lij_imag.i);
            outCols_real[i] = lij_real;
            outCols_imag[i] = lij_imag;
        }
    // update the rest of matA elements
    int k = pid + 1;
    for (; k < dim; k++) {
        lkj_real = outCols_real[k];
        lkj_imag = -(outCols_imag[k]);
        int len = (dim - k) % 4;
        for (int q = k; q < (len + k); q++) chess_prepare_for_pipelining {
                lij_real = outCols_real[q];
                lij_imag = outCols_imag[q];
                aik_real = *pIn1++; //READINCR(SS_rsrc1, matA_real);
                aik_imag = *pIn2++; //READINCR(SS_rsrc2, matA_imag);
                aik_real = aik_real - lij_real * lkj_real + lij_imag * lkj_imag;
                aik_imag = aik_imag - lij_real * lkj_imag - lij_imag * lkj_real;
                *pOut1++ = aik_real;//WRITEINCR(MS_rsrc1, matL_real, aik_real.i);
                *pOut2++ = aik_imag;//WRITEINCR(MS_rsrc2, matL_imag, aik_imag.i);
            }

        for (int i = k + len; i < dim - 3; i += 4) chess_prepare_for_pipelining {
                L_ij_real = aie::load_v<4>(outCols_real + i);
                L_ij_imag = aie::load_v<4>(outCols_imag + i);
                A_ik_real = upd_elem(A_ik_real, 0, *pIn1++);
                A_ik_real = upd_elem(A_ik_real, 1, *pIn1++);
                A_ik_real = upd_elem(A_ik_real, 2, *pIn1++);
                A_ik_real = upd_elem(A_ik_real, 3, *pIn1++);
                A_ik_imag = upd_elem(A_ik_imag, 0, *pIn2++);
                A_ik_imag = upd_elem(A_ik_imag, 1, *pIn2++);
                A_ik_imag = upd_elem(A_ik_imag, 2, *pIn2++);
                A_ik_imag = upd_elem(A_ik_imag, 3, *pIn2++);

                Acc_ik_real.from_vector(A_ik_real);
                Acc_ik_imag.from_vector(A_ik_imag);
                Acc_ik_real = aie::msc(Acc_ik_real, L_ij_real, lkj_real);
                Acc_ik_real = aie::mac(Acc_ik_real, L_ij_imag, lkj_imag);
                Acc_ik_imag = aie::msc(Acc_ik_imag, L_ij_real, lkj_imag);
                Acc_ik_imag = aie::msc(Acc_ik_imag, L_ij_imag, lkj_real);
                A_ik_real = Acc_ik_real.to_vector<float>();
                A_ik_imag = Acc_ik_imag.to_vector<float>();
                *pOut1++ = A_ik_real.get(0);
                *pOut1++ = A_ik_real.get(1);
                *pOut1++ = A_ik_real.get(2);
                *pOut1++ = A_ik_real.get(3);
                *pOut2++ = A_ik_imag.get(0);
                *pOut2++ = A_ik_imag.get(1);
                *pOut2++ = A_ik_imag.get(2);
                *pOut2++ = A_ik_imag.get(3);
            }
    }
} // end cholesky_complex_buffer

void mat_inv_L(
		adf::input_buffer<float> & __restrict LR,
		adf::input_buffer<float> & __restrict LI,
		adf::output_buffer<cfloat> & __restrict Inv)
{
	aie::vector<cfloat, 8> v1;  	// Temporary vector for intermediate values
	aie::vector<cfloat, 8> v2;   	// Temporary vector for intermediate values
	aie::vector<cfloat, 8> y;   	// Temporary vector for intermediate values

	alignas(aie::vector_decl_align) cfloat L_INV[8*8];// = {{ 0.0, 0.0 }};
	alignas(aie::vector_decl_align) cfloat L_BUF[8*8];

	cfloat * Lptr = L_BUF;		        	// pointer to L matrix
	cfloat * L_INVi;						// row pointer of L_INV matrix
	cfloat * L_INVj;						// row pointer of L_INV matrix
	cfloat * Lji;							// pointer to L[j, i] element
	cfloat * LCptr = L_BUF;    		    // pointer of LC matrix
	cfloat * Lj;    		        		// row pointer of QT matrix
	cfloat scale;

	cfloat* __restrict LIptr = L_INV;
	auto matIter = aie::begin_vector<8>(LIptr);
	for(int i=0; i<8; i++)
	chess_prepare_for_pipelining
	{
		*matIter++ = null_v8cfloat();
	}
	auto inIter1 = aie::begin(LR);
	auto inIter2 = aie::begin(LI);
	auto OutIter = aie::begin_vector<8>(Inv);

	cfloat a = {0.0,0.0};
	cfloat dim = {0, 0};
	cfloat pid = {0, 0};
	dim.real = *inIter1++;//readincr(LR);
	dim.imag = *inIter2++;//readincr(LI);
	pid.real = *inIter1++;//readincr(LR);
	pid.imag = *inIter2++;//readincr(LI);

	for(int row=0, k=0; row < 8; row++)
	{
		for (int col=0; col < 8; col++)
		{
			if(col >= row)
			{
				a.real = *inIter1++;//readincr(LR);
				a.imag = *inIter2++;//readincr(LI);
			}
			else
			{
				a.real = 0.0;
				a.imag = 0.0;
			}
			L_BUF[8*(col)+row] = a;
		}
	}

	// Inversion of L (lower-triangular matrix)
	Lji = L_BUF;
	L_INVj = L_INV;
	for (int j=0; j<8; j++) {

		L_INVi = L_INV;
		*(L_INVj + j) = reciprocal(*(Lji + j));

		for (int i=0; i<j; i++) {
			// Perform the row multiplication on all row elements even if some
			// are 0's to use vector by vector multiplications.
			v1 = aie::load_v(L_INVi);
			v2 = aie::load_v(Lji);
			y = aie::mul(v1, v2);
			scale = aie::reduce_add(y);

			*(L_INVi + j) = -1 * scale * *(L_INVj + j);
			L_INVi += 8;
		}
		L_INVj += 8;
		Lji += 8;
	}

	Lptr = L_INV;
	LCptr = L_INV;
	for (int i=0; i<8; i++) {
		Lj = Lptr;
		y = aie::load_v(LCptr);
		for (int j=0; j<8; j++) {
			v2 = aie::load_v(Lj);
			v2 = aie::mul(aie::op_conj(y), v2);
			v1.set(aie::reduce_add(v2), j);
			Lj += 8;
		}
		// Send the inverted matrix, row by row.
		*OutIter++ = v1;
		LCptr  += 8;
	}
}

void mat_inv_qrhhr_1(
		adf::input_buffer<cfloat> & __restrict AT,
		adf::output_buffer<cfloat> & __restrict out)
{
	aie::vector<cfloat, MAT_SIZE> v1;  		// Temporary vector for intermediate values
	aie::vector<cfloat, MAT_SIZE> y;   	    // Temporary vector for intermediate values
	aie::vector<cfloat, MAT_SIZE> y_null = null_v8cfloat();

	alignas(aie::vector_decl_align) cfloat H[MAT_SIZE*MAT_SIZE];

	// Two matrix buffers are share to store the previous RT and
	// the new computed RT.
	// MAT_BUF_1 is initially used to store RT
	//

	alignas(aie::vector_decl_align) cfloat MAT_BUF_1[MAT_SIZE*MAT_SIZE];
	alignas(aie::vector_decl_align) cfloat MAT_BUF_2[MAT_SIZE*MAT_SIZE];

	cfloat * RTptr = MAT_BUF_1;				// pointer to RT matrix
	cfloat * freeBufPtr = MAT_BUF_2;		// pointer to free matrix buffer
	cfloat * cfloatTmpPtr;				    // temporary pointer

	cfloat * RTji = MAT_BUF_1;				// pointer to RT[j, i] element

	cfloat * Hji = H;			// pointer to H[j, i] element

	float normVal;

	int first_idx = 0;

	auto InIter = aie::begin_vector<MAT_SIZE>(AT);
	auto OutIter = aie::begin_vector<MAT_SIZE>(out);

	// We assume the input matrix (A) is received already transposed.
	// Would need to transpose it if not.
	//

	// QR Decomposition using the Householder Method
	// This step produces the two matrices Q and RT (R').
	//
	for (int j=0; j<MAT_SIZE; j++) {

		// Initialization of RT with AT
		//
		v1 = *InIter++;
		aie::store_v(RTji, v1);
		RTji+=MAT_SIZE;
	}

// print_matrix(RTptr, MAT_SIZE, MAT_SIZE, "K1: Two pulse covariance matrix");

    for (int j=0; j<MAT_SIZE-1; j++) {

    	RTji = RTptr + j*MAT_SIZE;
    	v1 = aie::load_v(RTji);
		y = aie::shuffle_down_fill(v1, y_null, j);
		y[0] = y[0] + sign(RTji[j]) * norm(y);
//aie::print(y,true,"y=");
		normVal = inv_norm(y);
		v1 = aie::mul(y, normVal);
//aie::print(v1,true,"v1=");

		Hji = H + first_idx*MAT_SIZE; // Start of the first row

        for (int row=first_idx; row<MAT_SIZE; row++) {

        	Hji += first_idx; // Start of the first column
		    for (int col=first_idx; col<MAT_SIZE; col++) {
	            // First fill with Identity Matrix
	            if (row == col)
	             	*Hji = {1.0, 0.0};
	            else
	               	*Hji = {0.0, 0.0};

	            if (j <= row && j <= col) {
	                *Hji = *Hji -2 * v1[row-j] * aie::conj(v1[col-j]);
	            }
	            Hji++;
            }
        }

    	first_idx = j;

        // Calculating new matrix RT using H * RT
        // RT = (H*(RT.')).';
        //
        Hji = H;
        mat_mult_A_BT(Hji, RTptr, freeBufPtr);
        cfloatTmpPtr = RTptr;
        RTptr = freeBufPtr;
        freeBufPtr = cfloatTmpPtr;

		// Transmit the intermediate HT Matrices
		//
		for (int i=0; i<MAT_SIZE; i++) {
			v1 = aie::load_v(Hji);
printf("%.5e\n", (float) v1.get(0).real);
			*OutIter++ = v1;
			Hji += MAT_SIZE;
		}

    }

//print_matrix(RTptr, MAT_SIZE, MAT_SIZE, "K1: RT matrix");

    // Transmit the R Matrix
    //
	for (int i=0; i<MAT_SIZE; i++) {
		v1 = aie::load_v(RTptr);
		*OutIter++ = v1;
		RTptr  += MAT_SIZE;
	}

}


void mat_inv_qrhhr_2(
		adf::input_buffer<cfloat> & __restrict QR,
		adf::output_buffer<cfloat> & __restrict out)
{
	aie::vector<cfloat, MAT_SIZE> v1;  		// Temporary vector for intermediate values
	aie::vector<cfloat, MAT_SIZE> v2;   	// Temporary vector for intermediate values
	aie::vector<cfloat, MAT_SIZE> y;   	    // Temporary vector for intermediate values

	alignas(aie::vector_decl_align) cfloat R_INV[MAT_SIZE*MAT_SIZE] = {{ 0.0, 0.0 }};
	alignas(aie::vector_decl_align) cfloat RT[MAT_SIZE*MAT_SIZE];

	// MAT_BUF_0 is initially used to store QT
	//
	alignas(aie::vector_decl_align) cfloat MAT_BUF_0[MAT_SIZE*MAT_SIZE] = { // eye(MAT_SIZE), hard coded to MAT_SIZE = 8;
			{1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},
			{0.0, 0.0}, {1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},
			{0.0, 0.0}, {0.0, 0.0}, {1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},
			{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},
			{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},
			{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {1.0, 0.0}, {0.0, 0.0}, {0.0, 0.0},
			{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {1.0, 0.0}, {0.0, 0.0},
			{0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {0.0, 0.0}, {1.0, 0.0}
	};
	alignas(aie::vector_decl_align) cfloat MAT_BUF_1[MAT_SIZE*MAT_SIZE];

	cfloat * QTptr = MAT_BUF_0;		        // pointer to QT matrix
	cfloat * freeBufPtr = MAT_BUF_1;		// pointer to free matrix buffer
	cfloat * cfloatTmpPtr;				    // temporary pointer

	cfloat * RTptr = RT;					// pointer to RT matrix

	cfloat * R_INVi;						// row pointer of R_INV matrix
	cfloat * R_INVj;						// row pointer of R_INV matrix
	cfloat * RTji;							// pointer to RT[j, i] element
	cfloat * HTij = RT;						// pointer to HT matrix
	cfloat * QTj;    		        		// row pointer of QT matrix

	cfloat scale;

	auto InIter = aie::begin_vector<MAT_SIZE>(QR);
	auto OutIter = aie::begin_vector<MAT_SIZE>(out);

	for (int j=0; j<MAT_SIZE-1; j++) {

		// Get HT matrices to compute QT
		//
		HTij = RT; // use the RT space
		for (int j=0; j<MAT_SIZE; j++) {
			v1 = *InIter++;
			aie::store_v(HTij, v1);
			HTij+=MAT_SIZE;
		}
// print_matrix(RT, MAT_SIZE, MAT_SIZE, "K2: HT matrix");

		// Calculating orthogonal matrix, Q using Q=H1*H2*....*Hn
		// Q = Q*H;
		// QT = HT * QT;
		//
		mat_mult_AT_BT(RT, QTptr, freeBufPtr);
		cfloatTmpPtr = QTptr;
		QTptr = freeBufPtr;
		freeBufPtr = cfloatTmpPtr;
	}

	// Get RT matrix
	//
	RTji = RT;
	for (int j=0; j<MAT_SIZE; j++) {

		// Initialization of RT
		//
		v1 = *InIter++;
		aie::store_v(RTji, v1);
		RTji+=MAT_SIZE;
	}

//print_matrix(QTptr, MAT_SIZE, MAT_SIZE, "K2: QT matrix");
//print_matrix(RT, MAT_SIZE, MAT_SIZE, "K2: RT matrix");

	// Inversion of RT (lower-triangular matrix)
	//
	RTji = RT;
	R_INVj = R_INV;

	for (int j=0; j<MAT_SIZE; j++) {

		R_INVi = R_INV;
		*(R_INVj + j) = reciprocal(*(RTji + j));

		for (int i=0; i<j; i++) {
	        // Perform the row multiplication on all row elements even if some
	        // are 0's to use vector by vector multiplications.
			v1 = aie::load_v(R_INVi);
			v2 = aie::load_v(RTji);
			y = aie::mul(v1, v2);
			scale = aie::reduce_add(y);

			*(R_INVi + j) = -1 * scale * *(R_INVj + j);
			R_INVi += MAT_SIZE;
		}
		R_INVj += MAT_SIZE;
		RTji += MAT_SIZE;

	}

//print_matrix(R_INV, MAT_SIZE, MAT_SIZE, "R_INV matrix");

	// Final Matrix multiplication R_INV * conj(QT)
	//

	R_INVi = R_INV;

	for (int i=0; i<MAT_SIZE; i++) {
		QTj = QTptr;
		y = aie::load_v(R_INVi);
		for (int j=0; j<MAT_SIZE; j++) {
			v2 = aie::load_v(QTj);
			v2 = aie::mul(y, aie::op_conj(v2));
			v1.set(aie::reduce_add(v2), j);
			QTj += MAT_SIZE;
		}
		// Send the inverted matrix, row by row.
		//
		*OutIter++ = v1;
		R_INVi  += MAT_SIZE;
	}

}

void optimizedECIMatrix(
		adf::input_async_buffer<cfloat> & __restrict matRi,
	    adf::output_buffer<cfloat> & __restrict matOut)
{
	matRi.acquire();
	int matDim = NANTENNA*NPULSE;
	alignas(aie::vector_decl_align) cfloat matBuf[NANTENNA*NPULSE*NANTENNA*NPULSE]; // = {{0.0,0.0}};

    cfloat* __restrict matptr = matBuf;
    //printf("inside optECI, matIdx = %d",matIdx);
	auto matIter = aie::begin_vector<8>(matptr);
	for(int i=0; i<matDim*matDim/8; i++)
	chess_prepare_for_pipelining
	{
		*matIter++ = null_v8cfloat();
	}

	auto InIter  = aie::begin_vector<8>(matRi);

	for(int k=0; k<9; k++)
	chess_prepare_for_pipelining
	{
		auto BufIter = aie::begin_vector<8> (matptr+(matDim*NANTENNA+NANTENNA)*k);
		for(int row = 0; row<2*NANTENNA; row++)
		{
			*BufIter = aie::add(*BufIter, *InIter++);
			BufIter += (matDim/8);	// jump to next row, at the same column
		}
	}
	auto InIter1 = aie::begin_vector<8>(matptr);
	auto OutIter = aie::begin_vector<8>(matOut);
	for(int i=0; i<matDim*matDim/8; i++)
	chess_prepare_for_pipelining
	{
		*OutIter++ = *InIter1++;
	}
	matRi.release();
}
/**
 * MatTranspose function transposes the input matrix.
 * This function assumes input matrix sizes of NRows x NCols.
 *
 */
template <int NRows, int NCols>
void MatTranspose(
		adf::input_buffer<cfloat> & __restrict matIn,
		adf::output_buffer<cfloat> & __restrict matOut)
{
	aie::vector<cfloat, 4> v4In = null_v4cfloat();
	aie::vector<cfloat, 8> v8 = null_v8cfloat();

	auto InIter = aie::begin_vector<4>(matIn);
	auto OutIter = aie::begin_vector<8>(matOut);

	alignas(aie::vector_decl_align) cfloat matTrans[NRows*NCols];
	cfloat * matTransPtr = matTrans;

	for(int row=0; row < NRows; row++) {
		for (int col=0; col < NCols; col+=4) {
			v4In = *InIter++;
//			aie::print(v4In,true,"v4In=");

			for (int j=0; j<4; j++) {
				// Transpose and copy the value
//				printf("write %9.8e %9.8ei at idx %d\r\n", (float) (v4In.get(j).real), (float) (v4In.get(j).imag), NCols*(col+j)+row);
				matTrans[NRows*(col+j)+row] = v4In.get(j);
			}
		}
	}
	// Copy the transposed matrix to the output window
	for(int elm = 0; elm < NCols*NRows; elm+=8) {
		v8 = aie::load_v(matTransPtr);
		matTransPtr+=8;
//		aie::print(v8,true,"v8=");
		*OutIter++ = v8;
	}
}





constexpr float     _PI= 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348;
constexpr float _TWO_PI= 6.2831853071795864769252867665590057683943387987502116419498891846156328125724179972560696;

/**
 * Steering vector function calculates the steering vector based on normalized parameters
 * received via RTP to the AI SteeringVector kernel
 *
 * @param normDop Normalized doppler frequency
 * @param normSpa Normalized spatial constant
 * @return outvec complex floating point vector repreenting seeting kernel with dimensions of 40x1
 */
void SteeringVector( output_window_cfloat * out, float normDop, float normSpa) {

	//AI function needs mapping from -inf to +inf radians to -p to p radians.

	//Step 1 Calculate tp

	aie::vector<float,4> spvec;

	for(unsigned i=0; i<NANTENNA; i++)
	{
		spvec[i]= -2*_PI*(i)*normSpa;
	}
	spvec[3] = spvec[3] - 2*_PI;		//bring the 3rd element in -pi/pi range, this is done for default values, add check for generic
//	aie::print(spvec,true,"spdata=");

	aie::vector<cfloat,4> sp = aie::sincos_complex(spvec);
//    aie::print(sp,true,"sp=");

	//Step 2 Calculate sp

	aie::vector<float,16> tpvec;

	for(unsigned i=0; i<NPULSE; i++)
	{
		tpvec[i]= 2*_PI*(i)*normDop;
		if(tpvec[i] > _PI){
			tpvec[i] = tpvec[i] -2*_PI;
		}
	}
	tpvec[9] = tpvec[9] -2*_PI;
//    aie::print(tpvec,true,"tpdata=");

	aie::vector<cfloat,16> tp = aie::sincos_complex(tpvec);
//    aie::print(tp,true,"tp=");


	//Step 3 Calculate Kronecker product
    float scaling = 0.025;

        for (int j = 0; j < NPULSE; j++) {
                auto kp=aie::mul(tp[j],sp);
        		//aie::print(kp.to_vector<cfloat>(0),true,"\r\n kp=");

        		aie::vector<cfloat,4> kproworder = kp.to_vector<cfloat>(0);

        		auto kp2 =aie::mul(kproworder,scaling);
//        		aie::print(kp2.to_vector<cfloat>(0),true,"scaled kp=");

        		window_writeincr(out,  kp2.to_vector<cfloat>(0));
        }
}

/**
 * Templateized floating modulo implementation
 * Floating-point modulo
 * The result (the remainder) has same sign as the divisor.
 * Similar to matlab's mod(); Not similar to fmod() -   Mod(-3,4)= 1   fmod(-3,4)= -3
 * @param x start of interval
 * @param y end of interval
 * @return mod floating point modulo
 */
template<typename T>
T Mod(T x, T y)
{
    static_assert(!std::numeric_limits<T>::is_exact , "Mod: floating-point type expected");

    if (0. == y)
        return x;

    float m= x - y * floor(x/y);

    // handle boundary cases resulted from floating-point cut off:

    if (y > 0)              // modulo range: [0..y)
    {
        if (m>=y)           // Mod(-1e-16             , 360.    ): m= 360.
            return 0;

        if (m<0 )
        {
            if (y+m == y)
                return 0  ; // just in case...
            else
                return y+m; // Mod(106.81415022205296 , _TWO_PI ): m= -1.421e-14
        }
    }
    else                    // modulo range: (y..0]
    {
        if (m<=y)           // Mod(1e-16              , -360.   ): m= -360.
            return 0;

        if (m>0 )
        {
            if (y+m == y)
                return 0  ; // just in case...
            else
                return y+m; // Mod(-106.81415022205296, -_TWO_PI): m= 1.421e-14
        }
    }

    return m;
}


/**
 * wrap [rad] angle to [-PI..PI)
 *
 * @param normDop floating point angle
 * @return mod angle in -pi/pi interval
 */
inline float WrapAngletoPI(float fAng)
{
    return Mod(fAng + _PI, _TWO_PI) - _PI;
}

/**
 * Steering vector functions calculates the steering vector based on normalized parameters
 * received via RTP to the AI SteeringVector kernel
 *
 * @param normDop Normalized doppler frequency
 * @param normSpa Normalized spatial constant
 * @return outvec complex floating point vector repreenting seeting kernel with dimensions of 40x1
 */
void SteeringVectorV2(
		adf::output_buffer<cfloat> & __restrict out,
		float normDop, float normSpa)
{
	//AI function needs mapping from -inf to +inf radians to -p to p radians.

	//Step 1 Calculate tp

	aie::vector<float,4> spvec;
	auto OutIter = aie::begin_vector<4> (out);

	for(unsigned i=0; i<NANTENNA; i++)
	{
		spvec[i]= WrapAngletoPI(-2*_PI*(i)*normSpa); //bring the elements in -pi/pi range
	}
//	aie::print(spvec,true,"spdata=");

	aie::vector<cfloat,4> sp = aie::sincos_complex(spvec);
//    aie::print(sp,true,"sp=");

	//Step 2 Calculate sp

	aie::vector<float,16> tpvec;

	for(unsigned i=0; i<NPULSE; i++)
	{
		tpvec[i]= WrapAngletoPI(2*_PI*(i)*normDop);
	}
//    aie::print(tpvec,true,"tpdata=");

	aie::vector<cfloat,16> tp = aie::sincos_complex(tpvec);
//    aie::print(tp,true,"tp=");


	//Step 3 Calculate Kronecker product

    float scaling = 0.025;

        for (int j = 0; j < NPULSE; j++) {
                auto kp=aie::mul(tp[j],sp);
        		//aie::print(kp.to_vector<cfloat>(0),true,"\r\n kp=");

        		aie::vector<cfloat,4> kproworder = kp.to_vector<cfloat>(0);

        		auto kp2 =aie::mul(kproworder,scaling);
//        		aie::print(kp2.to_vector<cfloat>(0),true,"scaled kp=");

        		*OutIter++ = kp2.to_vector<cfloat>(0);
        }
}



void WeightCalculation(
		adf::input_buffer<cfloat> & __restrict st,
		adf::input_buffer<cfloat> & __restrict eci,
		adf::output_buffer<cfloat> & __restrict weight)
{

	aie::vector<cfloat, 8> va =  null_v8cfloat();
	aie::vector<cfloat, 8> vb =  null_v8cfloat();
	aie::vector<cfloat, 8> vc =  null_v8cfloat();

	auto InIter0 = aie::begin_vector<8>(st);
	auto InIter1 = aie::begin_vector<8>(eci);
	cfloat* OutIter = aie::begin(weight);

	cfloat dotp;
    for(int j=0;j<40;j++)
    {
		// printf("Row %d\r\n",j);

        dotp = {0.0, 0.0};

		for(int i=0; i<5; i++)
		chess_prepare_for_pipelining
		{
			va = *InIter0++;
			// aie::print(va,true,"cfloat vecA=");
			vb = *InIter1++;
			// aie::print(vb,true,"cfloat vecB=");
			vc = aie::mul(va,vb);
			dotp = dotp + aie::reduce_add(vc);
		}

		*OutIter++ = dotp;
		InIter0 -= 5;

	}

}
void WeightApplication(
      adf::input_async_buffer<cfloat> & __restrict weight,
      adf::input_buffer<cint16> & __restrict matB,
      adf::output_buffer<cfloat> & __restrict matC)
{
	cfloat rowVec[40*40];
	//alignas(aie::vector_decl_align) cfloat rowVec[40*40];
	cfloat* __restrict bufPtr = rowVec;

	auto InIter1 = aie::begin_vector<16>(matB);
	cfloat* OutIter = aie::begin(matC);

	#ifndef __X86SIM__
	 static int itr = 0;
	#else
	 static thread_local int itr = 0;
	#endif

	if (itr == 0) {
	  // First CPI, get a new weight vector
	  weight.acquire(); // acquire output window
	}

	  auto BufIter_e = aie::begin_vector<4>(bufPtr);
	  auto BufIter_o = aie::begin_vector<4>(bufPtr+40);

	  for(int i=0; i<NBATCH_WEIGHTAPP/2; i++)
	  chess_prepare_for_pipelining
	  {
		  for(int j=0; j<5; j++)
		  chess_prepare_for_pipelining
		  {
			  // Convert CINT16 to CFLOAT
			 //Step 1 read 16 elem complex cint16 vector
			 aie::vector<cint16,16> Indata = *InIter1++;
			 aie::vector<cint16,8> data_e = aie::filter_even(Indata);
			 aie::vector<cint16,8> data_o = aie::filter_odd(Indata);

			 //Step 2 convert even cint16 to complex float vector
			 v16int16 *ptr3 = (v16int16*)&data_e;
			 v8float out0, out1;
			 fix2float_vectorized2(out0, out1, *ptr3, 0);

			 cfloat *ptr1 = (cfloat*)&out0;
			 cfloat *ptr2 = (cfloat*)&out1;

			 auto iter0 = aie::begin_vector<4>(ptr1);
			 auto iter1 = aie::begin_vector<4>(ptr2);
			 *BufIter_e++ = *iter0;
			 *BufIter_e++ = *iter1;

			 //Step 2 convert odd cint16 to complex float vector
			 v16int16 *ptr4 = (v16int16*)&data_o;
			 v8float out2, out3;
			 fix2float_vectorized2(out2, out3, *ptr4, 0);

			 cfloat *ptr5 = (cfloat*)&out2;
			 cfloat *ptr6 = (cfloat*)&out3;

			 auto iter2 = aie::begin_vector<4>(ptr5);
			 auto iter3 = aie::begin_vector<4>(ptr6);
			 *BufIter_o++ = *iter2;
			 *BufIter_o++ = *iter3;
		 }
		BufIter_e += 10;
		BufIter_o += 10;
	}

    auto BufIter1 = aie::begin_vector<4>(bufPtr);
    auto BufIter2 = aie::begin_vector<4>(bufPtr+40);

    for(int k=0; k<NBATCH_WEIGHTAPP/2; k++)
    chess_prepare_for_pipelining
    {
		aie::accum<caccfloat,4> acc1 = aie::zeros<caccfloat,4>();
		aie::accum<caccfloat,4> acc2 = aie::zeros<caccfloat,4>();
		auto InIter0 = aie::begin_vector<4>(weight);
		for(int j=0;j<10;j++)
		chess_prepare_for_pipelining
		{
		  acc1 = aie::mac(acc1,aie::op_conj(*InIter0),*BufIter1++);
		  acc2 = aie::mac(acc2,aie::op_conj(*InIter0++),*BufIter2++);
		}
		auto dotp0 = aie::reduce_add(acc1.to_vector<cfloat>(0));
		*OutIter++ = dotp0;
		auto dotp1 = aie::reduce_add(acc2.to_vector<cfloat>(0));
		*OutIter++ = dotp1;
		BufIter1 += 10;
		BufIter2 += 10;
   }

   if (itr == CPI_PER_WEIGHTAPP-1) {
      // Last CPI, release the weight output window
       weight.release();
      itr = 0;
   }
   else {
      itr++;
   }
}

float reciprocal(float a) {

    return aie::inv(a);
}

// Debug stuff
void print_matrix(cfloat *matrix_in, int matrix_row, int matrix_col, const char *s) {

#ifdef DEBUG_PRINT

    printf("%s\n", s);
    for(int row=0; row<matrix_row; row++) {
        for(int col=0; col<matrix_col; col++) {
            printf(" Pos(%d, %d): %.5e %.5ei \n", row, col, (float) (*matrix_in).real, (float) (*matrix_in).imag);
            matrix_in++;
    	}
    }

#endif

}


void print_vector(aie::vector<cfloat, 4> v, int len, const char *s) {
//void print_vector(aie::vector<cfloat, MAT_SIZE> v, int len, const char *s) {

#ifdef DEBUG_PRINT

    printf("%s :\n", s);
    for(int idx=0; idx<len; idx++) {
		float real_part;
		float imag_part;
		printf(" %.5e %.5ei \n", (float) v.get(idx).real, (float) v.get(idx).imag);
    }

#endif

}


// Contains basic linear algebra operations on vectors
//

float norm(aie::vector<cfloat, MAT_SIZE> v) {

	aie::vector<cfloat, MAT_SIZE> vec = aie::mul(v, aie::op_conj(v));
	cfloat res = aie::reduce_add(vec);
    return aie::sqrt(res.real);

}


float inv_norm(aie::vector<cfloat, MAT_SIZE> v) {

	aie::vector<cfloat, MAT_SIZE> vec = aie::mul(v, aie::op_conj(v));
	cfloat res = aie::reduce_add(vec);
    return aie::invsqrt(res.real);
}

// The sign of a complex number z is defined as z/abs(z)
//
cfloat sign(cfloat z) {

    cfloat res;
    float absVal = aie::invsqrt(z.real*z.real + z.imag * z.imag);
    res.real = z.real*absVal;
    res.imag = z.imag*absVal;
    return res;

}


void mat_mult_A_BT(const cfloat* __restrict A, const cfloat* __restrict BT, cfloat* __restrict C) {

	const cfloat * Aptr = A;
	const cfloat * BTptr;
	cfloat * Cptr = C;

	aie::vector<cfloat, MAT_SIZE> A_vec;   	// row vector of input A matrix
	aie::vector<cfloat, MAT_SIZE> BT_vec;   	// row vector of input BT matrix

	aie::vector<cfloat, MAT_SIZE> v1;   		// temporary vector

	for (int row=0; row<MAT_SIZE; row++) {

		BTptr = BT;
		A_vec = aie::load_v(Aptr);

		for (int col=0; col<MAT_SIZE; col++)
		chess_prepare_for_pipelining
		{
			BT_vec = aie::load_v(BTptr);
			v1 = aie::mul(A_vec, BT_vec);
			Cptr[col*MAT_SIZE + row] = aie::reduce_add(v1);
			BTptr += MAT_SIZE;
		}
		Aptr += MAT_SIZE;
	}
}


void mat_mult_AT_BT(const cfloat* __restrict AT, const cfloat* __restrict BT, cfloat* __restrict C) {

	const cfloat * ATptr;
	const cfloat * BTptr;
	cfloat * Cptr = C;

	aie::vector<cfloat, MAT_SIZE> AT_vec;   	// row vector of input AT matrix
	aie::vector<cfloat, MAT_SIZE> BT_vec;   	// row vector of input BT matrix

	aie::vector<cfloat, MAT_SIZE> v1;   		// temporary vector

	for (int row=0; row<MAT_SIZE; row++) {

		ATptr = AT + row;
		BTptr = BT;

		// Load one column of AT (row of A)
		for(int idx=0; idx<MAT_SIZE; idx++) {
			AT_vec.set(*ATptr, idx);
			ATptr += MAT_SIZE;
		}

		for (int col=0; col<MAT_SIZE; col++)
		chess_prepare_for_pipelining
		{
			BT_vec = aie::load_v(BTptr);
			v1 = aie::mul(AT_vec, BT_vec);
			Cptr[col*MAT_SIZE + row] = aie::reduce_add(v1);
			BTptr += MAT_SIZE;
		}
	}
}


cfloat reciprocal(cfloat z) {

    cfloat res;
//    float absSqVal = 1/(z.real*z.real + z.imag * z.imag);
    float absSqVal = aie::inv(z.real*z.real + z.imag * z.imag);
    res.real = z.real*absSqVal;
    res.imag = -1*z.imag*absSqVal;
    return res;

}
