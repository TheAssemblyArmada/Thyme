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
    
    # Disable Run Time Checking.
    foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
        #message("Processing flags ${flag_var}")
        string(REGEX REPLACE "/RTC[^ ]*" "" ${flag_var} "${${flag_var}}")
    endforeach(flag_var)

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

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    # World builder will be Win32 only until enough is complete to port to other graphical toolkit.
    option(WORLD_BUILDER "Build World Builder." OFF)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${thyme_SOURCE_DIR}/cmake/modules)
set(CMAKE_NO_SYSTEM_FROM_IMPORTED TRUE) # Needed to prevent FindDirectX screwing up a mingw build.

include(CheckCXXCompilerFlag)

# Go lean and mean on windows.
if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
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
        set(CMAKE_SHARED_LIBRARY_PREFIX "")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${thyme_BINARY_DIR}/bin)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${thyme_BINARY_DIR}/bin)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -mabi=ms")
    endif()
endif ()

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
        find_package(DirectX COMPONENTS d3d8)
    endif()
endif()

find_package(ICU COMPONENTS data i18n io tu uc)

if(NOT WIN32 OR NOT "${CMAKE_SYSTEM}" MATCHES "Windows")
    if(NOT ICU_FOUND)
        message(FATAL_ERROR "ICU is required on non-windows platforms and was not found.")
    endif()
endif()

# Set where the build results will end up
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Set version info for the base config module
set(GITINFO_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(GITINFO_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(GITINFO_VERSION_PATCH ${PROJECT_VERSION_PATCH})

if(NOT STANDALONE)
    set(BASECONF_WINSOCK32 TRUE BOOL)
endif()

# Add base module
add_subdirectory(src/libs/baseconfig)

if(NOT STANDALONE)
    if(WORLD_BUILDER)
        set(SETSAIL_ENTRY 0x00B88F90)
        set(SETSAIL_DLL "thymewb.dll")
        set(SETSAIL_EXE "game.dat")
        set(SETSAIL_LAUNCHER launchthymewb)
        set(SETSAIL_HASH "3c1a1d431eae665978c9406f05c427252bc4e768")
        message("Configuring launcher as ${SETSAIL_LAUNCHER} using entry at ${SETSAIL_ENTRY} to inject ${SETSAIL_DLL} into ${SETSAIL_EXE}")
        # Build the launcher
        add_subdirectory(launcher)
    else()
        set(SETSAIL_ENTRY 0x008E0778)
        set(SETSAIL_DLL "thyme.dll")
        set(SETSAIL_EXE "game.dat")
        set(SETSAIL_LAUNCHER launchthyme)
        set(SETSAIL_HASH "5b805b4ecba79581fe8ec454b5acf7b7e13d4dd5")
        message("Configuring launcher as ${SETSAIL_LAUNCHER} using entry at ${SETSAIL_ENTRY} to inject ${SETSAIL_DLL} into ${SETSAIL_EXE}")
        # Build the launcher
        add_subdirectory(launcher)
    endif()
endif()

# Build Thyme
add_subdirectory(src)
