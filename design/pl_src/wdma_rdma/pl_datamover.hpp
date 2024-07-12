/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/


#ifndef _PL_DATAMOVER_HPP_
#define _PL_DATAMOVER_HPP_

#include <ap_int.h>
#include <ap_axi_sdata.h>
#include <hls_stream.h>
#include <hls_burst_maxi.h>
#include <hls_burst_maxi.h>
#include <hls_burst_maxi.h>
#include <hls_burst_maxi.h>
#include "pragma_macro.hpp"

/*
    3D Array Access, A[L][M][N], elements size

    1. 1D Access Pattern
    1.1 1D A[z][y][i : i + n), in sequence of : A[z][y][i], A[z][y][i + 1] ... A[z][y][i + n - 1]
        ptr + (z * (M * N) + y * N + i) + 0, 1, 2, 3, .. , n-1

    1.2 1D A[z][i : i + n)[x], in sequence of : A[z][i][x], A[z][i + 1][x] ... A[z][i + n - 1][x]
        ptr + (z * (M * N) + i * N + x) + 0, 1*N, 2*N, 3*N, .., (n-1)*N

    1.3 1D A[i : i + n)[y][x], in sequence of : A[i][y][x], A[i + 1][y][x] ... A[i + n - 1][y][x]
        ptr + (i * (M * N) + y * N + x) + 0, 1*M*N, 2*M*N, 3*M*N, .., (n-1)*M*N

    1.4 1D Access pattern could all fit into parameters:
        (0) ele_size, size of array element type, support 4 (for 32 bits integer or single-precesion floating point)
        (1) s_ptr, pointer to the 3-D array
        (2) offset, offset to the start element
        (3) incr_num, step of the pointer of following element will be increased by.
        (4) total_num, num of total elements accessed
        (5) (s_ptr + offset) + 0, incr_num * 1, incr_num * 2, .. incr_num * (total_num - 1)

    2. Input Port
        ap_uint<32>* input, to aligned with 32 bits data type

    3. Output Port
        hls::stream<ap_axiu<32, 0, 0, 0> >

    4. Command Buffer
        fixed size command buffer
 */

/**
 * Load Command to memory on chip
 *
 * @tparam CMDQ_DEPTH Depth of command buffer
 * @tparam LATENCY latency of MAXI port.
 * @tparam OUTSTANDING read outstanding of MAXI port, should be less than 512.
 * @tparam BURSTLEN read burst length of MAXI port, should be less than 64.
 *
 * @param data MAXI port for reading
 * @param cmd_num Number of commands
 * @param offset Offset of command
 * @param incr_num Increment number of adjacent elements
 * @param total_num Total number of elements to be accessed
 */

template <int CMDQ_DEPTH, int LATENCY, int OUTSTANDING, int BURSTLEN>
void cmdLoader(hls::burst_maxi<ap_uint<32> >& data,
               ap_uint<32> cmd_num,
               ap_uint<64> offset[CMDQ_DEPTH],
               ap_uint<64> incr_num[CMDQ_DEPTH],
               ap_uint<64> total_num[CMDQ_DEPTH]) {
    ap_uint<32> ptr = 0;

    for (ap_uint<32> i = 0; i < cmd_num; i += OUTSTANDING) {
        ap_uint<32> tmp_cmd_num;
        if ((i + OUTSTANDING) <= cmd_num) {
            tmp_cmd_num = OUTSTANDING;
        } else {
            tmp_cmd_num = cmd_num - i;
        }

        for (ap_uint<32> j = 0; j < tmp_cmd_num; j++) {
#pragma HLS pipeline II = 1
            data.read_request(ptr, 3 * 2); // 3 x 64bits
            ptr += 6;
        }

        for (ap_uint<32> j = 0; j < tmp_cmd_num; j++) {
#pragma HLS pipeline II = 6
            ap_uint<64> tmp_offset, tmp_incr_num, tmp_total_num;
            tmp_offset.range(31, 0) = data.read();
            tmp_offset.range(63, 32) = data.read();
            offset[i + j] = tmp_offset;
            tmp_incr_num.range(31, 0) = data.read();
            tmp_incr_num.range(63, 32) = data.read();
            incr_num[i + j] = tmp_incr_num;
            tmp_total_num.range(31, 0) = data.read();
            tmp_total_num.range(63, 32) = data.read();
            total_num[i + j] = tmp_total_num;
        }
    }
}

