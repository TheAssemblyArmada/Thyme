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
        if (strcasecmp(name, g_threadTracker[i]->name) == 0) {
            g_threadTracker[i]->id = id;

            return;
        }
    }

    ThreadTracker *tt = new ThreadTracker;
    tt->id = id;
    strlcpy(tt->name, name, sizeof(tt->name));
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
        if (strcasecmp(name, g_threadTracker[i]->name) == 0) {
            delete g_threadTracker[i];
            g_threadTracker.Delete(i);

            break;
        }
    }
}
