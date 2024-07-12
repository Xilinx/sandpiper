/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

/*******************************************************************************
 * File: serial_frame.cpp
 * 
 * Description:
 ******************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "serial_frame.hpp"

#define BYTES_PER_COMPLEX_INT (4)

#define CUBOID_FIRST  (0x01)
#define CUBOID_MIDDLE (0x02)
#define CUBOID_LAST   (0x04)

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
bool serial_frame::is_complete(uint32_t frame_length)
{
    size_t header_len  = sizeof(serial_frame);
    size_t payload_len = N * M * L * BYTES_PER_COMPLEX_INT;
    return ((start_of_frame == SERIAL_FRAME_SOF) &&
            (frame_length   >= header_len + payload_len));
}

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
bool serial_frame::is_valid()
{
    return (start_of_frame == SERIAL_FRAME_SOF);
}

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
bool serial_frame::is_first()
{
    return ((cuboid_num & CUBOID_FIRST) == CUBOID_FIRST);
}

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
bool serial_frame::is_last()
{
    return ((cuboid_num & CUBOID_LAST) == CUBOID_LAST);
}