/**
 * Parse Command Buffer, send out burst read request
 *
 * @tparam CMDQ_DEPTH max depth of command queue
 * @tparam BURSTLEN max burst length supported in the burst_maxi port, should be less than 1024
 *
 * @param offset, offset of raw command
 * @param incr_num, incr_num of raw command
 * @param total_num, total_num of raw command
 * @param r_offset, stream to get offset for burst read
 * @param r_burst, stream ot get length for burst read, should be no bigger BURSTLEN
 * @param e_r, end of read request
 */

template <int CMDQ_DEPTH, int BURSTLEN>
void cmdParser( // input
    ap_uint<64> offset[CMDQ_DEPTH],
    ap_uint<64> incr_num[CMDQ_DEPTH],
    ap_uint<64> total_num[CMDQ_DEPTH],
    ap_uint<32> cmd_num,
    // output
    hls::stream<ap_uint<64> >& r_offset,
    hls::stream<ap_uint<10> >& r_burst,
    hls::stream<bool>& e_r) {
    for (ap_uint<32> cmd = 0; cmd < cmd_num; cmd++) {
        ap_uint<64> tmp_offset = offset[cmd];
        ap_uint<64> tmp_total = total_num[cmd];
        ap_uint<64> tmp_incr_num = incr_num[cmd];
        ap_uint<64> tmp_offset_incr;
        ap_uint<64> tmp_i_incr;

        if (tmp_incr_num == 1) {
            tmp_offset_incr = BURSTLEN;
            tmp_i_incr = BURSTLEN;
        } else {
            tmp_offset_incr = incr_num[cmd];
            tmp_i_incr = 1;
        }
    CMD_LOOP:
        for (ap_uint<64> i = 0; i < tmp_total; i += tmp_i_incr) {
#pragma HLS pipeline II = 1

            r_offset.write(tmp_offset);
            tmp_offset += tmp_offset_incr;

            if (tmp_incr_num == 1) {
                if ((i + BURSTLEN) <= tmp_total) {
                    r_burst.write(ap_uint<10>(BURSTLEN));
                } else {
                    r_burst.write(ap_uint<10>(tmp_total - i));
                }
            } else {
                r_burst.write(ap_uint<10>(1));
            }

            e_r.write(false);
        }
    }
    e_r.write(true);
}

/**
 * A bit more about manual burst / write design
 * Both take "request params + data" and operate on burst_maxi
 * Both will act like "request at first, close request later"
 * This means there'll be requests on the fly, and they need buffer.
 *
 * One important limit to be noticed:
 * burst maxi port will first "cut" request into sub request, if data request ran across 4KB border.
 * Outstanding are upperbound of such sub request, in pragma for burst maxi port
 * To avoid deadlock caused by sending too much request, request from HLS side need to be less than outstanding.
 *
 * In actual design, burst maxi width is no bigger than 64 Bytes (512 bits).
 * If BURSTLEN <= 64, then total size of one quest is less than 64 * 64 = 4096 Byte
 * So actual sub request is at most twice as much as request from HLS side.
 * So we could at least create (outstanding / 2) HLS requests is safe to be handled.
 *
 * But BURSTLEN should not be too small, for best bandwidth, at least 16.
 */

/**
 * Read from stream and create burst write request to DDR/HBM
 *
 * @tparam WDATA width of MAXI port.
 * @tparam LATENCY latency of MAXI port.
 * @tparam OUTSTANDING read outstanding of MAXI port, should be less than 512.
 * @tparam BURSTLEN read burst length of MAXI port, should be less than 64.
 *
 * @param r_offset, stream to get offset for burst write
 * @param r_burst, stream ot get length for burst write, should be no bigger BURSTLEN
 * @param e_r, end of write request
 * @param w_data, stream to read data for writing
 * @param data MAXI port for writing
 */

