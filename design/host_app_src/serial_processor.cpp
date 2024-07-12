/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

/*******************************************************************************
 * File: serial_processor.cpp
 * 
 * Description:
*******************************************************************************/
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>
#include "adf.h"

#include <cstdint>
#include <cstring>
#include <stdio.h>

#include "serial_processor.hpp"
#include "serial_frame.hpp"

#include "adf/adf_api/XRTConfig.h"
#include "cube_params.h"

extern "C" {
#include "mem_dump.h"
}

/* 
 * Define the cube dimensions
 */
#define CUBE_M    (  10)
#define CUBE_N    (   4)
#define CUBE_L    (4000)

#define CUBOID_SIZE (CUBE_N * CUBE_M * CUBE_L)

/*
 * The Matlab client sends 10 data cubes
 */
#define NUM_CUBES   (10)

int run_adpca(serial_processor &sp, uint32_t *cube_data, size_t cube_data_size, cube_params_t cube_params[NUM_CUBES]);
/*
 * Allocate static array to hold the data cubes received from the Matlab client.
 * The data cube samples are complex int16_t,  so taking into consideration the 
 * in-phase and quadrature components, the size of each sample is equivalent to
 * 2 * sizeof(int16_t)
 */
static uint8_t m_cube_buffer_mapped[NUM_CUBES * CUBOID_SIZE * 2 * sizeof(int16_t)]; 
static cube_params_t cube_params[NUM_CUBES];

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
serial_processor::serial_processor(boost::asio::io_context& io_context)
    : m_socket(io_context)
{
    std::cout << "Serial Processor ctor" << std::endl;

    memset(rx_frm,  0, sizeof(rx_frm ));
    memset(aie_pld, 0, sizeof(aie_pld));
    memset(rd_buf,  0, sizeof(rd_buf ));

    m_buf_index = 0;
    frame_count = 0;
}

serial_processor::~serial_processor()
{
    std::cout << "Serial Processor dtor" << std::endl;
}


