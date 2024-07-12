/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/


#include <ap_int.h>
#include "pl_datamover.hpp"
#include "pragma_macro.hpp"
#define M 10
#define N 4
#define L 4000
#define LS (L / 16)

void wdma(
	hls::burst_maxi<ap_uint<64> > cfg,
	hls::burst_maxi<ap_uint<64> > data10,
	hls::burst_maxi<ap_uint<64> > data11,
	hls::burst_maxi<ap_uint<64> > data12,
	hls::burst_maxi<ap_uint<64> > data13,
	hls::burst_maxi<ap_uint<64> > data20,
	hls::burst_maxi<ap_uint<64> > data21,
	hls::burst_maxi<ap_uint<64> > data22,
	hls::burst_maxi<ap_uint<64> > data23,
	unsigned int num_cub,
	hls::stream<ap_axiu<64, 0, 0, 0> >& w_data0,
	hls::stream<ap_axiu<64, 0, 0, 0> >& w_data1,
	hls::stream<ap_axiu<64, 0, 0, 0> >& w_data2,
	hls::stream<ap_axiu<64, 0, 0, 0> >& w_data3,
	hls::stream<ap_axiu<8, 0, 0, 0> >& sync_out,
	hls::stream<ap_axiu<8, 0, 0, 0> >& sync_in
) {
    MAXI(cfg, gmem0, 32, 32, 32, 32, 32, N)
    MAXI(data10, gmem1, 32, 32, 32, 32, 32, N)
    MAXI(data11, gmem2, 32, 32, 32, 32, 32, N)
    MAXI(data12, gmem3, 32, 32, 32, 32, 32, N)
    MAXI(data13, gmem4, 32, 32, 32, 32, 32, N)
    MAXI(data20, gmem1, 32, 32, 32, 32, 32, N)
    MAXI(data21, gmem2, 32, 32, 32, 32, 32, N)
    MAXI(data22, gmem3, 32, 32, 32, 32, 32, N)
    MAXI(data23, gmem4, 32, 32, 32, 32, 32, N)
    SAXI(w_data0)
    SAXI(w_data1)
    SAXI(w_data2)
    SAXI(w_data3)
    SAXI(sync_out)
	SAXI(sync_in)

    ap_uint<1> ping = 0;
    ap_uint<64> p[8];
#pragma HLS ARRAY_PARTITION variable=p type=complete dim=0
    cmdLoader<64>(cfg, p);

    ap_axiu<8, 0, 0, 0> tmp_data;
    tmp_data.data = 0xFF;
    //tmp_data.keep_all();

  for(int i=0; i<num_cub; i++) {
    if(i>1)
  	   ap_axiu<8, 0, 0, 0> sync_data = sync_in.read();
    if(ping == 0) {
      cuboidWrite<64, 32, 32, 32>(0,    p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], w_data0, data10, 0);
      cuboidWrite<64, 32, 32, 32>(2000, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], w_data1, data11, 1);
      cuboidWrite<64, 32, 32, 32>(4000, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], w_data2, data12, 2);
      cuboidWrite<64, 32, 32, 32>(6000, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], w_data3, data13, 3);
    }
    else {
      cuboidWrite<64, 32, 32, 32>(0,    p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], w_data0, data20, 0);
      cuboidWrite<64, 32, 32, 32>(2000, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], w_data1, data21, 1);
      cuboidWrite<64, 32, 32, 32>(4000, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], w_data2, data22, 2);
      cuboidWrite<64, 32, 32, 32>(6000, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], w_data3, data23, 3);
    }
    ping++;
    sync_out.write(tmp_data);
  }
}

