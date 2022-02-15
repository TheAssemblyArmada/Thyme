cmake_minimum_required(VERSION 3.1.0)

if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

project(captnlog VERSION 1.00 LANGUAGES C)

# We don't support in tree builds, so help people make the right choice.
if(CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Building in-source is not supported! Create a build dir and remove ${CMAKE_SOURCE_DIR}/CMakeCache.txt")
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Check for threading, win32 or posix.
find_package(Threads)

# Only use the pthread header if we don't have win32 threads.
if(CMAKE_USE_PTHREADS_INIT AND NOT CMAKE_USE_WIN32_THREADS_INIT)
    set(HAVE_PTHREAD_H TRUE BOOL)
endif()

set(CAPTN_SRC 
    src/captnlog.c
    src/captnassert.c
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    list(APPEND CAPTN_SRC src/captnmessage_win32.c)
elseif("${CMAKE_SYSTEM}" MATCHES "Darwin")
    list(APPEND CAPTN_SRC src/captnmessage_macos.m)
else()
    # TODO make a null version so builds can be done without any dependencies.
    find_package(GTK3 REQUIRED gtk)
    if(GTK3_FOUND)
        message("Building GTK3 version.")
        list(APPEND CAPTN_SRC src/captnmessage_gtk.c)
    endif()
endif()

add_library(captnlog STATIC ${CAPTN_SRC} src/captainslog.h)
target_include_directories(captnlog PUBLIC src ${CMAKE_CURRENT_BINARY_DIR}/src)

# We don't need to link anything if we have win32 threads. Affects MinGW builds primarily.
if(CMAKE_USE_PTHREADS_INIT AND NOT CMAKE_USE_WIN32_THREADS_INIT)
    target_link_libraries(captnlog Threads::Threads)
    target_compile_definitions(captnlog PRIVATE USE_PTHREADS=1)
endif()

# Link needed platform libraries,
if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_link_libraries(captnlog comdlg32)
elseif("${CMAKE_SYSTEM}" MATCHES "Darwin")
    target_link_libraries(captnlog AppKit)
elseif(GTK3_FOUND)
    message("Building GTK3 version.")
    target_link_libraries(captnlog ${GTK3_LIBRARIES})
    target_include_directories(captnlog PRIVATE ${GTK3_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Missing captnlog messagebox backend")
endif()

# Set the max logging level to debug by default. See captnlog.h for possible levels.
if(DEFINED CAPTNLOG_LEVEL)
    target_compile_definitions(captnlog INTERFACE LOGGING_LEVEL=${CAPTNLOG_LEVEL})
endif()

# Set the max logging level to debug by default. See captnlog.h for possible levels.
if(DEFINED CAPTNASSERT_LEVEL)
    target_compile_definitions(captnlog INTERFACE ASSERT_LEVEL=${CAPTNASSERT_LEVEL})
endif()

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_compile_definitions(captnlog PRIVATE WIN32_LEAN_AND_MEAN=1 _CRT_SECURE_NO_DEPRECATE=1 _CRT_NONSTDC_NO_DEPRECATE=1 _WINSOCK_DEPRECATED_NO_WARNINGS=1 NOMINMAX=1)
endif()

include(CurrentFunction)

target_current_function_define(captnlog)
