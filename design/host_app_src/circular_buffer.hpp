/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

/*******************************************************************************
 * File: circular_buffer.hpp
 * 
 * Description: Circular Buffer class definition. 
 *              NOTE: Possibly replace with Boost circular buffer
 * 
 ******************************************************************************/
#ifndef CIRCULAR_BUFFER_HPP__
#define CIRCULAR_BUFFER_HPP__

#include <stdint.h>
#include <stdlib.h>

#define CIRCULAR_BUFFER_SIZE_MAX (180000 * sizeof(int))

class circular_buffer
{
public:
    circular_buffer()
    {
        size_max = CIRCULAR_BUFFER_SIZE_MAX;
        reset();
    } 
    bool is_full(void);
    bool is_empty(void);
    void reset(void);
    int32_t put(uint8_t *buf, size_t len);
    int32_t get(uint8_t *out, size_t len);
    int32_t peek(uint8_t *out, size_t len);
    size_t capacity();
    size_t size();
private:
    uint8_t buf[CIRCULAR_BUFFER_SIZE_MAX];
    size_t head;
    size_t tail;
    size_t size_max;
    bool full;

    void add(size_t len);
    void remove(size_t len);
};

#endif /* CIRCULAR_BUFER_HPP__ */
