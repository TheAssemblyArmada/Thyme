cmake_minimum_required(VERSION 3.19.0)

set(FETCHCONTENT_QUIET off)

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
    
    # Build with multiple processes: speeds up build on multi core processors
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

    # Use x87 floating point instructions
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:IA32")

    # Set warning level 3
    # disable C4244: conversion from 'double' to 'float', possible loss of data
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /wd4244 /w14456 /w15038 /w14552")

    if (MSVC_VERSION LESS 1920)
        # MSVC lower than 19
        # disable C4800: 'BOOL' : forcing value to bool 'true' or 'false' (performance warning)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4800")
    else()
        # newer MSVC
        # make C4800 error: Implicit conversion from 'type' to bool. Possible information loss
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /we4800")
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)

# We don't support in tree builds, so help people make the right choice.
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Building in-source is not supported! Create a build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    set(DEFAULT_STANDALONE OFF)
    set(DEFAULT_STDFS OFF)
    set(DEFAULT_FFMPEG OFF)
    set(DEFAULT_ALSOFT OFF)
    set(DEFAULT_SDL2 OFF)
    set(DEFAULT_FREETYPE OFF)
    set(DEFAULT_FONTCONFIG OFF)
else()
    set(DEFAULT_STANDALONE ON)
    include(CheckIncludeFileCXX)
    CHECK_INCLUDE_FILE_CXX(filesystem FS_HEADER_FOUND)
    if(FS_HEADER_FOUND)
        set(DEFAULT_STDFS ON)
    else()
        set(DEFAULT_STDFS OFF)
    endif()
    set(DEFAULT_FFMPEG ON)
    set(DEFAULT_SDL2 ON)
    set(DEFAULT_ALSOFT ON)
    set(DEFAULT_FREETYPE ON)
    if(NOT WIN32)
        set(DEFAULT_FONTCONFIG ON)
    endif()
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
option(USE_FFMPEG "Use FFMPEG for codec handling." ${DEFAULT_FFMPEG})
option(USE_ZLIB "Use zlib to support deflate compression." ON)
option(USE_SDL2 "Use SDL2 for crossplatform window handling." ${DEFAULT_SDL2})
option(USE_ALSOFT "Use OpenAL soft audio library" ${DEFAULT_ALSOFT})
option(USE_FREETYPE "Use FreeType font rasterization" ${DEFAULT_FREETYPE})
option(USE_FONTCONFIG "Use FontConfig for finding font files" ${DEFAULT_FONTCONFIG})
option(LOGGING "Enable debug logging." ${DEFAULT_LOGGING})
option(ASSERTIONS "Enable debug assertions." ${DEFAULT_ASSERTIONS})
option(USE_CRASHPAD "Enable the use of the Crashpad library for crash handling and reporting." OFF)
option(BUILD_TESTS "Builds the unit tests." OFF)
option(USE_SANITIZER "Builds with address sanitizer" OFF)
option(BUILD_COVERAGE "Instruments the code with coverage." OFF)
option(BUILD_TOOLS "Builds the developer/debug tools." OFF)

option(FETCHCONTENT_QUIET "" OFF)

include(CMakeDependentOption)
cmake_dependent_option(USE_STDFS "Use C++ 17 filesystem for crossplatform file handling." ${DEFAULT_STDFS} "STANDALONE" OFF)
cmake_dependent_option(USE_SDL2 "Use SDL2 for crossplatform window handling." ${DEFAULT_SDL2} "STANDALONE" OFF)
cmake_dependent_option(USE_ALSOFT "Use OpenAL soft audio library." ${DEFAULT_ALSOFT} "USE_FFMPEG" OFF)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        option(BUILD_DLL "Build an injectable dll version." ON)
    endif()
endif()

# Only use GCC for now, we can support the others later
if(USE_SANITIZER)
    if(NOT STANDALONE)
        message(FATAL_ERROR "Using sanitizers only works in standalone mode")
    endif()
    if(MSVC)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_C_FLAGS "-fsanitize=address -g")
        set(CMAKE_CXX_FLAGS "-fsanitize=address -g")
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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof")
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

# Enable coverage flags when we support coverage
if(BUILD_COVERAGE AND CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "Building with gcov code coverage")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
endif()


