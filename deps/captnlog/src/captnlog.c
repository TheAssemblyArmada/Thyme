/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Logging functions and configuration.
 *
 * @copyright CaptainsLog is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "captainslog.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#elif defined USE_PTHREADS
#include <pthread.h>
#endif

static struct
{
#ifdef _WIN32
    CRITICAL_SECTION mutex;
#elif defined USE_PTHREADS
    pthread_mutex_t mutex;
#endif
    FILE *fp;
    int level;
    bool console;
    bool syslog;
    bool print_time;
    bool initialized;
} g_state;

static const char *levels[] = {"NONE", "FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"};

static void lock()
{
#ifdef _WIN32
    EnterCriticalSection(&g_state.mutex);
#elif defined USE_PTHREADS
    pthread_mutex_lock(&g_state.mutex);
#endif
}

static void unlock()
{
#ifdef _WIN32
    LeaveCriticalSection(&g_state.mutex);
#elif defined USE_PTHREADS
    pthread_mutex_unlock(&g_state.mutex);
#endif
}

/**
 * Initialises the logging system.
 *
 * @param level Determines the max level of logging that will be carried out.
 * @param filename Sets the filename to log to, if this is NULL or the file cannot be opened, file logging is disabled.
 * @param console Does the logging system print to stderr?
 * @param syslog Does the logging system log to either syslog or the windows system logs?
 * @param print_time Does the logging system print the time on a log line?
 */
void captainslog_init(int level, const char *filename, bool console, bool syslog, bool print_time)
{
    if (!g_state.initialized) {
        if (filename != NULL) {
            g_state.fp = fopen(filename, "w");
        }

        g_state.level = level;
        g_state.console = console;
        g_state.syslog = syslog;
        g_state.print_time = print_time;

#ifdef _WIN32
        InitializeCriticalSection(&g_state.mutex);
#elif defined USE_PTHREADS
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&g_state.mutex, &attr);
#endif
        g_state.initialized = true;
    }
}

/**
 * Main log function, intended to be used from behind macros that pass in file and line details.
 */
void captainslog_log(int level, const char *file, int line, const char *fmt, ...)
{
    char buf[16];

    /* Don't log if the level is set lower than the level requested */
    if (level > g_state.level || !g_state.initialized) {
        return;
    }

    /* Lock the mutex to prevent multiple threads trying to write at the same time */
    lock();

    /* Get current time */
    if (g_state.print_time) {
        time_t t = time(NULL);
        struct tm *lt = localtime(&t);
        buf[strftime(buf, sizeof(buf), "%H:%M:%S ", lt)] = '\0';
    } else {
        buf[0] = '\0';
    }

    /* If we have a file pointer set we are logging to a file */
    if (g_state.fp != NULL) {
        va_list args;  
        fprintf(g_state.fp, "%s%-5s %s:%d: ", buf, levels[level], file, line);
        va_start(args, fmt);
        vfprintf(g_state.fp, fmt, args);
        fprintf(g_state.fp, "\n");
        va_end(args);
        fflush(g_state.fp);
    }

    /* Log to stderr if log to console is set */
    if (g_state.console) {
        va_list args;
        fprintf(stderr, "%s%-5s %s:%d: ", buf, levels[level], file, line);
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
        va_end(args);
        fflush(stderr);
    }

    /* TODO Log to syslog or windows system logs */

    unlock();
}

/**
 * Additional function for printing arbitrarily formatted lines.
 */
void captainslog_line(const char *fmt, ...)
{
    if (!g_state.initialized) {
        return;
    }

    /* Lock the mutex to prevent multiple threads trying to write at the same time */
    lock();

    /* If we have a file pointer set we are logging to a file */
    if (g_state.fp != NULL) {
        va_list args;
        va_start(args, fmt);
        vfprintf(g_state.fp, fmt, args);
        fprintf(g_state.fp, "\n");
        va_end(args);
        fflush(g_state.fp);
    }

    /* Log to stderr if log to console is set */
    if (g_state.console) {
        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
        va_end(args);
        fflush(stderr);
    }

    /* TODO Log to syslog or windows system logs */

    unlock();
}

/**
 * Uninitialise the logging system.
 */
void captainslog_deinit()
{
    if (g_state.initialized) {
        /* Don't start to deinit until we can lock the mutex in a threaded environment. */
        lock();
        g_state.initialized = false;

        if (g_state.fp != NULL) {
            fclose(g_state.fp);
            g_state.fp = NULL;
        }

        g_state.level = 0;
        g_state.console = false;
        g_state.syslog = false;
        g_state.print_time = false;
        unlock();
#ifdef _WIN32
        DeleteCriticalSection(&g_state.mutex);
#elif defined USE_PTHREADS
        pthread_mutex_destroy(&g_state.mutex);
#endif
    }
}
