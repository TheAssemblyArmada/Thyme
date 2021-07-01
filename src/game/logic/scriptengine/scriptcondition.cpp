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
#include "script.h"

Condition::Condition() :
    m_conditionType(CONDITION_FALSE),
    m_numParams(0),
    m_nextAndCondition(nullptr),
    m_hasWarnings(false),
    m_customData(0),
    m_unkInt1(0)
{
    memset(m_params, 0, sizeof(m_params));
}

Condition::Condition(ConditionType type) :
    m_conditionType(type), m_numParams(0), m_nextAndCondition(nullptr), m_hasWarnings(false), m_customData(0), m_unkInt1(0)
{
    memset(m_params, 0, sizeof(m_params));
    Set_Condition_Type(m_conditionType);
}

/**
 * 0x0051E480
 */
Condition::~Condition()
{
    // Clear our paramter instances.
    for (int i = 0; i < m_numParams; ++i) {
        m_params[i]->Delete_Instance();
        m_params[i] = nullptr;
    }

    // Clear our list of condition instances.
    for (Condition *next = m_nextAndCondition, *saved = nullptr; next != nullptr; next = saved) {
        saved = next->m_nextAndCondition;
        next->m_nextAndCondition = nullptr; // Prevent trying to next object twice
        next->Delete_Instance();
        next = saved;
    }
}

/**
 * @brief Returns a pointer to a duplicate of the conditon.
 *
 * 0x0051DD90
 */
Condition *Condition::Duplicate()
{
    // Parameters are allocated in Set_Condition_Type which the ctor calls.
    Condition *head_cond = NEW_POOL_OBJ(Condition, m_conditionType);

    for (int i = 0; i < m_numParams; ++i) {
        *head_cond->m_params[i] = *m_params[i];
    }

    Condition *new_cond = head_cond;

    for (Condition *next = m_nextAndCondition; next != nullptr; next = next->m_nextAndCondition) {
        Condition *new_next = NEW_POOL_OBJ(Condition, next->m_conditionType);

        new_cond->m_nextAndCondition = new_next;
        new_cond = new_next;

        for (int i = 0; i < next->m_numParams; ++i) {
            *new_cond->m_params[i] = *next->m_params[i];
        }
    }

    return head_cond;
}

/**
 * @brief Returns a pointer to a duplicate of the conditon, qualifying any parameters.
 *
 * @see Parameter::Qualify
 *
 * 0x0051E0B0
 */
Condition *Condition::Duplicate_And_Qualify(const Utf8String &str1, const Utf8String &str2, const Utf8String &str3)
{
    // Parameters are allocated in Set_Condition_Type which the ctor calls.
    Condition *new_cond = NEW_POOL_OBJ(Condition, m_conditionType);

    for (int i = 0; i < m_numParams; ++i) {
        *new_cond->m_params[i] = *m_params[i];
        new_cond->m_params[i]->Qualify(str1, str2, str3);
    }

    Condition *retval = new_cond;

    for (Condition *next = m_nextAndCondition; next != nullptr; next = next->m_nextAndCondition) {
        Condition *new_next = NEW_POOL_OBJ(Condition, next->m_conditionType);

        new_cond->m_nextAndCondition = new_next;
        new_cond = new_next;

        for (int i = 0; i < next->m_numParams; ++i) {
            *new_cond->m_params[i] = *next->m_params[i];
            new_cond->m_params[i]->Qualify(str1, str2, str3);
        }
    }

    return retval;
}

/**
 * @brief Sets the type of the condition.
 *
 * 0x0051DB90
 */
void Condition::Set_Condition_Type(ConditionType type)
{
#ifdef GAME_DLL
    Call_Method<void, Condition, ConditionType>(PICK_ADDRESS(0x0051DB90, 0), this, type);
#else
    // Clear existing paramters.
    for (int i = 0; i < m_numParams; ++i) {
        m_params[i]->Delete_Instance();
        m_params[i] = nullptr;
    }

    m_conditionType = type;
    // TODO, needs ScriptEngine
#endif
}