template <int WDATA, int LATENCY, int OUTSTANDING, int BURSTLEN>
void manualBurstWrite( // input
    hls::stream<ap_uint<64> >& r_offset,
    hls::stream<ap_uint<10> >& r_burst,
    hls::stream<bool>& e_r,
    hls::stream<ap_axiu<WDATA, 0, 0, 0> >& w_data,
    // output
    hls::burst_maxi<ap_uint<WDATA> >& data) {
    ap_uint<LATENCY> check = 0;
    ap_uint<10> req_left = OUTSTANDING / 2;
    ap_uint<10> write_left = 0;
    ap_uint<10> burst_record[1024];
    ap_uint<10> rec_head = 0;
    ap_uint<10> rec_tail = 0;
    bool last = e_r.read();

ACC_BURST_REQ_LOOP:
    // TODO: accumulate so many request at first might cause deadlock, to be fixed.
    while (!last && rec_tail != OUTSTANDING / 2) {
#pragma HLS pipeline II = 1
        ap_uint<64> tmp_offset = r_offset.read();
        ap_uint<64> tmp_burst = r_burst.read();
        last = e_r.read();
        burst_record[rec_tail++] = tmp_burst;
        data.write_request(tmp_offset, tmp_burst);
        req_left--;
    }
    if (rec_tail != 0) {
        write_left = burst_record[rec_head++];
    }

BURST_WRITE_LOOP:
    while (!last || write_left != 0 || rec_head != rec_tail) {
#pragma HLS pipeline II = 1
        bool check_l = check[LATENCY - 1];
        check <<= 1;

        if (write_left != 0) { // load data and write if possible
            ap_axiu<WDATA, 0, 0, 0> tmp_data = w_data.read();
            data.write(tmp_data.data); // haha, too many data

            if (--write_left == 0) { // if all data of current write request has been sent
                check[0] = 1;
                if (rec_head != rec_tail) {
                    write_left = burst_record[rec_head++];
                }
            }
        }

        if (check_l) { // if a write request has become old enough
            data.write_response();

            if (!last) {
                ap_uint<64> tmp_offset = r_offset.read();
                ap_uint<64> tmp_burst = r_burst.read();
                last = e_r.read();
                burst_record[rec_tail++] = tmp_burst;
                data.write_request(tmp_offset, tmp_burst);
            } else {
                req_left++;
            }
        }
    }

    while (check != 0) {
        bool check_l = check[LATENCY - 1];
        check <<= 1;
        if (check_l) {
            data.write_response();
            req_left--;
        }
    }
}

/**
 * Read from stream and create burst write request to DDR/HBM
 *
 * @tparam WDATA width of MAXI port.
 * @tparam LATENCY latency of MAXI port.
 * @tparam OUTSTANDING read outstanding of MAXI port, should be less than 512.
 * @tparam BURSTLEN read burst length of MAXI port, should be less than 64.
 *
 * @param r_offset, stream to get offset for burst write
 * @param r_burst, stream ot get length for burst write, should be no bigger BURSTLEN
 * @param e_r, end of write request
 * @param w_data, stream to read data for writing
 * @param data MAXI port for writing
 */

template <int WDATA, int LATENCY, int OUTSTANDING, int BURSTLEN>
void manualBurstWrite( // input
    hls::stream<ap_uint<64> >& r_offset,
    hls::stream<ap_uint<10> >& r_burst,
    hls::stream<bool>& e_r,
    hls::stream<ap_uint<WDATA> >& w_data,
    // output
    hls::burst_maxi<ap_uint<WDATA> >& data) {
    ap_uint<LATENCY> check = 0;
    ap_uint<10> req_left = OUTSTANDING / 2;
    ap_uint<10> write_left = 0;
    ap_uint<10> burst_record[1024];
    ap_uint<10> rec_head = 0;
    ap_uint<10> rec_tail = 0;
    bool last = e_r.read();

ACC_BURST_REQ_LOOP:
    // TODO: accumulate so many request at first might cause deadlock, to be fixed.
    while (!last && rec_tail != OUTSTANDING / 2) {
#pragma HLS pipeline II = 1
        ap_uint<64> tmp_offset = r_offset.read();
        ap_uint<64> tmp_burst = r_burst.read();
        last = e_r.read();
        burst_record[rec_tail++] = tmp_burst;
        data.write_request(tmp_offset, tmp_burst);
        req_left--;
    }
    if (rec_tail != 0) {
        write_left = burst_record[rec_head++];
    }

BURST_WRITE_LOOP:
    while (!last || write_left != 0 || rec_head != rec_tail) {
#pragma HLS pipeline II = 1
        bool check_l = check[LATENCY - 1];
        check <<= 1;

        if (write_left != 0) { // load data and write if possible
            ap_uint<WDATA> tmp_data = w_data.read();
            data.write(tmp_data); // haha, too many data

            if (--write_left == 0) { // if all data of current write request has been sent
                check[0] = 1;
                if (rec_head != rec_tail) {
                    write_left = burst_record[rec_head++];
                }
            }
        }

        if (check_l) { // if a write request has become old enough
            data.write_response();

            if (!last) {
                ap_uint<64> tmp_offset = r_offset.read();
                ap_uint<64> tmp_burst = r_burst.read();
                last = e_r.read();
                burst_record[rec_tail++] = tmp_burst;
                data.write_request(tmp_offset, tmp_burst);
            } else {
                req_left++;
            }
        }
    }

    while (check != 0) {
        bool check_l = check[LATENCY - 1];
        check <<= 1;
        if (check_l) {
            data.write_response();
            req_left--;
        }
    }
}

