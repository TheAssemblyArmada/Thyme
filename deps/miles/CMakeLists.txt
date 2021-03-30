cmake_minimum_required(VERSION 3.0.0)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

project(milesstub VERSION 1.0.0 LANGUAGES C)

add_library(milesstub SHARED miles.c miles.def)
set_target_properties(milesstub PROPERTIES OUTPUT_NAME mss32)
set_target_properties(milesstub PROPERTIES SOVERSION 1.0 VERSION 1.0.0)
target_include_directories(milesstub PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
