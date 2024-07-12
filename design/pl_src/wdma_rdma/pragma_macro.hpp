/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/


#ifndef _FAST_PRAGMA_HPP_
#define _FAST_PRAGMA_HPP_

#define _FAST_PRAGMA_(x) _Pragma(#x)
#define _FAST_PRAGMA(x) _FAST_PRAGMA_(x)

// pipeline
#define PPL(ii) _FAST_PRAGMA(HLS pipeline II = ii)
// axis
#define SAXI(_port) _FAST_PRAGMA(HLS interface axis port = _port)
// s_axilite
#define AXIL(_port) _FAST_PRAGMA(HLS interface s_axilite port = _port bundle = control)
// m_axi
#define MAXI(_port, _bundle, _latency, _r_out, _r_burst, _w_out, _w_burst, _depth)                                  \
    _FAST_PRAGMA(HLS interface m_axi offset = slave bundle = _bundle port = _port latency =                         \
                     _latency num_write_outstanding = _w_out num_read_outstanding = _r_out max_write_burst_length = \
                         _w_burst max_read_burst_length = _r_burst depth = _depth)
#define AXIMM(_port, _bundle, _latency, _r_out, _r_burst, _w_out, _w_burst, _depth)                                 \
    _FAST_PRAGMA(HLS interface m_axi offset = slave bundle = _bundle port = _port latency =                         \
                     _latency num_write_outstanding = _w_out num_read_outstanding = _r_out max_write_burst_length = \
                         _w_burst max_read_burst_length = _r_burst depth = _depth)                                  \
    _FAST_PRAGMA(HLS interface s_axilite port = _port bundle = control)

#ifndef __SYNTHESIS__
#include <iostream>
#define PRT(x) std::cout << #x << " = " << x << std::endl;
#define PRS(x) std::cout << #x << std::endl;
#define PRH(x) std::cout << std::hex << #x << " = " << x << std::dec << std::endl;
#else
#define PRT(x)
#define PRS(x)
#define PRH(x)
#endif

#endif
