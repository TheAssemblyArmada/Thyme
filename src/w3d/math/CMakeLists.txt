add_library(w3d_math STATIC
    aabox.cpp
    colmath.cpp
    colmathaabox.cpp
    colmathaabtri.cpp
    colmathaaplane.cpp
    colmathfrustum.cpp
    colmathline.cpp
    colmathobbobb.cpp
    colmathobbtri.cpp
    colmathplane.cpp
    cullsys.cpp
    frustum.cpp
    gamemath.cpp
    lineseg.cpp
    matrix3.cpp
    matrix3d.cpp
    matrix4.cpp
    quat.cpp
    tri.cpp
    v3_rnd.cpp
    vector4.cpp
    vp.cpp
)

target_include_directories(w3d_math PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(w3d_math PUBLIC w3d_lib)

if(D3D8_FOUND)
    target_link_libraries(w3d_math PUBLIC d3d8)
endif()