//#ifndef __SYNTHESIS__
//int main() {
//    ap_uint<64>* data_64 = (ap_uint<64>*)malloc(sizeof(ap_uint<32>) * L * M * N);
//    ap_uint<32>* data_32 = (ap_uint<32>*)(data_64);
//    hls::stream<ap_axiu<64, 0, 0, 0> > w_data;
//    /*
//     * Cuboid Data[M][N][L], M = 10, N = 4, L = 4000;
//     * Let Data[i][j][k]'s golden value be i * 100000 + j * 10000 + k, for verification
//     *
//     * XXX: NOTICE!!!
//     * cuboid reader does not have any built-in memory mapping!
//     * It only provides a 4-layer nested loop to match your access pattern
//     * So you need first design your memory mapping, then create suitable descriptor for data mover
//     * The access pattern described by [offset, i1, d1, i2, d2, i3, d3, i4, d4] is:
//     *
//     * for(w = 0; w < d4; w++) {
//     *     for(z = 0; z < d3; z++) {
//     *         for(y = 0; y < d3; y++) {
//     *             for(x = 0; x < d1; x++) {
//     *                  access_data on address s1 = offset + w * i4 + z * i3 + y * i2 + x * i1;
//     *             }
//     *          }
//     *      }
//     *  }
//     *
//     * If both WDMA and RDMA are working on same buffer, their descriptor should follow the same memory mapping;
//     *
//     * Memory mapping for this test
//     * Data[i][j][k]'s address = i * (L * N) + j * (L) + k;
//     */
//
//    ap_uint<64> cfg_ch0[9]; // for WDMA on incoming stream [0]
//    ap_uint<64> cfg_ch1[9]; // for WDMA on incoming stream [1]
//    ap_uint<64> cfg_ch2[9]; // for WDMA on incoming stream [2]
//    ap_uint<64> cfg_ch3[9]; // for WDMA on incoming stream [3]
//
//    /*
//     * For WDMA working with in-coming stream 0, it should write Data[0:9][0][0:3999]
//     * Starting from Data[0][0][0] to Data[0][0][3999], then Data[1][0][0] to Data[1][0][3999] ...
//     * Until finish Data[9][0][0] to Data[9][0][3999].
//     *
//     * It's basically a 2-layer nested loop.
//     * We need to map it to 4-layer nested loop.
//     */
//    cfg_ch0[0] = 0;           // offset = 0: since first element is Data[0][0][0];
//    cfg_ch0[1] = 1;           // linear access in first inner loop
//    cfg_ch0[2] = L / 2;       // L/2, due to input stream size is twice larger of data type.
//    cfg_ch0[3] = (L / 2) * N; // second inner loop jump by N * L elements
//    cfg_ch0[4] = M;           // d2 = M
//    cfg_ch0[5] = 0;           // third inner loop only iterate once, jump step does not matter
//    cfg_ch0[6] = 1;           // third inner loop only iterate once
//    cfg_ch0[7] = 0;           // fourth inner loop only iterate once, jump step does not matter
//    cfg_ch0[8] = 1;           // fourth inner loop only iterate once
//
//    cfg_ch1[0] = L / 2;       // offset = 0: since first element is Data[0][1][0];
//    cfg_ch1[1] = 1;           // linear access in first inner loop
//    cfg_ch1[2] = L / 2;       // L/2, due to input stream size is twice larger of data type.
//    cfg_ch1[3] = (L / 2) * N; // second inner loop jump by N * L elements
//    cfg_ch1[4] = M;           // d2 = M
//    cfg_ch1[5] = 0;           // third inner loop only iterate once, jump step does not matter
//    cfg_ch1[6] = 1;           // third inner loop only iterate once
//    cfg_ch1[7] = 0;           // fourth inner loop only iterate once, jump step does not matter
//    cfg_ch1[8] = 1;           // fourth inner loop only iterate once
//
//    cfg_ch2[0] = L / 2 * 2;   // offset = 0: since first element is Data[0][2][0];
//    cfg_ch2[1] = 1;           // linear access in first inner loop
//    cfg_ch2[2] = L / 2;       // L/2, due to input stream size is twice larger of data type.
//    cfg_ch2[3] = (L / 2) * N; // second inner loop jump by N * L elements
//    cfg_ch2[4] = M;           // d2 = M
//    cfg_ch2[5] = 0;           // third inner loop only iterate once, jump step does not matter
//    cfg_ch2[6] = 1;           // third inner loop only iterate once
//    cfg_ch2[7] = 0;           // fourth inner loop only iterate once, jump step does not matter
//    cfg_ch2[8] = 1;           // fourth inner loop only iterate once

