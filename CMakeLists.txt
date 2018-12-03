cmake_minimum_required(VERSION 3.1.0)

if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if(MSVC)
    set(MSVC_INCREMENTAL_DEFAULT ON)
endif()

project(thyme VERSION 1.04.0 LANGUAGES C CXX)

if(MSVC)
    string(REPLACE "INCREMENTAL" "INCREMENTAL:NO" replacementFlags ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DYNAMICBASE:NO /NXCOMPAT:NO /INCREMENTAL:NO ${replacementFlags}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/DYNAMICBASE:NO /NXCOMPAT:NO /INCREMENTAL:NO ${replacementFlags}")

    string(REPLACE "INCREMENTAL" "INCREMENTAL:NO" replacementFlags ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO})
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/INCREMENTAL:NO ${replacementFlags}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "/INCREMENTAL:NO ${replacementFlags}")
    # Set warning level 3
    # disable C4244: conversion from 'double' to 'float', possible loss of data
    # disable C4800: 'BOOL' : forcing value to bool 'true' or 'false' (performance warning)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /wd4244 /wd4800")
endif()

set(CMAKE_CXX_STANDARD 14)

# We don't support in tree builds, so help people make the right choice.
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Building in-source is not supported! Create a build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    set(DEFAULT_STANDALONE OFF)
else()
    set(DEFAULT_STANDALONE ON)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DEFAULT_LOGGING ON)
    set(DEFAULT_ASSERTIONS ON)
else()
    set(DEFAULT_ASSERTIONS OFF)
    set(DEFAULT_LOGGING OFF)
endif()

# This doesn't really work yet, work ongoing to make it usable
option(STANDALONE "Build a standalone version." ${DEFAULT_STANDALONE})
option(USE_GAMEMATH "Use own maths library rather than libc version for this platform." ON)
option(LOGGING "Enable debug logging." ${DEFAULT_LOGGING})
option(ASSERTIONS "Enable debug assertions." ${DEFAULT_ASSERTIONS})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${thyme_SOURCE_DIR}/cmake/modules)

include(CheckCXXCompilerFlag)

# Go lean and mean on windows.
if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
    if(NOT STANDALONE)
        add_definitions(-D_USE_32BIT_TIME_T) #This is for ABI compatibility with a few functions, remove when original binary no longer required.
    endif()
endif()

check_cxx_compiler_flag(-Wno-invalid-offsetof HAVE_NO_INVALID_OFFSETOF)

if(HAVE_NO_INVALID_OFFSETOF)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Prevent lib prefix when built with MinGW to target windows and move to own dir.
    if(MINGW)
        add_definitions(-D__STDC_FORMAT_MACROS)
        add_definitions(-D__STDC_LIMIT_MACROS)
        set(CMAKE_SHARED_LIBRARY_PREFIX "")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${thyme_BINARY_DIR}/bin)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${thyme_BINARY_DIR}/bin)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -mabi=ms")
    endif()
endif ()

# Include some standard CMake modules we use.
include(CheckSymbolExists)
include(CheckIncludeFile)

# Locate git binary to provide information to the build environment
find_package(Git)

if(GIT_FOUND)
    # Define current git branch.
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GITINFO_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    # Define commit hash.
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GITINFO_COMMIT_SHA1
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    # Define short commit hash.
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GITINFO_COMMIT_SHA1_SHORT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    # Define commit count.
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-list HEAD --count
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GITINFO_COMMIT_COUNT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    # Define commit time stamp.
    execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%ct
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GITINFO_COMMIT_TSTAMP
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    
    # Define commit author.
    execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:'%an'
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GITINFO_COMMIT_AUTHOR
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    string(REPLACE "'" "" GITINFO_COMMIT_AUTHOR ${GITINFO_COMMIT_AUTHOR})
    
    # Define commit date and time. Make builds more reproducible than using __DATE__ and __TIME__
    execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --date=iso --pretty=format:%cd
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GITINFO_DATE_AND_TIME
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    string(REPLACE " " ";" GITINFO_DATE_AND_TIME ${GITINFO_DATE_AND_TIME})
    list(GET GITINFO_DATE_AND_TIME 0 GITINFO_COMMIT_DATE)
    list(GET GITINFO_DATE_AND_TIME 1 GITINFO_COMMIT_TIME)
    list(GET GITINFO_DATE_AND_TIME 2 GITINFO_COMMIT_TZONE)
