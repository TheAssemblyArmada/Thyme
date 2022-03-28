add_executable(w3dview WIN32)
set(CMAKE_MFC_FLAG 2) #Enable MFC
target_sources(w3dview PRIVATE
    mainfrm.cpp
    toolbar.cpp
    w3dview.cpp
    ${CMAKE_SOURCE_DIR}/resources/w3dviewer/w3dview.rc
)
add_definitions(-D_AFXDLL)
target_include_directories(w3dview PRIVATE ${CMAKE_SOURCE_DIR}/resources/w3dviewer)
target_link_libraries(w3dview PRIVATE thyme_lib)
