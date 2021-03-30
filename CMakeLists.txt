cmake_minimum_required(VERSION 3.1.0)

if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, defaults to Debug")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release" FORCE)
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
    
    if(CMAKE_SIZEOF_VOID_P EQUAL 4) # Only needs disabling for 32bit builds with old DirectX files.
        set(CMAKE_EXE_LINKER_FLAGS "/SAFESEH:NO /NODEFAULTLIB:libci.lib")
        set(CMAKE_SHARED_LINKER_FLAGS "/SAFESEH:NO /NODEFAULTLIB:libci.lib")
    endif()

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
option(USE_CRASHPAD "Enable the use of the Crashpad library for crash handing and reporting" OFF)

# Custom default install location.
if(NOT STANDALONE)
    get_filename_component(DLL_INSTALL_PREFIX  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour;InstallPath]" ABSOLUTE CACHE)
endif()

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    # World builder will be Win32 only until enough is complete to port to other graphical toolkit.
    option(BUILD_EDITOR "Build World Builder." OFF)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${thyme_SOURCE_DIR}/cmake/modules)
set(CMAKE_NO_SYSTEM_FROM_IMPORTED TRUE) # Needed to prevent FindDirectX screwing up a mingw build.

# Set up a format target to do automated clang format checking.
find_package(ClangFormat)
include(ClangFormat)

include(CheckCXXCompilerFlag)
include(GNUInstallDirs)

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

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(D3D8_FOUND TRUE)
    endif()

    # Assume DINPUT is fine and is found in windows sdk.
    set(DINPUT8_FOUND TRUE)
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
add_subdirectory(src/libs/baseconfig EXCLUDE_FROM_ALL)

if(ICU_FOUND)
    target_compile_definitions(base PRIVATE -DBUILD_WITH_ICU)
endif()

if(NOT STANDALONE)
    add_subdirectory(deps/launcher)
endif()

# Build miles and bink stubs if we don't have an alternative.
if(NOT OPENAL_FOUND AND NOT FFMPEG_FOUND)
    add_subdirectory(deps/bink EXCLUDE_FROM_ALL)
    add_subdirectory(deps/miles EXCLUDE_FROM_ALL)
endif()

# Build Thyme
add_subdirectory(src)

# Setup the install target destination
if(DLL_INSTALL_PREFIX AND ${CMAKE_VERSION} VERSION_GREATER "3.13.0")
    install(TARGETS ${TO_INSTALL} 
        RUNTIME DESTINATION ${DLL_INSTALL_PREFIX}
        LIBRARY DESTINATION ${DLL_INSTALL_PREFIX})
endif()
