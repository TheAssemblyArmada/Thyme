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
#include "critsection.h"
#include "gamedebug.h"
#include "stringex.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef PLATFORM_WINDOWS
#include <conio.h>
#include <direct.h>
#include <winbase.h>
#include <wincon.h>
#include <winuser.h>
#endif

#ifndef PLATFORM_WINDOWS
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef GAME_LOGGING
int DebugFlags = 0;
FILE *DebugLogFile = nullptr;
char DebugBuffer[DEBUG_BUFFER_SIZE];
static const char *DebugCaption = "";
CriticalSectionClass DebugMutex;

#ifdef PLATFORM_WINDOWS
HANDLE DebugConsoleHandle = 0;
#endif

void Remove_Unprintable(char *buffer)
{
    for (char *i = &buffer[strlen(buffer) - 1]; i >= buffer; --i) {
        char tmp = *i;

        //
        // Chars in the range 0 - 32 ('\0' - ' ') are unprintable so remove.
        //
        if (tmp >= '\0' && tmp < ' ' && tmp != '\n' && tmp != '\r') {
            *i = ' ';
        }
    }
}

char *Get_Tick_String()
{
    static char _tick_string[32];
    unsigned long ticks;

#ifdef PLATFORM_WINDOWS
    ticks = GetTickCount();
#elif defined PLATFORM_APPLE
    ticks = mach_absolute_time() / 1000000;
#else
    struct timespec now;

    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
        ticks = 0;
    }

    ticks = now.tv_sec * 1000 + now.tv_nsec / 1000000;
#endif

    sprintf(_tick_string, "(T=%08lx)", ticks);

    return _tick_string;
}

const char *Prep_Buffer(const char *format, char *buffer)
{
    *buffer = '\0';

    if (DebugFlags & DEBUG_PREFIX_TICKS) {
        strcpy(buffer, Get_Tick_String());
        strcat(buffer, " ");
    }

    return format;
}

char *Get_Time_String()
{
    time_t aclock;

    time(&aclock);

    return asctime(localtime(&aclock));
}

#ifdef PLATFORM_WINDOWS
void Debug_Wait_For_Input()
{
    if (DebugFlags & DEBUG_LOG_TO_CONSOLE) {
        printf("*** WAITING TO USER INPUT - PRESS ANY KEY ***\n");
        while (!_getch()) {
            Sleep(50);
        }
    }
}
#endif

#ifdef PLATFORM_WINDOWS
bool Set_Console_Properties(HANDLE handle, int x, int y, int w, int h)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
    COORD coordMax;

    coordMax = GetLargestConsoleWindowSize(handle);

    if (h > coordMax.Y) {
        h = coordMax.Y;
    }

    if (w > coordMax.X) {
        w = coordMax.X;
    }

    if (!GetConsoleScreenBufferInfo(handle, &info)) {
        return false;
    }

    // Height
    info.srWindow.Left = 0;
    info.srWindow.Right = info.dwSize.X - 1;
    info.srWindow.Top = 0;
    info.srWindow.Bottom = h - 1;

    if (h < info.dwSize.Y) {
        if (!SetConsoleWindowInfo(handle, TRUE, &info.srWindow)) {
            return false;
        }

        info.dwSize.Y = h;

        if (!SetConsoleScreenBufferSize(handle, info.dwSize)) {
            return false;
        }
    } else if (h > info.dwSize.Y) {
        info.dwSize.Y = h;

        if (!SetConsoleScreenBufferSize(handle, info.dwSize)) {
            return false;
        }

        if (!SetConsoleWindowInfo(handle, TRUE, &info.srWindow)) {
            return false;
        }
    }

    if (!GetConsoleScreenBufferInfo(handle, &info)) {
        return false;
    }

    // Width
    info.srWindow.Left = 0;
    info.srWindow.Right = w - 1;
    info.srWindow.Top = 0;
    info.srWindow.Bottom = info.dwSize.Y - 1;

    if (w < info.dwSize.X) {
        if (!SetConsoleWindowInfo(handle, TRUE, &info.srWindow)) {
            return false;
        }

        info.dwSize.X = w;

        if (!SetConsoleScreenBufferSize(handle, info.dwSize)) {
            return false;
        }
    } else if (w > info.dwSize.X) {
        info.dwSize.X = w;

        if (!SetConsoleScreenBufferSize(handle, info.dwSize)) {
            return false;
        }

        if (!SetConsoleWindowInfo(handle, TRUE, &info.srWindow)) {
            return false;
        }
    }

    // Work around for Watcom...
    // http://www.cplusplus.com/forum/beginner/12001/
    HINSTANCE kernel32 = LoadLibraryA("Kernel32.dll");
    typedef HWND (*PGetConsoleWindow)();
    PGetConsoleWindow MyGetConsoleWindow = (PGetConsoleWindow)GetProcAddress(kernel32, "GetConsoleWindow");
    SetWindowPos(MyGetConsoleWindow(), 0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    FreeLibrary(kernel32);

    return true;
}
#endif

