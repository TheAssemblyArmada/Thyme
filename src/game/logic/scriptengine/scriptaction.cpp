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
#include "scriptaction.h"
#include "script.h"
#include "scriptengine.h"

ScriptAction::ScriptAction() : m_actionType(NO_OP), m_numParams(0), m_nextAction(nullptr), m_hasWarnings(false)
{
    memset(m_params, 0, sizeof(m_params));
}

ScriptAction::ScriptAction(ScriptActionType type) :
    m_actionType(type), m_numParams(0), m_nextAction(nullptr), m_hasWarnings(false)
{
    memset(m_params, 0, sizeof(m_params));
    Set_Action_Type(type);
}

ScriptAction::~ScriptAction()
{
    // Clear our paramter instances.
    for (int i = 0; i < m_numParams; ++i) {
        m_params[i]->Delete_Instance();
        m_params[i] = nullptr;
    }

    // Clear our list of action instances.
    for (ScriptAction *next = m_nextAction, *saved = nullptr; next != nullptr; next = saved) {
        saved = next->m_nextAction;
        next->m_nextAction = nullptr; // Prevent trying to next object twice
        next->Delete_Instance();
    }
}

/**
 * @brief Returns a pointer to a duplicate of this action.
 *
 * 0x0051FF80
 */
ScriptAction *ScriptAction::Duplicate()
{
    ScriptAction *new_action = NEW_POOL_OBJ(ScriptAction, m_actionType);

    for (int i = 0; i < m_numParams; ++i) {
        if (new_action->m_params[i] != nullptr) {
            *new_action->m_params[i] = *m_params[i];
        }
    }

    ScriptAction *retval = new_action;

    for (ScriptAction *next = m_nextAction; next != nullptr; next = next->m_nextAction) {
        ScriptAction *new_next = NEW_POOL_OBJ(ScriptAction, next->m_actionType);

        new_action->m_nextAction = new_next;
        new_action = new_next;

        for (int i = 0; i < next->m_numParams; ++i) {
            if (new_action->m_params[i] != nullptr && next->m_params[i] != nullptr) {
                *new_action->m_params[i] = *next->m_params[i];
            }
        }
    }

    return retval;
}

/**
 * @brief Returns a pointer to a duplicate of this action, qualifying any parameters.
 *
 * @see Parameter::Qualify
 *
 * 0x00520240
 */
ScriptAction *ScriptAction::Duplicate_And_Qualify(const Utf8String &str1, const Utf8String &str2, const Utf8String &str3)
{
    ScriptAction *new_action = NEW_POOL_OBJ(ScriptAction, m_actionType);

    for (int i = 0; i < m_numParams; ++i) {
        if (new_action->m_params[i] != nullptr) {
            *new_action->m_params[i] = *m_params[i];
            new_action->m_params[i]->Qualify(str1, str2, str3);
        }
    }

    ScriptAction *current_action = new_action;

    for (ScriptAction *next = m_nextAction; next != nullptr; next = next->m_nextAction) {
        ScriptAction *new_next = NEW_POOL_OBJ(ScriptAction, next->m_actionType);

        current_action->m_nextAction = new_next;
        current_action = current_action->m_nextAction;

        for (int i = 0; i < next->m_numParams; ++i) {
            if (current_action->m_params[i] != nullptr && next->m_params[i] != nullptr) {
                *current_action->m_params[i] = *next->m_params[i];
                current_action->m_params[i]->Qualify(str1, str2, str3);
            }
        }
    }

    return new_action;
}

/**
 * @brief Get the UI text for this action.
 *
 * 0x005206B0
 */
Utf8String ScriptAction::Get_UI_Text()
{
    Utf8String ui_text;
    Utf8String strings[MAX_ACTION_PARAMETERS];
    int num_strings = Get_UI_Strings(strings);

    if (m_hasWarnings) {
        ui_text = "[???]";
    }

    for (int i = 0; i < MAX_ACTION_PARAMETERS; i++) {
        if (i < num_strings) {
            ui_text += strings[i];
        }

        if (i < m_numParams) {
            ui_text += m_params[i]->Get_UI_Text();
        }
    }

    return ui_text;
}

