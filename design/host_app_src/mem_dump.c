/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

/*******************************************************************************
 * File: mem_dump.c
 *
 * Description:
 ******************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void mem_dump(uint8_t *buf, size_t len)
{
    int i;
    printf("mem dump:\n");
    for (i = 0; i < len; i++)
    {
        if ((i != 0) && (i %  8) == 0) {printf(" " ); }
        if ((i != 0) && (i % 16) == 0) {printf("\n"); }
        printf("%02x ", buf[i]);
    }
    printf("\n");
}

void mem_dump_float(uint8_t *buf, size_t len)
{
    int i;
    printf("mem dump:\n");
    for (i = 0; i < len; i += 2*sizeof(float))
    {
        float *real = (float *) &buf[i];
        float *imag = (float *) &buf[i + sizeof(float)];
        printf("r = %f, i = %f\n", *real, *imag);
    }
    printf("\n");
}
