add_executable(w3dviewer)
target_sources(w3dviewer PRIVATE
    main.cpp
)
target_link_libraries(w3dviewer PRIVATE thyme_lib)