/**
 * @brief Parses a condition from a datachunk stream.
 *
 * 0x0051E540
 */
bool Condition::Parse_Data_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data)
{
#ifdef GAME_DLL
    return Call_Function<bool, DataChunkInput &, DataChunkInfo *, void *>(PICK_ADDRESS(0x0051E540, 0), input, info, data);
#else
    Condition *new_condition = NEW_POOL_OBJ(Condition);

    new_condition->m_conditionType = ConditionType(input.Read_Int32());
    // TODO, needs ScriptEngine
    return false;
#endif
}

/**
 * 0x0051D750
 */
OrCondition::~OrCondition()
{
    m_firstAnd->Delete_Instance();
    m_firstAnd = nullptr;

    // Clear our list of OrCondition instances.
    OrCondition *saved;
    for (OrCondition *next = m_nextOr; next != nullptr; next = saved) {
        saved = next->m_nextOr;
        next->m_nextOr = nullptr;
        next->Delete_Instance();
        next = saved;
    }
}

/**
 * @brief Returns a pointer to a duplicate of the or conditon.
 *
 * 0x0051D7B0
 */
OrCondition *OrCondition::Duplicate()
{
    OrCondition *head_or = NEW_POOL_OBJ(OrCondition);

    if (m_firstAnd != nullptr) {
        head_or->m_firstAnd = m_firstAnd->Duplicate();
    }

    OrCondition *new_or = head_or;

    for (OrCondition *next = m_nextOr; next != nullptr; next = next->m_nextOr) {
        OrCondition *new_next = NEW_POOL_OBJ(OrCondition);
        new_or->m_nextOr = new_next;
        new_or = new_next;

        if (next->m_firstAnd != nullptr) {
            new_or->m_firstAnd = next->m_firstAnd->Duplicate();
        }
    }

    return head_or;
}

/**
 * @brief Returns a pointer to a duplicate of the or conditon, qualifying any parameters.
 *
 * @see Parameter::Qualify
 *
 * 0x0051D8A0
 */
OrCondition *OrCondition::Duplicate_And_Qualify(const Utf8String &str1, const Utf8String &str2, const Utf8String &str3)
{
    OrCondition *new_or = NEW_POOL_OBJ(OrCondition);

    if (m_firstAnd != nullptr) {
        new_or->m_firstAnd = m_firstAnd->Duplicate_And_Qualify(str1, str2, str3);
    }

    OrCondition *retval = new_or;

    for (OrCondition *next = m_nextOr; next != nullptr; next = next->m_nextOr) {
        OrCondition *new_next = NEW_POOL_OBJ(OrCondition);
        new_or->m_nextOr = new_next;
        new_or = new_next;

        if (next->m_firstAnd != nullptr) {
            new_or->m_firstAnd = next->m_firstAnd->Duplicate_And_Qualify(str1, str2, str3);
        }
    }

    return retval;
}

/**
 * @brief Parses an or condition from a datachunk stream.
 *
 * 0x0051D9B0
 */
bool OrCondition::Parse_OrCondition_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    Script *script = static_cast<Script *>(data);
    OrCondition *new_or = NEW_POOL_OBJ(OrCondition);
    OrCondition *script_or = script->Get_Condition();

    // Find the end of the list.
    while (script_or != nullptr) {
        if (script_or->m_nextOr == nullptr) {
            break;
        }

        script_or = script_or->m_nextOr;
    }

    // If there was a valid list, put the new entry on the end, if not, set it on the script as the first.
    if (script_or != nullptr) {
        script_or->m_nextOr = new_or;
    } else {
        script->Set_Condition(new_or);
    }

    input.Register_Parser("Condition", info->label, Condition::Parse_Data_Chunk, nullptr);

    return input.Parse(new_or);
}
