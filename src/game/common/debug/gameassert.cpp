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
#include "gameassert.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PLATFORM_WINDOWS
#include <winbase.h>
#include <winuser.h>
#endif

#ifdef GAME_ASSERTS

bool ExitOnAssert = true;
bool IgnoreAllAsserts = false;
int GlobalIgnoreCount = 0;
int TotalAssertions = 0;
bool BreakOnException = false;

bool ShowAssertionDialog
#ifdef PLATFORM_WINDOWS
 = true;
#else // !PLATFORM_WINDOWS
 = false;
#endif // PLATFORM_WINDOWS

enum AssertButtonType
{
    BUTTON_ABORT,
    BUTTON_RETRY,
    BUTTON_IGNORE,
    BUTTON_BREAK,
    BUTTON_SKIP
};

AssertButtonType Debug_Assertion_Dialog(char const *expr, char const *file, int const line, char const *func, char const *msg)
{
    AssertButtonType result = BUTTON_RETRY;

    char msgbuff[ASSERT_BUFFER_SIZE];

#ifdef PLATFORM_WINDOWS

    char filename[PATH_MAX];
    GetModuleFileNameA(nullptr, filename, PATH_MAX);

    snprintf(msgbuff,
             sizeof(msgbuff),
             "Assertion failed!\n\n"
             "Program: %s\nFile: %s\nFunction: %s\nLine: %d\n\nExpression: %s\n\nMessage: %s\n\n"
             "For more information on how your program can cause an assertion failure, see the C++ doccumentation on asserts.\n\n"
             "(Press About to exist the application)\n"
             "(Press Retry to debug the application - JIT must be enabled)\n"
             "(Press Ignore to ignore this assertion for this session)",
             filename,
             file,
             func,
             line,
             expr,
             msg);

    DWORD msgresult = MessageBoxA(nullptr, msgbuff, "Thyme - Assertion failed!", MB_ICONERROR | MB_ABORTRETRYIGNORE | MB_DEFBUTTON2);

    switch (msgresult) {
        case IDABORT:
            result = BUTTON_ABORT;
            break;

        default:
        case IDRETRY:
            result = BUTTON_RETRY;
            break;

        case IDIGNORE:
            result = BUTTON_IGNORE;
            break;
    };

    if (result == BUTTON_IGNORE) {
        int tmpres = MessageBoxA(nullptr,
            "Do you wish to ignore any further occurrences of this assertion?",
            "Thyme - Ignore Assertion?",
            MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON1);
        if (tmpres == IDNO) {
            result = BUTTON_SKIP;
        }
    }

#elif defined PLATFORM_LINUX
    // TODO: Linux GTK?
    // https://github.com/aaronmjacobs/Boxer

    // Break on the assert for now.
    result = BUTTON_BREAK;

#elif defined PLATFORM_OSX
    // TODO
    // http://stackoverflow.com/questions/17319180/sdl-cross-platform-message-box-for-linux
    // https://github.com/aaronmjacobs/Boxer/blob/master/src/boxer_osx.mm

    let alert = NSAlert();

    alert.alertStyle =.WarningAlertStyle;

    alert.messageText = "Assertion failed!";
    alert.informativeText = msgbuff;

    alert.addButtonWithTitle("Abort");
    alert.addButtonWithTitle("Retry");
    alert.addButtonWithTitle("Ignore");

    let dlg_result = alert.runModal();

    switch (dlg_result) {
        case NSAlertFirstButtonReturn:
            result = BUTTON_ABORT;
            break;

        case NSAlertSecondButtonReturn:
            result = BUTTON_IGNORE;
            break;

        default:
        case NSAlertThirdButtonReturn:
            result = BUTTON_RETRY;
            break;
    };
#else
    result = BUTTON_BREAK;
#endif

    // Return the button pressed.
    return result;
}

void Debug_Assert(char const *expr, char const *file, int const line, char const *func, char const *msg,
    volatile bool &_ignore, volatile bool &_break)
{
    ++TotalAssertions;

    if (!IgnoreAllAsserts) {

        if (!_ignore) {

            if (msg == nullptr) {
                msg = "No additional information.";
            }

        #ifdef PLATFORM_WINDOWS
            // If we are in the debugger, we want to break.
            if (IsDebuggerPresent()) {
                _break = true;
            }
        #endif // PLATFORM_WINDOWS

            if (ShowAssertionDialog) {

                switch (Debug_Assertion_Dialog(expr, file, line, func, msg)) {
                    case BUTTON_ABORT:
                        if (ExitOnAssert) {
                            //TODO: Need to move Emergency_Exit() from startup.
                            //Emergency_Exit(-1);
                        }
                        _break = true;
                        break;

                    case BUTTON_RETRY:
                        // break; // We want this to fall through for now.

                    case BUTTON_BREAK:
                        _break = true;
                        break;

                    case BUTTON_IGNORE:
                        _ignore = true;
                        ++GlobalIgnoreCount;
                        break;

                    case BUTTON_SKIP:
                        break;
                };

            } else {
                _break = true;
            }
        }
    }
}

#endif // GAME_ASSERTS
