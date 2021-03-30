include(CheckCXXSourceCompiles)
include(CheckSymbolExists)
include(CheckIncludeFile)
include(CheckFunctionExists)
include(CheckLibraryExists)

check_include_file(strings.h HAVE_STRINGS_H)
check_include_file(unistd.h HAVE_UNISTD_H)
check_include_file(libgen.h HAVE_LIBGEN_H)
check_include_file(dirent.h HAVE_DIRENT_H)
check_include_file(fnmatch.h HAVE_FNMATCH_H)
check_include_file(pwd.h HAVE_PWD_H)
check_include_file(sched.h HAVE_SCHED_H)
check_include_file(utime.h HAVE_UTIME_H)
check_include_file(sys/utime.h HAVE_SYS_UTIME_H)
check_include_file(sys/time.h HAVE_SYS_TIME_H)
check_include_file(sys/statvfs.h HAVE_SYS_STATVFS_H)
check_include_file(sys/sysinfo.h HAVE_SYS_SYSINFO_H)
check_include_file(sys/select.h HAVE_SYS_SELECT_H)
check_include_file(dispatch/dispatch.h HAVE_MACOS_GCD)

if(HAVE_STRINGS_H)
    check_symbol_exists(strcasecmp "strings.h" HAVE_STRCASECMP)
    check_symbol_exists(strncasecmp "strings.h" HAVE_STRNCASECMP)
endif()

# Check for the existence of some functions we need or their equivalents.
if(NOT HAVE_STRCASECMP)
    check_symbol_exists(_stricmp "string.h" HAVE__STRICMP)
    if(NOT HAVE__STRICMP)
        message(FATAL_ERROR "No case-insensitive compare function found, please report!")
    endif()
endif()

if(NOT HAVE_STRNCASECMP)
    check_symbol_exists(_strnicmp "string.h" HAVE__STRNICMP)
    if(NOT HAVE__STRNICMP)
        message(FATAL_ERROR "No case-insensitive size-limited compare function found, please report!")
    endif()
endif()

check_symbol_exists(strlwr "string.h" HAVE_STRLWR)
check_symbol_exists(strupr "string.h" HAVE_STRUPR)
check_symbol_exists(strlcat "string.h" HAVE_STRLCAT)
check_symbol_exists(strlcpy "string.h" HAVE_STRLCPY)
check_symbol_exists(nanosleep "time.h" HAVE_NANOSLEEP)
check_function_exists(timer_create HAVE_TIMER_CREATE)
check_function_exists(timer_settime HAVE_TIMER_SETTIME)

if(NOT HAVE_NANOSLEEP)
    check_symbol_exists(usleep "unistd.h" HAVE_USLEEP)
endif()

if(HAVE_TIMER_CREATE AND HAVE_TIMER_SETTIME)
    set(HAVE_POSIX_TIMERS TRUE BOOL)
else()
    check_library_exists(rt timer_create time.h HAVE_LIBRT)
    
    if (HAVE_LIBRT)
        set(HAVE_POSIX_TIMERS TRUE BOOL)
    endif()
endif()

check_cxx_source_compiles("
    #include <algorithm>
    int main(int argc, char **argv) {
        return std::clamp(argc, 0, 50);
    }" HAVE_STD_CLAMP)
    
if(NOT HAVE_STD_CLAMP)
    message("std::clamp not found, standard lower than C++17? Will provide implementation, though this is technically undefined behavior.")
endif()
