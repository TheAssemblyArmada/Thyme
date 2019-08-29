/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Track running threads in a vector.
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
#include "vector.h"

struct ThreadTracker
{
    char name[128];
    int id;
    int unknown;
    bool is_main;
};

void Register_Thread_ID(int id, const char *name, bool is_main);
void Unregister_Thread_ID(int id, const char *name);
int Get_Main_Thread_ID();

#ifdef GAME_DLL
extern DynamicVectorClass<ThreadTracker *> &g_threadTracker;
#else
extern DynamicVectorClass<ThreadTracker *> g_threadTracker;
#endif