/**
 * Create burst read request to DDR/HBM and write to stream
 *
 * @tparam WDATA width of MAXI port.
 * @tparam LATENCY latency of MAXI port.
 * @tparam OUTSTANDING read outstanding of MAXI port, should be less than 512.
 * @tparam BURSTLEN read burst length of MAXI port, should be less than 64.
 *
 * @param data MAXI port for reading
 * @param r_offset, stream to get offset for burst read
 * @param r_burst, stream ot get length for burst read, should be no bigger BURSTLEN
 * @param e_r, end of read request
 * @param w_data, stream to write read result
 */

template <int WDATA, int LATENCY, int OUTSTANDING, int BURSTLEN>
void manualBurstRead( // input
    hls::burst_maxi<ap_uint<WDATA> >& data,
    hls::stream<ap_uint<64> >& r_offset,
    hls::stream<ap_uint<10> >& r_burst,
    hls::stream<bool>& e_r,
    // output
    hls::stream<ap_axiu<WDATA, 0, 0, 0> >& w_data) {
    ap_uint<LATENCY> check = 0;             // delay check
    ap_uint<10> req_left = OUTSTANDING / 2; // how many more request could be issued, "/2" is to avoid hang
    ap_uint<10> req_ready = 0;              // how many request should be ready, according to delay check
    ap_uint<10> burst_record[1024];         // burstlen record
    ap_uint<10> rec_head = 0;               // record head
    ap_uint<10> rec_tail = 0;               // record tail
    ap_uint<10> read_left = 0;              // read left in 1 burst
    bool last = e_r.read();

BURST_READ_LOOP:
    while (!last || req_ready != 0 || check != 0) {
#pragma HLS pipeline II = 1
        //#pragma HLS pipeline II = 1

        bool check_l = check[LATENCY - 1];
        check <<= 1;

        if (req_left != 0 && !last) { // if read outstanding is not exhausted, issue more request
            ap_uint<64> tmp_offset = r_offset.read();
            ap_uint<10> tmp_burst = r_burst.read();
            last = e_r.read();

            data.read_request(tmp_offset, tmp_burst);
            check[0] = 1;
            req_left--;
            burst_record[rec_tail++] = tmp_burst;
        }

        if (req_ready != 0 || read_left != 0) { // if there's mature req
            if (read_left == 0) {
                read_left = burst_record[rec_head++] - 1;
                req_ready--;
                req_left++;
            } else {
                read_left--;
            }

            ap_axiu<WDATA, 0, 0, 0> tmp_data;
            tmp_data.data = data.read();
            tmp_data.keep = -1;
            tmp_data.last = 0;
            w_data.write(tmp_data);
        }

        if (check_l) { // if a new request has become old enough
            req_ready++;
        }
    }

    while (read_left != 0) {
#pragma HLS pipeline II = 1
        ap_axiu<WDATA, 0, 0, 0> tmp_data;
        tmp_data.data = data.read();
        tmp_data.keep = -1;
        tmp_data.last = 0;
        read_left--;
        w_data.write(tmp_data);
    }
}

/**
 * Create burst read request to DDR/HBM and write to stream
 *
 * @tparam WDATA width of MAXI port.
 * @tparam LATENCY latency of MAXI port.
 * @tparam OUTSTANDING read outstanding of MAXI port, should be less than 512.
 * @tparam BURSTLEN read burst length of MAXI port, should be less than 64.
 *
 * @param data MAXI port for reading
 * @param r_offset, stream to get offset for burst read
 * @param r_burst, stream ot get length for burst read, should be no bigger BURSTLEN
 * @param e_r, end of read request
 * @param w_data, stream to write read result
 */