/**
 * @brief Parses an action from a datachunk stream.
 *
 * 0x00521240
 */
bool ScriptAction::Parse_Action_Data_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    ScriptAction *paction = Parse_Action(input, info, data);
    ScriptAction *saction = static_cast<Script *>(data)->Get_Action();

    for (ScriptAction *next = saction; next != nullptr; next = next->m_nextAction) {
        if (next->m_nextAction == nullptr) {
            next->m_nextAction = paction;

            return true;
        }
    }

    static_cast<Script *>(data)->Set_Action(paction);

    captainslog_dbgassert(input.At_End_Of_Chunk(), "Unexpected data left over.");

    return true;
}

/**
 * @brief Parses a false action from a datachunk stream.
 *
 * 0x00521280
 */
bool ScriptAction::Parse_Action_False_Data_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    ScriptAction *paction = Parse_Action(input, info, data);
    ScriptAction *saction = static_cast<Script *>(data)->Get_False_Action();

    for (ScriptAction *next = saction; next != nullptr; next = next->m_nextAction) {
        if (next->m_nextAction == nullptr) {
            next->m_nextAction = paction;

            return true;
        }
    }

    static_cast<Script *>(data)->Set_False_Action(paction);

    captainslog_dbgassert(input.At_End_Of_Chunk(), "Unexpected data left over.");

    return true;
}

/**
 * @brief Sets the type of the action.
 *
 * 0x0051FE50
 */
void ScriptAction::Set_Action_Type(ScriptActionType type)
{
    for (int i = 0; i < m_numParams; i++) {
        if (m_params[i] != nullptr) {
            m_params[i]->Delete_Instance();
        }

        m_params[i] = nullptr;
    }

    m_actionType = type;
    ActionTemplate *action = g_theScriptEngine->Get_Action_Template(m_actionType);
    m_numParams = action->Get_Num_Parameters();

    for (int i = 0; i < m_numParams; i++) {
        m_params[i] = new Parameter(action->Get_Parameter_Type(i));
    }
}

/**
 * @brief Internal function for parsing actions from datachunk streams.
 *
 * 0x005208A0
 */
