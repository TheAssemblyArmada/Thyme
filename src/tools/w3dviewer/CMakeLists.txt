add_executable(w3dviewer WIN32)
set(CMAKE_MFC_FLAG 2) #Enable MFC
target_sources(w3dviewer PRIVATE
    w3dviewer.cpp
)
target_link_libraries(w3dviewer PRIVATE thyme_lib)
