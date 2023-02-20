#include <benchmark/benchmark.h>
#include <ctime>

#ifndef BUILD_WITH_GAMEMATH
#define BUILD_WITH_GAMEMATH
#else
#define HAS_GAMEMATH_IMPL
#endif
#include "gamemath.h"

static void GameMath_GM_Fast_To_Int_Floor(benchmark::State &state)
{
    GameMath::Init();
    for (auto _ : state) {
        state.PauseTiming();
        std::srand(unsigned(std::time(nullptr)));
        float v = std::rand() / 100.0f;
        state.ResumeTiming();
        GameMath::Fast_To_Int_Floor(v);
    }
}

static void GameMath_GM_Fast_To_Int_Ceil(benchmark::State &state)
{
    GameMath::Init();
    for (auto _ : state) {
        state.PauseTiming();
        std::srand(unsigned(std::time(nullptr)));
        float v = std::rand() / 100.0f;
        state.ResumeTiming();
        GameMath::Fast_To_Int_Ceil(v);
    }
}

static void GameMath_GM_Fast_To_Int_Truncate(benchmark::State &state)
{
    GameMath::Init();
    for (auto _ : state) {
        state.PauseTiming();
        std::srand(unsigned(std::time(nullptr)));
        float v = std::rand() / 100.0f;
        state.ResumeTiming();
        GameMath::Fast_To_Int_Truncate(v);
    }
}

static void GameMath_GM_Floor(benchmark::State &state)
{
    GameMath::Init();
    for (auto _ : state) {
        state.PauseTiming();
        std::srand(unsigned(std::time(nullptr)));
        char v = std::rand() & 0xFF;
        state.ResumeTiming();
        GameMath::Floor(v);
    }
}

static void GameMath_GM_Ceil(benchmark::State &state)
{
    GameMath::Init();
    for (auto _ : state) {
        state.PauseTiming();
        std::srand(unsigned(std::time(nullptr)));
        char v = std::rand() & 0xFF;
        state.ResumeTiming();
        GameMath::Ceil(v);
    }
}

// Register the function as a benchmark
BENCHMARK(GameMath_GM_Fast_To_Int_Floor);
BENCHMARK(GameMath_GM_Fast_To_Int_Ceil);
BENCHMARK(GameMath_GM_Fast_To_Int_Truncate);
BENCHMARK(GameMath_GM_Floor);
BENCHMARK(GameMath_GM_Ceil);

#ifdef HAS_GAMEMATH_IMPL
static void GameMath_GM_Cos(benchmark::State &state)
{
    GameMath::Init();
    for (auto _ : state) {
        state.PauseTiming();
        std::srand(unsigned(std::time(nullptr)));
        char v = std::rand() & 0xFF;
        state.ResumeTiming();
        GameMath::Cos(v);
    }
}

static void GameMath_GM_Sin(benchmark::State &state)
{
    GameMath::Init();
    for (auto _ : state) {
        state.PauseTiming();
        std::srand(unsigned(std::time(nullptr)));
        char v = std::rand() & 0xFF;
        state.ResumeTiming();
        GameMath::Sin(v);
    }
}

BENCHMARK(GameMath_GM_Cos);
BENCHMARK(GameMath_GM_Sin);
#endif
