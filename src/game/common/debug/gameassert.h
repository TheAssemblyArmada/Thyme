/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Debug assertion interface.
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

#ifdef __cplusplus
extern "C" {
#endif

#ifdef GAME_ASSERTS

extern bool ExitOnAssert; // Exit application on assertion when break button is pressed?
extern bool IgnoreAllAsserts; // Ignore all assertionss.
extern int GlobalIgnoreCount; // The number of assertions to ignore on a global basis.
extern int TotalAssertions; // The total number of assertions.
extern bool BreakOnException; // Break to debugger when a throw assertion is triggered.

enum
{
    ASSERT_BUFFER_SIZE = 4096
};

void Debug_Assert(char const *expr, char const *file, int const line, char const *func, char const *msg, volatile bool &_ignore, volatile bool &_break);

#endif // GAME_ASSERTS

#ifdef __cplusplus
} // extern "C"
#endif
