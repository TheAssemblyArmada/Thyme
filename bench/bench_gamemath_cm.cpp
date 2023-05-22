#include <benchmark/benchmark.h>
#include <ctime>

#undef BUILD_WITH_GAMEMATH
#include "gamemath.h"

static void GameMath_CM_Fast_To_Int_Floor(benchmark::State &state)
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

static void GameMath_CM_Fast_To_Int_Ceil(benchmark::State &state)
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

static void GameMath_CM_Fast_To_Int_Truncate(benchmark::State &state)
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

static void GameMath_CM_Floor(benchmark::State &state)
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

static void GameMath_CM_Ceil(benchmark::State &state)
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

static void GameMath_CM_Cos(benchmark::State &state)
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

static void GameMath_CM_Sin(benchmark::State &state)
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

// Register the function as a benchmark
BENCHMARK(GameMath_CM_Fast_To_Int_Floor);
BENCHMARK(GameMath_CM_Fast_To_Int_Ceil);
BENCHMARK(GameMath_CM_Fast_To_Int_Truncate);
BENCHMARK(GameMath_CM_Floor);
BENCHMARK(GameMath_CM_Ceil);
BENCHMARK(GameMath_CM_Cos);
BENCHMARK(GameMath_CM_Sin);
