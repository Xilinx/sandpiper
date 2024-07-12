/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

/*******************************************************************************
 * File: circular_buffer.cpp
 *
 * Description: Circular Buffer implementation
 * 
 * NOTE: Replace with Boost circular buffer
 ******************************************************************************/
#include <iostream>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "circular_buffer.hpp"
//#include "mem_dump.h"

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
bool circular_buffer::is_full(void)
{
    return full;
}

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
size_t circular_buffer::capacity()
{
    return (size_max - size());
}

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
size_t circular_buffer::size()
{
    size_t size;

    if (full)
    {
        size = size_max;
    }
    else
    {
        size = (head >= tail) ? (head - tail)
                              : (size_max - tail + head);
    }

    return size;
}

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
void circular_buffer::add(size_t len)
{
    head = (head + len) % size_max;
    full = (head == tail);
}

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
void circular_buffer::remove(size_t len)
{
    full = false;
    tail = (tail + len) % size_max;
}

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
void circular_buffer::reset()
{
    head = 0;
    tail = 0;
    full = false;
    memset(buf, 0, size_max);
}

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
int32_t circular_buffer::put(uint8_t *in, size_t len)
{
    int32_t r_val;

    if (len <= capacity())
    {
        if (head + len > size_max)
        {
            /*
             * The len parameter takes us past the end of the
             * array, so wrap-around.
             */
            size_t ar1_len = (size_max - head);
            size_t ar2_len = (len - ar1_len);
            memcpy(&buf[head], &in[0      ], ar1_len);
            memcpy(&buf[0   ], &in[ar1_len], ar2_len);
        }
        else
        {
            memcpy(&buf[head], &in[0      ], len);
        }

        add(len);

        r_val =  0;
    }
    else
    {
        r_val = -1;
    }

    return r_val;
}

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
int32_t circular_buffer::get(uint8_t *out, size_t len)
{
    int32_t r_val;

    r_val = peek(out, len);
    if (r_val == 0)
    {
        remove(len);
    }

    return r_val;
}

/*
 * FUNCTION:
 *
 * DESCRIPTION:
 */
int32_t circular_buffer::peek(uint8_t *out, size_t len)
{
    int32_t r_val;

    if (len <= size())
    {
        if (tail + len > size_max)
        {
            size_t ar1_len = (size_max - tail);
            size_t ar2_len = (len - ar1_len);
            memcpy(&out[0      ], &buf[tail], ar1_len);
            memcpy(&out[ar1_len], &buf[0   ], ar2_len);
        }
        else
        {
            memcpy(&out[0      ], &buf[tail], len);
        }

        r_val = 0;
    }
    else
    {
        r_val = -1;
    }

    return r_val;
}
