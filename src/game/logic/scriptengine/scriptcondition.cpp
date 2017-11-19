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
#include "scriptcondition.h"

Condition::Condition() :
    m_conditionType(DEFAULT),
    m_numParams(0),
    m_nextAndCondition(nullptr),
    m_hasWarnings(false),
    m_customData(0),
    m_unkInt1(0)
{

}

Condition::Condition(ConditionType type) :
    m_conditionType(type),
    m_numParams(0),
    m_nextAndCondition(nullptr),
    m_hasWarnings(false),
    m_customData(0),
    m_unkInt1(0)
{
}

Condition::~Condition()
{
    for (int i = m_numParams; i < MAX_CONDITION_PARAMETERS; ++i) {
        Delete_Instance(m_params[i]);
        m_params[i] = nullptr;
    }

    Condition *saved;
    for (Condition *next = m_nextAndCondition; next != nullptr; next = saved) {
        saved = next->m_nextAndCondition;
        next->m_nextAndCondition = nullptr;
        Delete_Instance(next);
        next = saved;
    }
}

OrCondition::~OrCondition()
{
    Delete_Instance(m_firstAnd);
    m_firstAnd = nullptr;

    OrCondition *saved;
    for (OrCondition  *next = m_nextOr; next != nullptr; next = saved) {
        saved = next->m_nextOr;
        next->m_nextOr = nullptr;
        Delete_Instance(next);
        next = saved;
    }
}
