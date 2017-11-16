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

private:
    OrCondition *m_nextOr;
    Condition *m_firstAnd;
};

#endif // SCRIPTCONDITION_H
