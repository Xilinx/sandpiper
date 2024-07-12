/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

/*******************************************************************************
 * File: serial_frame.hpp
 * 
 * Description:
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>

//#define SERIAL_FRAME_SOF (0xA5A55A5A)
#define SERIAL_FRAME_SOF (0x7FFF)

typedef struct
{
    float start_of_frame;
    float sequence_num;
    float cuboid_num;
    float N;
    float M;
    float L;
    float normDopFreq;
    float normSpaFreq;
    float n_guard_cells;
    float n_train_cells;
    float cell_under_test;
    float samp_freq;
    float reserved0;
    float reserved1;
    float reserved2;
    float reserved3;
} serial_frame_tx_header_t;

class serial_frame
{
public:
    int16_t start_of_frame;
    int16_t sequence_num;
    int16_t cuboid_num;
    int16_t N;
    int16_t M;
    int16_t L;
    int16_t normDopFreq;
    int16_t normSpaFreq;
    int16_t n_guard_cells;
    int16_t n_train_cells;
    int16_t cell_under_test;
    int16_t samp_freq;
    int16_t reserved0;
    int16_t reserved1;
    int16_t reserved2;
    int16_t reserved3;
    uint8_t payload[];

    bool is_valid();
    bool is_first();
    bool is_last();
    bool is_complete(uint32_t len);
};