//    cfg_ch3[0] = L / 2 * 3;   // offset = 0: since first element is Data[0][3][0];
//    cfg_ch3[1] = 1;           // linear access in first inner loop
//    cfg_ch3[2] = L / 2;       // L/2, due to input stream size is twice larger of data type.
//    cfg_ch3[3] = (L / 2) * N; // second inner loop jump by N * L elements
//    cfg_ch3[4] = M;           // d2 = M
//    cfg_ch3[5] = 0;           // third inner loop only iterate once, jump step does not matter
//    cfg_ch3[6] = 1;           // third inner loop only iterate once
//    cfg_ch3[7] = 0;           // fourth inner loop only iterate once, jump step does not matter
//    cfg_ch3[8] = 1;           // fourth inner loop only iterate once
//
    // test for ch0
//    for (int i = 0; i < M; i++) {
//        int j = 0;
//        for (int k = 0; k < L; k += 2) {
//            ap_uint<64> tmp_val;
//            tmp_val.range(31, 0) = i * 100000 + j * 10000 + k;
//            tmp_val.range(63, 32) = i * 100000 + j * 10000 + k + 1;
//
//            ap_axiu<64, 0, 0, 0> tmp_data;
//            tmp_data.data = tmp_val;
//            tmp_data.keep = -1;
//            tmp_data.last = 0;
//            w_data.write(tmp_data);
//        }
//    }
//    dut(data_64, cfg_ch0, w_data);
//
//    // test for ch1
//    for (int i = 0; i < M; i++) {
//        int j = 1;
//        for (int k = 0; k < L; k += 2) {
//            ap_uint<64> tmp_val;
//            tmp_val.range(31, 0) = i * 100000 + j * 10000 + k;
//            tmp_val.range(63, 32) = i * 100000 + j * 10000 + k + 1;
//
//            ap_axiu<64, 0, 0, 0> tmp_data;
//            tmp_data.data = tmp_val;
//            tmp_data.keep = -1;
//            tmp_data.last = 0;
//            w_data.write(tmp_data);
//        }
//    }
//    dut(data_64, cfg_ch1, w_data);
//
//    // test for ch2
//    for (int i = 0; i < M; i++) {
//        int j = 2;
//        for (int k = 0; k < L; k += 2) {
//            ap_uint<64> tmp_val;
//            tmp_val.range(31, 0) = i * 100000 + j * 10000 + k;
//            tmp_val.range(63, 32) = i * 100000 + j * 10000 + k + 1;
//
//            ap_axiu<64, 0, 0, 0> tmp_data;
//            tmp_data.data = tmp_val;
//            tmp_data.keep = -1;
//            tmp_data.last = 0;
//            w_data.write(tmp_data);
//        }
//    }
//    dut(data_64, cfg_ch2, w_data);
//
//    // test for ch3
//    for (int i = 0; i < M; i++) {
//        int j = 3;
//        for (int k = 0; k < L; k += 2) {
//            ap_uint<64> tmp_val;
//            tmp_val.range(31, 0) = i * 100000 + j * 10000 + k;
//            tmp_val.range(63, 32) = i * 100000 + j * 10000 + k + 1;
//
//            ap_axiu<64, 0, 0, 0> tmp_data;
//            tmp_data.data = tmp_val;
//            tmp_data.keep = -1;
//            tmp_data.last = 0;
//            w_data.write(tmp_data);
//        }
//    }
//    dut(data_64, cfg_ch3, w_data);
//
//    bool check = true;
//    std::cout << "start res checking" << std::endl;
//    for (int i = 0; i < M; i++) {
//        for (int j = 0; j < N; j++) {
//            for (int k = 0; k < L; k++) {
//                ap_uint<32> golden = i * 100000 + j * 10000 + k;
//                if (golden != data_32[i * N * L + j * L + k]) {
//                    check = false;
//                    std::cout << "[" << i << "][ " << j << "][" << k << "], golden = " << golden
//                              << ", res = " << data_32[i * N * L + j * L + k] << std::endl;
//                }
//            }
//        }
//    }
//
//    free(data_64);
//
//    if (check) {
//        std::cout << "TEST PASSED" << std::endl;
//        return 0;
//    } else {
//        return 1;
//    }
//}
//#endif
