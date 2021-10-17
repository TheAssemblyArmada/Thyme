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
option(USE_CRASHPAD "Enable the use of the Crashpad library for crash handling and reporting." OFF)
option(BUILD_TOOLS "Builds the developer/debug tools." OFF)
option(THYME_FEATURES "Enables features that extend the original game functionality." ON)
option(THYME_FIXES "Enables fixes that get rid of original game issues." ON)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        option(BUILD_DLL "Build an injectable dll version." ON)
    endif()
endif()

# Custom default install location.
if(BUILD_DLL)
    get_filename_component(DLL_INSTALL_PREFIX  "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour;InstallPath]" ABSOLUTE CACHE)
endif()

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    # World builder will be Win32 only until enough is complete to port to other graphical toolkit.
    option(BUILD_EDITOR "Build World Builder." OFF)
endif()

# If no binary target is set to build, build standalone.
if(NOT BUILD_DLL AND NOT STANDALONE AND NOT BUILD_TOOLS AND NOT BUILD_EDITOR)
    set(STANDALONE ON)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${thyme_SOURCE_DIR}/cmake/modules)

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
        add_subdirectory(deps/olddx EXCLUDE_FROM_ALL)
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

# Try and find wxWidgets which is used for some GUI tools.
find_package(wxWidgets COMPONENTS core base xrc expat xml html adv)

if(NOT wxWidgets_FOUND AND BUILD_TOOLS)
    message(WARNING "Debug tools are enabled, but wxWidgets was not found.") 
    message(WARNING "GUI tools will not be built.")
endif()

# Set where the build results will end up
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Set version info for the base config module
set(GITINFO_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(GITINFO_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(GITINFO_VERSION_PATCH ${PROJECT_VERSION_PATCH})

if(BUILD_DLL)
    set(BASECONF_WINSOCK32 TRUE BOOL)
endif()

# Add base module
add_subdirectory(deps/baseconfig EXCLUDE_FROM_ALL)

if(ICU_FOUND)
    target_compile_definitions(base PRIVATE -DBUILD_WITH_ICU)
endif()

# Enable Thyme debug logging.
if(LOGGING)
    set(CAPTNLOG_LEVEL $<$<CONFIG:Debug>:5>$<$<CONFIG:RelWithDebInfo>:3>$<$<CONFIG:Release>:3>$<$<CONFIG:MinSizeRel>:3>)
else()
    set(CAPTNLOG_LEVEL 0)
endif()

# Enable Thyme debug assertions.
if(ASSERTIONS)
    set(CAPTNASSERT_LEVEL $<$<CONFIG:Debug>:2>$<$<CONFIG:RelWithDebInfo>:1>$<$<CONFIG:Release>:1>$<$<CONFIG:MinSizeRel>:1>)
else()
    set(CAPTNASSERT_LEVEL 0)
endif()

# Setup various included libraries
add_subdirectory(deps/captnlog EXCLUDE_FROM_ALL)

# Build launchers for the various binaries we support
if(BUILD_DLL)
    add_subdirectory(deps/launcher)
    list(APPEND INSTALL_TARGETS thymelauncher)
endif()

# Build miles and bink stubs if we don't have an alternative.
if(CMAKE_SIZEOF_VOID_P EQUAL 4 AND NOT OPENAL_FOUND AND NOT FFMPEG_FOUND)
    add_subdirectory(deps/bink EXCLUDE_FROM_ALL)
    add_subdirectory(deps/miles EXCLUDE_FROM_ALL)
endif()

# If we want to include our own math library for cross platform binary consistency, fetch it and add it.
if(USE_GAMEMATH)
    include(FetchContent)

    FetchContent_Declare(
        gamemath_git
        GIT_REPOSITORY https://github.com/TheAssemblyArmada/GameMath.git
        GIT_TAG        e041ae49cdd097a2b8110a26d2bc20660d6c31fb
    )

    # We don't use FetchContent_MakeAvailable here because we don't want all gamemath targets including, just our dependencies.
    FetchContent_GetProperties(gamemath_git)
    if(NOT gamemath_git_POPULATED)
        FetchContent_Populate(gamemath_git)
        add_subdirectory(${gamemath_git_SOURCE_DIR} ${gamemath_git_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
endif()

if(USE_CRASHPAD)
    include(FetchContent)

    FetchContent_Declare(
        crashpad_cmake
        GIT_REPOSITORY https://github.com/TheAssemblyArmada/crashpad-cmake.git
        GIT_TAG        6e89ec1542627d7955561c2955e9ff20d6f69f65
    )

    # We don't use FetchContent_MakeAvailable here because we don't want all crashpad targets including, just our dependencies.
    FetchContent_GetProperties(crashpad_cmake)
    if(NOT crashpad_cmake_POPULATED)
        FetchContent_Populate(crashpad_cmake)
        add_subdirectory(${crashpad_cmake_SOURCE_DIR} ${crashpad_cmake_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()

    set_target_properties(crashpad_handler PROPERTIES OUTPUT_NAME thymecrashhandler)
endif()

# Build Thyme
add_subdirectory(src)

if(STANDALONE)
    list(APPEND INSTALL_TARGETS thyme)
endif()

if(BUILD_DLL)
    list(APPEND INSTALL_TARGETS thyme_dll)
endif()

# Setup the install target destination
if(DLL_INSTALL_PREFIX AND ${CMAKE_VERSION} VERSION_GREATER "3.13.0")
    install(TARGETS ${INSTALL_TARGETS}
        RUNTIME DESTINATION "${DLL_INSTALL_PREFIX}"
        LIBRARY DESTINATION "${DLL_INSTALL_PREFIX}")
    foreach(target IN LISTS INSTALL_TARGETS)
        install(FILES $<TARGET_PDB_FILE:${target}> DESTINATION "${DLL_INSTALL_PREFIX}" OPTIONAL)
    endforeach()
endif()
