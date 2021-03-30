/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Functions to call in main for win32 compatibility.
 *
 * @copyright Maverick is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "win32compat.h"

#ifdef _WIN32
#include <windows.h>

#include <stdlib.h>
#include <shellapi.h>
#include <stdio.h>
// Global so we can ensure the argument list is freed at exit.
static char **g_argv;

#ifdef MSC_VER
static void __cdecl Free_Argv(void)
#else
static void Free_Argv(void)
#endif
{
    LocalFree(g_argv);
}

// Taken from https://github.com/thpatch/win32_utf8/blob/master/src/shell32_dll.c
// Get the command line as UTF-8 as it would be on other platforms.
static char **CommandLineToArgvU(LPCWSTR lpCmdLine, int *pNumArgs)
{
    int cmd_line_pos; // Array "index" of the actual command line string
    // int lpCmdLine_len = wcslen(lpCmdLine) + 1;
    int lpCmdLine_len = WideCharToMultiByte(CP_UTF8, 0, lpCmdLine, -1, NULL, 0, NULL, NULL) + 1;
    char **argv_u;

    wchar_t **argv_w = CommandLineToArgvW(lpCmdLine, pNumArgs);

    if (!argv_w) {
        return NULL;
    }

    cmd_line_pos = *pNumArgs + 1;

    // argv is indeed terminated with an additional sentinel NULL pointer.
    argv_u = (char **)LocalAlloc(LMEM_FIXED, cmd_line_pos * sizeof(char *) + lpCmdLine_len);

    if (argv_u) {
        int i;
        char *cur_arg_u = (char *)&argv_u[cmd_line_pos];

        for (i = 0; i < *pNumArgs; i++) {
            size_t cur_arg_u_len;
            int conv_len;
            argv_u[i] = cur_arg_u;
            conv_len = WideCharToMultiByte(CP_UTF8, 0, argv_w[i], -1, cur_arg_u, lpCmdLine_len, NULL, NULL);

            cur_arg_u_len = argv_w[i] != NULL ? conv_len : conv_len + 1;
            cur_arg_u += cur_arg_u_len;
            lpCmdLine_len -= cur_arg_u_len;
        }

        argv_u[i] = NULL;

        if (g_argv != NULL) {
            LocalFree(g_argv);
        }

        g_argv = argv_u;
        atexit(Free_Argv);
    }

    LocalFree(argv_w);

    return argv_u;
}

void Handle_Win32_Args(int *argc, char ***argv)
{
    *argv = CommandLineToArgvU(GetCommandLineW(), argc);
}

void Handle_Win32_Console(void)
{
#ifndef __WATCOMC__
    /* Attach to the console that started us if any */
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        /* We attached successfully, lets redirect IO to the consoles handles */
        freopen("CONIN$", "r", stdin);
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
#endif
}
#endif
