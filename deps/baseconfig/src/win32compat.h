/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Functions to call in main for win32 compatibility.
 *
 * @copyright Baseconfig is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */

#ifndef BASECONFIG_WIN32COMPAT_H
#define BASECONFIG_WIN32COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
void Handle_Win32_Args(int *argc, char ***argv);
void Handle_Win32_Console(void);
#else
#define Handle_Win32_Args(x, y) ((void)0)
#define Handle_Win32_Console() ((void)0)
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* BASECONFIG_WIN32COMPAT_H */
