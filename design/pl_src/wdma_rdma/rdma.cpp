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
#define S 16
#define LS (L / S)

void rdma(
	hls::burst_maxi<ap_uint<64> > cfg,
	hls::burst_maxi<ap_uint<64> > data10,
	hls::burst_maxi<ap_uint<64> > data11,
	hls::burst_maxi<ap_uint<64> > data12,
	hls::burst_maxi<ap_uint<64> > data13,
	hls::burst_maxi<ap_uint<64> > data20,
	hls::burst_maxi<ap_uint<64> > data21,
	hls::burst_maxi<ap_uint<64> > data22,
	hls::burst_maxi<ap_uint<64> > data23,
	int num_cub,
	hls::stream<ap_axiu<64, 0, 0, 0> >& w_data0,
	hls::stream<ap_axiu<64, 0, 0, 0> >& w_data1,
	hls::stream<ap_axiu<64, 0, 0, 0> >& w_data2,
	hls::stream<ap_axiu<64, 0, 0, 0> >& w_data3,
	hls::stream<ap_axiu<8, 0, 0, 0> >& sync_in,
	hls::stream<ap_axiu<8, 0, 0, 0> >& sync_out
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

    SAXI(sync_in)
	SAXI(sync_out)

    ap_uint<1> ping = 0;
    ap_uint<64> p[8];
#pragma HLS ARRAY_PARTITION variable=p type=complete dim=0
    cmdLoader<64>(cfg, p);
	ap_axiu<8, 0, 0, 0> sync_data;
    sync_data.data = 0xFF;

  for(int i=0; i<num_cub; i++) {
    ap_axiu<8, 0, 0, 0> tmp_data = sync_in.read();
    if(ping == 0) {
      cuboidReadReorder<64, 32, 32, 32, 5000>(0   , p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data10, w_data0, 0);
      cuboidReadReorder<64, 32, 32, 32, 5000>(500 , p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data11, w_data1, 1);  
	  cuboidReadReorder<64, 32, 32, 32, 5000>(1000 , p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data12, w_data2, 2);
      cuboidReadReorder<64, 32, 32, 32, 5000>(1500 , p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data13, w_data3, 3);
      /*cuboidReadReorder<32, 32, 32, 32, 10000>(1000, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data14, w_data4, 4);
      cuboidReadReorder<32, 32, 32, 32, 10000>(1250, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data15, w_data5, 5);
      cuboidReadReorder<32, 32, 32, 32, 10000>(1500, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data16, w_data6, 6);
      cuboidReadReorder<32, 32, 32, 32, 10000>(1750, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data17, w_data7, 7);
      cuboidReadReorder<32, 32, 32, 32, 10000>(2000, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data18, w_data8, 8);
      cuboidReadReorder<32, 32, 32, 32, 10000>(2250, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data19, w_data9, 9);
      cuboidReadReorder<32, 32, 32, 32, 10000>(2500, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data110, w_data10, 10);
      cuboidReadReorder<32, 32, 32, 32, 10000>(2750, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data111, w_data11, 11);
      cuboidReadReorder<32, 32, 32, 32, 10000>(3000, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data112, w_data12, 12);
      cuboidReadReorder<32, 32, 32, 32, 10000>(3250, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data113, w_data13, 13);
      cuboidReadReorder<32, 32, 32, 32, 10000>(3500, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data114, w_data14, 14);
      cuboidReadReorder<32, 32, 32, 32, 10000>(3750, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data115, w_data15, 15);*/
    }
    else {
      cuboidReadReorder<64, 32, 32, 32, 5000>(0   , p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data20, w_data0, 0);
      cuboidReadReorder<64, 32, 32, 32, 5000>(500 , p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data21, w_data1, 1);
      cuboidReadReorder<64, 32, 32, 32, 5000>(1000 , p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data22, w_data2, 2);
      cuboidReadReorder<64, 32, 32, 32, 5000>(1500 , p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data23, w_data3, 3);
      /*cuboidReadReorder<32, 32, 32, 32, 10000>(1000, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data24, w_data4, 4);
      cuboidReadReorder<32, 32, 32, 32, 10000>(1250, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data25, w_data5, 5);
      cuboidReadReorder<32, 32, 32, 32, 10000>(1500, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data26, w_data6, 6);
      cuboidReadReorder<32, 32, 32, 32, 10000>(1750, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data27, w_data7, 7);
      cuboidReadReorder<32, 32, 32, 32, 10000>(2000, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data28, w_data8, 8);
      cuboidReadReorder<32, 32, 32, 32, 10000>(2250, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data29, w_data9, 9);
      cuboidReadReorder<32, 32, 32, 32, 10000>(2500, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data210, w_data10, 10);
      cuboidReadReorder<32, 32, 32, 32, 10000>(2750, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data211, w_data11, 11);
      cuboidReadReorder<32, 32, 32, 32, 10000>(3000, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data212, w_data12, 12);
      cuboidReadReorder<32, 32, 32, 32, 10000>(3250, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data213, w_data13, 13);
      cuboidReadReorder<32, 32, 32, 32, 10000>(3500, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data214, w_data14, 14);
      cuboidReadReorder<32, 32, 32, 32, 10000>(3750, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], data215, w_data15, 15);*/
    }
    ping++;
	if(i<num_cub-2)
		sync_out.write(sync_data);
  }
}
//#ifndef __SYNTHESIS__
//int main() {
//    ap_uint<32>* data_32 = (ap_uint<32>*)malloc(sizeof(ap_uint<32>) * L * M * N);
//    hls::stream<ap_axiu<32, 0, 0, 0> > w_data;
//
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
//     * If both WDMA and RDMA are working on same buffer, their descriptor should follow the same memory mapping;
//     *
//     * RDMA also have an re-order module, which will make the final output pattern like:
//     * for(w = 0; w < d4; w++) {
//     *     for(x = 0; x < d1; x++) {
//     *         for(y = 0; y < d2; y++) {
//     *             for(z = 0; z < d3; z++) {
//     *                  access_data on address s1 = offset + w * i4 + z * i3 + y * i2 + x * i1;
//     *             }
//     *          }
//     *      }
//     *  }
//     *
//     */
//
//    for (int i = 0; i < M; i++) {
//        for (int j = 0; j < N; j++) {
//            for (int k = 0; k < L; k++) {
//                ap_uint<32> val = i * 100000 + j * 10000 + k;
//                data_32[i * L * N + j * L + k] = val;
//            }
//        }
//    }
//    std::cout << "A" << std::endl;
//
//    const int X = 0; // for RDMA on outputing stream [x], X = 0~15
//    ap_uint<64> cfg_ch0[9];
//    cfg_ch0[0] = LS * X;
//    cfg_ch0[1] = 1;
//    cfg_ch0[2] = LS;
//    cfg_ch0[3] = L;
//    cfg_ch0[4] = N;
//    cfg_ch0[5] = L * N;
//    cfg_ch0[6] = M;
//    cfg_ch0[7] = 0;
//    cfg_ch0[8] = 1;
//    dut(data_32, cfg_ch0, w_data);
//    std::cout << "AA" << std::endl;
//
//    bool check = true;
//    for (int k = X; k < LS + X; k++) {
//        for (int j = 0; j < N; j++) {
//            for (int i = 0; i < M; i++) {
//                ap_axiu<32, 0, 0, 0> val = w_data.read();
//                ap_uint<32> golden = data_32[i * L * N + j * L + k];
//                if (val.data != golden) {
//                    check = false;
//                    std::cout << "[" << i << "][ " << j << "][" << k << "], golden = " << golden
//                              << ", res = " << val.data << std::endl;
//                }
//            }
//        }
//    }
//    std::cout << "AAA" << std::endl;
//
//    free(data_32);
//    if (check) {
//        std::cout << "TEST PASSED" << std::endl;
//        return 0;
//    } else {
//        return 1;
//    }
//}
////#endif
