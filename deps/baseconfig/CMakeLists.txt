cmake_minimum_required(VERSION 3.1.0)

if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

project(baseconfig VERSION 1.00 LANGUAGES C CXX)

# We don't support in tree builds, so help people make the right choice.
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Building in-source is not supported! Create a build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()

# Include some standard CMake modules we use.
include(CheckSymbolExists)
include(CheckIncludeFile)

# Locate git binary to provide information to the build environment
find_package(Git)

if(GIT_FOUND)
    # Define current git branch.
    if(NOT GITINFO_BRANCH)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GITINFO_BRANCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
    endif()
    
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

# Set default version info for the git version module.
if(NOT GITINFO_VERSION_MAJOR)
    set(GITINFO_VERSION_MAJOR 0)
endif()

if(NOT GITINFO_VERSION_MINOR)
    set(GITINFO_VERSION_MINOR 0)
endif()

if(NOT GITINFO_VERSION_PATCH)
    set(GITINFO_VERSION_PATCH 0)
endif()

configure_file(src/gitverinfo.c.in src/gitverinfo.c @ONLY)

# Check for threading, win32 or posix.
find_package(Threads)

# Only use the pthread header if we don't have win32 threads.
if(CMAKE_USE_PTHREADS_INIT AND NOT CMAKE_USE_WIN32_THREADS_INIT)
    set(HAVE_PTHREAD_H TRUE BOOL)
endif()

# Find intrinsics and functions
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckIntrinsics.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckFunctions.cmake)

configure_file(src/config.h.in src/config.h @ONLY)

if(CMAKE_CONFIGURATION_TYPES)
    file(GLOB_RECURSE BASE_HEADERS "*.h")
endif()

add_library(base STATIC src/stringex.c src/win32compat.c ${CMAKE_CURRENT_BINARY_DIR}/src/gitverinfo.c ${BASE_HEADERS})
target_include_directories(base PUBLIC src ${CMAKE_CURRENT_BINARY_DIR}/src)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_compile_definitions(base PUBLIC WIN32_LEAN_AND_MEAN=1 _CRT_SECURE_NO_DEPRECATE=1 _CRT_NONSTDC_NO_DEPRECATE=1 _WINSOCK_DEPRECATED_NO_WARNINGS=1 NOMINMAX=1)
    
    if(BASECONF_WINSOCK32)
        target_compile_definitions(base PUBLIC BUILD_WITH_WINSOCK=1)
        target_link_libraries(base INTERFACE wsock32) # Link original winsock32 to coincide with includes.
    else()
        target_link_libraries(base INTERFACE ws2_32)
    endif()
endif()

# We don't need to link anything if we have win32 threads. Affects MinGW builds primarily.
if(CMAKE_USE_PTHREADS_INIT AND NOT CMAKE_USE_WIN32_THREADS_INIT)
    target_link_libraries(base Threads::Threads)
endif()

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
                target_compile_definitions(base PUBLIC __CURRENT_FUNCTION__=__func__)
            endif()
        else()
            target_compile_definitions(base PUBLIC __CURRENT_FUNCTION__=__FUNCTION__)
        endif()
    else()
        target_compile_definitions(base PUBLIC __CURRENT_FUNCTION__=__FUNCSIG__)
    endif()
else()
    target_compile_definitions(base PUBLIC __CURRENT_FUNCTION__=__PRETTY_FUNCTION__)
endif()

