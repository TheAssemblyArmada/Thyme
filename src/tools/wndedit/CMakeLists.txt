add_executable(wndedit)
target_sources(wndedit PRIVATE
    main.cpp
)
target_link_libraries(wndedit PRIVATE thyme_lib)