/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Setup the crash handling functions to generate crash reports.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#if defined BUILD_WITH_CRASHPAD && defined _WIN32
void __cdecl Crashpad_Dump_Exception_Info(unsigned int u, struct _EXCEPTION_POINTERS *e_info);
#endif

bool Setup_Crash_Handler();
