add_executable(wdump WIN32)
target_sources(wdump PRIVATE
    wdump.cpp
    ${CMAKE_SOURCE_DIR}/resources/wdump/wdump.rc
)
target_include_directories(wdump PRIVATE ${CMAKE_SOURCE_DIR}/resources/wdump)
target_link_libraries(wdump PRIVATE thyme_lib comctl32)
