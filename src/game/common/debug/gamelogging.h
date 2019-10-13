/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Debug logging interface.
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
#include "captnlog.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef GAME_LOGGING

enum {
    DEBUG_BUFFER_SIZE = 4096
};

enum DebugOptions
{
    DEBUG_LOG_TO_FILE = 1 << 0,
    DEBUG_LOG_TO_DEBUGGER = 1 << 1,
    DEBUG_LOG_TO_CONSOLE = 1 << 2,
    DEBUG_PREFIX_TICKS = 1 << 3,
};

void Debug_Init(int flags);
void Debug_Shutdown();

#endif // GAME_LOGGING

#ifdef __cplusplus
} // extern "C"
#endif
