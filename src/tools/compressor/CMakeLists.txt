add_executable(compressor)
target_sources(compressor PRIVATE
    compressor.cpp
)
target_include_directories(compressor PRIVATE ${CMAKE_SOURCE_DIR}/resources/wdump)
target_link_libraries(compressor PRIVATE thyme_lib cxxopts::cxxopts)
