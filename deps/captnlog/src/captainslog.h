/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Logging and assertion library.
 *
 * @copyright CaptainsLog is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#ifndef CAPTAINSLOG_H
#define CAPTAINSLOG_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Default to debug logging */
#ifndef LOGGING_LEVEL
#define LOGGING_LEVEL 5
#endif

#define LOGLEVEL_NONE 0
#define LOGLEVEL_FATAL 1
#define LOGLEVEL_ERROR 2
#define LOGLEVEL_WARN 3
#define LOGLEVEL_INFO 4
#define LOGLEVEL_DEBUG 5
#define LOGLEVEL_TRACE 6

/* Conditionally define the function like macros for logging levels, allows only certain logging to be compiled into client
 * program. */
#if LOGLEVEL_TRACE <= LOGGING_LEVEL
#define captainslog_trace(x, ...) captainslog_log(LOGLEVEL_TRACE, __FILE__, __LINE__, x, ##__VA_ARGS__)
#else
#define captainslog_trace(x, ...) ((void)0)
#endif

#if LOGLEVEL_DEBUG <= LOGGING_LEVEL
#define captainslog_debug(x, ...) captainslog_log(LOGLEVEL_DEBUG, __FILE__, __LINE__, x, ##__VA_ARGS__)
#else
#define captainslog_debug(x, ...) ((void)0)
#endif

#if LOGLEVEL_INFO <= LOGGING_LEVEL
#define captainslog_info(x, ...) captainslog_log(LOGLEVEL_INFO, __FILE__, __LINE__, x, ##__VA_ARGS__)
#else
#define captainslog_info(x, ...) ((void)0)
#endif

#if LOGLEVEL_WARN <= LOGGING_LEVEL
#define captainslog_warn(x, ...) captainslog_log(LOGLEVEL_WARN, __FILE__, __LINE__, x, ##__VA_ARGS__)
#else
#define captainslog_warn(x, ...) ((void)0)
#endif

#if LOGLEVEL_ERROR <= LOGGING_LEVEL
#define captainslog_error(x, ...) captainslog_log(LOGLEVEL_ERROR, __FILE__, __LINE__, x, ##__VA_ARGS__)
#else
#define captainslog_error(x, ...) ((void)0)
#endif

#if LOGLEVEL_FATAL <= LOGGING_LEVEL
#define captainslog_fatal(x, ...) captainslog_log(LOGLEVEL_FATAL, __FILE__, __LINE__, x, ##__VA_ARGS__)
#define captainslog_fatal_alloc_varg_str(x, ...) captainslog_alloc_varg_str(LOGLEVEL_FATAL, x, ##__VA_ARGS__)
#define captainslog_fatal_free_varg_str(x) captainslog_free_varg_str(x)
#else
#define captainslog_fatal(x, ...) ((void)0)
#define captainslog_fatal_alloc_varg_str(x, ...) ((char *)NULL)
#define captainslog_fatal_free_varg_str(x, ...) ((void)0)
#endif

/**
 * Logging system settings
 *
 * @param level Determines the max level of logging that will be carried out.
 * @param filename Sets the filename to log to, if this is NULL or the file cannot be opened, file logging is disabled.
 * @param console Does the logging system print to stderr?
 * @param syslog Does the logging system log to either syslog or the windows system logs?
 * @param print_time Does the logging system print the time on a log line?
 * @param print_file Does the logging system print the file on a log line?
 */
typedef struct captains_settings_s
{
    int level;
    const char *filename;
    bool console;
    bool syslog;
    bool print_time;
    bool print_file;
} captains_settings_t;

/**
 * Logging control functions.
 */
#if LOGGING_LEVEL == LOGLEVEL_NONE
#define captainslog_init(settings) ((void)0)
#define captainslog_log(level, file, line, format, ...) ((void)0)
#define captainslog_line(fmt, ...) ((void)0)
#define captainslog_deinit() ((void)0)
#define captainslog_alloc_varg_str(level, format, ...) ((char *)NULL)
#define captainslog_free_varg_str(str) ((void)0)
#else
void captainslog_init(const captains_settings_t *settings);
void captainslog_log(int level, const char *file, int line, const char *fmt, ...);
void captainslog_line(const char *fmt, ...);
void captainslog_deinit();
char *captainslog_alloc_varg_str(int level, const char *fmt, ...);
void captainslog_free_varg_str(char *str);
#endif

/**
 * Debug break intrinsic.
 */
