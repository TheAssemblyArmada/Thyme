////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: GAMEDEBUG.H
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
#pragma once

#ifndef GAMEDEBUG_H
#define GAMEDEBUG_H

#include "always.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef GAME_DEBUG_LOG

#define DEBUG_LOG(message, ...)		        Debug_Log(message, ##__VA_ARGS__)
#define DEBUG_LINE_LOG(message, ...)		Debug_Log("%s %d " message, __FILE__, __LINE__, ##__VA_ARGS__)
#define DEBUG_INIT(flags)		            Debug_Init(flags)
#define DEBUG_STOP()		                Debug_Shutdown()
#define DEBUG_ASSERT(exp)                   if (!(exp)) { Debug_Log("%s %d Assert failed", __FILE__, __LINE__); }
#define ASSERT_PRINT(exp, msg, ...)         if (!(exp)) { Debug_Log("%s %d " msg, __FILE__, __LINE__, ##__VA_ARGS__); }
#define DEBUG_ASSERT_PRINT(exp, msg, ...)   if (!(exp)) { Debug_Log("%s %d " msg, __FILE__, __LINE__, ##__VA_ARGS__); }
#define ASSERT_THROW(exp, except)           if (!(exp)) { Debug_Log("Oops, thyme is throwing an exception.\n"); throw except; }
#define ASSERT_THROW_PRINT(exp, except, msg, ...)         if (!(exp)) { Debug_Log("%s %d " msg, __FILE__, __LINE__, ##__VA_ARGS__); throw except; }

enum DebugOptions {
    DEBUG_LOG_TO_FILE    = 1 << 0,
    DEBUG_LOG_TO_CONSOLE = 1 << 1,
    DEBUG_PREFIX_TICKS   = 1 << 2,
    DEBUG_BUFFER_SIZE    = 0x2000,
};

void Debug_Init(int flags);
char *Get_Time_String(void);
void Debug_Log(const char *format, ...);
const char *Prep_Buffer(const char *format, char *buffer);
char *Get_Tick_String(void);
void Log_Output(const char *buffer);
void Remove_Unprintable(char *buffer);
void Debug_Crash(const char *format, ...);
int Debug_Crash_Box(const char *buffer, bool log_result);
void Debug_Shutdown(void);
int Debug_Get_Flags(void);
void Debug_Set_Flags(int flags);

#else   //GAME_DEBUG_LOG

#define DEBUG_LOG(message, ...)             ((void)0)
#define DEBUG_LINE_LOG(message, ...)		((void)0)
#define DEBUG_INIT(flags)		            ((void)0)
#define DEBUG_STOP()                        ((void)0)
#define DEBUG_ASSERT(exp)                   ((void)0)
#define ASSERT_PRINT(exp, msg, ...)         if (!(exp)) { }
#define DEBUG_ASSERT_PRINT(exp, msg, ...)   ((void)0)
#define ASSERT_THROW(exp, except)           if (!(exp)) { throw except; }
#define ASSERT_THROW_PRINT(exp, except, msg, ...)         if (!(exp)) { throw except; }

#endif  //GAME_DEBUG_LOG

#ifdef __cplusplus
} // extern "C"
#endif

#endif // _GAMEDEBUG_H
