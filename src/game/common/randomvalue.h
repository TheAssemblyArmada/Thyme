////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: RANDOMVALUE.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Classes and functions for generating pseudo random numbers
//                 that are deterministic from a given seed.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#ifndef _RANDOMVALUE_H_
#define _RANDOMVALUE_H_

#include "bittype.h"
#include "hooker.h"

// Audio and Client could be other way around, will become clear as more functions
// are mapped.
//#define TheGameClientSeed (Make_Pointer<uint32_t>(0x009D76C4))
//#define TheGameAudioSeed (Make_Pointer<uint32_t>(0x009D76DC))
//#define TheGameLogicSeed (Make_Pointer<uint32_t>(0x009D76F4))
//#define TheGameLogicBaseSeed (Make_Global<uint32_t>(0x00A2B9FC))
//#define TheMultFactor (Make_Global<const float>(0x00942344))

void Init_Random();
void Init_Random(uint32_t initial);
void Init_Game_Logic_Random(uint32_t initial);
uint32_t Get_Logic_Random_Seed();
uint32_t Get_Logic_Random_Seed_CRC();
int32_t Get_Client_Random_Value(int32_t lo, int32_t hi, char const *file = nullptr, int line = 0);
int32_t Get_Audio_Random_Value(int32_t lo, int32_t hi, char const *file = nullptr, int line = 0);
int32_t Get_Logic_Random_Value(int32_t lo, int32_t hi, char const *file = nullptr, int line = 0);
float Get_Client_Random_Value_Real(float lo, float hi, char const *file = nullptr, int line = 0);
float Get_Audio_Random_Value_Real(float lo, float hi, char const *file = nullptr, int line = 0);
float Get_Logic_Random_Value_Real(float lo, float hi, char const *file = nullptr, int line = 0);
void Verify_Random_Value();

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

namespace RandomValue {

inline void Hook_Me()
{
    Hook_Function(Make_Function_Ptr<void>(0x0048DC00), Init_Random);
    Hook_Function(Make_Function_Ptr<void, uint32_t>(0x0048DCF0), Init_Random);
    Hook_Function(Make_Function_Ptr<void, uint32_t>(0x0048DDB0), Init_Game_Logic_Random);
    Hook_Function(Make_Function_Ptr<uint32_t>(0x0048DBB0), Get_Logic_Random_Seed_CRC);
    Hook_Function(Make_Function_Ptr<uint32_t>(0x0048DBA0), Get_Logic_Random_Seed);
    Hook_Function(Make_Function_Ptr<int32_t, int32_t, int32_t, char const *, int>(0x0048DEB0), Get_Client_Random_Value);
    Hook_Function(Make_Function_Ptr<int32_t, int32_t, int32_t, char const *, int>(0x0048DE00), Get_Logic_Random_Value);
    Hook_Function(Make_Function_Ptr<int32_t, int32_t, int32_t, char const *, int>(0x0048DEE0), Get_Audio_Random_Value);
    Hook_Function(Make_Function_Ptr<float, float, float, char const *, int>(0x0048DF70), Get_Client_Random_Value_Real);
    Hook_Function(Make_Function_Ptr<float, float, float, char const *, int>(0x0048DF10), Get_Logic_Random_Value_Real);
    Hook_Function(Make_Function_Ptr<float, float, float, char const *, int>(0x0048DFD0), Get_Audio_Random_Value_Real);
    Hook_Method(Make_Method_Ptr<float, GameClientRandomVariable>(0x0048E030), &GameClientRandomVariable::Get_Value);
    Hook_Method(Make_Method_Ptr<float, GameLogicRandomVariable>(0x0048E0D0), &GameLogicRandomVariable::Get_Value);
}

} // namespace RandomValue

#endif // _RANDOMVALUE_H_