template <int WDATA, int LATENCY, int OUTSTANDING, int BURSTLEN>
void manualBurstRead( // input
    hls::burst_maxi<ap_uint<WDATA> >& data,
    hls::stream<ap_uint<64> >& r_offset,
    hls::stream<ap_uint<10> >& r_burst,
    hls::stream<bool>& e_r,
    // output
    hls::stream<ap_uint<WDATA> >& w_data) {
    ap_uint<LATENCY> check = 0;             // delay check
    ap_uint<10> req_left = OUTSTANDING / 2; // how many more request could be issued, "/2" is to avoid hang
    ap_uint<10> req_ready = 0;              // how many request should be ready, according to delay check
    ap_uint<10> burst_record[1024];         // burstlen record
    ap_uint<10> rec_head = 0;               // record head
    ap_uint<10> rec_tail = 0;               // record tail
    ap_uint<10> read_left = 0;              // read left in 1 burst
    bool last = e_r.read();

BURST_READ_LOOP:
    while (!last || req_ready != 0 || check != 0) {
#pragma HLS pipeline II = 1
        //#pragma HLS pipeline II = 1

        bool check_l = check[LATENCY - 1];
        check <<= 1;

        if (req_left != 0 && !last) { // if read outstanding is not exhausted, issue more request
            ap_uint<64> tmp_offset = r_offset.read();
            ap_uint<10> tmp_burst = r_burst.read();
            last = e_r.read();

            data.read_request(tmp_offset, tmp_burst);
            check[0] = 1;
            req_left--;
            burst_record[rec_tail++] = tmp_burst;
        }

        if (req_ready != 0 || read_left != 0) { // if there's mature req
            if (read_left == 0) {
                read_left = burst_record[rec_head++] - 1;
                req_ready--;
                req_left++;
            } else {
                read_left--;
            }

            w_data.write(data.read());
        }

        if (check_l) { // if a new request has become old enough
            req_ready++;
        }
    }
    while (read_left != 0) {
#pragma HLS pipeline II = 1
        w_data.write(data.read());
        read_left--;
    }
}

/**
 * Create burst read request to DDR/HBM and write to stream
 *
 * @tparam WDATA width of MAXI port.
 * @tparam LATENCY latency of MAXI port.
 * @tparam OUTSTANDING read outstanding of MAXI port, should be less than 512.
 * @tparam BURSTLEN read burst length of MAXI port, should be less than 64.
 *
 * @param data MAXI port for reading
 * @param r_offset, stream to get offset for burst read
 * @param r_burst, stream ot get length for burst read, should be no bigger BURSTLEN
 * @param e_r, end of read request
 * @param w_data, stream to write read result
 */

template <int CMDQ_DEPTH, int WDATA, int LATENCY, int OUTSTANDING, int BURSTLEN>
void access1DWrapper( // input
    ap_uint<64> offset[CMDQ_DEPTH],
    ap_uint<64> incr_num[CMDQ_DEPTH],
    ap_uint<64> total_num[CMDQ_DEPTH],
    ap_uint<32> cmd_num,
    hls::burst_maxi<ap_uint<WDATA> >& data,
    // output
    hls::stream<ap_axiu<WDATA, 0, 0, 0> >& w_data) {
#pragma HLS dataflow
    hls::stream<ap_uint<64> > r_offset("r_offset");
#pragma HLS stream variable = r_offset depth = OUTSTANDING
    hls::stream<ap_uint<10> > r_burst("r_burst");
#pragma HLS stream variable = r_burst depth = OUTSTANDING
    hls::stream<bool> e_r("e_r");
#pragma HLS stream variable = e_r depth = OUTSTANDING

    cmdParser<CMDQ_DEPTH, BURSTLEN>(offset, incr_num, total_num, cmd_num, r_offset, r_burst, e_r);
    manualBurstRead<WDATA, LATENCY, OUTSTANDING, BURSTLEN>(data, r_offset, r_burst, e_r, w_data);
}

template <int CMDQ_DEPTH, int LATENCY, int OUTSTANDING, int BURSTLEN>
void access1D( // input
    ap_uint<32> cmd_num,
    hls::burst_maxi<ap_uint<32> > cmd_params,
    hls::burst_maxi<ap_uint<32> > data,
    // output
    hls::stream<ap_axiu<32, 0, 0, 0> >& w_data) {
    ap_uint<64> offset[CMDQ_DEPTH];
    ap_uint<64> incr_num[CMDQ_DEPTH];
    ap_uint<64> total_num[CMDQ_DEPTH];

    cmdLoader<CMDQ_DEPTH, LATENCY, OUTSTANDING, BURSTLEN>(cmd_params, cmd_num, offset, incr_num, total_num);
    access1DWrapper<CMDQ_DEPTH, 32, LATENCY, OUTSTANDING, BURSTLEN>(offset, incr_num, total_num, cmd_num, data, w_data);
}

