////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GAMEDEBUG.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: 
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#include "gamedebug.h"
#include "critsection.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef PLATFORM_WINDOWS
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#ifdef GAME_DEBUG_LOG
int DebugFlags;
FILE *DebugLogFile;
char DebugBuffer[DEBUG_BUFFER_SIZE];
static const char *DebugCaption = "";
CriticalSectionClass DebugMutex;

void Debug_Init(int flags)
{
    if ( DebugFlags != 0 ) {
        return;
    }

    char dirbuf[PATH_MAX];
    char curbuf[PATH_MAX];
    char prevbuf[PATH_MAX];

    DebugFlags = flags;
#ifdef PLATFORM_WINDOWS
    char *tmp = getenv("USERPROFILE");

    if ( tmp != NULL ) {
        strcpy(dirbuf, tmp);
        strcat(dirbuf, "\\Documents\\Command and Conquer Generals Zero Hour Data");
        mkdir(dirbuf);
        strcat(dirbuf, "\\");
    } else {
        GetModuleFileNameA(0, dirbuf, sizeof(dirbuf));

        // Get the path to the executable minus the actual filename.
        for ( char *i = &dirbuf[strlen(dirbuf)]; i >= dirbuf && (*i != '\\' || *i != '/'); --i ) {
            *i = '\0';
        }
    }
#else

#endif

    //const char *prefix = gAppPrefix;      //todo
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

    if ( DebugLogFile != nullptr ) {
        Debug_Log("Log %s opened: %s\n", curbuf, Get_Time_String());
    }
}

char *Get_Time_String(void)
{
    time_t aclock;

    time(&aclock);

    return asctime(localtime(&aclock));
}

void Debug_Log(const char *format, ...)
{
    va_list va;
    va_start(va, format);

    if ( DebugFlags == 0 ) {
    #ifdef PLATFORM_WINDOWS
        MessageBoxA(0, "Debug not initialised properly", DebugCaption, 0);
    #else

    #endif
    }

    vsprintf(&DebugBuffer[strlen(DebugBuffer)], Prep_Buffer(format, DebugBuffer), va);

    if ( strlen(DebugBuffer) >= 0x2000 ) {
    #ifdef PLATFORM_WINDOWS
        MessageBoxA(0, "String too long for debug buffer", DebugCaption, 0);
    #else

    #endif
    }

    Remove_Unprintable(DebugBuffer);
    Log_Output(DebugBuffer);
}

const char *Prep_Buffer(const char *format, char *buffer)
{
    *buffer = '\0';

    if ( DebugFlags & DEBUG_PREFIX_TICKS ) {
        strcpy(buffer, Get_Tick_String());
        strcat(buffer, " ");
    }

    return format;
}

char *Get_Tick_String(void)
{
    static char tick_string[32];
    unsigned long ticks;

#ifdef PLATFORM_WINDOWS
    ticks = GetTickCount();
#elif defined PLATFORM_APPLE
    ticks = mach_absolute_time() / 1000000;
#else
    struct timespec now;

    if ( clock_gettime(CLOCK_MONOTONIC, &now) != 0 ) {
        ticks = 0;
    }

    ticks = now.tv_sec * 1000 + now.tv_nsec / 1000000;
#endif

    sprintf(tick_string, "(T=%08lx)", ticks);

    return tick_string;
}

void Log_Output(const char *buffer)
{
    CriticalSectionClass::LockClass m(DebugMutex);

    if ( (DebugFlags & DEBUG_LOG_TO_FILE) && DebugLogFile != nullptr ) {
        fprintf(DebugLogFile, "%s", buffer);
        fflush(DebugLogFile);
    }

    if ( DebugFlags & DEBUG_LOG_TO_CONSOLE ) {
    #ifdef PLATFORM_WINDOWS
        OutputDebugStringA(buffer);
    #else
        printf(buffer);
    #endif
    }
}

void Remove_Unprintable(char *buffer)
{
    for ( char *i = &buffer[strlen(buffer) - 1]; i >= buffer; --i ) {
        char tmp = *i;

        //
        // Chars in the range 0 - 32 ('\0' - ' ') are unprintable so remove.
        //
        if ( tmp >= '\0' && tmp < ' ' && tmp != '\n' && tmp != '\r' ) {
            *i = ' ';
        }
    }
}

void Debug_Crash(const char *format, ...)
{
    va_list va;
    va_start(va, format);

    if ( DebugFlags = 0 ) {
    #ifdef PLATFORM_WINDOWS
        MessageBoxA(0, "Debug not initialized properly", DebugCaption, 0);
    #else

    #endif
    }

    strcat(DebugBuffer, "ASSERTION FAILURE: ");

    vsprintf(&DebugBuffer[strlen(DebugBuffer)], Prep_Buffer(format, DebugBuffer), va);

    if ( strlen(DebugBuffer) >= 0x2000 ) {
    #ifdef PLATFORM_WINDOWS
        MessageBoxA(0, "String too long for debug buffers", DebugCaption, 0);
    #else

    #endif
    }

    //if ( !DX8Wrapper_IsWindowed ) {
    //    Log_Output("**** CRASH IN FULL SCREEN - Auto-ignored, CHECK THIS LOG!\n");
    //}

    Remove_Unprintable(DebugBuffer);
    Log_Output(DebugBuffer);
    //doStackDump();
    //strcat(DebugBuffer, "\n\nAbort->exception; Retry->debugger; Ignore->continue\n");
    //if ( doCrashBox(DebugBuffer, 1) == 5 && TheCurrentAllowCrashPtr ) {
    //    if ( !DX8Wrapper_IsWindowed || MessageBoxA(0, "Ignore this crash from now on?", &password, 4u) == 6 )
    //        *(_BYTE *)TheCurrentAllowCrashPtr = 0;
    //    if ( TheKeyboard )
    //        Keyboard::resetKeys(TheKeyboard);
    //    if ( TheMouse )
    //        ((void(*)(void))TheMouse->vfptr->reset)();
    //}
}

int Debug_Crash_Box(const char *buffer, int log_result)
{
    return 0;
}

void Debug_Shutdown(void)
{
    if ( DebugLogFile ) {
        Debug_Log("Log closed: %s\n", Get_Time_String());
        fclose(DebugLogFile);
    }

    DebugLogFile = nullptr;
    DebugFlags = 0;
}

int Debug_Get_Flags(void)
{
    return DebugFlags;
}

void Debug_Set_Flags(int flags)
{
    DebugFlags = flags;
}

#else

#endif