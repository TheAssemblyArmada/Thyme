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
#include "threadtrack.h"
#include <cstring>

#pragma warning(push)
#pragma warning(disable : 4073) // warning C4073: initializers put in library initialization area
#pragma init_seg(lib) // Forces objects and variables in this file to initialize before other stuff.
#pragma warning(pop)

using std::strcmp;

#ifndef GAME_DLL
DynamicVectorClass<ThreadTracker *> g_threadTracker;
#endif

/**
 * Registers a thread in the tracker.
 */
void Register_Thread_ID(int id, const char *name, bool is_main)
{
    if (name == nullptr) {
        return;
    }

    for (int i = 0; i < g_threadTracker.Count(); ++i) {
        if (strcmp(name, g_threadTracker[i]->name) == 0) {
            g_threadTracker[i]->id = id;

            return;
        }
    }

    ThreadTracker *tt = new ThreadTracker;
    tt->id = id;
    strlcpy_tpl(tt->name, name);
    tt->is_main = is_main;
    tt->unknown = -1;
    g_threadTracker.Add(tt);
}

/**
 * Removes a thread from the tracker.
 */
void Unregister_Thread_ID(int id, const char *name)
{
    for (int i = 0; i < g_threadTracker.Count(); ++i) {
        if (strcmp(name, g_threadTracker[i]->name) == 0) {
            delete g_threadTracker[i];
            g_threadTracker.Delete(i);

            break;
        }
    }
}

/**
 * Gets the id of the first thread flagged as main or the first id in array if none flagged.
 */
int Get_Main_Thread_ID()
{
    for (int i = 0; i < g_threadTracker.Count(); ++i) {
        if (g_threadTracker[i]->is_main) {
            return g_threadTracker[i]->id;
        }
    }

    return g_threadTracker[0]->id;
}
