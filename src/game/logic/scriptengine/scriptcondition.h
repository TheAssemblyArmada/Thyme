/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class representing a script action.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef SCRIPTCONDITION_H
#define SCRIPTCONDITION_H

#include "always.h"
#include "datachunk.h"
#include "mempoolobj.h"
#include "scriptparam.h"

class Condition : public MemoryPoolObject
{
    IMPLEMENT_POOL(Condition);

    enum
    {
        MAX_CONDITION_PARAMETERS = 12,
    };

public:
    enum ConditionType
    {
        DEFAULT = 0,
    };

public:
    Condition();
    Condition(ConditionType type);
    virtual ~Condition();

    Condition *Duplicate();
    Condition *Duplicate_And_Qualify(const AsciiString &str1, const AsciiString &str2, const AsciiString &str3);
    void Set_Condition_Type(ConditionType type);

    static bool Parse_Data_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data);

#ifndef THYME_STANDALONE
    static void Hook_Me();
#endif

private:
    ConditionType m_conditionType;
    int m_numParams;
    Parameter *m_params[MAX_CONDITION_PARAMETERS];
    Condition *m_nextAndCondition;
    int m_hasWarnings;
    int m_customData;
    int m_unkInt1;
};

class OrCondition : public MemoryPoolObject
{
    IMPLEMENT_POOL(OrCondition);

public:
    OrCondition() : m_nextOr(nullptr), m_firstAnd(nullptr) {}
    virtual ~OrCondition();

    OrCondition *Duplicate();
    OrCondition *Duplicate_And_Qualify(const AsciiString &str1, const AsciiString &str2, const AsciiString &str3);
    
    static bool Parse_OrCondition_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data);

#ifndef THYME_STANDALONE
    static void Hook_Me();
#endif

private:
    OrCondition *m_nextOr;
    Condition *m_firstAnd;
};

#ifndef THYME_STANDALONE
#include "hooker.h"
inline void Condition::Hook_Me()
{
    Hook_Method(0x0051DD90, &Duplicate);
    Hook_Method(0x0051E0B0, &Duplicate_And_Qualify);
}

inline void OrCondition::Hook_Me()
{
    Hook_Method(0x0051D7B0, &Duplicate);
    Hook_Method(0x0051D8A0, &Duplicate_And_Qualify);
    Hook_Function(0x0051D9B0, Parse_OrCondition_Chunk);
}
#endif

#endif // SCRIPTCONDITION_H
