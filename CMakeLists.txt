#look at this at some point for structuring the project
#https://rix0r.nl/blog/2015/08/13/cmake-guide/

cmake_minimum_required(VERSION 2.8.11)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if(MSVC)
    set(MSVC_INCREMENTAL_DEFAULT ON)
endif()

set(PROJECT thyme)

project(${PROJECT})

if(MSVC)
    string(REPLACE "INCREMENTAL" "INCREMENTAL:NO" replacementFlags ${CMAKE_EXE_LINKER_FLAGS_DEBUG})
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DYNAMICBASE:NO /NXCOMPAT:NO /INCREMENTAL:NO ${replacementFlags}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/DYNAMICBASE:NO /NXCOMPAT:NO /INCREMENTAL:NO ${replacementFlags}")

    string(REPLACE "INCREMENTAL" "INCREMENTAL:NO" replacementFlags ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO})
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/INCREMENTAL:NO ${replacementFlags}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "/INCREMENTAL:NO ${replacementFlags}")
endif()

# We don't support in tree builds, so help people make the right choice.
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Building in-source is not supported! Create a build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()

# This doesn't really work yet, work ongoing to make it usable
option(STANDALONE "Build a standalone version." OFF)
option(THYME_USE_GAMEMATH "Use own maths library rather than libc version for this platform." ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${thyme_SOURCE_DIR}/cmake/modules)

# Go lean and mean on windows.
if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
    if(NOT STANDALONE)
        add_definitions(-D_USE_32BIT_TIME_T)	#This is for ABI compatibility with a few functions, remove when original binary no longer required.
    endif()
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wno-invalid-offsetof")

    # Prevent lib prefix when built with MinGW to target windows and move to own dir.
    if(MINGW)
        set(CMAKE_SHARED_LIBRARY_PREFIX "")
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${thyme_BINARY_DIR}/bin)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${thyme_BINARY_DIR}/bin)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -mabi=ms")
    endif()
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wno-invalid-offsetof")
endif()

# Enable debug features in MSVC Debug configs.
#if(MSVC)
    #set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /DGAME_DEBUG_LOG")
    #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DGAME_DEBUG_LOG")
    #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /DGAME_DEBUG_LOG")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DGAME_DEBUG_LOG")
#endif()

#Always enable debug logging for now.
#if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DGAME_DEBUG_LOG)
#endif()

# Locate git binary to provide information to the build environment
find_package(Git)

if(GIT_FOUND)
    # Define current git branch.
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE THYME_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    add_definitions(-DTHYME_BRANCH="${THYME_BRANCH}")

    # Define commit hash.
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE THYME_COMMIT_SHA1
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    add_definitions(-DTHYME_COMMIT_SHA1="${THYME_COMMIT_SHA1}")

    # Define short commit hash.
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE THYME_COMMIT_SHA1_SHORT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    add_definitions(-DTHYME_COMMIT_SHA1_SHORT="${THYME_COMMIT_SHA1_SHORT}")

    # Define commit count.
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-list HEAD --count
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE THYME_COMMIT_COUNT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    add_definitions(-DTHYME_COMMIT_COUNT=${THYME_COMMIT_COUNT})
    
    # Define commit date and time. Make builds more reproducible than using __DATE__ and __TIME__
    execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --date=iso --pretty=format:%cd
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE THYME_DATE_AND_TIME
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    string(REPLACE " " ";" THYME_DATE_AND_TIME ${THYME_DATE_AND_TIME})
    list(GET THYME_DATE_AND_TIME 0 THYME_DATE)
    list(GET THYME_DATE_AND_TIME 1 THYME_TIME)
    add_definitions(-DTHYME_COMMIT_DATE="${THYME_DATE}")
    add_definitions(-DTHYME_COMMIT_TIME="${THYME_TIME}")
else()
    add_definitions(-DTHYME_BRANCH="netherrealm")
    add_definitions(-DTHYME_COMMIT_SHA1="DEADBEEF")
    add_definitions(-DTHYME_COMMIT_SHA1_SHORT="DEADBEEF")
    add_definitions(-DTHYME_COMMIT_COUNT=0)
    add_definitions(-DTHYME_COMMIT_DATE="1955-11-12")
    add_definitions(-DTHYME_COMMIT_TIME="22:04:00")
endif()

if(STANDALONE)
    add_definitions(-DTHYME_STANDALONE)
endif()

if(NOT STANDALONE AND MSVC)
    set(USING_STLPORT TRUE BOOL INTERNAL)
endif()

if(USING_STLPORT)
    add_definitions(-DTHYME_USE_STLPORT)
endif()

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