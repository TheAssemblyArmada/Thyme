#look at this at some point for structuring the project
#https://rix0r.nl/blog/2015/08/13/cmake-guide/

cmake_minimum_required(VERSION 2.8.11)

if(MSVC)
    set(MSVC_INCREMENTAL_DEFAULT ON)
endif()

set(PROJECT thyme)
set(CMAKE_USE_RELATIVE_PATHS)

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

# Go lean and mean on windows.
if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif()

# Prevent lib prefix when built with MinGW to target windows and move to own dir.
if(MINGW)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${thyme_BINARY_DIR}/bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${thyme_BINARY_DIR}/bin)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -std=gnu++0x -mabi=ms")

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_definitions(-DGAME_DEBUG_LOG)
    endif()
endif()

# Enable debug features in MSVC Debug configs.
if(MSVC)
    #set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /DGAME_DEBUG_LOG")
    #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DGAME_DEBUG_LOG")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /DGAME_DEBUG_LOG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DGAME_DEBUG_LOG")
endif()

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

else()
    add_definitions(-DTHYME_BRANCH="NOGITBRANCH")
    add_definitions(-DTHYME_COMMIT_SHA1="NOGITHASH")
    add_definitions(-DTHYME_COMMIT_SHA1_SHORT="NOGITHASH")
    add_definitions(-DTHYME_COMMIT_COUNT=0)
endif()

# Build and link the launcher.
set(LAUNCHER_SRC launcher/launcher.cpp launcher/resource.rc)
add_executable(launchthyme ${LAUNCHER_SRC})
target_link_libraries(launchthyme)

# Build and link the DLL.
set(GAMEENGINE_INCLUDES
    src/base
    src/game
    src/game/client
    src/game/common
    src/game/common/ini
    src/game/common/system
    src/platform
    src/w3d/lib
)

set(HOOKER_SRC src/hooker/dllmain.cpp src/hooker/hooker.cpp)

set(GAMEENGINE_SRC
    src/game/copyprotect.cpp
    src/game/force_nocd.cpp
    src/game/main.cpp
    src/game/client/gametext.cpp
    src/game/common/gameengine.cpp
    src/game/common/globaldata.cpp
    src/game/common/version.cpp
    src/game/common/ini/ini.cpp
    src/game/common/system/archivefile.cpp
    src/game/common/system/archivefilesystem.cpp
    src/game/common/system/asciistring.cpp
    src/game/common/system/file.cpp
    src/game/common/system/filesystem.cpp
    src/game/common/system/gamedebug.cpp
    src/game/common/system/gamememory.cpp
    src/game/common/system/gamememoryinit.cpp
    src/game/common/system/localfile.cpp
    src/game/common/system/localfilesystem.cpp
    src/game/common/system/ramfile.cpp
    src/game/common/system/streamingarchivefile.cpp
    src/game/common/system/subsysteminterface.cpp
    src/game/common/system/unicodestring.cpp
    src/game/common/system/xfer.cpp
    src/platform/win32gameengine.cpp
    src/platform/win32localfile.cpp
    src/platform/win32localfilesystem.cpp
    src/w3d/lib/critsection.cpp
)

add_library(thyme SHARED ${HOOKER_SRC} ${GAMEENGINE_SRC})
target_include_directories(thyme BEFORE PUBLIC src/libs/stlport src/hooker ${GAMEENGINE_INCLUDES})

target_link_libraries(thyme)