void Debug_Console_Output(const char *buffer)
{
#ifdef PLATFORM_WINDOWS
    // TODO: Can we use the console handle to find the window the print to?
    printf("%s", buffer);
#else
    printf("%s", buffer);
#endif
}

void Debug_Init(int flags)
{
    if (DebugFlags != 0) {
        return;
    }

    char dirbuf[PATH_MAX];
    char curbuf[PATH_MAX];
    char prevbuf[PATH_MAX];

    DebugFlags = flags;

    if (DebugFlags & DEBUG_LOG_TO_FILE) {
#ifdef PLATFORM_WINDOWS
        char *tmp = getenv("USERPROFILE");

        if (tmp != NULL) {
            strcpy(dirbuf, tmp);
            strcat(dirbuf, "\\Documents\\Command and Conquer Generals Zero Hour Data");
            mkdir(dirbuf);
            strcat(dirbuf, "\\");
        } else {
            GetModuleFileNameA(0, dirbuf, sizeof(dirbuf));

            // Get the path to the executable minus the actual filename.
            for (char *i = &dirbuf[strlen(dirbuf)]; i >= dirbuf && (*i != '\\' || *i != '/'); --i) {
                *i = '\0';
            }
        }
#else
        /*char *homedir = getenv("HOME");
        if (homedir != nullptr) {
            strcpy(dirbuf, homedir);
        }
        if (homedir == nullptr)
            homedir = getpwuid(getuid())->pw_dir;
            if (homedir != nullptr) {
                strcpy(dirbuf, homedir);
            }
        }
        if (homedir != nullptr) {
            strcat(dirbuf, "/Command and Conquer Generals Zero Hour Data");
        }*/
#endif

        // const char *prefix = gAppPrefix;      //todo
        const char *prefix = "";
        strcpy(prevbuf, dirbuf);
        strcat(prevbuf, prefix);
        strcat(prevbuf, "ThymeDebugLogPrev.txt");
        strcpy(curbuf, dirbuf);
        strcat(curbuf, prefix);
        strcat(curbuf, "ThymeDebugLogFile.txt");
        remove(prevbuf);
        rename(curbuf, prevbuf);
        DebugLogFile = fopen(curbuf, "w");

        if (DebugLogFile != nullptr) {
            Debug_Log("Log %s opened: %s\n", curbuf, Get_Time_String());
        }
    }

    if (DebugFlags & DEBUG_LOG_TO_CONSOLE) {
#ifdef PLATFORM_WINDOWS
        AllocConsole();

        // Redirect the CRT standard input, output, and error handles to the console.
        freopen("CONIN$", "r", stdin);
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);

        // Set text colour to green.
        DebugConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(DebugConsoleHandle, FOREGROUND_INTENSITY | FOREGROUND_GREEN);

        Set_Console_Properties(DebugConsoleHandle, 0, 0, 120, 60);

        COORD buffsize = {120, 4096};
        SetConsoleScreenBufferSize(DebugConsoleHandle, buffsize);
#endif
    }
}

void Debug_Shutdown()
{
    if (DebugLogFile) {
        Debug_Log("Log closed: %s\n", Get_Time_String());
        fclose(DebugLogFile);
    }

    DebugLogFile = nullptr;
    DebugFlags = 0;
}

int Debug_Get_Flags()
{
    return DebugFlags;
}

void Debug_Set_Flags(int flags)
{
    DebugFlags = flags;
}

void Log_Output(const char *buffer)
{
    CriticalSectionClass::LockClass m(DebugMutex);

    if ((DebugFlags & DEBUG_LOG_TO_FILE)) {
        if (DebugLogFile != nullptr) {
            fprintf(DebugLogFile, "%s", buffer);
            fflush(DebugLogFile);
        }
    }

    if (DebugFlags & DEBUG_LOG_TO_DEBUGGER) {
#if defined(PLATFORM_WINDOWS)
        OutputDebugStringA(buffer);
#else
        printf("%s", buffer);
#endif
    }

    if (DebugFlags & DEBUG_LOG_TO_CONSOLE) {
        Debug_Console_Output(buffer);
    }
}

void Debug_Log(const char *format, ...)
{
    va_list va;
    va_start(va, format);

    if (DebugFlags == 0) {
#ifdef PLATFORM_WINDOWS
        MessageBoxA(nullptr, "Debug not initialised properly", DebugCaption, 0);
#else

#endif
    }

    vsprintf(&DebugBuffer[strlen(DebugBuffer)], Prep_Buffer(format, DebugBuffer), va);

    if (strlen(DebugBuffer) >= DEBUG_BUFFER_SIZE) {
#ifdef PLATFORM_WINDOWS
        MessageBoxA(nullptr, "String too long for debug buffer", DebugCaption, 0);
#else

#endif
    }

    Remove_Unprintable(DebugBuffer);

    Log_Output(DebugBuffer);
    va_end(va);
}

#endif // GAME_LOGGING
