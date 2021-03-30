/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Globals for accessing git version information from the build system.
 *
 * @copyright BaseConfig is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#ifndef BASE_GITVERINFO_H
#define BASE_GITVERINFO_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

extern char g_gitBranch[];
extern char g_gitSHA1[];
extern char g_gitShortSHA1[];
extern char g_gitCommitTime[];
extern char g_gitCommitDate[];
extern char g_gitCommitAuthor[];
extern unsigned g_gitCommitCount;
extern time_t g_gitCommitTimeStamp;
extern unsigned g_majorVersion;
extern unsigned g_minorVersion;
extern unsigned g_patchVersion;

#ifdef __cplusplus
} // extern "C"
#endif

#endif // BASE_GITVERINFO_H