template <int WDATA, int CMDQ_DEPTH, int LATENCY, int OUTSTANDING, int BURSTLEN>
void access1DW( // input
    ap_uint<32> cmd_num,
    hls::burst_maxi<ap_uint<32> >& cmd_params,
    hls::burst_maxi<ap_uint<WDATA> >& data,
    // output
    hls::stream<ap_axiu<WDATA, 0, 0, 0> >& w_data) {
    ap_uint<64> offset[CMDQ_DEPTH];
    ap_uint<64> incr_num[CMDQ_DEPTH];
    ap_uint<64> total_num[CMDQ_DEPTH];

    cmdLoader<CMDQ_DEPTH, LATENCY, OUTSTANDING, BURSTLEN>(cmd_params, cmd_num, offset, incr_num, total_num);
    access1DWrapper<CMDQ_DEPTH, WDATA, LATENCY, OUTSTANDING, BURSTLEN>(offset, incr_num, total_num, cmd_num, data,
                                                                       w_data);
}

template <int WDATA>
void simpleSinker(hls::stream<ap_axiu<WDATA, 0, 0, 0> >& w_data, ap_uint<32> num, ap_uint<WDATA>* res) {
    ap_uint<WDATA> vv = 0;
    for (ap_uint<32> i = 0; i < num; i++) {
#pragma HLS pipeline II = 1
        vv ^= w_data.read().data;
    }
    res[0] = vv;
}

template <int WDATA>
void simpleGenerator(hls::stream<ap_axiu<WDATA, 0, 0, 0> >& w_data, ap_uint<32> num) {
    for (ap_uint<64> i = 0; i < num; i++) {
#pragma HLS pipeline II = 1
        ap_axiu<WDATA, 0, 0, 0> tmp_data;
        tmp_data.data = i;
        tmp_data.keep = -1;
        tmp_data.last = 0;
        w_data.write(tmp_data);
    }
}

/*
 * Parameters to define Cuboid Access
 * Single Cuboid    : An L x M x N cuboid
 *
 * @tparam BURSTLEN, burst length in maxi pragma, should be no bigger than 256
 *
 * @param offset, offset of first cuboid
 * @param i1, distance of adjacent elements in 1st dimension
 * @param d1, number of elements in vector in 1st dimension
 * @param i2, distance of adjacent vectors in 2nd dimension
 * @param d2, number of vectors in planes in 2nd dimension
 * @param i3, distance of adjacent planes in 3rd dimension
 * @param d3, number of planes in cuboid in 3rd dimension
 * @param i4, distance of adjacent cuboid in 4th dimension
 * @param d4, number of cuboid in hypercube in 4th dimension
 * @param r_offset, stream to output offset for burst read/write
 * @param r_burst, stream to output burst length for burst read/write
 * @param e_r, stream to output "if end" of burst request
 */

template <int BURSTLEN>
void cmdParser(ap_uint<32> offset,
               ap_uint<32> i1,
               ap_uint<32> d1,
               ap_uint<32> i2,
               ap_uint<32> d2,
               ap_uint<32> i3,
               ap_uint<32> d3,
               ap_uint<32> i4,
               ap_uint<32> d4,
               hls::stream<ap_uint<64> >& r_offset,
               hls::stream<ap_uint<10> >& r_burst,
               hls::stream<bool>& e_r) {
    ap_uint<10> x_inc;
    if (i1 == 1) {
        x_inc = BURSTLEN;
    } else {
        x_inc = 1;
    }

    ap_uint<32> s1, s2, s3, s4;

    s4 = offset;
    s3 = s4;
    s2 = s3;
    s1 = s2;

    for (ap_uint<32> w = 0; w < d4; w++) {
        s3 = s4;
        for (ap_uint<32> z = 0; z < d3; z++) {
            s2 = s3;
            for (ap_uint<32> y = 0; y < d2; y++) {
                s1 = s2;
            cmd_parser_inner_loop:
                for (ap_uint<32> x = 0; x < d1; x += x_inc) {
#pragma HLS pipeline II = 1

                    ap_uint<10> burst;
                    if (i1 == 1) {
                        if ((x + BURSTLEN) <= d1) {
                            burst = BURSTLEN;
                        } else {
                            burst = d1 - x;
                        }
                    } else {
                        burst = 1;
                    }

                    r_offset.write(s1);
                    r_burst.write(burst);
                    e_r.write(false);

                    if (i1 == 1) {
                        s1 += x_inc;
                    } else {
                        s1 += i1;
                    }
                }
                s2 += i2;
            }
            s3 += i3;
        }
        s4 += i4;
    }
    e_r.write(true);
}

template <int WDATA>
void cmdLoader(hls::burst_maxi<ap_uint<WDATA> >& data, ap_uint<64> params[8]) {
    // WDATA should be 64 or larger
    data.read_request(0, 8);
    for (int i = 0; i < 8; i++) {
        params[i] = data.read().range(63, 0);
    }
}