else()
    # Fallback information if git isn't found. This should never be present in official builds.
    set(GITINFO_BRANCH "netherrealm")
    set(GITINFO_COMMIT_SHA1 "DEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEF")
    set(GITINFO_COMMIT_SHA1_SHORT "DEADBEEF")
    set(GITINFO_COMMIT_COUNT 0)
    set(GITINFO_COMMIT_TSTAMP 0)
    set(GITINFO_COMMIT_DATE "1955-11-12")
    set(GITINFO_COMMIT_TIME "22:04:00")
    set(GITINFO_COMMIT_TZONE "+0000")
    set(GITINFO_COMMIT_AUTHOR "Someone")
endif()

configure_file(src/base/gitverinfo.cpp.in src/base/gitverinfo.cpp @ONLY)

if(STANDALONE)
    add_definitions(-DTHYME_STANDALONE)
endif()

if(NOT STANDALONE AND MSVC)
    set(USING_STLPORT TRUE BOOL INTERNAL)
endif()

if(USING_STLPORT)
    add_definitions(-DTHYME_USE_STLPORT)
endif()

set(THREADS_PREFER_PTHREAD_FLAG TRUE BOOL)

if(CMAKE_SIZEOF_VOID_P EQUAL 4 AND STANDALONE)
    find_package(DirectX COMPONENTS d3d8)
endif()

find_package(ICU COMPONENTS data i18n io tu uc)

if(NOT WIN32 OR NOT "${CMAKE_SYSTEM}" MATCHES "Windows")
    if(NOT ICU_FOUND)
        message(FATAL_ERROR "ICU is required on none windows platforms and was not found.")
    endif()
endif()

find_package(Threads REQUIRED)

# Check for the existence of platform specific headers.
check_include_file(strings.h HAVE_STRINGS_H)
check_include_file(intrin.h HAVE_INTRIN_H)
check_include_file(x86intrin.h HAVE_X86INTRIN_H)
check_include_file(cpuid.h HAVE_CPUID_H)

# Only use the pthread header if we don't have win32 threads.
if(CMAKE_USE_PTHREADS_INIT AND NOT CMAKE_USE_WIN32_THREADS_INIT)
    set(HAVE_PTHREAD_H TRUE BOOL)
endif()

# Check for the existence of some functions we need or their equivalents.
check_symbol_exists(strcasecmp "strings.h" HAVE_STRCASECMP)
if(NOT HAVE_STRCASECMP)
    check_symbol_exists(_stricmp "string.h" HAVE__STRICMP)
    if(NOT HAVE__STRICMP)
        message(FATAL_ERROR "No case-insensitive compare function found, please report!")
    endif()
endif()

check_symbol_exists(strncasecmp "strings.h" HAVE_STRNCASECMP)
if(NOT HAVE_STRNCASECMP)
    check_symbol_exists(_strnicmp "string.h" HAVE__STRNICMP)
    if(NOT HAVE__STRNICMP)
        message(FATAL_ERROR "No case-insensitive size-limited compare function found, please report!")
    endif()
endif()

include(CheckIntrinsics)

# Check for best macro that expands to current function name.
check_symbol_exists(__PRETTY_FUNCTION__ "" HAVE_PRETTY_FUNCTION_MACRO)
if(NOT HAVE_PRETTY_FUNCTION_MACRO)
    check_symbol_exists(__FUNCSIG__ "" HAVE_FUNCSIG_MACRO)

    if(NOT HAVE_FUNCSIG_MACRO)
        check_symbol_exists(__FUNCTION__ "" HAVE_FUNCTION_MACRO)
        if(NOT HAVE_FUNCTION_MACRO)
            check_symbol_exists(__func__ "" HAVE_FUNC_MACRO)
            if(NOT HAVE_FUNCTION_MACRO)
                message(FATAL_ERROR "No function name macro found, please report!")
            else()
                add_definitions(-D__CURRENT_FUNCTION__=__func__)
            endif()
        else()
            add_definitions(-D__CURRENT_FUNCTION__=__FUNCTION__)
        endif()
    else()
        add_definitions(-D__CURRENT_FUNCTION__=__FUNCSIG__)
    endif()
else()
    add_definitions(-D__CURRENT_FUNCTION__=__PRETTY_FUNCTION__)
endif()

configure_file(src/base/config.h.in src/base/config.h @ONLY)

# Set where the build results will end up
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(NOT STANDALONE)
    # Build the launcher
    add_subdirectory(launcher)
endif()

# Build Thyme
add_subdirectory(src)
