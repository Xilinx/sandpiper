/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

/*******************************************************************************
 * File: mem_dump.h
 *
 * Description:
 ******************************************************************************/
#ifndef MEM_DUMP_HPP__
#include <stdint.h>
extern "C"
{
    void mem_dump(uint8_t *buf, size_t len);
    void mem_dump_float(uint8_t *buf, size_t len);
}
#endif /* MEM_DUMP_HPP__ */
