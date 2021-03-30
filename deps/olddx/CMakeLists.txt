add_library(d3d8 INTERFACE)
target_link_libraries(d3d8 INTERFACE d3d8 d3dx8)

if(MSVC)
    target_link_libraries(d3d8 INTERFACE legacy_stdio_definitions)
endif()

target_compile_definitions(d3d8 INTERFACE -DBUILD_WITH_D3D8)

if(MSVC AND ${CMAKE_VERSION} VERSION_GREATER "3.13.0")   
    target_link_directories(d3d8 BEFORE INTERFACE .)
    target_include_directories(d3d8 INTERFACE .)
    target_link_options(d3d8 INTERFACE /SAFESEH:NO /NODEFAULTLIB:libci.lib)
endif()