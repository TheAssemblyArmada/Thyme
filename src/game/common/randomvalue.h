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

#include "always.h"
#include "bittype.h"

class INI;

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
    friend class Xfer;

public:
    enum DistributionType : int32_t
    {
        CONSTANT = 0,
        UNIFORM,
    };

    operator float() const { return Get_Value(); }

    void Set_Range(float min, float max, DistributionType type);
    float Get_Value() const;
    float Get_Min() const { return m_low; }
    float Get_Max() const { return m_high; }
    DistributionType Get_Type() const { return m_type; }

private:
    DistributionType m_type;
    float m_low;
    float m_high;
};

class GameClientRandomVariable
{
    friend class Xfer;

public:
    enum DistributionType : int32_t
    {
        CONSTANT = 0,
        UNIFORM,
    };

    operator float() const { return Get_Value(); }

    void Set_Range(float min, float max, DistributionType type);
    float Get_Value() const;
    float Get_Min() const { return m_low; }
    float Get_Max() const { return m_high; }
    DistributionType Get_Type() const { return m_type; }

    static void Parse(INI *ini, void *, void *store, const void *);

private:
    DistributionType m_type;
    float m_low;
    float m_high;
};
