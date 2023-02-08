add_executable(compressor)
target_sources(compressor PRIVATE
    compressor.cpp
)
target_link_libraries(compressor PRIVATE thyme_lib cxxopts::cxxopts)
