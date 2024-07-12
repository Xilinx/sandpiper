/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

/*******************************************************************************
 * File: serial_processor.hpp
 * 
 * Description:
 ******************************************************************************/

#ifndef SERIAL_PROCESSOR_HPP__
#define SERIAL_PROCESSOR_HPP__

#include <cstdint>
#include <cstring>
#include <boost/asio.hpp>
#include "circular_buffer.hpp"

#include "xrt/xrt.h"
#include "xrt/experimental/xrt_kernel.h"
#include "adf.h"



#define R1 (  4)         // number of rows in Matrix-1
#define C1 ( 60)         // number of columns in Matrix-1
#define R2 ( 60)         // number of rows in Matrix-2
#define C2 (200)         // number of columns in Matrix-2

#define SERIAL_PROCESSOR_BUF_SIZE (650000)
#define NUM_DUPLICATE_ROWS        (R1)
#define RD_BUFFER_LEN             (2000)

typedef struct
{
    float r;
    float i;
} complex_t;

class serial_processor
{
    public:
        void process_frame(uint8_t *buf, size_t len);
        serial_processor(boost::asio::io_context& io_context);
        ~serial_processor();
        void start();
        boost::asio::ip::tcp::socket& get_socket();
        void tx_result(cfloat adpca_result[4][1000], int seq);
    private:
        void handle_read (const boost::system::error_code& error, size_t bytes_transferred);
	void handle_write(const boost::system::error_code& error);
        void cmd_matrix_mult(uint8_t *payload, size_t payload_len, float sequence_num);
        void cmd_cube_to_ddr(uint8_t *payload, size_t payload_len, float sequence_num);
	bool frame_recvd();
        void transpose(complex_t A[][C1], complex_t B[][R1]);
 
        /*
         * Rx buffer storage
         */
        circular_buffer cb;

        /*
         * Storage for a complete frame
         */
        uint8_t  rx_frm [SERIAL_PROCESSOR_BUF_SIZE];

	/*
	 * Matrix data to be passed to AI engine
	 */
        uint8_t  aie_pld[SERIAL_PROCESSOR_BUF_SIZE];

	/*
	 * Weights matrix and its transpose
	 */
        complex_t  w  [R1][C1];  // 4x60
        complex_t  w_t[C1][R1];  // 60x4

	/*
	 * Storage for matrix mult result
	 */
        complex_t rslt[R1][C2];

        /*
         * read buffer
         */
        uint8_t  rd_buf[RD_BUFFER_LEN];

	/*
	 * Server socket
	 */
        boost::asio::ip::tcp::socket m_socket;

	size_t m_buf_index;

    uint8_t frame_count;
};

#endif /* SERIAL_PROCESSOR_HPP__ */