/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
boost::asio::ip::tcp::socket& serial_processor::get_socket()
{
    return m_socket;
}

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
void serial_processor::start()
{
    m_socket.async_read_some(boost::asio::buffer(rd_buf, RD_BUFFER_LEN),
        boost::bind(&serial_processor::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
}

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
void serial_processor::handle_read(const boost::system::error_code& error, size_t bytes_transferred)
{
    if (!error)
    {
        process_frame(rd_buf, bytes_transferred);

        m_socket.async_read_some(boost::asio::buffer(rd_buf, RD_BUFFER_LEN),
          boost::bind(&serial_processor::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      std::cout << "handle_read: Delete connection" << std::endl;
      delete this;
    }
}

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
void serial_processor::handle_write(const boost::system::error_code& error)
{
    if (!error)
    {
        //std::cout << "Response sent" << std::endl;;
    }
    else
    {
      std::cout << "handle_write: Delete connection 0" << std::endl;
      delete this;
    }
}

/*
 * FUNCTION:
 *    serial_processor::process_frame
 *
 * DESCRIPTION:
 *    This function places received serial data in a circular buffer.
 *    Once a complete frame has been received, it is placed in rx_frm
 *    and the frame payload is passed to matrix_mult() 
 */
void serial_processor::process_frame(uint8_t *buf, size_t len)
{
    cb.put(buf, len);

    while (frame_recvd())
    {
        serial_frame *frm;

        /*
         * Place the serial frame header in rx_frame
         */
        cb.peek(rx_frm, sizeof(serial_frame));
        frm = (serial_frame *) rx_frm;

        /*
         * Now that we have the header, use payload_len to place
         * the entire frame in rx_frm
         */
        std::cout << "cb.size " << cb.size() << " bytes" << std::endl;

        size_t payload_len = frm->N * frm->M * frm->L * sizeof(int);
        std::cout << "cb.get " << sizeof(serial_frame) + payload_len << " bytes" << std::endl;
        cb.get(rx_frm, sizeof(serial_frame) + payload_len);

        std::cout << "cb.size " << cb.size() << " bytes" << std::endl;
        std::cout << "Entire frame received" << std::endl;
        printf("Frame is last: %s\n", frm->is_last() ? "YES" : "NO");

        memcpy(&m_cube_buffer_mapped[m_buf_index], frm->payload, payload_len);
        m_buf_index += payload_len;

        if (frame_count < NUM_CUBES)
        {
            cube_params[frame_count].norm_dop_freq = ((float)frm->normDopFreq) / 32768;
            cube_params[frame_count].norm_spa_freq = ((float)frm->normSpaFreq) / 32768;
            cube_params[frame_count].gc = frm->n_guard_cells;
            cube_params[frame_count].tc = frm->n_train_cells;
            cube_params[frame_count].trangeIdx = frm->cell_under_test;

            frame_count++;

            if (frm->is_last())
            {
                run_adpca(*this, (uint32_t *)m_cube_buffer_mapped, m_buf_index, cube_params);

                m_buf_index = 0;
                frame_count = 0;
            }
        }
    }
}

void serial_processor::tx_result(cfloat adpca_result[4][1000], int seq)
{
    /*
     * Define the serial frame header for the response
     */
    uint8_t hdr_bytes[64] = {0};
    serial_frame_tx_header_t *hdr = (serial_frame_tx_header_t *) hdr_bytes; 

    hdr->start_of_frame = 32767.0;
    hdr->sequence_num   = seq;

    std::cout << "Sending header back to client" << std::endl;
    /*
     * Send the header back to the client
     */
    boost::asio::async_write(m_socket,
            boost::asio::buffer(hdr_bytes, sizeof(hdr_bytes)),
                    boost::bind(&serial_processor::handle_write, this,
                         boost::asio::placeholders::error)
            );

    std::cout << "Sending data back to client" << std::endl;
    /*
     * Send the matrix mult result payload back to the client
     */
    boost::asio::async_write(m_socket,
            boost::asio::buffer(adpca_result, 16*250*sizeof(cfloat)),
                    boost::bind(&serial_processor::handle_write, this,
                         boost::asio::placeholders::error)
            );

    std::cout << "...done" << std::endl;
}

/*
 * FUNCTION:
 *     serial_processor::frame_recvd()
 *
 * DESCRIPTION:
 *     This function evaluates the received byte stream to
 *     determine whether a valid frame has been received.
 */
bool serial_processor::frame_recvd()
{
    bool rval = true;

    if (cb.size() > sizeof(serial_frame))
    {
        serial_frame *frm;

        /*
         * Received data is enought for a full header.  Check for
         * avalid start-of-frame indicator. If we did not receive
         * a valid start-of-frame indicator,discard the data, and
         * wait for valid start-of-frame.
         */

        /*
         * Place the frame header in rx_frm 
         */
        cb.peek(rx_frm, sizeof(serial_frame));


        /*
         * Check for valid start-of-frame
         */
        frm = (serial_frame *) rx_frm;
        if (frm->is_valid())
        {
            //std::cout << "Rx " << cb.size() << " bytes valid frame data." << std::endl;
            rval = frm->is_complete(cb.size());
            //std::cout << "Rx complete: " << rval << std::endl;
        }
        else
        {
            std::cout << "Rx " << cb.size() << " bytes invalid frame data." << std::endl;
            /*
             * Perhaps instead of discarding all data, search for 
             * start of frame.
             */
            cb.reset();
            rval = false;
        }
    }
    else if (cb.size() >= sizeof(uint32_t))
    {
        serial_frame *frm;

        /*
         * Received data is less than a full header,  but larger
         * than the start-of-frame indicator.  Check for a valid
         * start-of-frame indicator.     If we did not receive a 
         * valid start-of-frame indicator,  discard the data,and
         * wait for valid start-of-frame.
         */

        /*
         * Place the start of frame indicator in rx_frm 
         */
        cb.peek(rx_frm, sizeof(uint32_t));

        /*
         * Check for valid start-of-frame 
         */
        frm = (serial_frame *) rx_frm;
        if (frm->is_valid() == false)
        {
            cb.reset();
        }
        else
        {
            std::cout << "Rx " << cb.size() << " bytes valid frame data." << std::endl;
        }

        rval = false;
    }
    else
    {
        /* 
         * Not enough bytes have been received for a 
         * complete frame
         */
        rval = false;
    }
    
    return rval;
}



/*
 * FUNCTION:
 *    serial_processor::cmd_matrix_mult
 *
 * DESCRIPTION:
 *    This function takes a serial frame payload and performs the necessary
 *    matrix multiplications using the received data.
 *
 *  NOTES:
 *     The serial_processor::matrix_mult function is currently the sole 
 *     command expected to be received.  If desired, additional command
 *     frames can be processed by populating the 'cmd' field in the serial
 *     frame header.
 */
#if 0
void serial_processor::cmd_matrix_mult(uint8_t *payload, size_t payload_len, float sequence_num )
{
    std::cout << "Perform matrix multiplication " << payload_len << std::endl;

    size_t input_w_size  = 2*C1*sizeof(float);  // 480
    size_t input_st_size = payload_len - input_w_size;

    memset(rslt, 0, sizeof(rslt));
    memset(w,    0, sizeof(w));
    memset(w_t,  0, sizeof(w_t));

    /*
     * Due to the tiler constraints of the matrix mult kernel, we need to 
     * replicate the (1x60) weight vector four times to create a (4x60)
     * input matrix.  Define some pointers to the start of each row in 
     * the (4x60) matrix.
     */
    memcpy(&w[0][0], &payload[0], input_w_size);
    memcpy(&w[1][0], &payload[0], input_w_size);
    memcpy(&w[2][0], &payload[0], input_w_size);
    memcpy(&w[3][0], &payload[0], input_w_size);

    /*
     * The matrix mult kernel requires the input matrix to be presented
     * in column-major format, so perform a transpose on the (4x60) input
     */
    transpose(w, w_t);

    /*
     * Point to the location of the weight (w) and space-time (st) matrices 
     * in the aie payload
     */
    uint8_t *aie_pld_w  = &aie_pld[          0];
    uint8_t *aie_pld_st = &aie_pld[sizeof(w_t)];

    /*
     * Point to the source space-time matrix, which has already been transposed
     * by the client
     */
    uint8_t *st_t       = &payload[input_w_size];

    /* 
     * Populate the payload we send give to matrix_mult_aie.
     */
    memcpy(aie_pld_w,  w_t,  input_w_size * NUM_DUPLICATE_ROWS);
    memcpy(aie_pld_st, st_t, input_st_size);

    /*
     * Adjust payload length to account for the replicated weights matrix
     */
    payload_len += input_w_size;
    payload_len += input_w_size;
    payload_len += input_w_size;

    /*
     * Perform the matrix multiplication
     * TODO: matrix_mult_aie can be placed in a worker thread
     */
    matrix_mult_aie(aie_pld, payload_len, (uint8_t *)rslt);

    /*
     * Define the serial frame header for the response
     */
    uint8_t hdr_bytes[16];
    serial_frame *hdr = (serial_frame *) hdr_bytes; 

    hdr->start_of_frame = SERIAL_FRAME_SOF;
    hdr->sequence_num   = sequence_num;
    hdr->cmd            = 0;
    hdr->payload_len    = (float)(sizeof(rslt)/NUM_DUPLICATE_ROWS);

    /*
     * Send the header back to the client
     */
    boost::asio::async_write(m_socket,
		    boost::asio::buffer(hdr_bytes, sizeof(hdr_bytes)),
                    boost::bind(&serial_processor::handle_write, this,
                         boost::asio::placeholders::error)
		    );

    /*
     * Send the matrix mult result payload back to the client
     */
    boost::asio::async_write(m_socket,
		    boost::asio::buffer(rslt, sizeof(rslt)/NUM_DUPLICATE_ROWS),
                    boost::bind(&serial_processor::handle_write, this,
                         boost::asio::placeholders::error)
		    );
}

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 *
 */
void serial_processor::transpose(complex_t A[][C1], complex_t B[][R1])
{
    int i;
    int j;

    for (i = 0; i < C1; i++)
    {
        for (j = 0; j < R1; j++)
        {
            B[i][j].r = A[j][i].r;
            B[i][j].i = A[j][i].i;
        }
    }
}
#endif
