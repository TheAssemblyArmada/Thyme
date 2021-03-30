cmake_minimum_required(VERSION 3.0.0)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

project(binkstub VERSION 1.0.0 LANGUAGES C)

add_library(binkstub SHARED bink.c bink.def)
set_target_properties(binkstub PROPERTIES OUTPUT_NAME binkw32)
set_target_properties(binkstub PROPERTIES SOVERSION 1.0 VERSION 1.0.0)
target_include_directories(binkstub PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
