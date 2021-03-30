/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Assert macros to catch unexpected conditions with a variety of responses.
 *
 * @copyright CaptainsLog is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "captainslog.h"
#include "captnmessage.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define ASSERT_BUFFER_SIZE 4096
#define MESSAGE_BUFFER_SIZE 1024

static struct
{
    bool ignore_all;
    bool allow_popup;
} g_state;

void captainslog_assertfail(const char *expr, const char *file, int line, const char *func, volatile bool *ignore,
    volatile bool *allow_break, const char *msg, ...)
{
    char msg_buf[MESSAGE_BUFFER_SIZE];
    
    if (*ignore || g_state.ignore_all) {
        return;
    }
    
    if (msg == NULL) {
        strcpy(msg_buf, "No additional information.");
    } else {
        va_list args;
        va_start(args, msg);
        vsnprintf(msg_buf, sizeof(msg_buf), msg, args);
        va_end(args);
    }

#ifdef _WIN32
    /* If we are in the debugger, we want to break. */
    if (IsDebuggerPresent()) {
        *allow_break = true;
    }
#endif

    if (g_state.allow_popup) {
        char assert_buf[ASSERT_BUFFER_SIZE];

        snprintf(assert_buf,
            sizeof(assert_buf),
            "Assertion failed!\n\n"
            "File: %s\nFunction: %s\nLine: %d\n\nExpression: %s\n\nMessage: %s\n\n"
            "For more information on how your program can cause an assertion failure, see the C++ doccumentation on "
            "asserts.\n\n"
            "(Press Abort to exit the application)\n"
            "(Press Retry to debug the application - JIT must be enabled)\n"
            "(Press Ignore to ignore this assertion for this session)",
            file,
            func,
            line,
            expr,
            msg_buf);

        switch (captainslog_messagebox(msg)) {
            case CAPTMSG_ABRT:
                exit(-1);
                break;

            case CAPTMSG_RET:
                *allow_break = true;
                break;

            case CAPTMSG_IGN:
                *ignore = true;
                break;

            default:
                break;
        };
    } else {
        *allow_break = true;
    }
}

void captainslog_ignoreasserts(bool ignore)
{
    g_state.ignore_all = ignore;
}

void captainslog_allowpopups(bool allow)
{
    g_state.allow_popup = allow;
}
