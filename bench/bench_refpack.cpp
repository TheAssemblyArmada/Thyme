#include <benchmark/benchmark.h>
#include <ctime>

#include "refpack.h"

static void RefPack_Compress(benchmark::State &state)
{
    std::vector<uint8_t> src(0XFFFF0);
    std::vector<uint8_t> dst(0XFFFF0 * 2);
    for (auto _ : state) {
        state.PauseTiming();
        std::srand(unsigned(std::time(nullptr)));
        std::generate(src.begin(), src.end(), std::rand);
        state.ResumeTiming();

        RefPack_Compress(dst.data(), src.data(), src.size(), nullptr);
    }

    state.SetBytesProcessed(state.iterations() * src.size());
}

static void RefPack_Uncompress(benchmark::State &state)
{
    std::vector<uint8_t> src(0XFFFF0);
    std::vector<uint8_t> dst(0XFFFF0 * 2);

    std::srand(unsigned(std::time(nullptr)));
    std::generate(src.begin(), src.end(), std::rand);
    int compressed_size = RefPack_Compress(dst.data(), src.data(), src.size(), nullptr);

    for (auto _ : state) {
        RefPack_Uncompress(src.data(), dst.data(), nullptr);
    }

    state.SetBytesProcessed(state.iterations() * compressed_size);
}

// Register the function as a benchmark
BENCHMARK(RefPack_Compress);
BENCHMARK(RefPack_Uncompress);