template <int WDATA, int LATENCY, int OUTSTANDING, int BURSTLEN>
void cuboidRead(
    // input
    ap_uint<64> offset,
    ap_uint<64> i1,
    ap_uint<64> d1,
    ap_uint<64> i2,
    ap_uint<64> d2,
    ap_uint<64> i3,
    ap_uint<64> d3,
    ap_uint<64> i4,
    ap_uint<64> d4,
    hls::burst_maxi<ap_uint<WDATA> >& data,
    // ouput
    hls::stream<ap_uint<WDATA> >& w_data,
    ap_uint<4> init) {
#pragma HLS dataflow
    hls::stream<ap_uint<64> > r_offset("r_offset");
#pragma HLS stream variable = r_offset depth = OUTSTANDING
    hls::stream<ap_uint<10> > r_burst("r_burst");
#pragma HLS stream variable = r_burst depth = OUTSTANDING
    hls::stream<bool> e_r("e_r");
#pragma HLS stream variable = e_r depth = OUTSTANDING

#pragma HLS FUNCTION_INSTANTIATE variable=init

    cmdParser<BURSTLEN>(offset, i1, d1, i2, d2, i3, d3, i4, d4, r_offset, r_burst, e_r);
    manualBurstRead<WDATA, LATENCY, OUTSTANDING, BURSTLEN>(data, r_offset, r_burst, e_r, w_data);
}

template <int WDATA, int LATENCY, int OUTSTANDING, int BURSTLEN>
void cuboidRead(
    // input
    ap_uint<64> offset,
    ap_uint<64> i1,
    ap_uint<64> d1,
    ap_uint<64> i2,
    ap_uint<64> d2,
    ap_uint<64> i3,
    ap_uint<64> d3,
    ap_uint<64> i4,
    ap_uint<64> d4,
    hls::burst_maxi<ap_uint<WDATA> >& data,
    // ouput
    hls::stream<ap_axiu<WDATA, 0, 0, 0> >& w_data,
    ap_uint<4> init) {
#pragma HLS dataflow
    hls::stream<ap_uint<64> > r_offset("r_offset");
#pragma HLS stream variable = r_offset depth = OUTSTANDING
    hls::stream<ap_uint<10> > r_burst("r_burst");
#pragma HLS stream variable = r_burst depth = OUTSTANDING
    hls::stream<bool> e_r("e_r");
#pragma HLS stream variable = e_r depth = OUTSTANDING

#pragma HLS FUNCTION_INSTANTIATE variable=init

    cmdParser<BURSTLEN>(offset, i1, d1, i2, d2, i3, d3, i4, d4, r_offset, r_burst, e_r);
    manualBurstRead<WDATA, LATENCY, OUTSTANDING, BURSTLEN>(data, r_offset, r_burst, e_r, w_data);
}

template <int WDATA, int LMN>
void reorderMN(
    // input
    ap_uint<32> d1,
    ap_uint<32> d2,
    ap_uint<32> d3,
    ap_uint<32> d4,
    hls::stream<ap_uint<WDATA> >& in,
    // ouput
    hls::stream<ap_axiu<WDATA, 0, 0, 0> >& out) {
    ap_uint<WDATA> buffer[LMN * 2];
#pragma HLS bind_storage variable = buffer type = RAM_S2P impl = URAM

LOOP_d4:
    for (ap_uint<32> i = 0; i <= d4; i++) { // loop to switch lower / higher half buffer
        int ws, rs;
        if ((i % 2) == 0) {
            ws = 0;
            rs = LMN;
        } else {
            ws = LMN;
            rs = 0;
        }

        int os = 0;
    LOOP_d3:
        for (ap_uint<32> l = 0; l < d1; l++) {
        LOOP_d1:
            for (ap_uint<32> k = 0; k < d3; k++) {
            LOOP_d2:
                for (ap_uint<32> j = 0; j < d2; j++) {
#pragma HLS pipeline II = 1
#pragma HLS dependence variable=buffer type=intra false
                    if (i < d4) {
                        buffer[ws + os] = in.read();
                        os++;
                    }
                    if (i > 0) {
                        ap_axiu<WDATA, 0, 0, 0> tmp_data;
                        tmp_data.data = buffer[rs + l + d1 * (j + k * d2)];
                        tmp_data.keep = -1;
                        tmp_data.last = 0;
                        out.write(tmp_data);
                    }
                }
            }
        }
    }
}