#if defined __GNUC__ || defined __clang__
#define trap_inline extern __attribute__((gnu_inline, always_inline)) inline
#else
#define trap_inline inline
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if __has_builtin(__builtin_debugtrap)
#define captainslog_debugtrap __builtin_debugtrap
#elif defined _WIN32 && !defined __WATCOMC__
# if defined(_M_IX86)
// Avoids including <intrin.h> for faster compilation in MSVC.
trap_inline void captainslog_debugtrap(void)
{
    __asm int 3
}
# else
#  include <intrin.h>
#  define captainslog_debugtrap __debugbreak
# endif
/* If we have GCC or compiler that tries to be compatible, use GCC inline assembly. */
#elif defined __GNUC__ || defined __clang__
#if defined(__i386__) || defined(__x86_64__)
trap_inline void captainslog_debugtrap(void)
{
    __asm__ volatile("int $0x03");
}
#elif defined(__arm__)
trap_inline void captainslog_debugtrap(void)
{
    __asm__ volatile("bkpt #3");
}
#elif defined(__aarch64__)
trap_inline void captainslog_debugtrap(void)
{
    /* same values as used by msvc __debugbreak on arm64 */
    __asm__ volatile("brk #0xF000");
}
#elif defined(__powerpc__)
trap_inline void captainslog_debugtrap(void)
{
    __asm__ volatile(".4byte 0x7d821008");
}
#else
#error captainslog_debugtrap not currently supported on this processor platform, see captntrap.h
#endif /* CPU architectures on GCC like compilers */
#elif defined __WATCOMC__
void captainslog_debugtrap(void);
#pragma aux captainslog_debugtrap = "int 3"
#endif

#define ASSERTS_NONE 0
#define ASSERTS_RELEASE 1
#define ASSERTS_DEBUG 2

#ifndef ASSERT_LEVEL
#define ASSERT_LEVEL ASSERTS_DEBUG
#endif

#if ASSERT_LEVEL == ASSERTS_DEBUG
#define captainslog_relassert(exp, to_throw, msg, ...) \
    do { \
        if (!(exp)) { \
            static volatile bool _ignore_assert = false; \
            static volatile bool _break = false; \
            if (!_ignore_assert) { \
                char *vamsg = captainslog_fatal_alloc_varg_str(msg, ##__VA_ARGS__); \
                captainslog_fatal("ASSERTION FAILED!\n" \
                                  "  Function:%s\n" \
                                  "  Expression:%s\n" \
                                  "  Message:%s\n\n", \
                    __CURRENT_FUNCTION__, \
                    #exp, \
                    vamsg ? vamsg : ""); \
                captainslog_fatal_free_varg_str(vamsg); \
                captainslog_assertfail( \
                    #exp, __FILE__, __LINE__, __CURRENT_FUNCTION__, &_ignore_assert, &_break, msg, ##__VA_ARGS__); \
            } \
            if (_break) { \
                captainslog_debugtrap(); \
            } \
        } \
    } while (false)

#define captainslog_dbgassert(exp, msg, ...) \
    do { \
        if (!(exp)) { \
            static volatile bool _ignore_assert = false; \
            static volatile bool _break = false; \
            if (!_ignore_assert) { \
                char *vamsg = captainslog_fatal_alloc_varg_str(msg, ##__VA_ARGS__); \
                captainslog_fatal("ASSERTION FAILED!\n" \
                                  "  Function:%s\n" \
                                  "  Expression:%s\n" \
                                  "  Message:%s\n\n", \
                    __CURRENT_FUNCTION__, \
                    #exp, \
                    vamsg ? vamsg : ""); \
                captainslog_fatal_free_varg_str(vamsg); \
                captainslog_assertfail( \
                    #exp, __FILE__, __LINE__, __CURRENT_FUNCTION__, &_ignore_assert, &_break, msg, ##__VA_ARGS__); \
            } \
            if (_break) { \
                captainslog_debugtrap(); \
            } \
        } \
    } while (false)
#define captainslog_assert(exp) \
    do { \
        if (!(exp)) { \
            static volatile bool _ignore_assert = false; \
            static volatile bool _break = false; \
            if (!_ignore_assert) { \
                captainslog_fatal("ASSERTION FAILED!\n" \
                                  "  Function:%s\n" \
                                  "  Expression:%s\n\n", \
                    __CURRENT_FUNCTION__, \
                    #exp); \
                captainslog_assertfail(#exp, __FILE__, __LINE__, __CURRENT_FUNCTION__, &_ignore_assert, &_break, NULL); \
            } \
            if (_break) { \
                captainslog_debugtrap(); \
            } \
        } \
    } while (false)
#elif ASSERT_LEVEL == ASSERTS_RELEASE
#define captainslog_relassert(exp, to_throw, msg, ...) \
    do { \
        if (!(exp)) { \
            throw to_throw; \
        } \
    } while (false)

#define captainslog_dbgassert(exp, msg, ...) ((void)0)
#define captainslog_assert(exp) ((void)0)
#else
#define captainslog_relassert(exp, to_throw, msg, ...) ((void)0)
#define captainslog_dbgassert(exp, msg, ...) ((void)0)
#define captainslog_assert(exp) ((void)0)
#endif

#if ASSERT_LEVEL == ASSERTS_NONE
#define captainslog_assertfail(expr, file, line, func, ignore, allow_break, msg, ...) ((void)0)
#define captainslog_ignoreasserts(ignore) ((void)0)
#define captainslog_allowpopups(allow) ((void)0)
#else
void captainslog_assertfail(const char *expr,
    const char *file,
    int line,
    const char *func,
    volatile bool *ignore,
    volatile bool *allow_break,
    const char *msg,
    ...);
void captainslog_ignoreasserts(bool ignore);
void captainslog_allowpopups(bool allow);
#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* LOG_CAPTNLOG_H */