ScriptAction *ScriptAction::Parse_Action(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    ScriptAction *action = new ScriptAction();
    action->m_actionType = static_cast<ScriptAction::ScriptActionType>(input.Read_Int32());
    ActionTemplate *action_template = g_theScriptEngine->Get_Action_Template(action->m_actionType);

    if (info->version >= 2) {
        NameKeyType key = input.Read_Name_Key();
        bool match = false;

        if (action_template != nullptr && action_template->m_nameKey == key) {
            match = true;
        }

        if (!match) {
            for (int i = 0; i < ScriptAction::ACTION_COUNT; i++) {
                action_template = g_theScriptEngine->Get_Action_Template(i);

                if (key == action_template->m_nameKey) {
                    match = true;
                    captainslog_debug("Rematching script action %s", g_theNameKeyGenerator->Key_To_Name(key).Str());
                    action->m_actionType = static_cast<ScriptAction::ScriptActionType>(i);
                    break;
                }
            }

            if (!match) {
                captainslog_dbgassert(false, "Invalid script action.  Making it noop. jba.");
                action->m_actionType = ScriptAction::NO_OP;
                action->m_numParams = 0;
            }
        }
    }

    Script *script = static_cast<Script *>(data);

    if (action_template != nullptr) {
        captainslog_dbgassert(!action_template->Get_UI_Name().Is_Empty()
                && action_template->Get_UI_Name().Compare_No_Case("(placeholder)") != 0,
            "Invalid Script Action found in script '%s'",
            script->Get_Name().Str());
    }

    action->m_numParams = input.Read_Int32();

    for (int i = 0; i < action->m_numParams; i++) {
        action->m_params[i] = Parameter::Read_Parameter(input);
    }

    switch (action->Get_Action_Type()) {
        case MOVE_CAMERA_TO:
        case MOVE_CAMERA_ALONG_WAYPOINT_PATH:
        case CAMERA_LOOK_TOWARD_OBJECT:
            if (action->Get_Num_Parameters() == 3) {
                action->m_numParams = 5;
                action->m_params[3] = new Parameter(Parameter::REAL);
                action->m_params[4] = new Parameter(Parameter::REAL);
            }

            break;
        case ROTATE_CAMERA:
        case RESET_CAMERA:
        case ZOOM_CAMERA:
        case PITCH_CAMERA:
            if (action->Get_Num_Parameters() == 2) {
                action->m_numParams = 4;
                action->m_params[2] = new Parameter(Parameter::REAL);
                action->m_params[3] = new Parameter(Parameter::REAL);
            }

            break;
        case CAMERA_MOD_SET_FINAL_ZOOM:
        case CAMERA_MOD_SET_FINAL_PITCH:
            goto l1;
        case TEAM_FOLLOW_WAYPOINTS:
            if (action->m_numParams == 2) {
                action->m_numParams = 3;
                action->m_params[2] = new Parameter(Parameter::BOOLEAN, true);
            }

            break;
        case NAMED_SET_ATTITUDE:
        case TEAM_SET_ATTITUDE:
            if (action->m_numParams >= 2 && action->m_params[1]->Get_Parameter_Type() == Parameter::INT) {
                action->m_params[1] = new Parameter(Parameter::AI_MOOD, action->m_params[1]->Get_Int());
            }

            break;
        case SPEECH_PLAY:
            if (action->m_numParams == 1) {
                action->m_numParams = 2;
                action->m_params[1] = new Parameter(Parameter::BOOLEAN, true);
            } else {
            l1:
                if (action->m_numParams == 1) {
                    action->m_numParams = 3;
                    action->m_params[1] = new Parameter(Parameter::PERCENT);
                    action->m_params[2] = new Parameter(Parameter::PERCENT);
                }
            }

            break;
        case MAP_REVEAL_AT_WAYPOINT:
        case MAP_SHROUD_AT_WAYPOINT:
            if (action->m_numParams == 2) {
                action->m_numParams = 3;
                action->m_params[2] = new Parameter(Parameter::SIDE);
            }

            break;
        case MAP_REVEAL_ALL:
        case MAP_SHROUD_ALL:
        case MAP_REVEAL_ALL_PERM:
        case MAP_REVEAL_ALL_UNDO_PERM:
            if (action->Get_Num_Parameters() == 0) {
                action->m_numParams = 1;
                action->m_params[0] = new Parameter(Parameter::SIDE);
            }

            break;
        case CAMERA_LOOK_TOWARD_WAYPOINT:
            if (action->Get_Num_Parameters() == 2) {
                action->m_numParams = 5;
                action->m_params[2] = new Parameter(Parameter::REAL);
                action->m_params[3] = new Parameter(Parameter::REAL);
                action->m_params[4] = new Parameter(Parameter::BOOLEAN);
            } else if (action->Get_Num_Parameters() == 4) {
                action->m_numParams = 5;
                action->m_params[4] = new Parameter(Parameter::BOOLEAN);
            }

            break;
        case SKIRMISH_BUILD_BASE_DEFENSE_FRONT:
            if (action->m_numParams == 1) {
                bool b = action->m_params[0]->Get_Int() != 0;
                action->m_params[0]->Delete_Instance();
                action->m_numParams = 0;

                if (b) {
                    action->m_actionType = SKIRMISH_BUILD_BASE_DEFENSE_FLANK;
                }
            }

            break;
        case SKIRMISH_FIRE_SPECIAL_POWER_AT_MOST_COST:
            if (action->m_numParams == 1) {
                action->m_numParams = 2;
                action->m_params[1] = action->m_params[0];
                action->m_params[0] = new Parameter(Parameter::SIDE);
                action->m_params[0]->Set_String("<This Player>");
            }

            break;
        default:
            break;
    }

    if (action_template->Get_Num_Parameters() != action->Get_Num_Parameters()) {
        captainslog_dbgassert(false, "Invalid script action.  Making it noop. jba.");
        action->m_actionType = ScriptAction::NO_OP;
        action->m_numParams = 0;
    }

    captainslog_dbgassert(input.At_End_Of_Chunk(), "Unexpected data left over.");
    return action;
}

int ScriptAction::Get_UI_Strings(Utf8String *const strings)
{
    return g_theScriptEngine->Get_Action_Template(m_actionType)->Get_UI_Strings(strings);
}
