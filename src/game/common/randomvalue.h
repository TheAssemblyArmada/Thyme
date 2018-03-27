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
#pragma once

#include "bittype.h"

#ifndef THYME_STANDALONE
#include "hooker.h"
#endif

void Init_Random();
void Init_Random(uint32_t initial);
void Init_Game_Logic_Random(uint32_t initial);
uint32_t Get_Logic_Random_Seed();
uint32_t Get_Logic_Random_Seed_CRC();
int32_t Get_Client_Random_Value(int32_t lo, int32_t hi, const char *file = nullptr, int line = 0);
int32_t Get_Audio_Random_Value(int32_t lo, int32_t hi, const char *file = nullptr, int line = 0);
int32_t Get_Logic_Random_Value(int32_t lo, int32_t hi, const char *file = nullptr, int line = 0);
float Get_Client_Random_Value_Real(float lo, float hi, const char *file = nullptr, int line = 0);
float Get_Audio_Random_Value_Real(float lo, float hi, const char *file = nullptr, int line = 0);
float Get_Logic_Random_Value_Real(float lo, float hi, const char *file = nullptr, int line = 0);

class GameLogicRandomVariable
{
public:
    enum DistributionType
    {
        CONSTANT = 0,
        UNIFORM,
    };

    void Set_Range(float min, float max, DistributionType type);
    float Get_Value();
    float Get_Min() { return m_low; }
    float Get_Max() { return m_high; }
    DistributionType Get_Type() { return m_type; }

private:
    DistributionType m_type;
    float m_low;
    float m_high;
};

class GameClientRandomVariable
{
public:
    enum DistributionType
    {
        CONSTANT = 0,
        UNIFORM,
    };

    void Set_Range(float min, float max, DistributionType type);
    float Get_Value();
    float Get_Min() { return m_low; }
    float Get_Max() { return m_high; }
    DistributionType Get_Type() { return m_type; }

private:
    DistributionType m_type;
    float m_low;
    float m_high;
};

#ifndef THYME_STANDALONE
namespace RandomValue
{
inline void Hook_Me()
{
    Hook_Function(0x0048DC00, static_cast<void (*)()>(Init_Random));
    Hook_Function(0x0048DCF0, static_cast<void (*)(uint32_t)>(Init_Random));
    Hook_Function(0x0048DDB0, Init_Game_Logic_Random);
    Hook_Function(0x0048DBB0, Get_Logic_Random_Seed_CRC);
    Hook_Function(0x0048DBA0, Get_Logic_Random_Seed);
    Hook_Function(0x0048DEB0, Get_Client_Random_Value);
    Hook_Function(0x0048DE00, Get_Logic_Random_Value);
    Hook_Function(0x0048DEE0, Get_Audio_Random_Value);
    Hook_Function(0x0048DF70, Get_Client_Random_Value_Real);
    Hook_Function(0x0048DF10, Get_Logic_Random_Value_Real);
    Hook_Function(0x0048DFD0, Get_Audio_Random_Value_Real);
    Hook_Method(0x0048E030, &GameClientRandomVariable::Get_Value);
    Hook_Method(0x0048E0D0, &GameLogicRandomVariable::Get_Value);
}

} // namespace RandomValue
#endif