find_package(ICU COMPONENTS data i18n io tu uc)

if(NOT WIN32 OR NOT "${CMAKE_SYSTEM}" MATCHES "Windows")
    if(NOT ICU_FOUND)
        message(FATAL_ERROR "ICU is required on non-windows platforms and was not found.")
    endif()
endif()

# Try and find wxWidgets which is used for some GUI tools.
find_package(wxWidgets COMPONENTS core base xrc xml html adv)

if(NOT wxWidgets_FOUND AND BUILD_TOOLS)
    message(STATUS "Debug tools are enabled, but wxWidgets was not found. wxWidgets will be fetched and built.")
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
        GIT_TAG        59f7ccd494f7e7c916a784ac26ef266f9f09d78d
    )

    # We don't use FetchContent_MakeAvailable here because we don't want all gamemath targets including, just our dependencies.
    FetchContent_GetProperties(gamemath_git)
    if(NOT gamemath_git_POPULATED)
        FetchContent_Populate(gamemath_git)
        add_subdirectory(${gamemath_git_SOURCE_DIR} ${gamemath_git_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
endif()

if(USE_FFMPEG)
    find_package(FFmpeg COMPONENTS AVFORMAT AVCODEC AVUTIL SWSCALE REQUIRED)
endif()

if(USE_SDL2)
    find_package(SDL2)
    if(NOT SDL2_FOUND)
        message(WARNING "Found no system SDL2 - fetching & building")
        include(FetchContent)

        set(SDL_ATOMIC OFF CACHE INTERNAL "Turn off subsystem")
        set(SDL_AUDIO OFF CACHE INTERNAL "Turn off subsystem")
        set(SDL_RENDER OFF CACHE INTERNAL "Turn off subsystem")
        set(SDL_HAPTIC OFF CACHE INTERNAL "Turn off subsystem")
        set(SDL_FILE OFF CACHE INTERNAL "Turn off subsystem")
        set(SDL_FILESYSTEM OFF CACHE INTERNAL "Turn off subsystem")
        FetchContent_Declare(
            SDL2
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG        release-2.24.2
        )

        FetchContent_MakeAvailable(SDL2)
    else()
        message(STATUS "Found system SDL2")
    endif()
endif()

if(USE_ALSOFT)
    include(FetchContent)

    set(ALSOFT_UTILS OFF CACHE INTERNAL "Turn off subsystem")
    set(ALSOFT_EXAMPLES OFF CACHE INTERNAL "Turn off subsystem")
    set(ALSOFT_INSTALL OFF CACHE INTERNAL "Turn off subsystem")
    set(ALSOFT_INSTALL_CONFIG OFF CACHE INTERNAL "Turn off subsystem")
    set(ALSOFT_INSTALL_HRTF_DATA OFF CACHE INTERNAL "Turn off subsystem")
    set(ALSOFT_INSTALL_AMBDEC_PRESETS OFF CACHE INTERNAL "Turn off subsystem")
    FetchContent_Declare(
        ALSoft
        GIT_REPOSITORY https://github.com/kcat/openal-soft.git
        GIT_TAG        1.23.0
    )

    FetchContent_MakeAvailable(ALSoft)
endif()

if(USE_FREETYPE)
    find_package(Freetype)

    if(NOT FREETYPE_FOUND AND NOT TARGET Freetype::Freetype)
        include(FetchContent)

        set(SKIP_INSTALL_ALL ON CACHE INTERNAL "Turn off install all")
        FetchContent_Declare(
            Freetype
            GIT_REPOSITORY https://github.com/freetype/freetype.git
            GIT_TAG        VER-2-13-2
        )

        FetchContent_MakeAvailable(Freetype)
        add_library(Freetype::Freetype ALIAS freetype)
    endif()
endif()

if(USE_FONTCONFIG)
    find_package(Fontconfig REQUIRED)
endif()

if(USE_ZLIB)
    find_package(ZLIB)

    if(NOT ZLIB_FOUND AND NOT TARGET ZLIB::ZLIB)
        include(FetchContent)

        # We define an empty test macro here, so zlib doesn't add it's tests to our global scope
        set(EMPTY_ADD_TEST TRUE)
        function(add_test)
            if(EMPTY_ADD_TEST)
                message(STATUS "Skipping test: ${ARGV0}")
                return()
            endif()
            _add_test(${ARGV})
        endfunction(add_test)

        FetchContent_Declare(
            zlib 
            GIT_REPOSITORY https://github.com/madler/zlib.git
            GIT_TAG        v1.2.13
        )
        
        # We don't use FetchContent_MakeAvailable here because we don't want all zlib targets including, just our dependency.
        FetchContent_GetProperties(zlib)
        if(NOT zlib_POPULATED)
            FetchContent_Populate(zlib)
            add_subdirectory(${zlib_SOURCE_DIR} ${zlib_BINARY_DIR} EXCLUDE_FROM_ALL)
        endif()

        # Make sure headers are available for the static target and make an alias to match the Find module output.
        target_include_directories(zlibstatic INTERFACE ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
        add_library(ZLIB::ZLIB ALIAS zlibstatic)
        set(EMPTY_ADD_TEST FALSE)
    endif()
endif()

if(USE_CRASHPAD)
    include(FetchContent)

    FetchContent_Declare(
        crashpad_cmake
        GIT_REPOSITORY https://github.com/TheAssemblyArmada/crashpad-cmake.git
        GIT_TAG        80573ad
    )

    # We don't use FetchContent_MakeAvailable here because we don't want all crashpad targets including, just our dependencies.
    FetchContent_GetProperties(crashpad_cmake)
    if(NOT crashpad_cmake_POPULATED)
        FetchContent_Populate(crashpad_cmake)
        add_subdirectory(${crashpad_cmake_SOURCE_DIR} ${crashpad_cmake_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()

    set_target_properties(crashpad_handler PROPERTIES OUTPUT_NAME thymecrashhandler)
endif()

if(BUILD_TOOLS)
    include(FetchContent)
    FetchContent_Declare(
            cxxopts 
            GIT_REPOSITORY https://github.com/jarro2783/cxxopts.git
            GIT_TAG        v3.0.0
            GIT_SHALLOW    TRUE
        )
    FetchContent_MakeAvailable(cxxopts)

    if(NOT wxWidgets_FOUND)
        set(wxBUILD_SHARED OFF CACHE BOOL "Build wx libraries as shared libs")
        set(wxBUILD_PRECOMP OFF CACHE BOOL "Use precompiled headers")
        set(wxBUILD_MONOLITHIC OFF CACHE BOOL "Build a single library")
        set(wxUSE_WINSOCK2 ON CACHE BOOL "include <winsock2.h> rather than <winsock.h>")

        FetchContent_Declare(
            wxWidgets 
            GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
            GIT_TAG        v3.2.1
            GIT_SHALLOW    TRUE
        )
        
        FetchContent_MakeAvailable(wxWidgets)
        if(IS_DIRECTORY "${wxWidgets_SOURCE_DIR}")
            set_property(DIRECTORY ${wxWidgets_SOURCE_DIR} PROPERTY EXCLUDE_FROM_ALL YES)
        endif()

        set(wxWidgets_FOUND TRUE)
        set(wxWidgets_LIBRARIES wxcore wxbase wxxrc wxxml wxhtml wxadv)

        # Will need manually setting for cross compiles for windows or macos from another OS.
        if(NOT wxWidgets_wxrc_EXECUTABLE)
            set(wxWidgets_wxrc_EXECUTABLE wxrc)
        endif()
    endif()
endif()

if(BUILD_DLL)
    add_subdirectory(src/hookproxy)  
    list(APPEND INSTALL_TARGETS proxydll)
endif()

# Build Thyme
add_subdirectory(src)

if(STANDALONE)
    list(APPEND INSTALL_TARGETS thyme)
endif()

if(BUILD_DLL)
    list(APPEND INSTALL_TARGETS thyme_dll)
endif()

# Add resources (optional)
if (MSVC_IDE)
    add_library(resources INTERFACE
        resources/visualstudio/stllist.natvis
        resources/visualstudio/stlvector.natvis
        resources/visualstudio/utf8string.natvis
        resources/visualstudio/utf16string.natvis
    )
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

# Build tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
