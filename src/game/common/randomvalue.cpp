/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Classes and functions for generating pseudo random numbers that are deterministic from a given seed.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "randomvalue.h"
#include "crc.h"
#include "gamemath.h"
#include "ini.h"
#include <captainslog.h>
#include <ctime>

using GameMath::Pow;
using std::time;

namespace
{

uint32_t g_theGameAudioSeed[6] = { 0xF22D0E56, 0x883126E9, 0xC624DD2F, 0x0702C49C, 0x9E353F7D, 0x6FDF3B64 };

uint32_t g_theGameClientSeed[6] = { 0xF22D0E56, 0x883126E9, 0xC624DD2F, 0x0702C49C, 0x9E353F7D, 0x6FDF3B64 };

uint32_t g_theGameLogicSeed[6] = { 0xF22D0E56, 0x883126E9, 0xC624DD2F, 0x0702C49C, 0x9E353F7D, 0x6FDF3B64 };

uint32_t g_theGameLogicBaseSeed;

const float g_theMultFactor = 1.0 / (Pow(2.0, 32.0) - 1.0);

// Get the next random value based on the given seed.
uint32_t Random_Value(uint32_t seed[6])
{
    uint32_t tmp0 = seed[0];
    uint32_t tmp1 = seed[1];
    uint32_t tmp2 = seed[2];
    uint32_t tmp3 = seed[3];
    uint32_t tmp4 = seed[4];
    uint32_t tmp5 = seed[5];

    seed[4] += tmp5;
    seed[3] += (seed[4] < tmp4 || seed[4] < tmp5) + seed[4];
    seed[2] += (seed[3] < tmp3) + seed[3];
    seed[1] += (seed[2] < tmp2) + seed[2];
    seed[0] += (seed[1] < tmp1) + seed[1];

    if (++seed[5] == 0) {
        if (++seed[4] == 0) {
            if (++seed[3] == 0) {
                if (++seed[2] == 0) {
                    if (++seed[1] == 0) {
                        ++seed[0];
                    }
                }
            }
        }
    }

    return seed[0];
}

// Initialise a seed from an initial seed value.
void Seed_Random(uint32_t initial, uint32_t seed[6])
{
    seed[0] = initial - 0x0DD2F1AA;
    seed[1] = initial - 0x77CED917;
    seed[2] = initial - 0x77CED917 + 0x3DF3B646;
    seed[3] = initial - 0x77CED917 + 0x7ED19DB3;
    seed[4] = initial - 0x77CED917 + 0x16041894;
    seed[5] = initial + 0x6FDF3B64;
}

} // namespace

// Initialise the RNG system based on the current time.
void Init_Random()
{
    captainslog_info("Initialising random seeds with time() as the initial value.");
    time_t initial = time(nullptr);
    Seed_Random(initial, g_theGameAudioSeed);
    Seed_Random(initial, g_theGameClientSeed);
    Seed_Random(initial, g_theGameLogicSeed);
    g_theGameLogicBaseSeed = initial;
}

// Initialise the RNG system based on an initial value.
void Init_Random(uint32_t initial)
{
    captainslog_info("Initialising random seeds with %04X as the initial value.", initial);
    Seed_Random(initial, g_theGameAudioSeed);
    Seed_Random(initial, g_theGameClientSeed);
    Seed_Random(initial, g_theGameLogicSeed);
}

// Initialise only the logic RNG system based on an initial value.
void Init_Game_Logic_Random(uint32_t initial)
{
    captainslog_info("Initialising logic random seed with %04X as the initial value.", initial);
    Seed_Random(initial, g_theGameLogicSeed);
}

// Get the initial value used for Logic RNG
uint32_t Get_Logic_Random_Seed()
{
    return g_theGameLogicBaseSeed;
}

// Get the CRC of the Logic seed.
uint32_t Get_Logic_Random_Seed_CRC()
{
    CRC c;

    c.Compute_CRC(g_theGameLogicSeed, 24);

    return c.Get_CRC();
}

