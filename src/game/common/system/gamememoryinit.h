/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Custom memory manager designed to limit OS calls to allocate heap memory.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"

struct PoolInitRec
{
    const char *pool_name;
    int allocation_size;
    int initial_allocation_count;
    int overflow_allocation_count;
};

struct PoolSizeRec
{
    const char *pool_name;
    int initial_allocation_count;
    int overflow_allocation_count;
};

void User_Memory_Adjust_Pool_Size(const char *name, int &initial_alloc, int &overflow_alloc);
void User_Memory_Get_DMA_Params(int *count, PoolInitRec const **params);
void User_Memory_Init_Pools();