template <int WDATA, int LATENCY, int OUTSTANDING, int BURSTLEN, int LMN>
void cuboidReadReorder(
    // input
    ap_uint<64> offset,
    const ap_uint<64> i1,
    const ap_uint<64> d1,
    const ap_uint<64> i2,
    const ap_uint<64> d2,
    const ap_uint<64> i3,
    const ap_uint<64> d3,
    const ap_uint<64> i4,
    const ap_uint<64> d4,
    hls::burst_maxi<ap_uint<WDATA> >& data,
    // ouput
    hls::stream<ap_axiu<WDATA, 0, 0, 0> >& w_data,
    ap_uint<4> init) {
#pragma HLS dataflow
    hls::stream<ap_uint<64> > r_offset("r_offset");
#pragma HLS stream variable = r_offset depth = OUTSTANDING
    hls::stream<ap_uint<10> > r_burst("r_burst");
#pragma HLS stream variable = r_burst depth = OUTSTANDING
    hls::stream<bool> e_r("e_r");
#pragma HLS stream variable = e_r depth = OUTSTANDING
    hls::stream<ap_uint<WDATA> > wi_data;
#pragma HLS stream variable = wi_data depth = OUTSTANDING

#pragma HLS FUNCTION_INSTANTIATE variable=init

    cmdParser<BURSTLEN>(offset, i1, d1, i2, d2, i3, d3, i4, d4, r_offset, r_burst, e_r);
    manualBurstRead<WDATA, LATENCY, OUTSTANDING, BURSTLEN>(data, r_offset, r_burst, e_r, wi_data);
    reorderMN<WDATA, LMN>(d1, d2, d3, d4, wi_data, w_data);
}

template <int WDATA, int LATENCY, int OUTSTANDING, int BURSTLEN>
void cuboidWrite(
    // input
    ap_uint<64> offset,
    ap_uint<64> i1,
    ap_uint<64> d1,
    ap_uint<64> i2,
    ap_uint<64> d2,
    ap_uint<64> i3,
    ap_uint<64> d3,
    ap_uint<64> i4,
    ap_uint<64> d4,
    hls::stream<ap_uint<WDATA> >& w_data,
    // ouput
    hls::burst_maxi<ap_uint<WDATA> >& data,
    ap_uint<4> init) {
#pragma HLS dataflow
    hls::stream<ap_uint<64> > r_offset("r_offset");
#pragma HLS stream variable = r_offset depth = OUTSTANDING
    hls::stream<ap_uint<10> > r_burst("r_burst");
#pragma HLS stream variable = r_burst depth = OUTSTANDING
    hls::stream<bool> e_r("e_r");
#pragma HLS stream variable = e_r depth = OUTSTANDING

#pragma HLS FUNCTION_INSTANTIATE variable=init

    cmdParser<BURSTLEN>(offset, i1, d1, i2, d2, i3, d3, i4, d4, r_offset, r_burst, e_r);
    manualBurstWrite<WDATA, LATENCY, OUTSTANDING, BURSTLEN>(r_offset, r_burst, e_r, w_data, data);
}

template <int WDATA, int LATENCY, int OUTSTANDING, int BURSTLEN>
void cuboidWrite(
    // input
    ap_uint<64> offset,
    ap_uint<64> i1,
    ap_uint<64> d1,
    ap_uint<64> i2,
    ap_uint<64> d2,
    ap_uint<64> i3,
    ap_uint<64> d3,
    ap_uint<64> i4,
    ap_uint<64> d4,
    hls::stream<ap_axiu<WDATA, 0, 0, 0> >& w_data,
    // ouput
    hls::burst_maxi<ap_uint<WDATA> >& data,
    ap_uint<4> init) {
#pragma HLS dataflow
    hls::stream<ap_uint<64> > r_offset("r_offset");
#pragma HLS stream variable = r_offset depth = OUTSTANDING
    hls::stream<ap_uint<10> > r_burst("r_burst");
#pragma HLS stream variable = r_burst depth = OUTSTANDING
    hls::stream<bool> e_r("e_r");
#pragma HLS stream variable = e_r depth = OUTSTANDING

#pragma HLS FUNCTION_INSTANTIATE variable=init

    cmdParser<BURSTLEN>(offset, i1, d1, i2, d2, i3, d3, i4, d4, r_offset, r_burst, e_r);
    manualBurstWrite<WDATA, LATENCY, OUTSTANDING, BURSTLEN>(r_offset, r_burst, e_r, w_data, data);
}

#endif