int32_t Get_Client_Random_Value(int32_t lo, int32_t hi, const char *file, int line)
{
    if (hi - lo != -1) {
        return lo + Random_Value(g_theGameClientSeed) % (hi - lo + 1);
    }

    return hi;
}

int32_t Get_Audio_Random_Value(int32_t lo, int32_t hi, const char *file, int line)
{
    if (hi - lo != -1) {
        return lo + Random_Value(g_theGameAudioSeed) % (hi - lo + 1);
    }

    return hi;
}

int32_t Get_Logic_Random_Value(int32_t lo, int32_t hi, const char *file, int line)
{
    if (hi - lo != -1) {
        return lo + Random_Value(g_theGameLogicSeed) % (hi - lo + 1);
    }

    return hi;
}

float Get_Client_Random_Value_Real(float lo, float hi, const char *file, int line)
{
    float diff = hi - lo;

    if (diff > 0.0f) {
        return float(float(float(Random_Value(g_theGameClientSeed) * g_theMultFactor) * diff) + lo);
    }

    return hi;
}

float Get_Audio_Random_Value_Real(float lo, float hi, const char *file, int line)
{
    float diff = hi - lo;

    if (diff > 0.0f) {
        return float(float(float(Random_Value(g_theGameAudioSeed) * g_theMultFactor) * diff) + lo);
    }

    return hi;
}

float Get_Logic_Random_Value_Real(float lo, float hi, const char *file, int line)
{
    float diff = hi - lo;

    if (diff > 0.0f) {
        return float(float(float(Random_Value(g_theGameLogicSeed) * g_theMultFactor) * diff) + lo);
    }

    return hi;
}

void GameLogicRandomVariable::Set_Range(float min, float max, DistributionType type)
{
    m_low = min;
    m_high = max;
    m_type = type;
}

float GameLogicRandomVariable::Get_Value() const
{
    switch (m_type) {
        case CONSTANT:
            if (m_low == m_high) {
                return m_low;
            } else {
                captainslog_error("m_low doesn't match m_high for CONSTANT GameLogicRandomVariable.");
            }
        case UNIFORM: // Intentional fallthrough
            return Get_Logic_Random_Value_Real(m_low, m_high);
        default:
            captainslog_error("Unsupported distribution type in GameLogicRandomVariable.");
            break;
    }

    return 0.0f;
}

void GameClientRandomVariable::Set_Range(float min, float max, DistributionType type)
{
    m_low = min;
    m_high = max;
    m_type = type;
}

float GameClientRandomVariable::Get_Value() const
{
    switch (m_type) {
        case CONSTANT:
            if (m_low == m_high) {
                return m_low;
            } else {
                captainslog_info("m_low doesn't match m_high for CONSTANT GameClientRandomVariable.");
            }
        case UNIFORM: // Intentional fallthrough
            return Get_Client_Random_Value_Real(m_low, m_high);
        default:
            captainslog_info("Unsupported distribution type in GameClientRandomVariable.");
            break;
    }

    return 0.0f;
}

constexpr const char *DistributionTypeNames[] = {
    "CONSTANT",
    "UNIFORM",
    // Additional types have not been included as they are not usable
    nullptr,
};

// zh: 0x0041D9F0 wb: 0x007A4B4C
void GameClientRandomVariable::Parse(INI *ini, void *, void *store, const void *)
{
    const auto min = ini->Scan_Real(ini->Get_Next_Token());
    const auto max = ini->Scan_Real(ini->Get_Next_Token());

    DistributionType type = DistributionType::UNIFORM;
    auto *token = ini->Get_Next_Token_Or_Null();
    if (token != nullptr) {
        type = static_cast<DistributionType>(ini->Scan_IndexList(token, DistributionTypeNames));
    }
    static_cast<GameClientRandomVariable *>(store)->Set_Range(min, max, type);
}
