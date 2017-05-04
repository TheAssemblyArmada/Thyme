////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GAMEMEMORY.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: 
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef GAMEMEMORYINIT_H
#define GAMEMEMORYINIT_H

#include "always.h"

struct PoolInitRec
{
    const char *PoolName;
    int AllocationSize;
    int InitialAllocationCount;
    int OverflowAllocationCount;
};

struct PoolSizeRec
{
    char const *PoolName;
    int InitialAllocationCount;
    int OverflowAllocationCount;
};

void User_Memory_Adjust_Pool_Size(char const *name, int &initial_alloc, int &overflow_alloc);
void User_Memory_Get_DMA_Params(int *count, PoolInitRec const **params);
void User_Memory_Init_Pools();

#endif // GAMEMEMORYINIT_H
