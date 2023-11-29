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
#include "scriptengine.h"

Condition::Condition() :
    m_conditionType(CONDITION_FALSE),
    m_numParams(0),
    m_nextAndCondition(nullptr),
    m_hasWarnings(false),
    m_customData(0),
    m_frame(0)
{
    memset(m_params, 0, sizeof(m_params));
}

Condition::Condition(ConditionType type) :
    m_conditionType(type), m_numParams(0), m_nextAndCondition(nullptr), m_hasWarnings(false), m_customData(0), m_frame(0)
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
    for (int i = 0; i < m_numParams; ++i) {
        m_params[i]->Delete_Instance();
        m_params[i] = nullptr;
    }

    m_conditionType = type;
    ConditionTemplate *condition_template = g_theScriptEngine->Get_Condition_Template(m_conditionType);
    m_numParams = condition_template->Get_Num_Parameters();

    for (int i = 0; i < m_numParams; i++) {
        m_params[i] = new Parameter(condition_template->Get_Parameter_Type(i));
    }
}

/**
 * @brief Parses a condition from a datachunk stream.
 *
 * 0x0051E540
 */
bool Condition::Parse_Condition_Data_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    Condition *new_condition = new Condition();
    OrCondition *or_condition = static_cast<OrCondition *>(data);
    new_condition->m_conditionType = ConditionType(input.Read_Int32());
    ConditionTemplate *condition_template = g_theScriptEngine->Get_Condition_Template(new_condition->m_conditionType);

    if (info->version >= 4) {
        NameKeyType key = input.Read_Name_Key();
        bool match = false;

        if (condition_template != nullptr && condition_template->m_nameKey == key) {
            match = true;
        }

        if (!match) {
            for (int i = 0; i < Condition::CONDITION_COUNT; i++) {
                condition_template = g_theScriptEngine->Get_Condition_Template(i);

                if (key == condition_template->m_nameKey) {
                    match = true;
                    captainslog_debug("Rematching script condition %s", g_theNameKeyGenerator->Key_To_Name(key).Str());
                    new_condition->m_conditionType = static_cast<ConditionType>(i);
                    break;
                }
            }
        }

        if (!match) {
            captainslog_dbgassert(false, "Invalid script condition.  Making it false. jba.");
            new_condition->m_conditionType = CONDITION_FALSE;
            new_condition->m_numParams = 0;
        }
    }

    new_condition->m_numParams = input.Read_Int32();

    for (int j = 0; j < new_condition->m_numParams; j++) {
        new_condition->m_params[j] = Parameter::Read_Parameter(input);
    }

    if (input.Get_Chunk_Version() < 2) {
        static int condition_types[] = { TEAM_INSIDE_AREA_PARTIALLY,
            TEAM_INSIDE_AREA_ENTIRELY,
            TEAM_OUTSIDE_AREA_ENTIRELY,
            TEAM_ENTERED_AREA_PARTIALLY,
            TEAM_ENTERED_AREA_ENTIRELY,
            TEAM_EXITED_AREA_ENTIRELY,
            TEAM_EXITED_AREA_PARTIALLY,
            -1 };

        for (int k = 0; condition_types[k] != -1; k++) {
            if (new_condition->m_conditionType == condition_types[k]) {
                new_condition->m_params[new_condition->m_numParams] = new Parameter(Parameter::SURFACES_ALLOWED, 3);
                new_condition->m_numParams = 3;
            }
        }
    }

    if (new_condition->Get_Condition_Type() == SKIRMISH_SPECIAL_POWER_READY && new_condition->m_numParams == 1) {
        new_condition->m_numParams = 2;
        new_condition->m_params[1] = new_condition->m_params[0];
        new_condition->m_params[0] = new Parameter(Parameter::SIDE);
        new_condition->m_params[0]->Set_String("<This Player>");
    }

    if (condition_template->Get_Num_Parameters() != new_condition->Get_Num_Parameters()) {
        captainslog_dbgassert(false, "Invalid script condition.  Making it false. jba.");
        new_condition->m_conditionType = CONDITION_FALSE;
        new_condition->m_numParams = 0;
    }

    Condition *condition;

    for (condition = or_condition->Get_First_And_Condition(); condition != nullptr && condition->Get_Next() != nullptr;
         condition = condition->Get_Next()) {
    }

    if (condition != nullptr) {
        condition->Set_Next_Condition(new_condition);
    } else {
        or_condition->Set_First_And_Condition(new_condition);
    }

    captainslog_dbgassert(input.At_End_Of_Chunk(), "Unexpected data left over.");
    return true;
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
    OrCondition *script_or = script->Get_Or_Condition();

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
        script->Set_Or_Condition(new_or);
    }

    input.Register_Parser("Condition", info->label, Condition::Parse_Condition_Data_Chunk, nullptr);

    return input.Parse(new_or);
}
