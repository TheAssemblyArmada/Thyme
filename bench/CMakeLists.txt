FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.7.1
)

FetchContent_MakeAvailable(benchmark)

set(BENCH_SRCS
  bench_refpack.cpp
)

add_executable(thyme_benchmarks ${BENCH_SRCS})
target_link_libraries(thyme_benchmarks benchmark::benchmark benchmark::benchmark_main)
target_compile_definitions(thyme_benchmarks PRIVATE)

if(STANDALONE)
  target_link_libraries(thyme_benchmarks thyme_lib)
else()
  target_link_libraries(thyme_benchmarks thyme_dll)
endif()
