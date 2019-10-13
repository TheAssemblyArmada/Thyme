/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Macros for making use of the various debugging functions.
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
#include "captnassert.h"
#include "gameassert.h"
#include "gamelogging.h"

#ifdef GAME_LOGGING

#define DEBUG_INIT(flags) Debug_Init(flags)
#define DEBUG_STOP() Debug_Shutdown()

#define DEBUG_LOG(msg, ...) captain_line(msg, ##__VA_ARGS__)
#define DEBUG_LINE_LOG(msg, ...) captain_line("%s %d " msg, __FILE__, __LINE__, ##__VA_ARGS__)

#else // !GAME_LOGGING

#define DEBUG_INIT(flags) ((void)0)
#define DEBUG_STOP() ((void)0)

#define DEBUG_LOG(msg, ...) ((void)0)
#define DEBUG_LINE_LOG(msg, ...) ((void)0)

#endif // GAME_LOGGING

#ifdef GAME_ASSERTS
#define DEBUG_ASSERT(exp) captain_dbgassert(exp, "")
#define DEBUG_ASSERT_PRINT(exp, msg, ...) captain_dbgassert(exp, msg, ##__VA_ARGS__)
#define DEBUG_ASSERT_THROW(exp, except, msg, ...) captain_assert(exp, except, msg, ##__VA_ARGS__)

#else // !GAME_ASSERTS

#define DEBUG_ASSERT(exp) if (!(exp)) {}
#define DEBUG_ASSERT_PRINT(exp, msg, ...) if (!(exp)) {}
#define DEBUG_ASSERT_THROW(exp, except, msg, ...)  if (!(exp)) { throw except; }

#endif // GAME_ASSERTS
