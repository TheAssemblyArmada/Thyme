/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Script engine
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "scriptengine.h"
#include "ai.h"
#include "assetmgr.h"
#include "audioeventrts.h"
#include "audiomanager.h"
#include "campaignmanager.h"
#include "file.h"
#include "filesystem.h"
#include "gameengine.h"
#include "gamelogic.h"
#include "latchrestore.h"
#include "messagestream.h"
#include "object.h"
#include "objecttypes.h"
#include "particleinfo.h"
#include "particlesys.h"
#include "particlesysmanager.h"
#include "particlesystemplate.h"
#include "playerlist.h"
#include "scriptactions.h"
#include "scriptconditions.h"
#include "sequentialscript.h"
#include "sideslist.h"
#include "team.h"
#include "terrainlogic.h"
#include "thingfactory.h"
#include <algorithm>

#ifndef GAME_DLL
ScriptEngine *g_theScriptEngine = nullptr;
#endif

bool ScriptEngine::s_canAppContinue = false;
int ScriptEngine::s_currentFrame = 0;
int ScriptEngine::s_lastFrame = 0;
ParticleSystem *ScriptEngine::s_particleSystem = nullptr;
char ScriptEngine::s_bufferToCheck[1032];
bool ScriptEngine::s_particlesForever = false;
bool ScriptEngine::s_appIsFast = false;
#ifdef PLATFORM_WINDOWS
HMODULE ScriptEngine::s_debugDll = nullptr;
HMODULE ScriptEngine::s_particleDll = nullptr;
#endif

AttackPriorityInfo::~AttackPriorityInfo()
{
    if (m_priorityMap != nullptr) {
        delete m_priorityMap;
        m_priorityMap = nullptr;
    }
}

void AttackPriorityInfo::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferAsciiString(&m_name);
    xfer->xferInt(&m_defaultPriority);
    unsigned short priority_count = 0;

    if (m_priorityMap != nullptr) {
        for (auto it = m_priorityMap->begin(); it != m_priorityMap->end(); it++) {
            priority_count++;
        }
    }

    xfer->xferUnsignedShort(&priority_count);
    Utf8String name;

    if (xfer->Get_Mode() == XFER_SAVE) {
        if (m_priorityMap != nullptr) {
            unsigned short new_count = 0;

            for (auto it = m_priorityMap->begin(); it != m_priorityMap->end(); it++) {
                new_count++;
                name = it->first->Get_Name();
                captainslog_dbgassert(!name.Is_Empty(), "AttackPriorityInfo::xfer - Writing an empty thing template name");
                xfer->xferAsciiString(&name);
                int priority = it->second;
                xfer->xferInt(&priority);
            }

            captainslog_dbgassert(new_count == priority_count,
                "AttackPriorityInfo::xfer - Mismatch in priority map size.  Size() method returned '%d' "
                "but actual iteration count was '%d'",
                priority_count,
                new_count);
        }
    } else {
        for (unsigned short i = 0; i < priority_count; i++) {
            xfer->xferAsciiString(&name);
            ThingTemplate *tmplate = g_theThingFactory->Find_Template(name, true);
            captainslog_relassert(
                tmplate != nullptr, 6, "AttackPriorityInfo::xfer - Unable to find thing template '%s'", name.Str());
            int priority;
            xfer->xferInt(&priority);
            Set_Priority(tmplate, priority);
        }
    }
}

void AttackPriorityInfo::Reset()
{
    m_name.Clear();
    m_defaultPriority = 1;
    delete m_priorityMap;
    m_priorityMap = nullptr;
}

void AttackPriorityInfo::Set_Priority(const ThingTemplate *tmplate, int priority)
{
    if (tmplate != nullptr) {
        if (m_priorityMap == nullptr) {
            m_priorityMap = new std::map<const ThingTemplate *, int>();
        }

        (*m_priorityMap)[static_cast<const ThingTemplate *>(tmplate->Get_Final_Override())] = priority;
    }
}

int AttackPriorityInfo::Get_Priority(const ThingTemplate *tmplate)
{
    if (tmplate == nullptr) {
        return m_defaultPriority;
    }

    if (m_priorityMap != nullptr && !m_priorityMap->empty()) {
        auto it = m_priorityMap->find(tmplate);

        if (it != m_priorityMap->end()) {
            return it->second;
        }
    }

    return m_defaultPriority;
}

ScriptEngine::ScriptEngine() :
    m_numCounters(0),
    m_numFlags(0),
    m_numAttackInfo(0),
    m_endGameTimer(0),
    m_closeWindowTimer(0),
    m_callingTeam(nullptr),
    m_callingObject(nullptr),
    m_conditionTeam(nullptr),
    m_conditionObject(nullptr),
    m_firstUpdate(true),
    m_currentPlayer(nullptr),
    m_unkInt1(0),
    m_fade(FADE_NONE),
    m_minFade(0.0f),
    m_maxFade(0.0f),
    m_curFadeValue(0.0f),
    m_curFadeFrame(0),
    m_fadeFramesIncrease(0),
    m_fadeFramesHold(0),
    m_fadeFramesDecrease(0),
    m_objectCreationDestructionFrame(0),
    m_freezeByScript(false),
    m_useObjectDifficultyBonuses(true),
    m_chooseVictimAlwaysUsesNormal(false),
    m_hasShownMPLocalDefeatWindow(false)
{
    s_canAppContinue = true;
    s_currentFrame = 0;
    s_lastFrame = s_currentFrame;
    Set_Global_Difficulty(DIFFICULTY_NORMAL);
}

ScriptEngine::~ScriptEngine()
{
#ifdef PLATFORM_WINDOWS
    if (s_debugDll != nullptr) {
        void (*DestroyDebugDialog)(void) =
            reinterpret_cast<void (*)(void)>(GetProcAddress(s_debugDll, "DestroyDebugDialog"));

        if (DestroyDebugDialog != nullptr) {
            DestroyDebugDialog();
        }

        FreeLibrary(s_debugDll);
        s_debugDll = nullptr;
    }

    if (s_particleDll != nullptr) {
        void (*DestroyParticleSystemDialog)(void) =
            reinterpret_cast<void (*)(void)>(GetProcAddress(s_particleDll, "DestroyParticleSystemDialog"));

        if (DestroyParticleSystemDialog != nullptr) {
            DestroyParticleSystemDialog();
        }

        FreeLibrary(s_particleDll);
        s_particleDll = nullptr;
    }
#endif

    Reset();
}

void ScriptEngine::Parse_Script_Action(INI *ini)
{
    // clang-format off
    static const FieldParse s_theTemplateFieldParseTable[] = {
        { "InternalName", &INI::Parse_AsciiString, nullptr, offsetof(Template, m_internalName) },
        { "UIName", &INI::Parse_AsciiString, nullptr, offsetof(Template, m_uiName) },
        { "UIName2", &INI::Parse_AsciiString, nullptr, offsetof(Template, m_uiName2) },
        { "HelpText", &INI::Parse_AsciiString, nullptr, offsetof(Template, m_helpString) },
        { nullptr, nullptr, nullptr, 0 },
    };
    // clang-format on

    ActionTemplate action;
    ini->Init_From_INI(&action, s_theTemplateFieldParseTable);

    if (g_theScriptEngine != nullptr) {
        g_theScriptEngine->Add_Action_Template_Info(&action);
    }
}

void ScriptEngine::Add_Action_Template_Info(Template *tmplate)
{
    for (int i = 0; i < ScriptAction::ACTION_COUNT; i++) {
        if (m_actionTemplates[i].m_internalName == tmplate->m_internalName) {
            m_actionTemplates[i].m_uiName = tmplate->m_uiName;
            m_actionTemplates[i].m_uiName2 = tmplate->m_uiName2;
            m_actionTemplates[i].m_helpString = tmplate->m_helpString;
            return;
        }
    }

    captainslog_debug("Couldn't find script action named %s", tmplate->m_internalName.Str());
}

void ScriptEngine::Parse_Script_Condition(INI *ini)
{
    // clang-format off
    static const FieldParse s_theTemplateFieldParseTable[] = {
        { "InternalName", &INI::Parse_AsciiString, nullptr, offsetof(Template, m_internalName) },
        { "UIName", &INI::Parse_AsciiString, nullptr, offsetof(Template, m_uiName) },
        { "UIName2", &INI::Parse_AsciiString, nullptr, offsetof(Template, m_uiName2) },
        { "HelpText", &INI::Parse_AsciiString, nullptr, offsetof(Template, m_helpString) },
        { nullptr, nullptr, nullptr, 0 },
    };
    // clang-format on

    ConditionTemplate condition;
    ini->Init_From_INI(&condition, s_theTemplateFieldParseTable);

    if (g_theScriptEngine != nullptr) {
        g_theScriptEngine->Add_Condition_Template_Info(&condition);
    }
}

void ScriptEngine::Add_Condition_Template_Info(Template *tmplate)
{
    for (int i = 0; i < Condition::CONDITION_COUNT; i++) {
        if (m_conditionTemplates[i].m_internalName == tmplate->m_internalName) {
            m_conditionTemplates[i].m_uiName = tmplate->m_uiName;
            m_conditionTemplates[i].m_uiName2 = tmplate->m_uiName2;
            m_conditionTemplates[i].m_helpString = tmplate->m_helpString;
            return;
        }
    }

    captainslog_debug("Couldn't find script condition named %s", tmplate->m_internalName.Str());
}

void ScriptEngine::Init()
{
#ifdef PLATFORM_WINDOWS
    if (g_theWriteableGlobalData->m_windowed) {
        if (g_theWriteableGlobalData->m_scriptDebug) {
            s_debugDll = LoadLibrary("DebugWindow.dll");
        } else {
            s_debugDll = nullptr;
        }
    }

    if (g_theWriteableGlobalData->m_particleEdit) {
        s_particleDll = LoadLibrary("ParticleEditor.dll");
    } else {
        s_particleDll = nullptr;
    }

    if (s_debugDll != nullptr) {
        void (*CreateDebugDialog)(void) = reinterpret_cast<void (*)(void)>(GetProcAddress(s_debugDll, "CreateDebugDialog"));

        if (CreateDebugDialog != nullptr) {
            CreateDebugDialog();
        }
    }

    if (s_particleDll != nullptr) {
        void (*CreateParticleSystemDialog)(void) =
            reinterpret_cast<void (*)(void)>(GetProcAddress(s_particleDll, "CreateParticleSystemDialog"));

        if (CreateParticleSystemDialog != nullptr) {
            CreateParticleSystemDialog();
        }
    }
#endif

    // vtune code removed

#ifdef GAME_DEBUG_STRUCTS
    m_numFrames = 0.0;
    m_totalUpdateTime = 0.0;
    m_maxUpdateTime = 0.0;
#endif

    if (g_theScriptActions != nullptr) {
        g_theScriptActions->Init();
    }

    if (g_theScriptConditions != nullptr) {
        g_theScriptConditions->Init();
    }

    m_actionTemplates[ScriptAction::DEBUG_MESSAGE_BOX].m_internalName = "DEBUG_MESSAGE_BOX";
    m_actionTemplates[ScriptAction::DEBUG_MESSAGE_BOX].m_uiName = "Scripting_/Debug/Display message and pause.";
    m_actionTemplates[ScriptAction::DEBUG_MESSAGE_BOX].m_numParameters = 1;
    m_actionTemplates[ScriptAction::DEBUG_MESSAGE_BOX].m_parameters[0] = Parameter::TEXT_STRING;
    m_actionTemplates[ScriptAction::DEBUG_MESSAGE_BOX].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::DEBUG_MESSAGE_BOX].m_uiStrings[0] = "Show debug string and pause: ";

    m_actionTemplates[ScriptAction::DEBUG_STRING].m_internalName = "DEBUG_STRING";
    m_actionTemplates[ScriptAction::DEBUG_STRING].m_uiName = "Scripting_/Debug/Display string.";
    m_actionTemplates[ScriptAction::DEBUG_STRING].m_numParameters = 1;
    m_actionTemplates[ScriptAction::DEBUG_STRING].m_parameters[0] = Parameter::TEXT_STRING;
    m_actionTemplates[ScriptAction::DEBUG_STRING].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::DEBUG_STRING].m_uiStrings[0] = "Show debug string without pausing: ";

    m_actionTemplates[ScriptAction::DEBUG_CRASH_BOX].m_internalName = "DEBUG_CRASH_BOX";
    m_actionTemplates[ScriptAction::DEBUG_CRASH_BOX].m_uiName =
        "{INTERNAL}_/Debug/Display a crash box (debug/internal builds only).";
    m_actionTemplates[ScriptAction::DEBUG_CRASH_BOX].m_numParameters = 1;
    m_actionTemplates[ScriptAction::DEBUG_CRASH_BOX].m_parameters[0] = Parameter::TEXT_STRING;
    m_actionTemplates[ScriptAction::DEBUG_CRASH_BOX].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::DEBUG_CRASH_BOX].m_uiStrings[0] = "Display a crash box with the text: ";

    m_actionTemplates[ScriptAction::SET_FLAG].m_internalName = "SET_FLAG";
    m_actionTemplates[ScriptAction::SET_FLAG].m_uiName = "Scripting_/Flags/Set flag to value.";
    m_actionTemplates[ScriptAction::SET_FLAG].m_numParameters = 2;
    m_actionTemplates[ScriptAction::SET_FLAG].m_parameters[0] = Parameter::FLAG;
    m_actionTemplates[ScriptAction::SET_FLAG].m_parameters[1] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::SET_FLAG].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::SET_FLAG].m_uiStrings[0] = "Set ";
    m_actionTemplates[ScriptAction::SET_FLAG].m_uiStrings[1] = " to ";

    m_actionTemplates[ScriptAction::SET_COUNTER].m_internalName = "SET_COUNTER";
    m_actionTemplates[ScriptAction::SET_COUNTER].m_uiName = "Scripting_/Counters/Set counter to a value.";
    m_actionTemplates[ScriptAction::SET_COUNTER].m_numParameters = 2;
    m_actionTemplates[ScriptAction::SET_COUNTER].m_parameters[0] = Parameter::COUNTER;
    m_actionTemplates[ScriptAction::SET_COUNTER].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::SET_COUNTER].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::SET_COUNTER].m_uiStrings[0] = "Set ";
    m_actionTemplates[ScriptAction::SET_COUNTER].m_uiStrings[1] = " to ";

    m_actionTemplates[ScriptAction::SET_TREE_SWAY].m_internalName = "SET_TREE_SWAY";
    m_actionTemplates[ScriptAction::SET_TREE_SWAY].m_uiName = "Map_/Environment/Set wind sway amount and direction.";
    m_actionTemplates[ScriptAction::SET_TREE_SWAY].m_numParameters = 5;
    m_actionTemplates[ScriptAction::SET_TREE_SWAY].m_parameters[0] = Parameter::ANGLE;
    m_actionTemplates[ScriptAction::SET_TREE_SWAY].m_parameters[1] = Parameter::ANGLE;
    m_actionTemplates[ScriptAction::SET_TREE_SWAY].m_parameters[2] = Parameter::ANGLE;
    m_actionTemplates[ScriptAction::SET_TREE_SWAY].m_parameters[3] = Parameter::INT;
    m_actionTemplates[ScriptAction::SET_TREE_SWAY].m_parameters[4] = Parameter::REAL;
    m_actionTemplates[ScriptAction::SET_TREE_SWAY].m_numUiStrings = 6;
    m_actionTemplates[ScriptAction::SET_TREE_SWAY].m_uiStrings[0] = "Set wind direction to ";
    m_actionTemplates[ScriptAction::SET_TREE_SWAY].m_uiStrings[1] = ", amount to sway ";
    m_actionTemplates[ScriptAction::SET_TREE_SWAY].m_uiStrings[2] = ", amount to lean with the wind ";
    m_actionTemplates[ScriptAction::SET_TREE_SWAY].m_uiStrings[3] = ", frames to take to sway once ";
    m_actionTemplates[ScriptAction::SET_TREE_SWAY].m_uiStrings[4] = ", randomness ";
    m_actionTemplates[ScriptAction::SET_TREE_SWAY].m_uiStrings[5] = "(0=lock step, 1=large random variation).";

    m_actionTemplates[ScriptAction::SET_INFANTRY_LIGHTING_OVERRIDE].m_internalName = "SET_INFANTRY_LIGHTING_OVERRIDE";
    m_actionTemplates[ScriptAction::SET_INFANTRY_LIGHTING_OVERRIDE].m_uiName = "Map_/Environment/Infantry Lighting - Set.";
    m_actionTemplates[ScriptAction::SET_INFANTRY_LIGHTING_OVERRIDE].m_numParameters = 1;
    m_actionTemplates[ScriptAction::SET_INFANTRY_LIGHTING_OVERRIDE].m_parameters[0] = Parameter::REAL;
    m_actionTemplates[ScriptAction::SET_INFANTRY_LIGHTING_OVERRIDE].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::SET_INFANTRY_LIGHTING_OVERRIDE].m_uiStrings[0] = "Set lighting percent on infantry to ";
    m_actionTemplates[ScriptAction::SET_INFANTRY_LIGHTING_OVERRIDE].m_uiStrings[1] =
        " (0.0==min, 1.0==normal day, 2.0==max (which is normal night)).";

    m_actionTemplates[ScriptAction::RESET_INFANTRY_LIGHTING_OVERRIDE].m_internalName = "RESET_INFANTRY_LIGHTING_OVERRIDE";
    m_actionTemplates[ScriptAction::RESET_INFANTRY_LIGHTING_OVERRIDE].m_uiName =
        "Map_/Environment/Infantry Lighting - Reset.";
    m_actionTemplates[ScriptAction::RESET_INFANTRY_LIGHTING_OVERRIDE].m_numParameters = 0;
    m_actionTemplates[ScriptAction::RESET_INFANTRY_LIGHTING_OVERRIDE].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::RESET_INFANTRY_LIGHTING_OVERRIDE].m_uiStrings[0] =
        "Reset infantry lighting to the normal setting. 1.0 for the two day states, 2.0 for the two night states. (Look in "
        "GamesData.ini).";

    m_actionTemplates[ScriptAction::QUICKVICTORY].m_internalName = "QUICKVICTORY";
    m_actionTemplates[ScriptAction::QUICKVICTORY].m_uiName = "User_/ Announce quick win.";
    m_actionTemplates[ScriptAction::QUICKVICTORY].m_numParameters = 0;
    m_actionTemplates[ScriptAction::QUICKVICTORY].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::QUICKVICTORY].m_uiStrings[0] = "End game in victory immediately.";

    m_actionTemplates[ScriptAction::VICTORY].m_internalName = "VICTORY";
    m_actionTemplates[ScriptAction::VICTORY].m_uiName = "User_/ Announce win.";
    m_actionTemplates[ScriptAction::VICTORY].m_numParameters = 0;
    m_actionTemplates[ScriptAction::VICTORY].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::VICTORY].m_uiStrings[0] = "Announce win.";

    m_actionTemplates[ScriptAction::DEFEAT].m_internalName = "DEFEAT";
    m_actionTemplates[ScriptAction::DEFEAT].m_uiName = "User_/ Announce lose.";
    m_actionTemplates[ScriptAction::DEFEAT].m_numParameters = 0;
    m_actionTemplates[ScriptAction::DEFEAT].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::DEFEAT].m_uiStrings[0] = "Announce lose.";

    m_actionTemplates[ScriptAction::NO_OP].m_internalName = "NO_OP";
    m_actionTemplates[ScriptAction::NO_OP].m_uiName = "Scripting_/Debug/Null operation.";
    m_actionTemplates[ScriptAction::NO_OP].m_numParameters = 0;
    m_actionTemplates[ScriptAction::NO_OP].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::NO_OP].m_uiStrings[0] = "Null operation (Does nothing).";

    m_actionTemplates[ScriptAction::SET_TIMER].m_internalName = "SET_TIMER";
    m_actionTemplates[ScriptAction::SET_TIMER].m_uiName = "Scripting_/Timer/Frame countdown timer -- set.";
    m_actionTemplates[ScriptAction::SET_TIMER].m_numParameters = 2;
    m_actionTemplates[ScriptAction::SET_TIMER].m_parameters[0] = Parameter::COUNTER;
    m_actionTemplates[ScriptAction::SET_TIMER].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::SET_TIMER].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::SET_TIMER].m_uiStrings[0] = "Set timer ";
    m_actionTemplates[ScriptAction::SET_TIMER].m_uiStrings[1] = " to expire in ";
    m_actionTemplates[ScriptAction::SET_TIMER].m_uiStrings[2] = " frames.";

    m_actionTemplates[ScriptAction::SET_RANDOM_TIMER].m_internalName = "SET_RANDOM_TIMER";
    m_actionTemplates[ScriptAction::SET_RANDOM_TIMER].m_uiName = "Scripting_/Timer/Frame countdown timer -- set random.";
    m_actionTemplates[ScriptAction::SET_RANDOM_TIMER].m_numParameters = 3;
    m_actionTemplates[ScriptAction::SET_RANDOM_TIMER].m_parameters[0] = Parameter::COUNTER;
    m_actionTemplates[ScriptAction::SET_RANDOM_TIMER].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::SET_RANDOM_TIMER].m_parameters[2] = Parameter::INT;
    m_actionTemplates[ScriptAction::SET_RANDOM_TIMER].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::SET_RANDOM_TIMER].m_uiStrings[0] = "Set timer ";
    m_actionTemplates[ScriptAction::SET_RANDOM_TIMER].m_uiStrings[1] = " to expire between ";
    m_actionTemplates[ScriptAction::SET_RANDOM_TIMER].m_uiStrings[2] = " and ";
    m_actionTemplates[ScriptAction::SET_RANDOM_TIMER].m_uiStrings[3] = " frames.";

    m_actionTemplates[ScriptAction::STOP_TIMER].m_internalName = "STOP_TIMER";
    m_actionTemplates[ScriptAction::STOP_TIMER].m_uiName = "Scripting_/Timer/Timer -- stop.";
    m_actionTemplates[ScriptAction::STOP_TIMER].m_numParameters = 1;
    m_actionTemplates[ScriptAction::STOP_TIMER].m_parameters[0] = Parameter::COUNTER;
    m_actionTemplates[ScriptAction::STOP_TIMER].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::STOP_TIMER].m_uiStrings[0] = "Stop timer ";

    m_actionTemplates[ScriptAction::RESTART_TIMER].m_internalName = "RESTART_TIMER";
    m_actionTemplates[ScriptAction::RESTART_TIMER].m_uiName = "Scripting_/Timer/Timer -- restart stopped.";
    m_actionTemplates[ScriptAction::RESTART_TIMER].m_numParameters = 1;
    m_actionTemplates[ScriptAction::RESTART_TIMER].m_parameters[0] = Parameter::COUNTER;
    m_actionTemplates[ScriptAction::RESTART_TIMER].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::RESTART_TIMER].m_uiStrings[0] = "Restart timer ";

    m_actionTemplates[ScriptAction::PLAY_SOUND_EFFECT].m_internalName = "PLAY_SOUND_EFFECT";
    m_actionTemplates[ScriptAction::PLAY_SOUND_EFFECT].m_uiName = "Multimedia_/Sound Effect/Play sound effect.";
    m_actionTemplates[ScriptAction::PLAY_SOUND_EFFECT].m_numParameters = 1;
    m_actionTemplates[ScriptAction::PLAY_SOUND_EFFECT].m_parameters[0] = Parameter::SOUND;
    m_actionTemplates[ScriptAction::PLAY_SOUND_EFFECT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::PLAY_SOUND_EFFECT].m_uiStrings[0] = "Play ";
    m_actionTemplates[ScriptAction::PLAY_SOUND_EFFECT].m_uiStrings[1] = ".";

    m_actionTemplates[ScriptAction::ENABLE_SCRIPT].m_internalName = "ENABLE_SCRIPT";
    m_actionTemplates[ScriptAction::ENABLE_SCRIPT].m_uiName = "Scripting_/Script/Enable Script.";
    m_actionTemplates[ScriptAction::ENABLE_SCRIPT].m_numParameters = 1;
    m_actionTemplates[ScriptAction::ENABLE_SCRIPT].m_parameters[0] = Parameter::SCRIPT;
    m_actionTemplates[ScriptAction::ENABLE_SCRIPT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::ENABLE_SCRIPT].m_uiStrings[0] = "Enable ";
    m_actionTemplates[ScriptAction::ENABLE_SCRIPT].m_uiStrings[1] = ".";

    m_actionTemplates[ScriptAction::DISABLE_SCRIPT].m_internalName = "DISABLE_SCRIPT";
    m_actionTemplates[ScriptAction::DISABLE_SCRIPT].m_uiName = "Scripting_/Script/Disable Script.";
    m_actionTemplates[ScriptAction::DISABLE_SCRIPT].m_numParameters = 1;
    m_actionTemplates[ScriptAction::DISABLE_SCRIPT].m_parameters[0] = Parameter::SCRIPT;
    m_actionTemplates[ScriptAction::DISABLE_SCRIPT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::DISABLE_SCRIPT].m_uiStrings[0] = "Disable ";
    m_actionTemplates[ScriptAction::DISABLE_SCRIPT].m_uiStrings[1] = ".";

    m_actionTemplates[ScriptAction::CALL_SUBROUTINE].m_internalName = "CALL_SUBROUTINE";
    m_actionTemplates[ScriptAction::CALL_SUBROUTINE].m_uiName = "Scripting_/Script/Run subroutine script.";
    m_actionTemplates[ScriptAction::CALL_SUBROUTINE].m_numParameters = 1;
    m_actionTemplates[ScriptAction::CALL_SUBROUTINE].m_parameters[0] = Parameter::SCRIPT_SUBROUTINE;
    m_actionTemplates[ScriptAction::CALL_SUBROUTINE].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::CALL_SUBROUTINE].m_uiStrings[0] = "Run ";
    m_actionTemplates[ScriptAction::CALL_SUBROUTINE].m_uiStrings[1] = ".";

    m_actionTemplates[ScriptAction::PLAY_SOUND_EFFECT_AT].m_internalName = "PLAY_SOUND_EFFECT_AT";
    m_actionTemplates[ScriptAction::PLAY_SOUND_EFFECT_AT].m_uiName =
        "Multimedia_/Sound Effect/Play sound effect at waypoint.";
    m_actionTemplates[ScriptAction::PLAY_SOUND_EFFECT_AT].m_numParameters = 2;
    m_actionTemplates[ScriptAction::PLAY_SOUND_EFFECT_AT].m_parameters[0] = Parameter::SOUND;
    m_actionTemplates[ScriptAction::PLAY_SOUND_EFFECT_AT].m_parameters[1] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::PLAY_SOUND_EFFECT_AT].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::PLAY_SOUND_EFFECT_AT].m_uiStrings[0] = "Play ";
    m_actionTemplates[ScriptAction::PLAY_SOUND_EFFECT_AT].m_uiStrings[1] = " at ";
    m_actionTemplates[ScriptAction::PLAY_SOUND_EFFECT_AT].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::DAMAGE_MEMBERS_OF_TEAM].m_internalName = "DAMAGE_MEMBERS_OF_TEAM";
    m_actionTemplates[ScriptAction::DAMAGE_MEMBERS_OF_TEAM].m_uiName = "Team_/Damage/Damage the members of a team.";
    m_actionTemplates[ScriptAction::DAMAGE_MEMBERS_OF_TEAM].m_numParameters = 2;
    m_actionTemplates[ScriptAction::DAMAGE_MEMBERS_OF_TEAM].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::DAMAGE_MEMBERS_OF_TEAM].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::DAMAGE_MEMBERS_OF_TEAM].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::DAMAGE_MEMBERS_OF_TEAM].m_uiStrings[0] = "Damage ";
    m_actionTemplates[ScriptAction::DAMAGE_MEMBERS_OF_TEAM].m_uiStrings[1] = ", amount=";
    m_actionTemplates[ScriptAction::DAMAGE_MEMBERS_OF_TEAM].m_uiStrings[2] = " (-1==kill).";

    m_actionTemplates[ScriptAction::MOVE_TEAM_TO].m_internalName = "MOVE_TEAM_TO";
    m_actionTemplates[ScriptAction::MOVE_TEAM_TO].m_uiName = "Team_/Move/Set to move to a location.";
    m_actionTemplates[ScriptAction::MOVE_TEAM_TO].m_numParameters = 2;
    m_actionTemplates[ScriptAction::MOVE_TEAM_TO].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::MOVE_TEAM_TO].m_parameters[1] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::MOVE_TEAM_TO].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::MOVE_TEAM_TO].m_uiStrings[0] = "Move ";
    m_actionTemplates[ScriptAction::MOVE_TEAM_TO].m_uiStrings[1] = " to ";
    m_actionTemplates[ScriptAction::MOVE_TEAM_TO].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS].m_internalName = "TEAM_FOLLOW_WAYPOINTS";
    m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS].m_uiName = "Team_/Move/Set to follow a waypoint path.";
    m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS].m_numParameters = 3;
    m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS].m_parameters[1] = Parameter::WAYPOINT_PATH;
    m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS].m_parameters[2] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS].m_uiStrings[0] = "Have ";
    m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS].m_uiStrings[1] = " follow ";
    m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS].m_uiStrings[2] = ", as a team is ";

    m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS_EXACT].m_internalName = "TEAM_FOLLOW_WAYPOINTS_EXACT";
    m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS_EXACT].m_uiName =
        "Team_/Move/Set to EXACTLY follow a waypoint path.";
    m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS_EXACT].m_numParameters = 3;
    m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS_EXACT].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS_EXACT].m_parameters[1] = Parameter::WAYPOINT_PATH;
    m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS_EXACT].m_parameters[2] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS_EXACT].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS_EXACT].m_uiStrings[0] = "Have ";
    m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS_EXACT].m_uiStrings[1] = " EXACTLY follow ";
    m_actionTemplates[ScriptAction::TEAM_FOLLOW_WAYPOINTS_EXACT].m_uiStrings[2] = ", as a team is ";

    m_actionTemplates[ScriptAction::TEAM_WANDER_IN_PLACE].m_internalName = "TEAM_WANDER_IN_PLACE";
    m_actionTemplates[ScriptAction::TEAM_WANDER_IN_PLACE].m_uiName = "Team_/Move/Set to wander around current location.";
    m_actionTemplates[ScriptAction::TEAM_WANDER_IN_PLACE].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_WANDER_IN_PLACE].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_WANDER_IN_PLACE].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_WANDER_IN_PLACE].m_uiStrings[0] = "Have ";
    m_actionTemplates[ScriptAction::TEAM_WANDER_IN_PLACE].m_uiStrings[1] = " wander around it's current location.";

    m_actionTemplates[ScriptAction::TEAM_INCREASE_PRIORITY].m_internalName = "TEAM_INCREASE_PRIORITY";
    m_actionTemplates[ScriptAction::TEAM_INCREASE_PRIORITY].m_uiName =
        "Team_/AI/Increase priority by Success Priority Increase amount.";
    m_actionTemplates[ScriptAction::TEAM_INCREASE_PRIORITY].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_INCREASE_PRIORITY].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_INCREASE_PRIORITY].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_INCREASE_PRIORITY].m_uiStrings[0] = "Increase the AI priority for ";
    m_actionTemplates[ScriptAction::TEAM_INCREASE_PRIORITY].m_uiStrings[1] = "  by its Success Priority Increase amount.";

    m_actionTemplates[ScriptAction::TEAM_DECREASE_PRIORITY].m_internalName = "TEAM_DECREASE_PRIORITY";
    m_actionTemplates[ScriptAction::TEAM_DECREASE_PRIORITY].m_uiName =
        "Team_/AI/Reduce priority by Failure Priority Decrease amount.";
    m_actionTemplates[ScriptAction::TEAM_DECREASE_PRIORITY].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_DECREASE_PRIORITY].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_DECREASE_PRIORITY].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_DECREASE_PRIORITY].m_uiStrings[0] = "Reduce the AI priority for ";
    m_actionTemplates[ScriptAction::TEAM_DECREASE_PRIORITY].m_uiStrings[1] = "  by its Failure Priority Decrease amount.";

    m_actionTemplates[ScriptAction::TEAM_WANDER].m_internalName = "TEAM_WANDER";
    m_actionTemplates[ScriptAction::TEAM_WANDER].m_uiName = "Team_/Move/Set to follow a waypoint path -- wander.";
    m_actionTemplates[ScriptAction::TEAM_WANDER].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_WANDER].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_WANDER].m_parameters[1] = Parameter::WAYPOINT_PATH;
    m_actionTemplates[ScriptAction::TEAM_WANDER].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_WANDER].m_uiStrings[0] = "Have ";
    m_actionTemplates[ScriptAction::TEAM_WANDER].m_uiStrings[1] = " wander along ";

    m_actionTemplates[ScriptAction::TEAM_PANIC].m_internalName = "TEAM_PANIC";
    m_actionTemplates[ScriptAction::TEAM_PANIC].m_uiName = "Team_/Move/Set to follow a waypoint path -- panic.";
    m_actionTemplates[ScriptAction::TEAM_PANIC].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_PANIC].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_PANIC].m_parameters[1] = Parameter::WAYPOINT_PATH;
    m_actionTemplates[ScriptAction::TEAM_PANIC].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_PANIC].m_uiStrings[0] = "Have ";
    m_actionTemplates[ScriptAction::TEAM_PANIC].m_uiStrings[1] = " move in panic along ";

    m_actionTemplates[ScriptAction::MOVE_NAMED_UNIT_TO].m_internalName = "MOVE_NAMED_UNIT_TO";
    m_actionTemplates[ScriptAction::MOVE_NAMED_UNIT_TO].m_uiName = "Unit_/Move/Move a specific unit to a location.";
    m_actionTemplates[ScriptAction::MOVE_NAMED_UNIT_TO].m_numParameters = 2;
    m_actionTemplates[ScriptAction::MOVE_NAMED_UNIT_TO].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::MOVE_NAMED_UNIT_TO].m_parameters[1] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::MOVE_NAMED_UNIT_TO].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::MOVE_NAMED_UNIT_TO].m_uiStrings[0] = "Move ";
    m_actionTemplates[ScriptAction::MOVE_NAMED_UNIT_TO].m_uiStrings[1] = " to ";
    m_actionTemplates[ScriptAction::MOVE_NAMED_UNIT_TO].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::TEAM_SET_STATE].m_internalName = "TEAM_SET_STATE";
    m_actionTemplates[ScriptAction::TEAM_SET_STATE].m_uiName = "Team_/Misc/Team custom state - set state.";
    m_actionTemplates[ScriptAction::TEAM_SET_STATE].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_SET_STATE].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_SET_STATE].m_parameters[1] = Parameter::TEAM_STATE;
    m_actionTemplates[ScriptAction::TEAM_SET_STATE].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_SET_STATE].m_uiStrings[0] = "Set ";
    m_actionTemplates[ScriptAction::TEAM_SET_STATE].m_uiStrings[1] = " to ";
    m_actionTemplates[ScriptAction::TEAM_SET_STATE].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::CREATE_REINFORCEMENT_TEAM].m_internalName = "CREATE_REINFORCEMENT_TEAM";
    m_actionTemplates[ScriptAction::CREATE_REINFORCEMENT_TEAM].m_uiName = "Team_/ Spawn a reinforcement team.";
    m_actionTemplates[ScriptAction::CREATE_REINFORCEMENT_TEAM].m_numParameters = 2;
    m_actionTemplates[ScriptAction::CREATE_REINFORCEMENT_TEAM].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::CREATE_REINFORCEMENT_TEAM].m_parameters[1] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::CREATE_REINFORCEMENT_TEAM].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::CREATE_REINFORCEMENT_TEAM].m_uiStrings[0] = "Spawn an instance of ";
    m_actionTemplates[ScriptAction::CREATE_REINFORCEMENT_TEAM].m_uiStrings[1] = " at ";
    m_actionTemplates[ScriptAction::CREATE_REINFORCEMENT_TEAM].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_BUILDING].m_internalName = "SKIRMISH_BUILD_BUILDING";
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_BUILDING].m_uiName = "Skirmish Only_/ Build a building.";
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_BUILDING].m_numParameters = 1;
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_BUILDING].m_parameters[0] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_BUILDING].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_BUILDING].m_uiStrings[0] = "Build a building of type ";
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_BUILDING].m_uiStrings[1] = ".";

    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_SUPPLY_CENTER].m_internalName = "AI_PLAYER_BUILD_SUPPLY_CENTER";
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_SUPPLY_CENTER].m_uiName =
        "Player_/AI/AI player build near a supply source.";
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_SUPPLY_CENTER].m_numParameters = 3;
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_SUPPLY_CENTER].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_SUPPLY_CENTER].m_parameters[1] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_SUPPLY_CENTER].m_parameters[2] = Parameter::INT;
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_SUPPLY_CENTER].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_SUPPLY_CENTER].m_uiStrings[0] = "Have AI ";
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_SUPPLY_CENTER].m_uiStrings[1] = " build a ";
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_SUPPLY_CENTER].m_uiStrings[2] = " near a supply src with at least ";
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_SUPPLY_CENTER].m_uiStrings[3] = " available resources.";

    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_TYPE_NEAREST_TEAM].m_internalName = "AI_PLAYER_BUILD_TYPE_NEAREST_TEAM";
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_TYPE_NEAREST_TEAM].m_uiName =
        "Player_/AI/AI player build nearest specified team.";
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_TYPE_NEAREST_TEAM].m_numParameters = 3;
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_TYPE_NEAREST_TEAM].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_TYPE_NEAREST_TEAM].m_parameters[1] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_TYPE_NEAREST_TEAM].m_parameters[2] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_TYPE_NEAREST_TEAM].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_TYPE_NEAREST_TEAM].m_uiStrings[0] = "Have AI ";
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_TYPE_NEAREST_TEAM].m_uiStrings[1] = " build a ";
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_TYPE_NEAREST_TEAM].m_uiStrings[2] = " nearest team ";
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_TYPE_NEAREST_TEAM].m_uiStrings[3] = ".";

    m_actionTemplates[ScriptAction::TEAM_GUARD_SUPPLY_CENTER].m_internalName = "TEAM_GUARD_SUPPLY_CENTER";
    m_actionTemplates[ScriptAction::TEAM_GUARD_SUPPLY_CENTER].m_uiName = "Team_/Guard/Set to guard a supply source.";
    m_actionTemplates[ScriptAction::TEAM_GUARD_SUPPLY_CENTER].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_GUARD_SUPPLY_CENTER].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_GUARD_SUPPLY_CENTER].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::TEAM_GUARD_SUPPLY_CENTER].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_GUARD_SUPPLY_CENTER].m_uiStrings[0] = "Have Team ";
    m_actionTemplates[ScriptAction::TEAM_GUARD_SUPPLY_CENTER].m_uiStrings[1] =
        " guard attacked or closest supply src with at least ";
    m_actionTemplates[ScriptAction::TEAM_GUARD_SUPPLY_CENTER].m_uiStrings[2] = " available resources";

    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_UPGRADE].m_internalName = "AI_PLAYER_BUILD_UPGRADE";
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_UPGRADE].m_uiName = "Player_/AI/AI player build an upgrade.";
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_UPGRADE].m_numParameters = 2;
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_UPGRADE].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_UPGRADE].m_parameters[1] = Parameter::UPGRADE;
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_UPGRADE].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_UPGRADE].m_uiStrings[0] = "Have AI ";
    m_actionTemplates[ScriptAction::AI_PLAYER_BUILD_UPGRADE].m_uiStrings[1] = " build this upgrade: ";

    m_actionTemplates[ScriptAction::SKIRMISH_FOLLOW_APPROACH_PATH].m_internalName = "SKIRMISH_FOLLOW_APPROACH_PATH";
    m_actionTemplates[ScriptAction::SKIRMISH_FOLLOW_APPROACH_PATH].m_uiName =
        "Skirmish Only_/Move/Team follow approach path.";
    m_actionTemplates[ScriptAction::SKIRMISH_FOLLOW_APPROACH_PATH].m_numParameters = 3;
    m_actionTemplates[ScriptAction::SKIRMISH_FOLLOW_APPROACH_PATH].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::SKIRMISH_FOLLOW_APPROACH_PATH].m_parameters[1] = Parameter::SKIRMISH_APPROACH_PATH;
    m_actionTemplates[ScriptAction::SKIRMISH_FOLLOW_APPROACH_PATH].m_parameters[2] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::SKIRMISH_FOLLOW_APPROACH_PATH].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::SKIRMISH_FOLLOW_APPROACH_PATH].m_uiStrings[0] = "Have ";
    m_actionTemplates[ScriptAction::SKIRMISH_FOLLOW_APPROACH_PATH].m_uiStrings[1] = " approach the enemy using path ";
    m_actionTemplates[ScriptAction::SKIRMISH_FOLLOW_APPROACH_PATH].m_uiStrings[2] = ", as a team is ";

    m_actionTemplates[ScriptAction::SKIRMISH_MOVE_TO_APPROACH_PATH].m_internalName = "SKIRMISH_MOVE_TO_APPROACH_PATH";
    m_actionTemplates[ScriptAction::SKIRMISH_MOVE_TO_APPROACH_PATH].m_uiName =
        "Skirmish Only_/Move/Team move to approach path.";
    m_actionTemplates[ScriptAction::SKIRMISH_MOVE_TO_APPROACH_PATH].m_numParameters = 2;
    m_actionTemplates[ScriptAction::SKIRMISH_MOVE_TO_APPROACH_PATH].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::SKIRMISH_MOVE_TO_APPROACH_PATH].m_parameters[1] = Parameter::SKIRMISH_APPROACH_PATH;
    m_actionTemplates[ScriptAction::SKIRMISH_MOVE_TO_APPROACH_PATH].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::SKIRMISH_MOVE_TO_APPROACH_PATH].m_uiStrings[0] = "Have ";
    m_actionTemplates[ScriptAction::SKIRMISH_MOVE_TO_APPROACH_PATH].m_uiStrings[1] = " move to the start of enemy path ";
    m_actionTemplates[ScriptAction::SKIRMISH_MOVE_TO_APPROACH_PATH].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_BASE_DEFENSE_FRONT].m_internalName = "SKIRMISH_BUILD_BASE_DEFENSE_FRONT";
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_BASE_DEFENSE_FRONT].m_uiName =
        "Skirmish Only_/Build/Build base defense on front perimeter.";
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_BASE_DEFENSE_FRONT].m_numParameters = 0;
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_BASE_DEFENSE_FRONT].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_BASE_DEFENSE_FRONT].m_uiStrings[0] =
        "Build one additional perimeter base defenses, on the front.";

    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_BASE_DEFENSE_FLANK].m_internalName = "SKIRMISH_BUILD_BASE_DEFENSE_FLANK";
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_BASE_DEFENSE_FLANK].m_uiName =
        "Skirmish Only_/Build/Build base defense on flank perimeter.";
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_BASE_DEFENSE_FLANK].m_numParameters = 0;
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_BASE_DEFENSE_FLANK].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_BASE_DEFENSE_FLANK].m_uiStrings[0] =
        "Build one additional perimeter base defenses, on the flank.";

    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_STRUCTURE_FRONT].m_internalName = "SKIRMISH_BUILD_STRUCTURE_FRONT";
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_STRUCTURE_FRONT].m_uiName =
        "Skirmish Only_/Build/Build structure on front perimeter.";
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_STRUCTURE_FRONT].m_numParameters = 1;
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_STRUCTURE_FRONT].m_parameters[0] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_STRUCTURE_FRONT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_STRUCTURE_FRONT].m_uiStrings[0] = "Build one additional ";
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_STRUCTURE_FRONT].m_uiStrings[1] = ", on the front.";

    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_STRUCTURE_FLANK].m_internalName = "SKIRMISH_BUILD_STRUCTURE_FLANK";
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_STRUCTURE_FLANK].m_uiName =
        "Skirmish Only_/Build/Build structure on flank perimeter.";
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_STRUCTURE_FLANK].m_numParameters = 1;
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_STRUCTURE_FLANK].m_parameters[0] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_STRUCTURE_FLANK].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_STRUCTURE_FLANK].m_uiStrings[0] = "Build one additional ";
    m_actionTemplates[ScriptAction::SKIRMISH_BUILD_STRUCTURE_FLANK].m_uiStrings[1] = ", on the flank.";

    m_actionTemplates[ScriptAction::RECRUIT_TEAM].m_internalName = "RECRUIT_TEAM";
    m_actionTemplates[ScriptAction::RECRUIT_TEAM].m_uiName = "Team_/Create/Recruit a team.";
    m_actionTemplates[ScriptAction::RECRUIT_TEAM].m_numParameters = 2;
    m_actionTemplates[ScriptAction::RECRUIT_TEAM].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::RECRUIT_TEAM].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::RECRUIT_TEAM].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::RECRUIT_TEAM].m_uiStrings[0] = "Recruit an instance of ";
    m_actionTemplates[ScriptAction::RECRUIT_TEAM].m_uiStrings[1] = ", maximum recruiting distance (feet):";
    m_actionTemplates[ScriptAction::RECRUIT_TEAM].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::MOVE_CAMERA_TO].m_internalName = "MOVE_CAMERA_TO";
    m_actionTemplates[ScriptAction::MOVE_CAMERA_TO].m_uiName = "Camera_/Move/Move the camera to a location.";
    m_actionTemplates[ScriptAction::MOVE_CAMERA_TO].m_numParameters = 5;
    m_actionTemplates[ScriptAction::MOVE_CAMERA_TO].m_parameters[0] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::MOVE_CAMERA_TO].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::MOVE_CAMERA_TO].m_parameters[2] = Parameter::REAL;
    m_actionTemplates[ScriptAction::MOVE_CAMERA_TO].m_parameters[3] = Parameter::REAL;
    m_actionTemplates[ScriptAction::MOVE_CAMERA_TO].m_parameters[4] = Parameter::REAL;
    m_actionTemplates[ScriptAction::MOVE_CAMERA_TO].m_numUiStrings = 6;
    m_actionTemplates[ScriptAction::MOVE_CAMERA_TO].m_uiStrings[0] = "Move camera to ";
    m_actionTemplates[ScriptAction::MOVE_CAMERA_TO].m_uiStrings[1] = " in ";
    m_actionTemplates[ScriptAction::MOVE_CAMERA_TO].m_uiStrings[2] = " seconds, camera shutter ";
    m_actionTemplates[ScriptAction::MOVE_CAMERA_TO].m_uiStrings[3] = " seconds, ease-in ";
    m_actionTemplates[ScriptAction::MOVE_CAMERA_TO].m_uiStrings[4] = " seconds, ease-out ";
    m_actionTemplates[ScriptAction::MOVE_CAMERA_TO].m_uiStrings[5] = " seconds.";

    m_actionTemplates[ScriptAction::ZOOM_CAMERA].m_internalName = "ZOOM_CAMERA";
    m_actionTemplates[ScriptAction::ZOOM_CAMERA].m_uiName = "Camera_/Adjust/Change the camera zoom.";
    m_actionTemplates[ScriptAction::ZOOM_CAMERA].m_numParameters = 4;
    m_actionTemplates[ScriptAction::ZOOM_CAMERA].m_parameters[0] = Parameter::REAL;
    m_actionTemplates[ScriptAction::ZOOM_CAMERA].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::ZOOM_CAMERA].m_parameters[2] = Parameter::REAL;
    m_actionTemplates[ScriptAction::ZOOM_CAMERA].m_parameters[3] = Parameter::REAL;
    m_actionTemplates[ScriptAction::ZOOM_CAMERA].m_numUiStrings = 5;
    m_actionTemplates[ScriptAction::ZOOM_CAMERA].m_uiStrings[0] = "Change camera zoom to ";
    m_actionTemplates[ScriptAction::ZOOM_CAMERA].m_uiStrings[1] = " in ";
    m_actionTemplates[ScriptAction::ZOOM_CAMERA].m_uiStrings[2] = " seconds, ease-in ";
    m_actionTemplates[ScriptAction::ZOOM_CAMERA].m_uiStrings[3] = " seconds, ease-out ";
    m_actionTemplates[ScriptAction::ZOOM_CAMERA].m_uiStrings[4] = " seconds.";

    m_actionTemplates[ScriptAction::CAMERA_FADE_ADD].m_internalName = "CAMERA_FADE_ADD";
    m_actionTemplates[ScriptAction::CAMERA_FADE_ADD].m_uiName = "Camera_/Fade Effects/Fade using an add blend to white.";
    m_actionTemplates[ScriptAction::CAMERA_FADE_ADD].m_numParameters = 5;
    m_actionTemplates[ScriptAction::CAMERA_FADE_ADD].m_parameters[0] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_FADE_ADD].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_FADE_ADD].m_parameters[2] = Parameter::INT;
    m_actionTemplates[ScriptAction::CAMERA_FADE_ADD].m_parameters[3] = Parameter::INT;
    m_actionTemplates[ScriptAction::CAMERA_FADE_ADD].m_parameters[4] = Parameter::INT;
    m_actionTemplates[ScriptAction::CAMERA_FADE_ADD].m_numUiStrings = 6;
    m_actionTemplates[ScriptAction::CAMERA_FADE_ADD].m_uiStrings[0] = "Fade (0-1) from ";
    m_actionTemplates[ScriptAction::CAMERA_FADE_ADD].m_uiStrings[1] = " to ";
    m_actionTemplates[ScriptAction::CAMERA_FADE_ADD].m_uiStrings[2] = " adding toward white. Take ";
    m_actionTemplates[ScriptAction::CAMERA_FADE_ADD].m_uiStrings[3] = " frames to increase, hold for ";
    m_actionTemplates[ScriptAction::CAMERA_FADE_ADD].m_uiStrings[4] = " fames, and decrease ";
    m_actionTemplates[ScriptAction::CAMERA_FADE_ADD].m_uiStrings[5] = " frames.";

    m_actionTemplates[ScriptAction::CAMERA_FADE_SUBTRACT].m_internalName = "CAMERA_FADE_SUBTRACT";
    m_actionTemplates[ScriptAction::CAMERA_FADE_SUBTRACT].m_uiName =
        "Camera_/Fade Effects/Fade using a subtractive blend to black.";
    m_actionTemplates[ScriptAction::CAMERA_FADE_SUBTRACT].m_numParameters = 5;
    m_actionTemplates[ScriptAction::CAMERA_FADE_SUBTRACT].m_parameters[0] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_FADE_SUBTRACT].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_FADE_SUBTRACT].m_parameters[2] = Parameter::INT;
    m_actionTemplates[ScriptAction::CAMERA_FADE_SUBTRACT].m_parameters[3] = Parameter::INT;
    m_actionTemplates[ScriptAction::CAMERA_FADE_SUBTRACT].m_parameters[4] = Parameter::INT;
    m_actionTemplates[ScriptAction::CAMERA_FADE_SUBTRACT].m_numUiStrings = 6;
    m_actionTemplates[ScriptAction::CAMERA_FADE_SUBTRACT].m_uiStrings[0] = "Fade (0-1) from ";
    m_actionTemplates[ScriptAction::CAMERA_FADE_SUBTRACT].m_uiStrings[1] = " to ";
    m_actionTemplates[ScriptAction::CAMERA_FADE_SUBTRACT].m_uiStrings[2] = " subtracting toward black. Take ";
    m_actionTemplates[ScriptAction::CAMERA_FADE_SUBTRACT].m_uiStrings[3] = " frames to increase, hold for ";
    m_actionTemplates[ScriptAction::CAMERA_FADE_SUBTRACT].m_uiStrings[4] = " fames, and decrease ";
    m_actionTemplates[ScriptAction::CAMERA_FADE_SUBTRACT].m_uiStrings[5] = " frames.";

    m_actionTemplates[ScriptAction::CAMERA_FADE_MULTIPLY].m_internalName = "CAMERA_FADE_MULTIPLY";
    m_actionTemplates[ScriptAction::CAMERA_FADE_MULTIPLY].m_uiName =
        "Camera_/Fade Effects/Fade using a multiply blend to black.";
    m_actionTemplates[ScriptAction::CAMERA_FADE_MULTIPLY].m_numParameters = 5;
    m_actionTemplates[ScriptAction::CAMERA_FADE_MULTIPLY].m_parameters[0] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_FADE_MULTIPLY].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_FADE_MULTIPLY].m_parameters[2] = Parameter::INT;
    m_actionTemplates[ScriptAction::CAMERA_FADE_MULTIPLY].m_parameters[3] = Parameter::INT;
    m_actionTemplates[ScriptAction::CAMERA_FADE_MULTIPLY].m_parameters[4] = Parameter::INT;
    m_actionTemplates[ScriptAction::CAMERA_FADE_MULTIPLY].m_numUiStrings = 6;
    m_actionTemplates[ScriptAction::CAMERA_FADE_MULTIPLY].m_uiStrings[0] = "Fade (1-0) from ";
    m_actionTemplates[ScriptAction::CAMERA_FADE_MULTIPLY].m_uiStrings[1] = " to ";
    m_actionTemplates[ScriptAction::CAMERA_FADE_MULTIPLY].m_uiStrings[2] = " multiplying toward black. Take ";
    m_actionTemplates[ScriptAction::CAMERA_FADE_MULTIPLY].m_uiStrings[3] = " frames to increase, hold for ";
    m_actionTemplates[ScriptAction::CAMERA_FADE_MULTIPLY].m_uiStrings[4] = " fames, and decrease ";
    m_actionTemplates[ScriptAction::CAMERA_FADE_MULTIPLY].m_uiStrings[5] = " frames.";

    m_actionTemplates[ScriptAction::CAMERA_FADE_SATURATE].m_internalName = "CAMERA_FADE_SATURATE";
    m_actionTemplates[ScriptAction::CAMERA_FADE_SATURATE].m_uiName = "Camera_/Fade Effects/Fade using a saturate blend.";
    m_actionTemplates[ScriptAction::CAMERA_FADE_SATURATE].m_numParameters = 5;
    m_actionTemplates[ScriptAction::CAMERA_FADE_SATURATE].m_parameters[0] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_FADE_SATURATE].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_FADE_SATURATE].m_parameters[2] = Parameter::INT;
    m_actionTemplates[ScriptAction::CAMERA_FADE_SATURATE].m_parameters[3] = Parameter::INT;
    m_actionTemplates[ScriptAction::CAMERA_FADE_SATURATE].m_parameters[4] = Parameter::INT;
    m_actionTemplates[ScriptAction::CAMERA_FADE_SATURATE].m_numUiStrings = 6;
    m_actionTemplates[ScriptAction::CAMERA_FADE_SATURATE].m_uiStrings[0] = "Fade (0.5-1) from ";
    m_actionTemplates[ScriptAction::CAMERA_FADE_SATURATE].m_uiStrings[1] = " to ";
    m_actionTemplates[ScriptAction::CAMERA_FADE_SATURATE].m_uiStrings[2] = " increasing saturation. Take ";
    m_actionTemplates[ScriptAction::CAMERA_FADE_SATURATE].m_uiStrings[3] = " frames to increase, hold for ";
    m_actionTemplates[ScriptAction::CAMERA_FADE_SATURATE].m_uiStrings[4] = " fames, and decrease ";
    m_actionTemplates[ScriptAction::CAMERA_FADE_SATURATE].m_uiStrings[5] = " frames.";

    m_actionTemplates[ScriptAction::PITCH_CAMERA].m_internalName = "PITCH_CAMERA";
    m_actionTemplates[ScriptAction::PITCH_CAMERA].m_uiName = "Camera_/Adjust/Change the camera pitch.";
    m_actionTemplates[ScriptAction::PITCH_CAMERA].m_numParameters = 4;
    m_actionTemplates[ScriptAction::PITCH_CAMERA].m_parameters[0] = Parameter::REAL;
    m_actionTemplates[ScriptAction::PITCH_CAMERA].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::PITCH_CAMERA].m_parameters[2] = Parameter::REAL;
    m_actionTemplates[ScriptAction::PITCH_CAMERA].m_parameters[3] = Parameter::REAL;
    m_actionTemplates[ScriptAction::PITCH_CAMERA].m_numUiStrings = 5;
    m_actionTemplates[ScriptAction::PITCH_CAMERA].m_uiStrings[0] = "Change camera pitch to ";
    m_actionTemplates[ScriptAction::PITCH_CAMERA].m_uiStrings[1] = " in ";
    m_actionTemplates[ScriptAction::PITCH_CAMERA].m_uiStrings[2] = " seconds, ease-in ";
    m_actionTemplates[ScriptAction::PITCH_CAMERA].m_uiStrings[3] = " seconds, ease-out ";
    m_actionTemplates[ScriptAction::PITCH_CAMERA].m_uiStrings[4] = " seconds.";

    m_actionTemplates[ScriptAction::CAMERA_FOLLOW_NAMED].m_internalName = "CAMERA_FOLLOW_NAMED";
    m_actionTemplates[ScriptAction::CAMERA_FOLLOW_NAMED].m_uiName = "Camera_/Move/Follow a specific unit.";
    m_actionTemplates[ScriptAction::CAMERA_FOLLOW_NAMED].m_numParameters = 2;
    m_actionTemplates[ScriptAction::CAMERA_FOLLOW_NAMED].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::CAMERA_FOLLOW_NAMED].m_parameters[1] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::CAMERA_FOLLOW_NAMED].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::CAMERA_FOLLOW_NAMED].m_uiStrings[0] = "Have the camera follow ";
    m_actionTemplates[ScriptAction::CAMERA_FOLLOW_NAMED].m_uiStrings[1] = ".  Snap camera to object is ";
    m_actionTemplates[ScriptAction::CAMERA_FOLLOW_NAMED].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::CAMERA_STOP_FOLLOW].m_internalName = "CAMERA_STOP_FOLLOW";
    m_actionTemplates[ScriptAction::CAMERA_STOP_FOLLOW].m_uiName = "Camera_/Move/Stop following any units.";
    m_actionTemplates[ScriptAction::CAMERA_STOP_FOLLOW].m_numParameters = 0;
    m_actionTemplates[ScriptAction::CAMERA_STOP_FOLLOW].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::CAMERA_STOP_FOLLOW].m_uiStrings[0] = "Stop following any units.";

    m_actionTemplates[ScriptAction::SETUP_CAMERA].m_internalName = "SETUP_CAMERA";
    m_actionTemplates[ScriptAction::SETUP_CAMERA].m_uiName = "Camera_/Adjust/Set up the camera.";
    m_actionTemplates[ScriptAction::SETUP_CAMERA].m_numParameters = 4;
    m_actionTemplates[ScriptAction::SETUP_CAMERA].m_parameters[0] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::SETUP_CAMERA].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::SETUP_CAMERA].m_parameters[2] = Parameter::REAL;
    m_actionTemplates[ScriptAction::SETUP_CAMERA].m_parameters[3] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::SETUP_CAMERA].m_numUiStrings = 5;
    m_actionTemplates[ScriptAction::SETUP_CAMERA].m_uiStrings[0] = "Position camera at ";
    m_actionTemplates[ScriptAction::SETUP_CAMERA].m_uiStrings[1] = ", zoom = ";
    m_actionTemplates[ScriptAction::SETUP_CAMERA].m_uiStrings[2] = "(0.0 to 1.0), pitch = ";
    m_actionTemplates[ScriptAction::SETUP_CAMERA].m_uiStrings[3] = "(1.0==default), looking towards ";
    m_actionTemplates[ScriptAction::SETUP_CAMERA].m_uiStrings[4] = ".";

    m_actionTemplates[ScriptAction::INCREMENT_COUNTER].m_internalName = "INCREMENT_COUNTER";
    m_actionTemplates[ScriptAction::INCREMENT_COUNTER].m_uiName = "Scripting_/Counters/Increment counter.";
    m_actionTemplates[ScriptAction::INCREMENT_COUNTER].m_numParameters = 2;
    m_actionTemplates[ScriptAction::INCREMENT_COUNTER].m_parameters[0] = Parameter::INT;
    m_actionTemplates[ScriptAction::INCREMENT_COUNTER].m_parameters[1] = Parameter::COUNTER;
    m_actionTemplates[ScriptAction::INCREMENT_COUNTER].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::INCREMENT_COUNTER].m_uiStrings[0] = "Add ";
    m_actionTemplates[ScriptAction::INCREMENT_COUNTER].m_uiStrings[1] = " to counter ";

    m_actionTemplates[ScriptAction::DECREMENT_COUNTER].m_internalName = "DECREMENT_COUNTER";
    m_actionTemplates[ScriptAction::DECREMENT_COUNTER].m_uiName = "Scripting_/Counters/Decrement counter.";
    m_actionTemplates[ScriptAction::DECREMENT_COUNTER].m_numParameters = 2;
    m_actionTemplates[ScriptAction::DECREMENT_COUNTER].m_parameters[0] = Parameter::INT;
    m_actionTemplates[ScriptAction::DECREMENT_COUNTER].m_parameters[1] = Parameter::COUNTER;
    m_actionTemplates[ScriptAction::DECREMENT_COUNTER].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::DECREMENT_COUNTER].m_uiStrings[0] = "Subtract ";
    m_actionTemplates[ScriptAction::DECREMENT_COUNTER].m_uiStrings[1] = " from counter ";

    m_actionTemplates[ScriptAction::MOVE_CAMERA_ALONG_WAYPOINT_PATH].m_internalName = "MOVE_CAMERA_ALONG_WAYPOINT_PATH";
    m_actionTemplates[ScriptAction::MOVE_CAMERA_ALONG_WAYPOINT_PATH].m_uiName = "Camera_/Move/Move along a waypoint path.";
    m_actionTemplates[ScriptAction::MOVE_CAMERA_ALONG_WAYPOINT_PATH].m_numParameters = 5;
    m_actionTemplates[ScriptAction::MOVE_CAMERA_ALONG_WAYPOINT_PATH].m_parameters[0] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::MOVE_CAMERA_ALONG_WAYPOINT_PATH].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::MOVE_CAMERA_ALONG_WAYPOINT_PATH].m_parameters[2] = Parameter::REAL;
    m_actionTemplates[ScriptAction::MOVE_CAMERA_ALONG_WAYPOINT_PATH].m_parameters[3] = Parameter::REAL;
    m_actionTemplates[ScriptAction::MOVE_CAMERA_ALONG_WAYPOINT_PATH].m_parameters[4] = Parameter::REAL;
    m_actionTemplates[ScriptAction::MOVE_CAMERA_ALONG_WAYPOINT_PATH].m_numUiStrings = 6;
    m_actionTemplates[ScriptAction::MOVE_CAMERA_ALONG_WAYPOINT_PATH].m_uiStrings[0] = "Move along path starting with ";
    m_actionTemplates[ScriptAction::MOVE_CAMERA_ALONG_WAYPOINT_PATH].m_uiStrings[1] = " in ";
    m_actionTemplates[ScriptAction::MOVE_CAMERA_ALONG_WAYPOINT_PATH].m_uiStrings[2] = " seconds, camera shutter ";
    m_actionTemplates[ScriptAction::MOVE_CAMERA_ALONG_WAYPOINT_PATH].m_uiStrings[3] = " seconds, ease-in ";
    m_actionTemplates[ScriptAction::MOVE_CAMERA_ALONG_WAYPOINT_PATH].m_uiStrings[4] = " seconds, ease-out ";
    m_actionTemplates[ScriptAction::MOVE_CAMERA_ALONG_WAYPOINT_PATH].m_uiStrings[5] = " seconds.";

    m_actionTemplates[ScriptAction::ROTATE_CAMERA].m_internalName = "ROTATE_CAMERA";
    m_actionTemplates[ScriptAction::ROTATE_CAMERA].m_uiName = "Camera_/Rotate/ Rotate around the current viewpoint.";
    m_actionTemplates[ScriptAction::ROTATE_CAMERA].m_numParameters = 4;
    m_actionTemplates[ScriptAction::ROTATE_CAMERA].m_parameters[0] = Parameter::REAL;
    m_actionTemplates[ScriptAction::ROTATE_CAMERA].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::ROTATE_CAMERA].m_parameters[2] = Parameter::REAL;
    m_actionTemplates[ScriptAction::ROTATE_CAMERA].m_parameters[3] = Parameter::REAL;
    m_actionTemplates[ScriptAction::ROTATE_CAMERA].m_numUiStrings = 5;
    m_actionTemplates[ScriptAction::ROTATE_CAMERA].m_uiStrings[0] = "Rotate ";
    m_actionTemplates[ScriptAction::ROTATE_CAMERA].m_uiStrings[1] = " times, taking ";
    m_actionTemplates[ScriptAction::ROTATE_CAMERA].m_uiStrings[2] = " seconds, ease-in ";
    m_actionTemplates[ScriptAction::ROTATE_CAMERA].m_uiStrings[3] = " seconds, ease-out ";
    m_actionTemplates[ScriptAction::ROTATE_CAMERA].m_uiStrings[4] = " seconds.";

    m_actionTemplates[ScriptAction::RESET_CAMERA].m_internalName = "RESET_CAMERA";
    m_actionTemplates[ScriptAction::RESET_CAMERA].m_uiName = "Camera_/Move/ Reset to the default view.";
    m_actionTemplates[ScriptAction::RESET_CAMERA].m_numParameters = 4;
    m_actionTemplates[ScriptAction::RESET_CAMERA].m_parameters[0] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::RESET_CAMERA].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::RESET_CAMERA].m_parameters[2] = Parameter::REAL;
    m_actionTemplates[ScriptAction::RESET_CAMERA].m_parameters[3] = Parameter::REAL;
    m_actionTemplates[ScriptAction::RESET_CAMERA].m_numUiStrings = 5;
    m_actionTemplates[ScriptAction::RESET_CAMERA].m_uiStrings[0] = "Reset to ";
    m_actionTemplates[ScriptAction::RESET_CAMERA].m_uiStrings[1] = ", taking ";
    m_actionTemplates[ScriptAction::RESET_CAMERA].m_uiStrings[2] = " seconds, ease-in ";
    m_actionTemplates[ScriptAction::RESET_CAMERA].m_uiStrings[3] = " seconds, ease-out ";
    m_actionTemplates[ScriptAction::RESET_CAMERA].m_uiStrings[4] = " seconds.";

    m_actionTemplates[ScriptAction::MOVE_CAMERA_TO_SELECTION].m_internalName = "MOVE_CAMERA_TO_SELECTION";
    m_actionTemplates[ScriptAction::MOVE_CAMERA_TO_SELECTION].m_uiName =
        "Camera_/Move/Modify/ End movement at selected unit.";
    m_actionTemplates[ScriptAction::MOVE_CAMERA_TO_SELECTION].m_numParameters = 0;
    m_actionTemplates[ScriptAction::MOVE_CAMERA_TO_SELECTION].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::MOVE_CAMERA_TO_SELECTION].m_uiStrings[0] = "End movement at selected unit.";

    m_actionTemplates[ScriptAction::SET_MILLISECOND_TIMER].m_internalName = "SET_MILLISECOND_TIMER";
    m_actionTemplates[ScriptAction::SET_MILLISECOND_TIMER].m_uiName = "Scripting_/Timer/Seconds countdown timer -- set.";
    m_actionTemplates[ScriptAction::SET_MILLISECOND_TIMER].m_numParameters = 2;
    m_actionTemplates[ScriptAction::SET_MILLISECOND_TIMER].m_parameters[0] = Parameter::COUNTER;
    m_actionTemplates[ScriptAction::SET_MILLISECOND_TIMER].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::SET_MILLISECOND_TIMER].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::SET_MILLISECOND_TIMER].m_uiStrings[0] = "Set timer ";
    m_actionTemplates[ScriptAction::SET_MILLISECOND_TIMER].m_uiStrings[1] = " to expire in ";
    m_actionTemplates[ScriptAction::SET_MILLISECOND_TIMER].m_uiStrings[2] = " seconds.";

    m_actionTemplates[ScriptAction::SET_RANDOM_MSEC_TIMER].m_internalName = "SET_RANDOM_MSEC_TIMER";
    m_actionTemplates[ScriptAction::SET_RANDOM_MSEC_TIMER].m_uiName =
        "Scripting_/Timer/Seconds countdown timer -- set random.";
    m_actionTemplates[ScriptAction::SET_RANDOM_MSEC_TIMER].m_numParameters = 3;
    m_actionTemplates[ScriptAction::SET_RANDOM_MSEC_TIMER].m_parameters[0] = Parameter::COUNTER;
    m_actionTemplates[ScriptAction::SET_RANDOM_MSEC_TIMER].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::SET_RANDOM_MSEC_TIMER].m_parameters[2] = Parameter::REAL;
    m_actionTemplates[ScriptAction::SET_RANDOM_MSEC_TIMER].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::SET_RANDOM_MSEC_TIMER].m_uiStrings[0] = "Set timer ";
    m_actionTemplates[ScriptAction::SET_RANDOM_MSEC_TIMER].m_uiStrings[1] = " to expire between ";
    m_actionTemplates[ScriptAction::SET_RANDOM_MSEC_TIMER].m_uiStrings[2] = " and ";
    m_actionTemplates[ScriptAction::SET_RANDOM_MSEC_TIMER].m_uiStrings[3] = " seconds.";

    m_actionTemplates[ScriptAction::ADD_TO_MSEC_TIMER].m_internalName = "ADD_TO_MSEC_TIMER";
    m_actionTemplates[ScriptAction::ADD_TO_MSEC_TIMER].m_uiName = "Scripting_/Timer/Seconds countdown timer -- add seconds.";
    m_actionTemplates[ScriptAction::ADD_TO_MSEC_TIMER].m_numParameters = 2;
    m_actionTemplates[ScriptAction::ADD_TO_MSEC_TIMER].m_parameters[0] = Parameter::REAL;
    m_actionTemplates[ScriptAction::ADD_TO_MSEC_TIMER].m_parameters[1] = Parameter::COUNTER;
    m_actionTemplates[ScriptAction::ADD_TO_MSEC_TIMER].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::ADD_TO_MSEC_TIMER].m_uiStrings[0] = "Add ";
    m_actionTemplates[ScriptAction::ADD_TO_MSEC_TIMER].m_uiStrings[1] = " seconds to timer ";
    m_actionTemplates[ScriptAction::ADD_TO_MSEC_TIMER].m_uiStrings[2] = " .";

    m_actionTemplates[ScriptAction::SUB_FROM_MSEC_TIMER].m_internalName = "SUB_FROM_MSEC_TIMER";
    m_actionTemplates[ScriptAction::SUB_FROM_MSEC_TIMER].m_uiName =
        "Scripting_/Timer/Seconds countdown timer -- subtract seconds.";
    m_actionTemplates[ScriptAction::SUB_FROM_MSEC_TIMER].m_numParameters = 2;
    m_actionTemplates[ScriptAction::SUB_FROM_MSEC_TIMER].m_parameters[0] = Parameter::REAL;
    m_actionTemplates[ScriptAction::SUB_FROM_MSEC_TIMER].m_parameters[1] = Parameter::COUNTER;
    m_actionTemplates[ScriptAction::SUB_FROM_MSEC_TIMER].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::SUB_FROM_MSEC_TIMER].m_uiStrings[0] = "Subtract ";
    m_actionTemplates[ScriptAction::SUB_FROM_MSEC_TIMER].m_uiStrings[1] = " seconds from timer ";
    m_actionTemplates[ScriptAction::SUB_FROM_MSEC_TIMER].m_uiStrings[2] = " .";

    m_actionTemplates[ScriptAction::CAMERA_MOD_FREEZE_TIME].m_internalName = "CAMERA_MOD_FREEZE_TIME";
    m_actionTemplates[ScriptAction::CAMERA_MOD_FREEZE_TIME].m_uiName =
        "Camera_/Move/Modify/ Freeze time during the camera movement.";
    m_actionTemplates[ScriptAction::CAMERA_MOD_FREEZE_TIME].m_numParameters = 0;
    m_actionTemplates[ScriptAction::CAMERA_MOD_FREEZE_TIME].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::CAMERA_MOD_FREEZE_TIME].m_uiStrings[0] = "Freeze time during the camera movement.";

    m_actionTemplates[ScriptAction::CAMERA_MOD_FREEZE_ANGLE].m_internalName = "CAMERA_MOD_FREEZE_ANGLE";
    m_actionTemplates[ScriptAction::CAMERA_MOD_FREEZE_ANGLE].m_uiName =
        "Camera_/Move/Modify/ Freeze camera angle during the camera movement.";
    m_actionTemplates[ScriptAction::CAMERA_MOD_FREEZE_ANGLE].m_numParameters = 0;
    m_actionTemplates[ScriptAction::CAMERA_MOD_FREEZE_ANGLE].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::CAMERA_MOD_FREEZE_ANGLE].m_uiStrings[0] =
        "Freeze camera angle during the camera movement.";

    m_actionTemplates[ScriptAction::SUSPEND_BACKGROUND_SOUNDS].m_internalName = "SUSPEND_BACKGROUND_SOUNDS";
    m_actionTemplates[ScriptAction::SUSPEND_BACKGROUND_SOUNDS].m_uiName = "Multimedia_/All Sounds/Suspend all sounds.";
    m_actionTemplates[ScriptAction::SUSPEND_BACKGROUND_SOUNDS].m_numParameters = 0;
    m_actionTemplates[ScriptAction::SUSPEND_BACKGROUND_SOUNDS].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::SUSPEND_BACKGROUND_SOUNDS].m_uiStrings[0] = "Suspend background sounds.";

    m_actionTemplates[ScriptAction::RESUME_BACKGROUND_SOUNDS].m_internalName = "RESUME_BACKGROUND_SOUNDS";
    m_actionTemplates[ScriptAction::RESUME_BACKGROUND_SOUNDS].m_uiName = "Multimedia_/All Sounds/Resume all sounds.";
    m_actionTemplates[ScriptAction::RESUME_BACKGROUND_SOUNDS].m_numParameters = 0;
    m_actionTemplates[ScriptAction::RESUME_BACKGROUND_SOUNDS].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::RESUME_BACKGROUND_SOUNDS].m_uiStrings[0] = "Resume background sounds.";

    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_ZOOM].m_internalName = "CAMERA_MOD_SET_FINAL_ZOOM";
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_ZOOM].m_uiName =
        "Camera_/Move/Modify/Set Final zoom for camera movement.";
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_ZOOM].m_numParameters = 3;
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_ZOOM].m_parameters[0] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_ZOOM].m_parameters[1] = Parameter::PERCENT;
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_ZOOM].m_parameters[2] = Parameter::PERCENT;
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_ZOOM].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_ZOOM].m_uiStrings[0] = "Adjust zoom to ";
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_ZOOM].m_uiStrings[1] = " (1.0==max height, 0.0==in the ground) ";
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_ZOOM].m_uiStrings[2] = " ease-in ";
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_ZOOM].m_uiStrings[3] = " ease-out.";

    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_PITCH].m_internalName = "CAMERA_MOD_SET_FINAL_PITCH";
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_PITCH].m_uiName =
        "Camera_/Move/Modify/Set Final pitch for camera movement.";
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_PITCH].m_numParameters = 3;
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_PITCH].m_parameters[0] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_PITCH].m_parameters[1] = Parameter::PERCENT;
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_PITCH].m_parameters[2] = Parameter::PERCENT;
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_PITCH].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_PITCH].m_uiStrings[0] = "Adjust pitch to ";
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_PITCH].m_uiStrings[1] =
        " (1.0==default, 0.0==toward horizon, >1 = toward ground) ";
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_PITCH].m_uiStrings[2] = " ease-in ";
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_PITCH].m_uiStrings[3] = " ease-out.";

    m_actionTemplates[ScriptAction::SET_VISUAL_SPEED_MULTIPLIER].m_internalName = "SET_VISUAL_SPEED_MULTIPLIER";
    m_actionTemplates[ScriptAction::SET_VISUAL_SPEED_MULTIPLIER].m_uiName =
        "{Compatibility}_/Multimedia/Modify visual game time.";
    m_actionTemplates[ScriptAction::SET_VISUAL_SPEED_MULTIPLIER].m_numParameters = 1;
    m_actionTemplates[ScriptAction::SET_VISUAL_SPEED_MULTIPLIER].m_parameters[0] = Parameter::INT;
    m_actionTemplates[ScriptAction::SET_VISUAL_SPEED_MULTIPLIER].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::SET_VISUAL_SPEED_MULTIPLIER].m_uiStrings[0] = "Make visual time ";
    m_actionTemplates[ScriptAction::SET_VISUAL_SPEED_MULTIPLIER].m_uiStrings[1] =
        " time normal (1=normal, 2 = twice as fast, ...).";
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_SPEED_MULTIPLIER].m_internalName =
        "CAMERA_MOD_SET_FINAL_SPEED_MULTIPLIER";
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_SPEED_MULTIPLIER].m_uiName =
        "{Compatibility}_/Camera/Modify/Final visual game time for camera movement.";
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_SPEED_MULTIPLIER].m_numParameters = 1;
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_SPEED_MULTIPLIER].m_parameters[0] = Parameter::INT;
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_SPEED_MULTIPLIER].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_SPEED_MULTIPLIER].m_uiStrings[0] = "Adjust game time to";
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_FINAL_SPEED_MULTIPLIER].m_uiStrings[1] =
        " times normal (1=normal, 2 = twice as fast, ...).";

    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_ROLLING_AVERAGE].m_internalName = "CAMERA_MOD_SET_ROLLING_AVERAGE";
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_ROLLING_AVERAGE].m_uiName =
        "Camera_/Move/Modify/ Number of frames to average movements.";
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_ROLLING_AVERAGE].m_numParameters = 1;
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_ROLLING_AVERAGE].m_parameters[0] = Parameter::INT;
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_ROLLING_AVERAGE].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_ROLLING_AVERAGE].m_uiStrings[0] =
        "Average position and angle changes over";
    m_actionTemplates[ScriptAction::CAMERA_MOD_SET_ROLLING_AVERAGE].m_uiStrings[1] =
        " frames. (1=no smoothing, 5 = very smooth)";

    m_actionTemplates[ScriptAction::CAMERA_MOD_FINAL_LOOK_TOWARD].m_internalName = "CAMERA_MOD_FINAL_LOOK_TOWARD";
    m_actionTemplates[ScriptAction::CAMERA_MOD_FINAL_LOOK_TOWARD].m_uiName =
        "{Compatibility}_/Camera/Modify/Move/ Final camera look toward point.";
    m_actionTemplates[ScriptAction::CAMERA_MOD_FINAL_LOOK_TOWARD].m_numParameters = 1;
    m_actionTemplates[ScriptAction::CAMERA_MOD_FINAL_LOOK_TOWARD].m_parameters[0] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::CAMERA_MOD_FINAL_LOOK_TOWARD].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::CAMERA_MOD_FINAL_LOOK_TOWARD].m_uiStrings[0] = "Look toward";
    m_actionTemplates[ScriptAction::CAMERA_MOD_FINAL_LOOK_TOWARD].m_uiStrings[1] = " at the end of the camera movement.";

    m_actionTemplates[ScriptAction::CAMERA_MOD_LOOK_TOWARD].m_internalName = "CAMERA_MOD_LOOK_TOWARD";
    m_actionTemplates[ScriptAction::CAMERA_MOD_LOOK_TOWARD].m_uiName =
        "Camera_/Modify/Move/Camera look toward point while moving.";
    m_actionTemplates[ScriptAction::CAMERA_MOD_LOOK_TOWARD].m_numParameters = 1;
    m_actionTemplates[ScriptAction::CAMERA_MOD_LOOK_TOWARD].m_parameters[0] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::CAMERA_MOD_LOOK_TOWARD].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::CAMERA_MOD_LOOK_TOWARD].m_uiStrings[0] = "Look toward";
    m_actionTemplates[ScriptAction::CAMERA_MOD_LOOK_TOWARD].m_uiStrings[1] = " during the camera movement.";

    m_actionTemplates[ScriptAction::CREATE_OBJECT].m_internalName = "CREATE_OBJECT";
    m_actionTemplates[ScriptAction::CREATE_OBJECT].m_uiName = "Unit_/Spawn/Spawn object.";
    m_actionTemplates[ScriptAction::CREATE_OBJECT].m_numParameters = 4;
    m_actionTemplates[ScriptAction::CREATE_OBJECT].m_parameters[0] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::CREATE_OBJECT].m_parameters[1] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::CREATE_OBJECT].m_parameters[2] = Parameter::COORD3D;
    m_actionTemplates[ScriptAction::CREATE_OBJECT].m_parameters[3] = Parameter::ANGLE;
    m_actionTemplates[ScriptAction::CREATE_OBJECT].m_numUiStrings = 5;
    m_actionTemplates[ScriptAction::CREATE_OBJECT].m_uiStrings[0] = "Spawn object ";
    m_actionTemplates[ScriptAction::CREATE_OBJECT].m_uiStrings[1] = " in ";
    m_actionTemplates[ScriptAction::CREATE_OBJECT].m_uiStrings[2] = " at position (";
    m_actionTemplates[ScriptAction::CREATE_OBJECT].m_uiStrings[3] = "), rotated ";
    m_actionTemplates[ScriptAction::CREATE_OBJECT].m_uiStrings[4] = " .";

    m_actionTemplates[ScriptAction::TEAM_ATTACK_TEAM].m_internalName = "TEAM_ATTACK_TEAM";
    m_actionTemplates[ScriptAction::TEAM_ATTACK_TEAM].m_uiName = "Team_/Attack/Set to attack -- another team.";
    m_actionTemplates[ScriptAction::TEAM_ATTACK_TEAM].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_ATTACK_TEAM].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_ATTACK_TEAM].m_parameters[1] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_ATTACK_TEAM].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_ATTACK_TEAM].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_ATTACK_TEAM].m_uiStrings[1] = " begin attack on ";

    m_actionTemplates[ScriptAction::NAMED_ATTACK_NAMED].m_internalName = "NAMED_ATTACK_NAMED";
    m_actionTemplates[ScriptAction::NAMED_ATTACK_NAMED].m_uiName = "Unit_/Attack/Set unit to attack another unit.";
    m_actionTemplates[ScriptAction::NAMED_ATTACK_NAMED].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_ATTACK_NAMED].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_ATTACK_NAMED].m_parameters[1] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_ATTACK_NAMED].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_ATTACK_NAMED].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_ATTACK_NAMED].m_uiStrings[1] = " begin attack on ";

    m_actionTemplates[ScriptAction::CREATE_NAMED_ON_TEAM_AT_WAYPOINT].m_internalName = "CREATE_NAMED_ON_TEAM_AT_WAYPOINT";
    m_actionTemplates[ScriptAction::CREATE_NAMED_ON_TEAM_AT_WAYPOINT].m_uiName =
        "Unit_/Spawn/Spawn -- named unit on a team at a waypoint.";
    m_actionTemplates[ScriptAction::CREATE_NAMED_ON_TEAM_AT_WAYPOINT].m_numParameters = 4;
    m_actionTemplates[ScriptAction::CREATE_NAMED_ON_TEAM_AT_WAYPOINT].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::CREATE_NAMED_ON_TEAM_AT_WAYPOINT].m_parameters[1] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::CREATE_NAMED_ON_TEAM_AT_WAYPOINT].m_parameters[2] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::CREATE_NAMED_ON_TEAM_AT_WAYPOINT].m_parameters[3] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::CREATE_NAMED_ON_TEAM_AT_WAYPOINT].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::CREATE_NAMED_ON_TEAM_AT_WAYPOINT].m_uiStrings[0] = "Spawn ";
    m_actionTemplates[ScriptAction::CREATE_NAMED_ON_TEAM_AT_WAYPOINT].m_uiStrings[1] = " of type ";
    m_actionTemplates[ScriptAction::CREATE_NAMED_ON_TEAM_AT_WAYPOINT].m_uiStrings[2] = " on ";
    m_actionTemplates[ScriptAction::CREATE_NAMED_ON_TEAM_AT_WAYPOINT].m_uiStrings[3] = " at waypoint ";

    m_actionTemplates[ScriptAction::CREATE_UNNAMED_ON_TEAM_AT_WAYPOINT].m_internalName =
        "CREATE_UNNAMED_ON_TEAM_AT_WAYPOINT";
    m_actionTemplates[ScriptAction::CREATE_UNNAMED_ON_TEAM_AT_WAYPOINT].m_uiName =
        "Unit_/Spawn/Spawn -- unnamed unit on a team at a waypoint.";
    m_actionTemplates[ScriptAction::CREATE_UNNAMED_ON_TEAM_AT_WAYPOINT].m_numParameters = 3;
    m_actionTemplates[ScriptAction::CREATE_UNNAMED_ON_TEAM_AT_WAYPOINT].m_parameters[0] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::CREATE_UNNAMED_ON_TEAM_AT_WAYPOINT].m_parameters[1] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::CREATE_UNNAMED_ON_TEAM_AT_WAYPOINT].m_parameters[2] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::CREATE_UNNAMED_ON_TEAM_AT_WAYPOINT].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::CREATE_UNNAMED_ON_TEAM_AT_WAYPOINT].m_uiStrings[0] = "Spawn unit of type ";
    m_actionTemplates[ScriptAction::CREATE_UNNAMED_ON_TEAM_AT_WAYPOINT].m_uiStrings[1] = " on ";
    m_actionTemplates[ScriptAction::CREATE_UNNAMED_ON_TEAM_AT_WAYPOINT].m_uiStrings[2] = " at waypoint ";

    m_actionTemplates[ScriptAction::NAMED_APPLY_ATTACK_PRIORITY_SET].m_internalName = "NAMED_APPLY_ATTACK_PRIORITY_SET";
    m_actionTemplates[ScriptAction::NAMED_APPLY_ATTACK_PRIORITY_SET].m_uiName =
        "AttackPrioritySet_/Apply/Unit/Apply unit's attack priority set.";
    m_actionTemplates[ScriptAction::NAMED_APPLY_ATTACK_PRIORITY_SET].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_APPLY_ATTACK_PRIORITY_SET].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_APPLY_ATTACK_PRIORITY_SET].m_parameters[1] = Parameter::ATTACK_PRIORITY_SET;
    m_actionTemplates[ScriptAction::NAMED_APPLY_ATTACK_PRIORITY_SET].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::NAMED_APPLY_ATTACK_PRIORITY_SET].m_uiStrings[0] = "Have ";
    m_actionTemplates[ScriptAction::NAMED_APPLY_ATTACK_PRIORITY_SET].m_uiStrings[1] = " use ";
    m_actionTemplates[ScriptAction::NAMED_APPLY_ATTACK_PRIORITY_SET].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::TEAM_APPLY_ATTACK_PRIORITY_SET].m_internalName = "TEAM_APPLY_ATTACK_PRIORITY_SET";
    m_actionTemplates[ScriptAction::TEAM_APPLY_ATTACK_PRIORITY_SET].m_uiName =
        "AttackPrioritySet_/Apply/Team/Apply a team's attack priority set.";
    m_actionTemplates[ScriptAction::TEAM_APPLY_ATTACK_PRIORITY_SET].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_APPLY_ATTACK_PRIORITY_SET].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_APPLY_ATTACK_PRIORITY_SET].m_parameters[1] = Parameter::ATTACK_PRIORITY_SET;
    m_actionTemplates[ScriptAction::TEAM_APPLY_ATTACK_PRIORITY_SET].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_APPLY_ATTACK_PRIORITY_SET].m_uiStrings[0] = "Have ";
    m_actionTemplates[ScriptAction::TEAM_APPLY_ATTACK_PRIORITY_SET].m_uiStrings[1] = " use ";
    m_actionTemplates[ScriptAction::TEAM_APPLY_ATTACK_PRIORITY_SET].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_THING].m_internalName = "SET_ATTACK_PRIORITY_THING";
    m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_THING].m_uiName =
        "AttackPrioritySet_/Set/Modify a set's priority for a single unit type.";
    m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_THING].m_numParameters = 3;
    m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_THING].m_parameters[0] = Parameter::ATTACK_PRIORITY_SET;
    m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_THING].m_parameters[1] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_THING].m_parameters[2] = Parameter::INT;
    m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_THING].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_THING].m_uiStrings[0] = "For ";
    m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_THING].m_uiStrings[1] = " set the priority of object type ";
    m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_THING].m_uiStrings[2] = " to ";

    m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_KIND_OF].m_internalName = "SET_ATTACK_PRIORITY_KIND_OF";
    m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_KIND_OF].m_uiName =
        "AttackPrioritySet_/Set/Modify a set's priorities for all of a kind.";
    m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_KIND_OF].m_numParameters = 3;
    m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_KIND_OF].m_parameters[0] = Parameter::ATTACK_PRIORITY_SET;
    m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_KIND_OF].m_parameters[1] = Parameter::KIND_OF_PARAM;
    m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_KIND_OF].m_parameters[2] = Parameter::INT;
    m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_KIND_OF].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_KIND_OF].m_uiStrings[0] = "For ";
    m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_KIND_OF].m_uiStrings[1] = " set the priority of object type ";
    m_actionTemplates[ScriptAction::SET_ATTACK_PRIORITY_KIND_OF].m_uiStrings[2] = " to ";

    m_actionTemplates[ScriptAction::SET_DEFAULT_ATTACK_PRIORITY].m_internalName = "SET_DEFAULT_ATTACK_PRIORITY";
    m_actionTemplates[ScriptAction::SET_DEFAULT_ATTACK_PRIORITY].m_uiName =
        "AttackPrioritySet_/Set/Specify the set's default priority.";
    m_actionTemplates[ScriptAction::SET_DEFAULT_ATTACK_PRIORITY].m_numParameters = 2;
    m_actionTemplates[ScriptAction::SET_DEFAULT_ATTACK_PRIORITY].m_parameters[0] = Parameter::ATTACK_PRIORITY_SET;
    m_actionTemplates[ScriptAction::SET_DEFAULT_ATTACK_PRIORITY].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::SET_DEFAULT_ATTACK_PRIORITY].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::SET_DEFAULT_ATTACK_PRIORITY].m_uiStrings[0] = "For ";
    m_actionTemplates[ScriptAction::SET_DEFAULT_ATTACK_PRIORITY].m_uiStrings[1] = " set the default priority to ";

    m_actionTemplates[ScriptAction::PLAYER_ADD_SKILLPOINTS].m_internalName = "PLAYER_ADD_SKILLPOINTS";
    m_actionTemplates[ScriptAction::PLAYER_ADD_SKILLPOINTS].m_uiName = "Player_/Experience/Add or Subtract Skill Points.";
    m_actionTemplates[ScriptAction::PLAYER_ADD_SKILLPOINTS].m_numParameters = 2;
    m_actionTemplates[ScriptAction::PLAYER_ADD_SKILLPOINTS].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_ADD_SKILLPOINTS].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::PLAYER_ADD_SKILLPOINTS].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::PLAYER_ADD_SKILLPOINTS].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_ADD_SKILLPOINTS].m_uiStrings[1] = " is given ";
    m_actionTemplates[ScriptAction::PLAYER_ADD_SKILLPOINTS].m_uiStrings[2] = " Skill Points.";

    m_actionTemplates[ScriptAction::PLAYER_ADD_RANKLEVEL].m_internalName = "PLAYER_ADD_RANKLEVEL";
    m_actionTemplates[ScriptAction::PLAYER_ADD_RANKLEVEL].m_uiName = "Player_/Experience/Add or Subtract Rank Levels.";
    m_actionTemplates[ScriptAction::PLAYER_ADD_RANKLEVEL].m_numParameters = 2;
    m_actionTemplates[ScriptAction::PLAYER_ADD_RANKLEVEL].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_ADD_RANKLEVEL].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::PLAYER_ADD_RANKLEVEL].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::PLAYER_ADD_RANKLEVEL].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_ADD_RANKLEVEL].m_uiStrings[1] = " is given ";
    m_actionTemplates[ScriptAction::PLAYER_ADD_RANKLEVEL].m_uiStrings[2] = " Rank Levels.";

    m_actionTemplates[ScriptAction::PLAYER_SET_RANKLEVEL].m_internalName = "PLAYER_SET_RANKLEVEL";
    m_actionTemplates[ScriptAction::PLAYER_SET_RANKLEVEL].m_uiName = "Player_/Experience/Set Rank Level.";
    m_actionTemplates[ScriptAction::PLAYER_SET_RANKLEVEL].m_numParameters = 2;
    m_actionTemplates[ScriptAction::PLAYER_SET_RANKLEVEL].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_SET_RANKLEVEL].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::PLAYER_SET_RANKLEVEL].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::PLAYER_SET_RANKLEVEL].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_SET_RANKLEVEL].m_uiStrings[1] = " is given a Rank Level of ";
    m_actionTemplates[ScriptAction::PLAYER_SET_RANKLEVEL].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::PLAYER_SET_RANKLEVELLIMIT].m_internalName = "PLAYER_SET_RANKLEVELLIMIT";
    m_actionTemplates[ScriptAction::PLAYER_SET_RANKLEVELLIMIT].m_uiName =
        "Map_/Experience/Set Rank Level Limit for current Map.";
    m_actionTemplates[ScriptAction::PLAYER_SET_RANKLEVELLIMIT].m_numParameters = 1;
    m_actionTemplates[ScriptAction::PLAYER_SET_RANKLEVELLIMIT].m_parameters[0] = Parameter::INT;
    m_actionTemplates[ScriptAction::PLAYER_SET_RANKLEVELLIMIT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::PLAYER_SET_RANKLEVELLIMIT].m_uiStrings[0] =
        "The current map is given a Rank Level Limit of ";
    m_actionTemplates[ScriptAction::PLAYER_SET_RANKLEVELLIMIT].m_uiStrings[1] = ".";

    m_actionTemplates[ScriptAction::PLAYER_GRANT_SCIENCE].m_internalName = "PLAYER_GRANT_SCIENCE";
    m_actionTemplates[ScriptAction::PLAYER_GRANT_SCIENCE].m_uiName =
        "Player_/Science/Grant a Science to a given Player (ignoring prerequisites).";
    m_actionTemplates[ScriptAction::PLAYER_GRANT_SCIENCE].m_numParameters = 2;
    m_actionTemplates[ScriptAction::PLAYER_GRANT_SCIENCE].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_GRANT_SCIENCE].m_parameters[1] = Parameter::SCIENCE;
    m_actionTemplates[ScriptAction::PLAYER_GRANT_SCIENCE].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::PLAYER_GRANT_SCIENCE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_GRANT_SCIENCE].m_uiStrings[1] = " is granted ";
    m_actionTemplates[ScriptAction::PLAYER_GRANT_SCIENCE].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::PLAYER_PURCHASE_SCIENCE].m_internalName = "PLAYER_PURCHASE_SCIENCE";
    m_actionTemplates[ScriptAction::PLAYER_PURCHASE_SCIENCE].m_uiName =
        "Player_/Science/Player attempts to purchase a Science.";
    m_actionTemplates[ScriptAction::PLAYER_PURCHASE_SCIENCE].m_numParameters = 2;
    m_actionTemplates[ScriptAction::PLAYER_PURCHASE_SCIENCE].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_PURCHASE_SCIENCE].m_parameters[1] = Parameter::SCIENCE;
    m_actionTemplates[ScriptAction::PLAYER_PURCHASE_SCIENCE].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::PLAYER_PURCHASE_SCIENCE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_PURCHASE_SCIENCE].m_uiStrings[1] = " attempts to purchase Science ";
    m_actionTemplates[ScriptAction::PLAYER_PURCHASE_SCIENCE].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::PLAYER_SCIENCE_AVAILABILITY].m_internalName = "PLAYER_SCIENCE_AVAILABILITY";
    m_actionTemplates[ScriptAction::PLAYER_SCIENCE_AVAILABILITY].m_uiName = "Player_/Science/Set science availability.";
    m_actionTemplates[ScriptAction::PLAYER_SCIENCE_AVAILABILITY].m_numParameters = 3;
    m_actionTemplates[ScriptAction::PLAYER_SCIENCE_AVAILABILITY].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_SCIENCE_AVAILABILITY].m_parameters[1] = Parameter::SCIENCE;
    m_actionTemplates[ScriptAction::PLAYER_SCIENCE_AVAILABILITY].m_parameters[2] = Parameter::SCIENCE_AVAILABILITY;
    m_actionTemplates[ScriptAction::PLAYER_SCIENCE_AVAILABILITY].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::PLAYER_SCIENCE_AVAILABILITY].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_SCIENCE_AVAILABILITY].m_uiStrings[1] = " set ";
    m_actionTemplates[ScriptAction::PLAYER_SCIENCE_AVAILABILITY].m_uiStrings[2] = " availability to ";
    m_actionTemplates[ScriptAction::PLAYER_SCIENCE_AVAILABILITY].m_uiStrings[3] = ".";

    m_actionTemplates[ScriptAction::SET_BASE_CONSTRUCTION_SPEED].m_internalName = "SET_BASE_CONSTRUCTION_SPEED";
    m_actionTemplates[ScriptAction::SET_BASE_CONSTRUCTION_SPEED].m_uiName =
        "Player_/AI/Set the delay between building teams.";
    m_actionTemplates[ScriptAction::SET_BASE_CONSTRUCTION_SPEED].m_numParameters = 2;
    m_actionTemplates[ScriptAction::SET_BASE_CONSTRUCTION_SPEED].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::SET_BASE_CONSTRUCTION_SPEED].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::SET_BASE_CONSTRUCTION_SPEED].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::SET_BASE_CONSTRUCTION_SPEED].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::SET_BASE_CONSTRUCTION_SPEED].m_uiStrings[1] = " will delay ";
    // BUGFIX
    m_actionTemplates[ScriptAction::SET_BASE_CONSTRUCTION_SPEED].m_uiStrings[2] = " seconds between building teams.";

    m_actionTemplates[ScriptAction::NAMED_SET_ATTITUDE].m_internalName = "NAMED_SET_ATTITUDE";
    m_actionTemplates[ScriptAction::NAMED_SET_ATTITUDE].m_uiName = "Unit_/Mood/Set the general attitude of a specific unit.";
    m_actionTemplates[ScriptAction::NAMED_SET_ATTITUDE].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_SET_ATTITUDE].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_SET_ATTITUDE].m_parameters[1] = Parameter::AI_MOOD;
    m_actionTemplates[ScriptAction::NAMED_SET_ATTITUDE].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_SET_ATTITUDE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_SET_ATTITUDE].m_uiStrings[1] = " changes his attitude to ";

    m_actionTemplates[ScriptAction::TEAM_SET_ATTITUDE].m_internalName = "TEAM_SET_ATTITUDE";
    m_actionTemplates[ScriptAction::TEAM_SET_ATTITUDE].m_uiName = "Team_/Mood/Set the general attitude of a team.";
    m_actionTemplates[ScriptAction::TEAM_SET_ATTITUDE].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_SET_ATTITUDE].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_SET_ATTITUDE].m_parameters[1] = Parameter::AI_MOOD;
    m_actionTemplates[ScriptAction::TEAM_SET_ATTITUDE].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_SET_ATTITUDE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_SET_ATTITUDE].m_uiStrings[1] = " change their attitude to ";

    m_actionTemplates[ScriptAction::NAMED_SET_REPULSOR].m_internalName = "NAMED_SET_REPULSOR";
    m_actionTemplates[ScriptAction::NAMED_SET_REPULSOR].m_uiName =
        "Unit_/Internal/Repulsor/Set the REPULSOR flag of a specific unit.";
    m_actionTemplates[ScriptAction::NAMED_SET_REPULSOR].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_SET_REPULSOR].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_SET_REPULSOR].m_parameters[1] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::NAMED_SET_REPULSOR].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_SET_REPULSOR].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_SET_REPULSOR].m_uiStrings[1] = " REPULSOR flag is ";

    m_actionTemplates[ScriptAction::TEAM_SET_REPULSOR].m_internalName = "TEAM_SET_REPULSOR";
    m_actionTemplates[ScriptAction::TEAM_SET_REPULSOR].m_uiName = "Team_/Internal/Repulsor/Set the REPULSOR flag of a team.";
    m_actionTemplates[ScriptAction::TEAM_SET_REPULSOR].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_SET_REPULSOR].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_SET_REPULSOR].m_parameters[1] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::TEAM_SET_REPULSOR].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_SET_REPULSOR].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_SET_REPULSOR].m_uiStrings[1] = " REPULSOR flag is ";

    m_actionTemplates[ScriptAction::NAMED_ATTACK_AREA].m_internalName = "NAMED_ATTACK_AREA";
    m_actionTemplates[ScriptAction::NAMED_ATTACK_AREA].m_uiName =
        "Unit_/Attack/Set a specific unit to attack a specific trigger area.";
    m_actionTemplates[ScriptAction::NAMED_ATTACK_AREA].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_ATTACK_AREA].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_ATTACK_AREA].m_parameters[1] = Parameter::TRIGGER_AREA;
    m_actionTemplates[ScriptAction::NAMED_ATTACK_AREA].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_ATTACK_AREA].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_ATTACK_AREA].m_uiStrings[1] = " attacks anything in ";

    m_actionTemplates[ScriptAction::NAMED_ATTACK_TEAM].m_internalName = "NAMED_ATTACK_TEAM";
    m_actionTemplates[ScriptAction::NAMED_ATTACK_TEAM].m_uiName = "Unit_/Attack/Set a specific unit to attack a team.";
    m_actionTemplates[ScriptAction::NAMED_ATTACK_TEAM].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_ATTACK_TEAM].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_ATTACK_TEAM].m_parameters[1] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::NAMED_ATTACK_TEAM].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_ATTACK_TEAM].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_ATTACK_TEAM].m_uiStrings[1] = " attacks ";

    m_actionTemplates[ScriptAction::TEAM_ATTACK_AREA].m_internalName = "TEAM_ATTACK_AREA";
    m_actionTemplates[ScriptAction::TEAM_ATTACK_AREA].m_uiName = "Team_/Attack/Set to attack -- trigger area.";
    m_actionTemplates[ScriptAction::TEAM_ATTACK_AREA].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_ATTACK_AREA].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_ATTACK_AREA].m_parameters[1] = Parameter::TRIGGER_AREA;
    m_actionTemplates[ScriptAction::TEAM_ATTACK_AREA].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_ATTACK_AREA].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_ATTACK_AREA].m_uiStrings[1] = " attack anything in ";

    m_actionTemplates[ScriptAction::TEAM_ATTACK_NAMED].m_internalName = "TEAM_ATTACK_NAMED";
    m_actionTemplates[ScriptAction::TEAM_ATTACK_NAMED].m_uiName = "Team_/Attack/Set to attack -- specific unit.";
    m_actionTemplates[ScriptAction::TEAM_ATTACK_NAMED].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_ATTACK_NAMED].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_ATTACK_NAMED].m_parameters[1] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::TEAM_ATTACK_NAMED].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_ATTACK_NAMED].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_ATTACK_NAMED].m_uiStrings[1] = " attacks ";

    m_actionTemplates[ScriptAction::TEAM_LOAD_TRANSPORTS].m_internalName = "TEAM_LOAD_TRANSPORTS";
    m_actionTemplates[ScriptAction::TEAM_LOAD_TRANSPORTS].m_uiName = "Team_/Transport/Transport -- automatically load.";
    m_actionTemplates[ScriptAction::TEAM_LOAD_TRANSPORTS].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_LOAD_TRANSPORTS].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_LOAD_TRANSPORTS].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_LOAD_TRANSPORTS].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_LOAD_TRANSPORTS].m_uiStrings[1] = " load into transports.";

    m_actionTemplates[ScriptAction::NAMED_ENTER_NAMED].m_internalName = "NAMED_ENTER_NAMED";
    m_actionTemplates[ScriptAction::NAMED_ENTER_NAMED].m_uiName = "Unit_/Transport/Transport -- load unit into specific.";
    m_actionTemplates[ScriptAction::NAMED_ENTER_NAMED].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_ENTER_NAMED].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_ENTER_NAMED].m_parameters[1] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_ENTER_NAMED].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_ENTER_NAMED].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_ENTER_NAMED].m_uiStrings[1] = " loads into ";

    m_actionTemplates[ScriptAction::TEAM_ENTER_NAMED].m_internalName = "TEAM_ENTER_NAMED";
    m_actionTemplates[ScriptAction::TEAM_ENTER_NAMED].m_uiName = "Team_/Transport/Transport -- load team into specific.";
    m_actionTemplates[ScriptAction::TEAM_ENTER_NAMED].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_ENTER_NAMED].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_ENTER_NAMED].m_parameters[1] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::TEAM_ENTER_NAMED].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_ENTER_NAMED].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_ENTER_NAMED].m_uiStrings[1] = " attempt to load into ";

    m_actionTemplates[ScriptAction::NAMED_EXIT_ALL].m_internalName = "NAMED_EXIT_ALL";
    m_actionTemplates[ScriptAction::NAMED_EXIT_ALL].m_uiName = "Unit_/Transport/Transport -- unload units from specific.";
    m_actionTemplates[ScriptAction::NAMED_EXIT_ALL].m_numParameters = 1;
    m_actionTemplates[ScriptAction::NAMED_EXIT_ALL].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_EXIT_ALL].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_EXIT_ALL].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_EXIT_ALL].m_uiStrings[1] = " unloads.";

    m_actionTemplates[ScriptAction::TEAM_EXIT_ALL].m_internalName = "TEAM_EXIT_ALL";
    m_actionTemplates[ScriptAction::TEAM_EXIT_ALL].m_uiName = "Team_/Transport/Transport -- unload team from all.";
    m_actionTemplates[ScriptAction::TEAM_EXIT_ALL].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_EXIT_ALL].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_EXIT_ALL].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_EXIT_ALL].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_EXIT_ALL].m_uiStrings[1] = " unload.";

    m_actionTemplates[ScriptAction::NAMED_FOLLOW_WAYPOINTS].m_internalName = "NAMED_FOLLOW_WAYPOINTS";
    m_actionTemplates[ScriptAction::NAMED_FOLLOW_WAYPOINTS].m_uiName =
        "Unit_/Move/Set a specific unit to follow a waypoint path.";
    m_actionTemplates[ScriptAction::NAMED_FOLLOW_WAYPOINTS].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_FOLLOW_WAYPOINTS].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_FOLLOW_WAYPOINTS].m_parameters[1] = Parameter::WAYPOINT_PATH;
    m_actionTemplates[ScriptAction::NAMED_FOLLOW_WAYPOINTS].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_FOLLOW_WAYPOINTS].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_FOLLOW_WAYPOINTS].m_uiStrings[1] = " follows waypoints, beginning at ";

    m_actionTemplates[ScriptAction::NAMED_FOLLOW_WAYPOINTS_EXACT].m_internalName = "NAMED_FOLLOW_WAYPOINTS_EXACT";
    m_actionTemplates[ScriptAction::NAMED_FOLLOW_WAYPOINTS_EXACT].m_uiName =
        "Unit_/Move/Set a specific unit to EXACTLY follow a waypoint path.";
    m_actionTemplates[ScriptAction::NAMED_FOLLOW_WAYPOINTS_EXACT].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_FOLLOW_WAYPOINTS_EXACT].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_FOLLOW_WAYPOINTS_EXACT].m_parameters[1] = Parameter::WAYPOINT_PATH;
    m_actionTemplates[ScriptAction::NAMED_FOLLOW_WAYPOINTS_EXACT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_FOLLOW_WAYPOINTS_EXACT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_FOLLOW_WAYPOINTS_EXACT].m_uiStrings[1] =
        " EXACTLY follows waypoints, beginning at ";

    m_actionTemplates[ScriptAction::NAMED_GUARD].m_internalName = "NAMED_GUARD";
    m_actionTemplates[ScriptAction::NAMED_GUARD].m_uiName = "Unit_/Move/Set to guard.";
    m_actionTemplates[ScriptAction::NAMED_GUARD].m_numParameters = 1;
    m_actionTemplates[ScriptAction::NAMED_GUARD].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_GUARD].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_GUARD].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_GUARD].m_uiStrings[1] = " begins guarding.";

    m_actionTemplates[ScriptAction::TEAM_GUARD].m_internalName = "TEAM_GUARD";
    m_actionTemplates[ScriptAction::TEAM_GUARD].m_uiName = "Team_/Guard/Set to guard -- current location.";
    m_actionTemplates[ScriptAction::TEAM_GUARD].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_GUARD].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_GUARD].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_GUARD].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_GUARD].m_uiStrings[1] = " begins guarding.";

    m_actionTemplates[ScriptAction::TEAM_GUARD_POSITION].m_internalName = "TEAM_GUARD_POSITION";
    m_actionTemplates[ScriptAction::TEAM_GUARD_POSITION].m_uiName = "Team_/Guard/Set to guard -- location.";
    m_actionTemplates[ScriptAction::TEAM_GUARD_POSITION].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_GUARD_POSITION].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_GUARD_POSITION].m_parameters[1] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::TEAM_GUARD_POSITION].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_GUARD_POSITION].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_GUARD_POSITION].m_uiStrings[1] = " begins guarding at ";

    m_actionTemplates[ScriptAction::TEAM_GUARD_OBJECT].m_internalName = "TEAM_GUARD_OBJECT";
    m_actionTemplates[ScriptAction::TEAM_GUARD_OBJECT].m_uiName = "Team_/Guard/Set to guard -- specific unit.";
    m_actionTemplates[ScriptAction::TEAM_GUARD_OBJECT].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_GUARD_OBJECT].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_GUARD_OBJECT].m_parameters[1] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::TEAM_GUARD_OBJECT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_GUARD_OBJECT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_GUARD_OBJECT].m_uiStrings[1] = " begins guarding ";

    m_actionTemplates[ScriptAction::TEAM_GUARD_AREA].m_internalName = "TEAM_GUARD_AREA";
    m_actionTemplates[ScriptAction::TEAM_GUARD_AREA].m_uiName = "Team_/Guard/Set to guard -- area.";
    m_actionTemplates[ScriptAction::TEAM_GUARD_AREA].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_GUARD_AREA].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_GUARD_AREA].m_parameters[1] = Parameter::TRIGGER_AREA;
    m_actionTemplates[ScriptAction::TEAM_GUARD_AREA].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_GUARD_AREA].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_GUARD_AREA].m_uiStrings[1] = " begins guarding ";

    m_actionTemplates[ScriptAction::NAMED_HUNT].m_internalName = "NAMED_HUNT";
    m_actionTemplates[ScriptAction::NAMED_HUNT].m_uiName = "Unit_/Hunt/Set a specific unit to hunt.";
    m_actionTemplates[ScriptAction::NAMED_HUNT].m_numParameters = 1;
    m_actionTemplates[ScriptAction::NAMED_HUNT].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_HUNT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_HUNT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_HUNT].m_uiStrings[1] = " begins hunting.";

    m_actionTemplates[ScriptAction::TEAM_HUNT_WITH_COMMAND_BUTTON].m_internalName = "TEAM_HUNT_WITH_COMMAND_BUTTON";
    m_actionTemplates[ScriptAction::TEAM_HUNT_WITH_COMMAND_BUTTON].m_uiName =
        "Team_/Hunt/Set to hunt using commandbutton ability.";
    m_actionTemplates[ScriptAction::TEAM_HUNT_WITH_COMMAND_BUTTON].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_HUNT_WITH_COMMAND_BUTTON].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_HUNT_WITH_COMMAND_BUTTON].m_parameters[1] = Parameter::TEAM_COMMANDBUTTON_ABILITY;
    m_actionTemplates[ScriptAction::TEAM_HUNT_WITH_COMMAND_BUTTON].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_HUNT_WITH_COMMAND_BUTTON].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_HUNT_WITH_COMMAND_BUTTON].m_uiStrings[1] = " begins hunting using ";
    m_actionTemplates[ScriptAction::TEAM_HUNT_WITH_COMMAND_BUTTON].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::TEAM_HUNT].m_internalName = "TEAM_HUNT";
    m_actionTemplates[ScriptAction::TEAM_HUNT].m_uiName = "Team_/Hunt/Set to hunt.";
    m_actionTemplates[ScriptAction::TEAM_HUNT].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_HUNT].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_HUNT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_HUNT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_HUNT].m_uiStrings[1] = " begins hunting.";

    m_actionTemplates[ScriptAction::PLAYER_HUNT].m_internalName = "PLAYER_HUNT";
    m_actionTemplates[ScriptAction::PLAYER_HUNT].m_uiName = "Player_/Hunt/Set all of a player's units to hunt.";
    m_actionTemplates[ScriptAction::PLAYER_HUNT].m_numParameters = 1;
    m_actionTemplates[ScriptAction::PLAYER_HUNT].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_HUNT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::PLAYER_HUNT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_HUNT].m_uiStrings[1] = " begins hunting.";

    m_actionTemplates[ScriptAction::PLAYER_SELL_EVERYTHING].m_internalName = "PLAYER_SELL_EVERYTHING";
    m_actionTemplates[ScriptAction::PLAYER_SELL_EVERYTHING].m_uiName = "Player_/Set/Set a player to sell everything.";
    m_actionTemplates[ScriptAction::PLAYER_SELL_EVERYTHING].m_numParameters = 1;
    m_actionTemplates[ScriptAction::PLAYER_SELL_EVERYTHING].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_SELL_EVERYTHING].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::PLAYER_SELL_EVERYTHING].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_SELL_EVERYTHING].m_uiStrings[1] = " sells everything.";

    m_actionTemplates[ScriptAction::PLAYER_DISABLE_BASE_CONSTRUCTION].m_internalName = "PLAYER_DISABLE_BASE_CONSTRUCTION";
    m_actionTemplates[ScriptAction::PLAYER_DISABLE_BASE_CONSTRUCTION].m_uiName =
        "Player_/Build/Set a player to be unable to build buildings.";
    m_actionTemplates[ScriptAction::PLAYER_DISABLE_BASE_CONSTRUCTION].m_numParameters = 1;
    m_actionTemplates[ScriptAction::PLAYER_DISABLE_BASE_CONSTRUCTION].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_DISABLE_BASE_CONSTRUCTION].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::PLAYER_DISABLE_BASE_CONSTRUCTION].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_DISABLE_BASE_CONSTRUCTION].m_uiStrings[1] = " is unable to build buildings.";

    m_actionTemplates[ScriptAction::PLAYER_DISABLE_FACTORIES].m_internalName = "PLAYER_DISABLE_FACTORIES";
    m_actionTemplates[ScriptAction::PLAYER_DISABLE_FACTORIES].m_uiName =
        "Player_/Build/Set a player to be unable to build from a specific building.";
    m_actionTemplates[ScriptAction::PLAYER_DISABLE_FACTORIES].m_numParameters = 2;
    m_actionTemplates[ScriptAction::PLAYER_DISABLE_FACTORIES].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_DISABLE_FACTORIES].m_parameters[1] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::PLAYER_DISABLE_FACTORIES].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::PLAYER_DISABLE_FACTORIES].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_DISABLE_FACTORIES].m_uiStrings[1] = " is unable to build from ";

    m_actionTemplates[ScriptAction::PLAYER_DISABLE_UNIT_CONSTRUCTION].m_internalName = "PLAYER_DISABLE_UNIT_CONSTRUCTION";
    m_actionTemplates[ScriptAction::PLAYER_DISABLE_UNIT_CONSTRUCTION].m_uiName =
        "Player_/Build/Set a player to be unable to build units.";
    m_actionTemplates[ScriptAction::PLAYER_DISABLE_UNIT_CONSTRUCTION].m_numParameters = 1;
    m_actionTemplates[ScriptAction::PLAYER_DISABLE_UNIT_CONSTRUCTION].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_DISABLE_UNIT_CONSTRUCTION].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::PLAYER_DISABLE_UNIT_CONSTRUCTION].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_DISABLE_UNIT_CONSTRUCTION].m_uiStrings[1] = " is unable to build units.";

    m_actionTemplates[ScriptAction::PLAYER_ENABLE_BASE_CONSTRUCTION].m_internalName = "PLAYER_ENABLE_BASE_CONSTRUCTION";
    m_actionTemplates[ScriptAction::PLAYER_ENABLE_BASE_CONSTRUCTION].m_uiName =
        "Player_/Build/Set a player to be able to build buildings.";
    m_actionTemplates[ScriptAction::PLAYER_ENABLE_BASE_CONSTRUCTION].m_numParameters = 1;
    m_actionTemplates[ScriptAction::PLAYER_ENABLE_BASE_CONSTRUCTION].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_ENABLE_BASE_CONSTRUCTION].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::PLAYER_ENABLE_BASE_CONSTRUCTION].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_ENABLE_BASE_CONSTRUCTION].m_uiStrings[1] = " is able to build buildings.";

    m_actionTemplates[ScriptAction::PLAYER_ENABLE_FACTORIES].m_internalName = "PLAYER_ENABLE_FACTORIES";
    m_actionTemplates[ScriptAction::PLAYER_ENABLE_FACTORIES].m_uiName =
        "Player_/Build/Set a player to be able to build from a specific building.";
    m_actionTemplates[ScriptAction::PLAYER_ENABLE_FACTORIES].m_numParameters = 2;
    m_actionTemplates[ScriptAction::PLAYER_ENABLE_FACTORIES].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_ENABLE_FACTORIES].m_parameters[1] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::PLAYER_ENABLE_FACTORIES].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::PLAYER_ENABLE_FACTORIES].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_ENABLE_FACTORIES].m_uiStrings[1] = " is able to build from ";

    m_actionTemplates[ScriptAction::PLAYER_ENABLE_UNIT_CONSTRUCTION].m_internalName = "PLAYER_ENABLE_UNIT_CONSTRUCTION";
    m_actionTemplates[ScriptAction::PLAYER_ENABLE_UNIT_CONSTRUCTION].m_uiName =
        "Player_/Build/Set a player to be able to build units.";
    m_actionTemplates[ScriptAction::PLAYER_ENABLE_UNIT_CONSTRUCTION].m_numParameters = 1;
    m_actionTemplates[ScriptAction::PLAYER_ENABLE_UNIT_CONSTRUCTION].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_ENABLE_UNIT_CONSTRUCTION].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::PLAYER_ENABLE_UNIT_CONSTRUCTION].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_ENABLE_UNIT_CONSTRUCTION].m_uiStrings[1] = " is able to build units.";

    m_actionTemplates[ScriptAction::CAMERA_MOVE_HOME].m_internalName = "CAMERA_MOVE_HOME";
    m_actionTemplates[ScriptAction::CAMERA_MOVE_HOME].m_uiName = "Camera_/Move/Move the camera to the home position.";
    m_actionTemplates[ScriptAction::CAMERA_MOVE_HOME].m_numParameters = 0;
    m_actionTemplates[ScriptAction::CAMERA_MOVE_HOME].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::CAMERA_MOVE_HOME].m_uiStrings[0] = "The camera moves to the home base.";

    m_actionTemplates[ScriptAction::OVERSIZE_TERRAIN].m_internalName = "OVERSIZE_TERRAIN";
    m_actionTemplates[ScriptAction::OVERSIZE_TERRAIN].m_uiName = "Camera_/Terrain/Oversize the terrain.";
    m_actionTemplates[ScriptAction::OVERSIZE_TERRAIN].m_numParameters = 1;
    m_actionTemplates[ScriptAction::OVERSIZE_TERRAIN].m_parameters[0] = Parameter::INT;
    m_actionTemplates[ScriptAction::OVERSIZE_TERRAIN].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::OVERSIZE_TERRAIN].m_uiStrings[0] = "Oversize the terrain ";
    m_actionTemplates[ScriptAction::OVERSIZE_TERRAIN].m_uiStrings[1] = " tiles on each side [0 = reset to normal].";

    m_actionTemplates[ScriptAction::BUILD_TEAM].m_internalName = "BUILD_TEAM";
    m_actionTemplates[ScriptAction::BUILD_TEAM].m_uiName = "Team_/AI/Start building a team.";
    m_actionTemplates[ScriptAction::BUILD_TEAM].m_numParameters = 1;
    m_actionTemplates[ScriptAction::BUILD_TEAM].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::BUILD_TEAM].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::BUILD_TEAM].m_uiStrings[0] = "Start building team ";

    m_actionTemplates[ScriptAction::NAMED_DAMAGE].m_internalName = "NAMED_DAMAGE";
    m_actionTemplates[ScriptAction::NAMED_DAMAGE].m_uiName = "Unit_/Damage/Deal damage to a specific unit.";
    m_actionTemplates[ScriptAction::NAMED_DAMAGE].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_DAMAGE].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_DAMAGE].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::NAMED_DAMAGE].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::NAMED_DAMAGE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_DAMAGE].m_uiStrings[1] = " takes ";
    m_actionTemplates[ScriptAction::NAMED_DAMAGE].m_uiStrings[2] = " points of damage.";

    m_actionTemplates[ScriptAction::NAMED_DELETE].m_internalName = "NAMED_DELETE";
    m_actionTemplates[ScriptAction::NAMED_DELETE].m_uiName = "Unit_/Damage or Remove/Delete a specific unit.";
    m_actionTemplates[ScriptAction::NAMED_DELETE].m_numParameters = 1;
    m_actionTemplates[ScriptAction::NAMED_DELETE].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_DELETE].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_DELETE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_DELETE].m_uiStrings[1] = " is removed from the world.";

    m_actionTemplates[ScriptAction::TEAM_DELETE].m_internalName = "TEAM_DELETE";
    m_actionTemplates[ScriptAction::TEAM_DELETE].m_uiName = "Team_/Damage or Remove/Delete a team.";
    m_actionTemplates[ScriptAction::TEAM_DELETE].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_DELETE].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_DELETE].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_DELETE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_DELETE].m_uiStrings[1] = " is removed from the world.";

    m_actionTemplates[ScriptAction::TEAM_DELETE_LIVING].m_internalName = "TEAM_DELETE_LIVING";
    m_actionTemplates[ScriptAction::TEAM_DELETE_LIVING].m_uiName =
        "Team_/Damage or Remove/Delete a team, but ignore dead guys.";
    m_actionTemplates[ScriptAction::TEAM_DELETE_LIVING].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_DELETE_LIVING].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_DELETE_LIVING].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_DELETE_LIVING].m_uiStrings[0] = "Each living member of team ";
    m_actionTemplates[ScriptAction::TEAM_DELETE_LIVING].m_uiStrings[1] = " is removed from the world.";

    m_actionTemplates[ScriptAction::NAMED_KILL].m_internalName = "NAMED_KILL";
    m_actionTemplates[ScriptAction::NAMED_KILL].m_uiName = "Unit_/Damage or Remove/Kill a specific unit.";
    m_actionTemplates[ScriptAction::NAMED_KILL].m_numParameters = 1;
    m_actionTemplates[ScriptAction::NAMED_KILL].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_KILL].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_KILL].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_KILL].m_uiStrings[1] = "is dealt a lethal amount of damage.";

    m_actionTemplates[ScriptAction::TEAM_KILL].m_internalName = "TEAM_KILL";
    m_actionTemplates[ScriptAction::TEAM_KILL].m_uiName = "Team_/Damage or Remove/Kill an entire team.";
    m_actionTemplates[ScriptAction::TEAM_KILL].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_KILL].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_KILL].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_KILL].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_KILL].m_uiStrings[1] = " is dealt a lethal amount of damage.";

    m_actionTemplates[ScriptAction::PLAYER_KILL].m_internalName = "PLAYER_KILL";
    m_actionTemplates[ScriptAction::PLAYER_KILL].m_uiName = "Player_/Damage or Remove/Kill a player.";
    m_actionTemplates[ScriptAction::PLAYER_KILL].m_numParameters = 1;
    m_actionTemplates[ScriptAction::PLAYER_KILL].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_KILL].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::PLAYER_KILL].m_uiStrings[0] = "All of ";
    m_actionTemplates[ScriptAction::PLAYER_KILL].m_uiStrings[1] =
        "'s buildings and units are dealt a lethal amount of damage.";

    m_actionTemplates[ScriptAction::DISPLAY_TEXT].m_internalName = "DISPLAY_TEXT";
    m_actionTemplates[ScriptAction::DISPLAY_TEXT].m_uiName = "User_/String/Display a string.";
    m_actionTemplates[ScriptAction::DISPLAY_TEXT].m_numParameters = 1;
    m_actionTemplates[ScriptAction::DISPLAY_TEXT].m_parameters[0] = Parameter::LOCALIZED_TEXT;
    m_actionTemplates[ScriptAction::DISPLAY_TEXT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::DISPLAY_TEXT].m_uiStrings[0] = "Displays ";
    m_actionTemplates[ScriptAction::DISPLAY_TEXT].m_uiStrings[1] = " in the text log and message area.";

    m_actionTemplates[ScriptAction::DISPLAY_CINEMATIC_TEXT].m_internalName = "DISPLAY_CINEMATIC_TEXT";
    m_actionTemplates[ScriptAction::DISPLAY_CINEMATIC_TEXT].m_uiName = "User_/String/Display a cinematic string.";
    m_actionTemplates[ScriptAction::DISPLAY_CINEMATIC_TEXT].m_numParameters = 3;
    m_actionTemplates[ScriptAction::DISPLAY_CINEMATIC_TEXT].m_parameters[0] = Parameter::LOCALIZED_TEXT;
    m_actionTemplates[ScriptAction::DISPLAY_CINEMATIC_TEXT].m_parameters[1] = Parameter::FONT;
    m_actionTemplates[ScriptAction::DISPLAY_CINEMATIC_TEXT].m_parameters[2] = Parameter::INT;
    m_actionTemplates[ScriptAction::DISPLAY_CINEMATIC_TEXT].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::DISPLAY_CINEMATIC_TEXT].m_uiStrings[0] = "Displays ";
    m_actionTemplates[ScriptAction::DISPLAY_CINEMATIC_TEXT].m_uiStrings[1] = " with font type ";
    m_actionTemplates[ScriptAction::DISPLAY_CINEMATIC_TEXT].m_uiStrings[2] = " in the bottom letterbox for ";
    m_actionTemplates[ScriptAction::DISPLAY_CINEMATIC_TEXT].m_uiStrings[3] = " seconds.";

    m_actionTemplates[ScriptAction::CAMEO_FLASH].m_internalName = "CAMEO_FLASH";
    m_actionTemplates[ScriptAction::CAMEO_FLASH].m_uiName = "User_/Flash/Flash a cameo for a specified amount of time.";
    m_actionTemplates[ScriptAction::CAMEO_FLASH].m_numParameters = 2;
    m_actionTemplates[ScriptAction::CAMEO_FLASH].m_parameters[0] = Parameter::COMMANDBUTTON;
    m_actionTemplates[ScriptAction::CAMEO_FLASH].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::CAMEO_FLASH].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::CAMEO_FLASH].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::CAMEO_FLASH].m_uiStrings[1] = " flashes for ";
    m_actionTemplates[ScriptAction::CAMEO_FLASH].m_uiStrings[2] = " seconds.";

    m_actionTemplates[ScriptAction::NAMED_FLASH].m_internalName = "NAMED_FLASH";
    m_actionTemplates[ScriptAction::NAMED_FLASH].m_uiName =
        "User_/Flash/Flash a specific unit for a specified amount of time.";
    m_actionTemplates[ScriptAction::NAMED_FLASH].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_FLASH].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_FLASH].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::NAMED_FLASH].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::NAMED_FLASH].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_FLASH].m_uiStrings[1] = " flashes for ";
    m_actionTemplates[ScriptAction::NAMED_FLASH].m_uiStrings[2] = " seconds.";

    m_actionTemplates[ScriptAction::TEAM_FLASH].m_internalName = "TEAM_FLASH";
    m_actionTemplates[ScriptAction::TEAM_FLASH].m_uiName = "User_/Flash/Flash a team for a specified amount of time.";
    m_actionTemplates[ScriptAction::TEAM_FLASH].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_FLASH].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_FLASH].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::TEAM_FLASH].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_FLASH].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_FLASH].m_uiStrings[1] = " flashes for ";
    m_actionTemplates[ScriptAction::TEAM_FLASH].m_uiStrings[2] = " seconds.";

    m_actionTemplates[ScriptAction::NAMED_CUSTOM_COLOR].m_internalName = "NAMED_CUSTOM_COLOR";
    m_actionTemplates[ScriptAction::NAMED_CUSTOM_COLOR].m_uiName =
        "User_/Flash/Set a specific unit to use a special indicator color.";
    m_actionTemplates[ScriptAction::NAMED_CUSTOM_COLOR].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_CUSTOM_COLOR].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_CUSTOM_COLOR].m_parameters[1] = Parameter::COLOR;
    m_actionTemplates[ScriptAction::NAMED_CUSTOM_COLOR].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::NAMED_CUSTOM_COLOR].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_CUSTOM_COLOR].m_uiStrings[1] = " uses the color ";
    m_actionTemplates[ScriptAction::NAMED_CUSTOM_COLOR].m_uiStrings[2] = " .";

    m_actionTemplates[ScriptAction::NAMED_FLASH_WHITE].m_internalName = "NAMED_FLASH_WHITE";
    m_actionTemplates[ScriptAction::NAMED_FLASH_WHITE].m_uiName =
        "User_/Flash/Flash a specific unit white for a specified amount of time.";
    m_actionTemplates[ScriptAction::NAMED_FLASH_WHITE].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_FLASH_WHITE].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_FLASH_WHITE].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::NAMED_FLASH_WHITE].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::NAMED_FLASH_WHITE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_FLASH_WHITE].m_uiStrings[1] = " flashes white for ";
    m_actionTemplates[ScriptAction::NAMED_FLASH_WHITE].m_uiStrings[2] = " seconds.";

    m_actionTemplates[ScriptAction::TEAM_FLASH_WHITE].m_internalName = "TEAM_FLASH_WHITE";
    m_actionTemplates[ScriptAction::TEAM_FLASH_WHITE].m_uiName =
        "User_/Flash/Flash a team white for a specified amount of time.";
    m_actionTemplates[ScriptAction::TEAM_FLASH_WHITE].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_FLASH_WHITE].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_FLASH_WHITE].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::TEAM_FLASH_WHITE].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_FLASH_WHITE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_FLASH_WHITE].m_uiStrings[1] = " flashes white for ";
    m_actionTemplates[ScriptAction::TEAM_FLASH_WHITE].m_uiStrings[2] = " seconds.";

    m_actionTemplates[ScriptAction::INGAME_POPUP_MESSAGE].m_internalName = "INGAME_POPUP_MESSAGE";
    m_actionTemplates[ScriptAction::INGAME_POPUP_MESSAGE].m_uiName = "User_/String/Display Popup Message Box.";
    m_actionTemplates[ScriptAction::INGAME_POPUP_MESSAGE].m_numParameters = 5;
    m_actionTemplates[ScriptAction::INGAME_POPUP_MESSAGE].m_parameters[0] = Parameter::LOCALIZED_TEXT;
    m_actionTemplates[ScriptAction::INGAME_POPUP_MESSAGE].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::INGAME_POPUP_MESSAGE].m_parameters[2] = Parameter::INT;
    m_actionTemplates[ScriptAction::INGAME_POPUP_MESSAGE].m_parameters[3] = Parameter::INT;
    m_actionTemplates[ScriptAction::INGAME_POPUP_MESSAGE].m_parameters[4] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::INGAME_POPUP_MESSAGE].m_numUiStrings = 6;
    m_actionTemplates[ScriptAction::INGAME_POPUP_MESSAGE].m_uiStrings[0] = "Displays ";
    m_actionTemplates[ScriptAction::INGAME_POPUP_MESSAGE].m_uiStrings[1] = " at ";
    m_actionTemplates[ScriptAction::INGAME_POPUP_MESSAGE].m_uiStrings[2] = " percent of the screen Width ";
    m_actionTemplates[ScriptAction::INGAME_POPUP_MESSAGE].m_uiStrings[3] = " percent of the screen Height and a width of ";
    m_actionTemplates[ScriptAction::INGAME_POPUP_MESSAGE].m_uiStrings[4] = " pixels and pauses the game (";
    m_actionTemplates[ScriptAction::INGAME_POPUP_MESSAGE].m_uiStrings[5] = " )";

    m_actionTemplates[ScriptAction::MOVIE_PLAY_FULLSCREEN].m_internalName = "MOVIE_PLAY_FULLSCREEN";
    m_actionTemplates[ScriptAction::MOVIE_PLAY_FULLSCREEN].m_uiName = "Multimedia_/Movie/Play a movie in fullscreen mode.";
    m_actionTemplates[ScriptAction::MOVIE_PLAY_FULLSCREEN].m_numParameters = 1;
    m_actionTemplates[ScriptAction::MOVIE_PLAY_FULLSCREEN].m_parameters[0] = Parameter::MOVIE;
    m_actionTemplates[ScriptAction::MOVIE_PLAY_FULLSCREEN].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::MOVIE_PLAY_FULLSCREEN].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::MOVIE_PLAY_FULLSCREEN].m_uiStrings[1] = " plays fullscreen.";

    m_actionTemplates[ScriptAction::MOVIE_PLAY_RADAR].m_internalName = "MOVIE_PLAY_RADAR";
    m_actionTemplates[ScriptAction::MOVIE_PLAY_RADAR].m_uiName = "Multimedia_/Movie/Play a movie in the radar.";
    m_actionTemplates[ScriptAction::MOVIE_PLAY_RADAR].m_numParameters = 1;
    m_actionTemplates[ScriptAction::MOVIE_PLAY_RADAR].m_parameters[0] = Parameter::MOVIE;
    m_actionTemplates[ScriptAction::MOVIE_PLAY_RADAR].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::MOVIE_PLAY_RADAR].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::MOVIE_PLAY_RADAR].m_uiStrings[1] = " plays in the radar window.";

    m_actionTemplates[ScriptAction::SOUND_PLAY_NAMED].m_internalName = "SOUND_PLAY_NAMED";
    m_actionTemplates[ScriptAction::SOUND_PLAY_NAMED].m_uiName =
        "Multimedia_/Sound Effects/Play a sound as though coming from a specific unit.";
    m_actionTemplates[ScriptAction::SOUND_PLAY_NAMED].m_numParameters = 2;
    m_actionTemplates[ScriptAction::SOUND_PLAY_NAMED].m_parameters[0] = Parameter::TEXT_STRING;
    m_actionTemplates[ScriptAction::SOUND_PLAY_NAMED].m_parameters[1] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::SOUND_PLAY_NAMED].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::SOUND_PLAY_NAMED].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::SOUND_PLAY_NAMED].m_uiStrings[1] = " plays as though coming from ";

    m_actionTemplates[ScriptAction::SPEECH_PLAY].m_internalName = "SPEECH_PLAY";
    m_actionTemplates[ScriptAction::SPEECH_PLAY].m_uiName = "Multimedia_/Sound Effects/Play a speech file.";
    m_actionTemplates[ScriptAction::SPEECH_PLAY].m_numParameters = 2;
    m_actionTemplates[ScriptAction::SPEECH_PLAY].m_parameters[0] = Parameter::DIALOG;
    m_actionTemplates[ScriptAction::SPEECH_PLAY].m_parameters[1] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::SPEECH_PLAY].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::SPEECH_PLAY].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::SPEECH_PLAY].m_uiStrings[1] = " plays, allowing overlap ";
    m_actionTemplates[ScriptAction::SPEECH_PLAY].m_uiStrings[2] = " (true to allow, false to disallow).";

    m_actionTemplates[ScriptAction::PLAYER_TRANSFER_OWNERSHIP_PLAYER].m_internalName = "PLAYER_TRANSFER_OWNERSHIP_PLAYER";
    m_actionTemplates[ScriptAction::PLAYER_TRANSFER_OWNERSHIP_PLAYER].m_uiName =
        "Player_/Transfer/Transfer assets from one player to another player.";
    m_actionTemplates[ScriptAction::PLAYER_TRANSFER_OWNERSHIP_PLAYER].m_numParameters = 2;
    m_actionTemplates[ScriptAction::PLAYER_TRANSFER_OWNERSHIP_PLAYER].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_TRANSFER_OWNERSHIP_PLAYER].m_parameters[1] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_TRANSFER_OWNERSHIP_PLAYER].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::PLAYER_TRANSFER_OWNERSHIP_PLAYER].m_uiStrings[0] = "All assets of ";
    m_actionTemplates[ScriptAction::PLAYER_TRANSFER_OWNERSHIP_PLAYER].m_uiStrings[1] = " are transferred to ";

    m_actionTemplates[ScriptAction::NAMED_TRANSFER_OWNERSHIP_PLAYER].m_internalName = "NAMED_TRANSFER_OWNERSHIP_PLAYER";
    m_actionTemplates[ScriptAction::NAMED_TRANSFER_OWNERSHIP_PLAYER].m_uiName =
        "Player_/Transfer/Transfer a specific unit to the control of a player.";
    m_actionTemplates[ScriptAction::NAMED_TRANSFER_OWNERSHIP_PLAYER].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_TRANSFER_OWNERSHIP_PLAYER].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_TRANSFER_OWNERSHIP_PLAYER].m_parameters[1] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::NAMED_TRANSFER_OWNERSHIP_PLAYER].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_TRANSFER_OWNERSHIP_PLAYER].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_TRANSFER_OWNERSHIP_PLAYER].m_uiStrings[1] = " is transferred to the command of ";

    m_actionTemplates[ScriptAction::PLAYER_EXCLUDE_FROM_SCORE_SCREEN].m_internalName = "PLAYER_EXCLUDE_FROM_SCORE_SCREEN";
    m_actionTemplates[ScriptAction::PLAYER_EXCLUDE_FROM_SCORE_SCREEN].m_uiName =
        "Player_/Score/Exclude this player from the score screen.";
    m_actionTemplates[ScriptAction::PLAYER_EXCLUDE_FROM_SCORE_SCREEN].m_numParameters = 1;
    m_actionTemplates[ScriptAction::PLAYER_EXCLUDE_FROM_SCORE_SCREEN].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_EXCLUDE_FROM_SCORE_SCREEN].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::PLAYER_EXCLUDE_FROM_SCORE_SCREEN].m_uiStrings[0] = "Exclude ";
    m_actionTemplates[ScriptAction::PLAYER_EXCLUDE_FROM_SCORE_SCREEN].m_uiStrings[1] = " from the score screen.";

    m_actionTemplates[ScriptAction::ENABLE_SCORING].m_internalName = "ENABLE_SCORING";
    m_actionTemplates[ScriptAction::ENABLE_SCORING].m_uiName = "Player_/Score/Turn on scoring.";
    m_actionTemplates[ScriptAction::ENABLE_SCORING].m_numParameters = 0;
    m_actionTemplates[ScriptAction::ENABLE_SCORING].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::ENABLE_SCORING].m_uiStrings[0] = "Turn on scoring.";

    m_actionTemplates[ScriptAction::DISABLE_SCORING].m_internalName = "DISABLE_SCORING";
    m_actionTemplates[ScriptAction::DISABLE_SCORING].m_uiName = "Player_/Score/Turn off scoring.";
    m_actionTemplates[ScriptAction::DISABLE_SCORING].m_numParameters = 0;
    m_actionTemplates[ScriptAction::DISABLE_SCORING].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::DISABLE_SCORING].m_uiStrings[0] = "Turn off scoring.";

    m_actionTemplates[ScriptAction::PLAYER_RELATES_PLAYER].m_internalName = "PLAYER_RELATES_PLAYER";
    m_actionTemplates[ScriptAction::PLAYER_RELATES_PLAYER].m_uiName =
        "Player_/Alliances/Change how a player relates to another player.";
    m_actionTemplates[ScriptAction::PLAYER_RELATES_PLAYER].m_numParameters = 3;
    m_actionTemplates[ScriptAction::PLAYER_RELATES_PLAYER].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_RELATES_PLAYER].m_parameters[1] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_RELATES_PLAYER].m_parameters[2] = Parameter::RELATION;
    m_actionTemplates[ScriptAction::PLAYER_RELATES_PLAYER].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::PLAYER_RELATES_PLAYER].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_RELATES_PLAYER].m_uiStrings[1] = " considers ";
    m_actionTemplates[ScriptAction::PLAYER_RELATES_PLAYER].m_uiStrings[2] = " to be ";

    m_actionTemplates[ScriptAction::RADAR_CREATE_EVENT].m_internalName = "RADAR_CREATE_EVENT";
    m_actionTemplates[ScriptAction::RADAR_CREATE_EVENT].m_uiName =
        "Radar_/Create Event/Create a radar event at a specified location.";
    m_actionTemplates[ScriptAction::RADAR_CREATE_EVENT].m_numParameters = 2;
    m_actionTemplates[ScriptAction::RADAR_CREATE_EVENT].m_parameters[0] = Parameter::COORD3D;
    m_actionTemplates[ScriptAction::RADAR_CREATE_EVENT].m_parameters[1] = Parameter::RADAR_EVENT_TYPE;
    m_actionTemplates[ScriptAction::RADAR_CREATE_EVENT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::RADAR_CREATE_EVENT].m_uiStrings[0] = "A radar event occurs at ";
    m_actionTemplates[ScriptAction::RADAR_CREATE_EVENT].m_uiStrings[1] = " of type ";

    m_actionTemplates[ScriptAction::OBJECT_CREATE_RADAR_EVENT].m_internalName = "OBJECT_CREATE_RADAR_EVENT";
    m_actionTemplates[ScriptAction::OBJECT_CREATE_RADAR_EVENT].m_uiName =
        "Radar_/Create Event/Create  a radar event at a specific object.";
    m_actionTemplates[ScriptAction::OBJECT_CREATE_RADAR_EVENT].m_numParameters = 2;
    m_actionTemplates[ScriptAction::OBJECT_CREATE_RADAR_EVENT].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::OBJECT_CREATE_RADAR_EVENT].m_parameters[1] = Parameter::RADAR_EVENT_TYPE;
    m_actionTemplates[ScriptAction::OBJECT_CREATE_RADAR_EVENT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::OBJECT_CREATE_RADAR_EVENT].m_uiStrings[0] = "A radar event occurs at ";
    m_actionTemplates[ScriptAction::OBJECT_CREATE_RADAR_EVENT].m_uiStrings[1] = " of type ";

    m_actionTemplates[ScriptAction::TEAM_CREATE_RADAR_EVENT].m_internalName = "TEAM_CREATE_RADAR_EVENT";
    m_actionTemplates[ScriptAction::TEAM_CREATE_RADAR_EVENT].m_uiName =
        "Radar_/Create Event/Create  a radar event at a specific team.";
    m_actionTemplates[ScriptAction::TEAM_CREATE_RADAR_EVENT].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_CREATE_RADAR_EVENT].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_CREATE_RADAR_EVENT].m_parameters[1] = Parameter::RADAR_EVENT_TYPE;
    m_actionTemplates[ScriptAction::TEAM_CREATE_RADAR_EVENT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_CREATE_RADAR_EVENT].m_uiStrings[0] = "A radar event occurs at ";
    m_actionTemplates[ScriptAction::TEAM_CREATE_RADAR_EVENT].m_uiStrings[1] = " of type ";

    m_actionTemplates[ScriptAction::RADAR_DISABLE].m_internalName = "RADAR_DISABLE";
    m_actionTemplates[ScriptAction::RADAR_DISABLE].m_uiName = "Radar_/Control/Disable the radar.";
    m_actionTemplates[ScriptAction::RADAR_DISABLE].m_numParameters = 0;
    m_actionTemplates[ScriptAction::RADAR_DISABLE].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::RADAR_DISABLE].m_uiStrings[0] = "The radar is disabled.";

    m_actionTemplates[ScriptAction::RADAR_ENABLE].m_internalName = "RADAR_ENABLE";
    m_actionTemplates[ScriptAction::RADAR_ENABLE].m_uiName = "Radar_/Control/Enable the radar.";
    m_actionTemplates[ScriptAction::RADAR_ENABLE].m_numParameters = 0;
    m_actionTemplates[ScriptAction::RADAR_ENABLE].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::RADAR_ENABLE].m_uiStrings[0] = "The radar is enabled.";

    m_actionTemplates[ScriptAction::NAMED_SET_STEALTH_ENABLED].m_internalName = "NAMED_SET_STEALTH_ENABLED";
    m_actionTemplates[ScriptAction::NAMED_SET_STEALTH_ENABLED].m_uiName = "Unit_/Status/Stealth set enabled or disabled.";
    m_actionTemplates[ScriptAction::NAMED_SET_STEALTH_ENABLED].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_SET_STEALTH_ENABLED].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_SET_STEALTH_ENABLED].m_parameters[1] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::NAMED_SET_STEALTH_ENABLED].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::NAMED_SET_STEALTH_ENABLED].m_uiStrings[0] = "Set ";
    m_actionTemplates[ScriptAction::NAMED_SET_STEALTH_ENABLED].m_uiStrings[1] = " stealth ability to ";
    m_actionTemplates[ScriptAction::NAMED_SET_STEALTH_ENABLED].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::TEAM_SET_STEALTH_ENABLED].m_internalName = "TEAM_SET_STEALTH_ENABLED";
    m_actionTemplates[ScriptAction::TEAM_SET_STEALTH_ENABLED].m_uiName = "Team_/Status/Stealth set enabled or disabled.";
    m_actionTemplates[ScriptAction::TEAM_SET_STEALTH_ENABLED].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_SET_STEALTH_ENABLED].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_SET_STEALTH_ENABLED].m_parameters[1] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::TEAM_SET_STEALTH_ENABLED].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_SET_STEALTH_ENABLED].m_uiStrings[0] = "Set ";
    m_actionTemplates[ScriptAction::TEAM_SET_STEALTH_ENABLED].m_uiStrings[1] = " stealth ability to ";
    m_actionTemplates[ScriptAction::TEAM_SET_STEALTH_ENABLED].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::NAMED_SET_UNMANNED_STATUS].m_internalName = "NAMED_SET_UNMANNED_STATUS";
    m_actionTemplates[ScriptAction::NAMED_SET_UNMANNED_STATUS].m_uiName = "Unit_/Status/Make unmanned.";
    m_actionTemplates[ScriptAction::NAMED_SET_UNMANNED_STATUS].m_numParameters = 1;
    m_actionTemplates[ScriptAction::NAMED_SET_UNMANNED_STATUS].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_SET_UNMANNED_STATUS].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_SET_UNMANNED_STATUS].m_uiStrings[0] = "Make ";
    m_actionTemplates[ScriptAction::NAMED_SET_UNMANNED_STATUS].m_uiStrings[1] = " unmanned.";

    m_actionTemplates[ScriptAction::TEAM_SET_UNMANNED_STATUS].m_internalName = "TEAM_SET_UNMANNED_STATUS";
    m_actionTemplates[ScriptAction::TEAM_SET_UNMANNED_STATUS].m_uiName = "Team_/Status/Make unmanned.";
    m_actionTemplates[ScriptAction::TEAM_SET_UNMANNED_STATUS].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_SET_UNMANNED_STATUS].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_SET_UNMANNED_STATUS].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_SET_UNMANNED_STATUS].m_uiStrings[0] = "Make ";
    m_actionTemplates[ScriptAction::TEAM_SET_UNMANNED_STATUS].m_uiStrings[1] = " unmanned.";

    m_actionTemplates[ScriptAction::NAMED_SET_BOOBYTRAPPED].m_internalName = "NAMED_SET_BOOBYTRAPPED";
    m_actionTemplates[ScriptAction::NAMED_SET_BOOBYTRAPPED].m_uiName = "Unit_/Status/Add boobytrap.";
    m_actionTemplates[ScriptAction::NAMED_SET_BOOBYTRAPPED].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_SET_BOOBYTRAPPED].m_parameters[0] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::NAMED_SET_BOOBYTRAPPED].m_parameters[1] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_SET_BOOBYTRAPPED].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::NAMED_SET_BOOBYTRAPPED].m_uiStrings[0] = "Add boobytrap of type ";
    m_actionTemplates[ScriptAction::NAMED_SET_BOOBYTRAPPED].m_uiStrings[1] = " to ";
    m_actionTemplates[ScriptAction::NAMED_SET_BOOBYTRAPPED].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::TEAM_SET_BOOBYTRAPPED].m_internalName = "TEAM_SET_BOOBYTRAPPED";
    m_actionTemplates[ScriptAction::TEAM_SET_BOOBYTRAPPED].m_uiName = "Team_/Status/Add boobytrap.";
    m_actionTemplates[ScriptAction::TEAM_SET_BOOBYTRAPPED].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_SET_BOOBYTRAPPED].m_parameters[0] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::TEAM_SET_BOOBYTRAPPED].m_parameters[1] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_SET_BOOBYTRAPPED].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_SET_BOOBYTRAPPED].m_uiStrings[0] = "Add boobytrap of type ";
    m_actionTemplates[ScriptAction::TEAM_SET_BOOBYTRAPPED].m_uiStrings[1] = " to team ";
    m_actionTemplates[ScriptAction::TEAM_SET_BOOBYTRAPPED].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::MAP_REVEAL_AT_WAYPOINT].m_internalName = "MAP_REVEAL_AT_WAYPOINT";
    m_actionTemplates[ScriptAction::MAP_REVEAL_AT_WAYPOINT].m_uiName =
        "Map_/Shroud or Reveal/Reveal map at waypoint -- fog.";
    m_actionTemplates[ScriptAction::MAP_REVEAL_AT_WAYPOINT].m_numParameters = 3;
    m_actionTemplates[ScriptAction::MAP_REVEAL_AT_WAYPOINT].m_parameters[0] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::MAP_REVEAL_AT_WAYPOINT].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::MAP_REVEAL_AT_WAYPOINT].m_parameters[2] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::MAP_REVEAL_AT_WAYPOINT].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::MAP_REVEAL_AT_WAYPOINT].m_uiStrings[0] = "The map is revealed at ";
    m_actionTemplates[ScriptAction::MAP_REVEAL_AT_WAYPOINT].m_uiStrings[1] = " with a radius of ";
    m_actionTemplates[ScriptAction::MAP_REVEAL_AT_WAYPOINT].m_uiStrings[2] = " feet for ";
    m_actionTemplates[ScriptAction::MAP_REVEAL_AT_WAYPOINT].m_uiStrings[3] = ".";

    m_actionTemplates[ScriptAction::MAP_SHROUD_AT_WAYPOINT].m_internalName = "MAP_SHROUD_AT_WAYPOINT";
    m_actionTemplates[ScriptAction::MAP_SHROUD_AT_WAYPOINT].m_uiName =
        "Map_/Shroud or Reveal/Shroud map at waypoint -- add fog.";
    m_actionTemplates[ScriptAction::MAP_SHROUD_AT_WAYPOINT].m_numParameters = 3;
    m_actionTemplates[ScriptAction::MAP_SHROUD_AT_WAYPOINT].m_parameters[0] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::MAP_SHROUD_AT_WAYPOINT].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::MAP_SHROUD_AT_WAYPOINT].m_parameters[2] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::MAP_SHROUD_AT_WAYPOINT].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::MAP_SHROUD_AT_WAYPOINT].m_uiStrings[0] = "The map is shrouded at ";
    m_actionTemplates[ScriptAction::MAP_SHROUD_AT_WAYPOINT].m_uiStrings[1] = " with a radius of ";
    m_actionTemplates[ScriptAction::MAP_SHROUD_AT_WAYPOINT].m_uiStrings[2] = " feet for ";
    m_actionTemplates[ScriptAction::MAP_SHROUD_AT_WAYPOINT].m_uiStrings[3] = ".";

    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL].m_internalName = "MAP_REVEAL_ALL";
    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL].m_uiName = "Map_/Shroud or Reveal/Reveal the entire map for a player.";
    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL].m_numParameters = 1;
    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL].m_uiStrings[0] = "The world is revealed for ";
    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL].m_uiStrings[1] = ".";

    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL_PERM].m_internalName = "MAP_REVEAL_ALL_PERM";
    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL_PERM].m_uiName =
        "Map_/Shroud or Reveal/Reveal the entire map permanently for a player.";
    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL_PERM].m_numParameters = 1;
    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL_PERM].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL_PERM].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL_PERM].m_uiStrings[0] = "The world is revealed permanently for ";
    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL_PERM].m_uiStrings[1] = ".";

    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL_UNDO_PERM].m_internalName = "MAP_REVEAL_ALL_UNDO_PERM";
    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL_UNDO_PERM].m_uiName =
        "Map_/Shroud or Reveal/Un-Reveal the entire map permanently for a player.";
    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL_UNDO_PERM].m_numParameters = 1;
    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL_UNDO_PERM].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL_UNDO_PERM].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL_UNDO_PERM].m_uiStrings[0] = "Undo the permanent reveal for ";
    m_actionTemplates[ScriptAction::MAP_REVEAL_ALL_UNDO_PERM].m_uiStrings[1] =
        ".  This will mess things up badly if called when there has been no permanent reveal.";

    m_actionTemplates[ScriptAction::MAP_SHROUD_ALL].m_internalName = "MAP_SHROUD_ALL";
    m_actionTemplates[ScriptAction::MAP_SHROUD_ALL].m_uiName = "Map_/Shroud or Reveal/Shroud the entire map for a player.";
    m_actionTemplates[ScriptAction::MAP_SHROUD_ALL].m_numParameters = 1;
    m_actionTemplates[ScriptAction::MAP_SHROUD_ALL].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::MAP_SHROUD_ALL].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::MAP_SHROUD_ALL].m_uiStrings[0] = "The world is shrouded for ";
    m_actionTemplates[ScriptAction::MAP_SHROUD_ALL].m_uiStrings[1] = ".";

    m_actionTemplates[ScriptAction::DISABLE_BORDER_SHROUD].m_internalName = "DISABLE_BORDER_SHROUD";
    m_actionTemplates[ScriptAction::DISABLE_BORDER_SHROUD].m_uiName = "Map_/Shroud or Reveal/Border Shroud is turned off.";
    m_actionTemplates[ScriptAction::DISABLE_BORDER_SHROUD].m_numParameters = 0;
    m_actionTemplates[ScriptAction::DISABLE_BORDER_SHROUD].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::DISABLE_BORDER_SHROUD].m_uiStrings[0] = "Shroud off the map edges is turned off.";

    m_actionTemplates[ScriptAction::ENABLE_BORDER_SHROUD].m_internalName = "ENABLE_BORDER_SHROUD";
    m_actionTemplates[ScriptAction::ENABLE_BORDER_SHROUD].m_uiName = "Map_/Shroud or Reveal/Border Shroud is turned on.";
    m_actionTemplates[ScriptAction::ENABLE_BORDER_SHROUD].m_numParameters = 0;
    m_actionTemplates[ScriptAction::ENABLE_BORDER_SHROUD].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::ENABLE_BORDER_SHROUD].m_uiStrings[0] = "Shroud off the map edges is turned on.";

    m_actionTemplates[ScriptAction::TEAM_GARRISON_SPECIFIC_BUILDING].m_internalName = "TEAM_GARRISON_SPECIFIC_BUILDING";
    m_actionTemplates[ScriptAction::TEAM_GARRISON_SPECIFIC_BUILDING].m_uiName =
        "Team_/Garrison/Garrison a specific building with a team.";
    m_actionTemplates[ScriptAction::TEAM_GARRISON_SPECIFIC_BUILDING].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_GARRISON_SPECIFIC_BUILDING].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_GARRISON_SPECIFIC_BUILDING].m_parameters[1] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::TEAM_GARRISON_SPECIFIC_BUILDING].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_GARRISON_SPECIFIC_BUILDING].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_GARRISON_SPECIFIC_BUILDING].m_uiStrings[1] = " enters into building named ";

    m_actionTemplates[ScriptAction::EXIT_SPECIFIC_BUILDING].m_internalName = "EXIT_SPECIFIC_BUILDING";
    m_actionTemplates[ScriptAction::EXIT_SPECIFIC_BUILDING].m_uiName = "Unit_/Garrison/Empty a specific building.";
    m_actionTemplates[ScriptAction::EXIT_SPECIFIC_BUILDING].m_numParameters = 1;
    m_actionTemplates[ScriptAction::EXIT_SPECIFIC_BUILDING].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::EXIT_SPECIFIC_BUILDING].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::EXIT_SPECIFIC_BUILDING].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::EXIT_SPECIFIC_BUILDING].m_uiStrings[1] = " empties.";

    m_actionTemplates[ScriptAction::TEAM_GARRISON_NEAREST_BUILDING].m_internalName = "TEAM_GARRISON_NEAREST_BUILDING";
    m_actionTemplates[ScriptAction::TEAM_GARRISON_NEAREST_BUILDING].m_uiName =
        "Team_/Garrison/Garrison a nearby building with a team.";
    m_actionTemplates[ScriptAction::TEAM_GARRISON_NEAREST_BUILDING].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_GARRISON_NEAREST_BUILDING].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_GARRISON_NEAREST_BUILDING].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_GARRISON_NEAREST_BUILDING].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_GARRISON_NEAREST_BUILDING].m_uiStrings[1] = " garrison a nearby building.";

    m_actionTemplates[ScriptAction::TEAM_EXIT_ALL_BUILDINGS].m_internalName = "TEAM_EXIT_ALL_BUILDINGS";
    m_actionTemplates[ScriptAction::TEAM_EXIT_ALL_BUILDINGS].m_uiName = "Team_/Garrison/Exit all buildings a team is in.";
    m_actionTemplates[ScriptAction::TEAM_EXIT_ALL_BUILDINGS].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_EXIT_ALL_BUILDINGS].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_EXIT_ALL_BUILDINGS].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_EXIT_ALL_BUILDINGS].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_EXIT_ALL_BUILDINGS].m_uiStrings[1] = " exits all buildings.";

    m_actionTemplates[ScriptAction::NAMED_GARRISON_SPECIFIC_BUILDING].m_internalName = "NAMED_GARRISON_SPECIFIC_BUILDING";
    m_actionTemplates[ScriptAction::NAMED_GARRISON_SPECIFIC_BUILDING].m_uiName =
        "Unit_/Garrison/Garrison a specific building with a specific unit.";
    m_actionTemplates[ScriptAction::NAMED_GARRISON_SPECIFIC_BUILDING].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_GARRISON_SPECIFIC_BUILDING].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_GARRISON_SPECIFIC_BUILDING].m_parameters[1] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_GARRISON_SPECIFIC_BUILDING].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_GARRISON_SPECIFIC_BUILDING].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_GARRISON_SPECIFIC_BUILDING].m_uiStrings[1] = " garrison building ";

    m_actionTemplates[ScriptAction::NAMED_GARRISON_NEAREST_BUILDING].m_internalName = "NAMED_GARRISON_NEAREST_BUILDING";
    m_actionTemplates[ScriptAction::NAMED_GARRISON_NEAREST_BUILDING].m_uiName =
        "Unit_/Garrison/Garrison a nearby building with a specific unit.";
    m_actionTemplates[ScriptAction::NAMED_GARRISON_NEAREST_BUILDING].m_numParameters = 1;
    m_actionTemplates[ScriptAction::NAMED_GARRISON_NEAREST_BUILDING].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_GARRISON_NEAREST_BUILDING].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_GARRISON_NEAREST_BUILDING].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_GARRISON_NEAREST_BUILDING].m_uiStrings[1] = " garrison a nearby building.";

    m_actionTemplates[ScriptAction::NAMED_EXIT_BUILDING].m_internalName = "NAMED_EXIT_BUILDING";
    m_actionTemplates[ScriptAction::NAMED_EXIT_BUILDING].m_uiName = "Unit_/Garrison/Exit the building the unit is in.";
    m_actionTemplates[ScriptAction::NAMED_EXIT_BUILDING].m_numParameters = 1;
    m_actionTemplates[ScriptAction::NAMED_EXIT_BUILDING].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_EXIT_BUILDING].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_EXIT_BUILDING].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_EXIT_BUILDING].m_uiStrings[1] = " leaves the building.";

    m_actionTemplates[ScriptAction::PLAYER_GARRISON_ALL_BUILDINGS].m_internalName = "PLAYER_GARRISON_ALL_BUILDINGS";
    m_actionTemplates[ScriptAction::PLAYER_GARRISON_ALL_BUILDINGS].m_uiName =
        "Player_/Garrison/Garrison as many buildings as player has units for.";
    m_actionTemplates[ScriptAction::PLAYER_GARRISON_ALL_BUILDINGS].m_numParameters = 1;
    m_actionTemplates[ScriptAction::PLAYER_GARRISON_ALL_BUILDINGS].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_GARRISON_ALL_BUILDINGS].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::PLAYER_GARRISON_ALL_BUILDINGS].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_GARRISON_ALL_BUILDINGS].m_uiStrings[1] = " garrison buildings.";

    m_actionTemplates[ScriptAction::PLAYER_EXIT_ALL_BUILDINGS].m_internalName = "PLAYER_EXIT_ALL_BUILDINGS";
    m_actionTemplates[ScriptAction::PLAYER_EXIT_ALL_BUILDINGS].m_uiName =
        "Player_/Garrison/All units leave their garrisons.";
    m_actionTemplates[ScriptAction::PLAYER_EXIT_ALL_BUILDINGS].m_numParameters = 1;
    m_actionTemplates[ScriptAction::PLAYER_EXIT_ALL_BUILDINGS].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_EXIT_ALL_BUILDINGS].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::PLAYER_EXIT_ALL_BUILDINGS].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_EXIT_ALL_BUILDINGS].m_uiStrings[1] = " evacuate.";

    m_actionTemplates[ScriptAction::TEAM_AVAILABLE_FOR_RECRUITMENT].m_internalName = "TEAM_AVAILABLE_FOR_RECRUITMENT";
    m_actionTemplates[ScriptAction::TEAM_AVAILABLE_FOR_RECRUITMENT].m_uiName =
        "Team_/AI/Set whether members of a team can be recruited into another team.";
    m_actionTemplates[ScriptAction::TEAM_AVAILABLE_FOR_RECRUITMENT].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_AVAILABLE_FOR_RECRUITMENT].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_AVAILABLE_FOR_RECRUITMENT].m_parameters[1] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::TEAM_AVAILABLE_FOR_RECRUITMENT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_AVAILABLE_FOR_RECRUITMENT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_AVAILABLE_FOR_RECRUITMENT].m_uiStrings[1] =
        " sets their willingness to join teams to ";

    m_actionTemplates[ScriptAction::TEAM_COLLECT_NEARBY_FOR_TEAM].m_internalName = "TEAM_COLLECT_NEARBY_FOR_TEAM";
    m_actionTemplates[ScriptAction::TEAM_COLLECT_NEARBY_FOR_TEAM].m_uiName = "Team_/AI/Set to collect nearby units.";
    m_actionTemplates[ScriptAction::TEAM_COLLECT_NEARBY_FOR_TEAM].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_COLLECT_NEARBY_FOR_TEAM].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_COLLECT_NEARBY_FOR_TEAM].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_COLLECT_NEARBY_FOR_TEAM].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_COLLECT_NEARBY_FOR_TEAM].m_uiStrings[1] =
        " attempts to collect nearby units for a team.";

    m_actionTemplates[ScriptAction::TEAM_MERGE_INTO_TEAM].m_internalName = "TEAM_MERGE_INTO_TEAM";
    m_actionTemplates[ScriptAction::TEAM_MERGE_INTO_TEAM].m_uiName = "Team_/Merge/Merge a team into another team.";
    m_actionTemplates[ScriptAction::TEAM_MERGE_INTO_TEAM].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_MERGE_INTO_TEAM].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_MERGE_INTO_TEAM].m_parameters[1] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_MERGE_INTO_TEAM].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_MERGE_INTO_TEAM].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_MERGE_INTO_TEAM].m_uiStrings[1] = " merges onto ";

    m_actionTemplates[ScriptAction::IDLE_ALL_UNITS].m_internalName = "IDLE_ALL_UNITS";
    m_actionTemplates[ScriptAction::IDLE_ALL_UNITS].m_uiName = "Scripting_/Idle or Restart/Idle all units for all players.";
    m_actionTemplates[ScriptAction::IDLE_ALL_UNITS].m_numParameters = 0;
    m_actionTemplates[ScriptAction::IDLE_ALL_UNITS].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::IDLE_ALL_UNITS].m_uiStrings[0] = "Idle all units for all players.";

    m_actionTemplates[ScriptAction::RESUME_SUPPLY_TRUCKING].m_internalName = "RESUME_SUPPLY_TRUCKING";
    m_actionTemplates[ScriptAction::RESUME_SUPPLY_TRUCKING].m_uiName =
        "Scripting_/Idle or Restart/All idle Supply Trucks attempt to resume supply routes.";
    m_actionTemplates[ScriptAction::RESUME_SUPPLY_TRUCKING].m_numParameters = 0;
    m_actionTemplates[ScriptAction::RESUME_SUPPLY_TRUCKING].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::RESUME_SUPPLY_TRUCKING].m_uiStrings[0] =
        "All idle Supply Trucks attempt to resume supply routes.";

    m_actionTemplates[ScriptAction::DISABLE_INPUT].m_internalName = "DISABLE_INPUT";
    m_actionTemplates[ScriptAction::DISABLE_INPUT].m_uiName = "User_/Input/User input -- disable.";
    m_actionTemplates[ScriptAction::DISABLE_INPUT].m_numParameters = 0;
    m_actionTemplates[ScriptAction::DISABLE_INPUT].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::DISABLE_INPUT].m_uiStrings[0] = "Disable mouse and keyboard input.";

    m_actionTemplates[ScriptAction::ENABLE_INPUT].m_internalName = "ENABLE_INPUT";
    m_actionTemplates[ScriptAction::ENABLE_INPUT].m_uiName = "User_/Input/User input -- enable.";
    m_actionTemplates[ScriptAction::ENABLE_INPUT].m_numParameters = 0;
    m_actionTemplates[ScriptAction::ENABLE_INPUT].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::ENABLE_INPUT].m_uiStrings[0] = "Enable mouse and keyboard input.";

    m_actionTemplates[ScriptAction::SOUND_AMBIENT_PAUSE].m_internalName = "SOUND_AMBIENT_PAUSE";
    m_actionTemplates[ScriptAction::SOUND_AMBIENT_PAUSE].m_uiName = "Multimedia_/SoundEffects/Pause the ambient sounds.";
    m_actionTemplates[ScriptAction::SOUND_AMBIENT_PAUSE].m_numParameters = 0;
    m_actionTemplates[ScriptAction::SOUND_AMBIENT_PAUSE].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::SOUND_AMBIENT_PAUSE].m_uiStrings[0] = "Pause the ambient sounds.";

    m_actionTemplates[ScriptAction::SOUND_AMBIENT_RESUME].m_internalName = "SOUND_AMBIENT_RESUME";
    m_actionTemplates[ScriptAction::SOUND_AMBIENT_RESUME].m_uiName = "Multimedia_/SoundEffects/Resume the ambient sounds.";
    m_actionTemplates[ScriptAction::SOUND_AMBIENT_RESUME].m_numParameters = 0;
    m_actionTemplates[ScriptAction::SOUND_AMBIENT_RESUME].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::SOUND_AMBIENT_RESUME].m_uiStrings[0] = "Resume the ambient sounds.";

    m_actionTemplates[ScriptAction::MUSIC_SET_TRACK].m_internalName = "MUSIC_SET_TRACK";
    m_actionTemplates[ScriptAction::MUSIC_SET_TRACK].m_uiName = "Multimedia_/Music/Play a music track.";
    m_actionTemplates[ScriptAction::MUSIC_SET_TRACK].m_numParameters = 3;
    m_actionTemplates[ScriptAction::MUSIC_SET_TRACK].m_parameters[0] = Parameter::MUSIC;
    m_actionTemplates[ScriptAction::MUSIC_SET_TRACK].m_parameters[1] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::MUSIC_SET_TRACK].m_parameters[2] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::MUSIC_SET_TRACK].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::MUSIC_SET_TRACK].m_uiStrings[0] = "Play ";
    m_actionTemplates[ScriptAction::MUSIC_SET_TRACK].m_uiStrings[1] = " using fadeout (";
    m_actionTemplates[ScriptAction::MUSIC_SET_TRACK].m_uiStrings[2] = ") and fadein (";
    m_actionTemplates[ScriptAction::MUSIC_SET_TRACK].m_uiStrings[3] = ").";

    m_actionTemplates[ScriptAction::CAMERA_LETTERBOX_BEGIN].m_internalName = "CAMERA_LETTERBOX_BEGIN";
    m_actionTemplates[ScriptAction::CAMERA_LETTERBOX_BEGIN].m_uiName = "Camera_/Letterbox/Start letterbox mode.";
    m_actionTemplates[ScriptAction::CAMERA_LETTERBOX_BEGIN].m_numParameters = 0;
    m_actionTemplates[ScriptAction::CAMERA_LETTERBOX_BEGIN].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::CAMERA_LETTERBOX_BEGIN].m_uiStrings[0] = "Start letterbox mode (hide UI, add border).";

    m_actionTemplates[ScriptAction::CAMERA_LETTERBOX_END].m_internalName = "CAMERA_LETTERBOX_END";
    m_actionTemplates[ScriptAction::CAMERA_LETTERBOX_END].m_uiName = "Camera_/ End letterbox mode.";
    m_actionTemplates[ScriptAction::CAMERA_LETTERBOX_END].m_numParameters = 0;
    m_actionTemplates[ScriptAction::CAMERA_LETTERBOX_END].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::CAMERA_LETTERBOX_END].m_uiStrings[0] = "End letterbox mode (show UI, remove border).";

    m_actionTemplates[ScriptAction::CAMERA_BW_MODE_BEGIN].m_internalName = "CAMERA_BW_MODE_BEGIN";
    m_actionTemplates[ScriptAction::CAMERA_BW_MODE_BEGIN].m_uiName = "Camera_/Fade Effects/Start black & white mode.";
    m_actionTemplates[ScriptAction::CAMERA_BW_MODE_BEGIN].m_numParameters = 1;
    m_actionTemplates[ScriptAction::CAMERA_BW_MODE_BEGIN].m_parameters[0] = Parameter::INT;
    m_actionTemplates[ScriptAction::CAMERA_BW_MODE_BEGIN].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::CAMERA_BW_MODE_BEGIN].m_uiStrings[0] = "Frames to fade into black & white mode = ";

    m_actionTemplates[ScriptAction::CAMERA_BW_MODE_END].m_internalName = "CAMERA_BW_MODE_END";
    m_actionTemplates[ScriptAction::CAMERA_BW_MODE_END].m_uiName = "Camera_/Fade Effects/End black & white mode.";
    m_actionTemplates[ScriptAction::CAMERA_BW_MODE_END].m_numParameters = 1;
    m_actionTemplates[ScriptAction::CAMERA_BW_MODE_END].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::CAMERA_BW_MODE_END].m_uiStrings[0] = "Frames to fade into color mode = ";

    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR].m_internalName = "CAMERA_MOTION_BLUR";
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR].m_uiName = "Camera_/Fade Effects/Motion blur zoom.";
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR].m_numParameters = 2;
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR].m_parameters[0] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR].m_parameters[1] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR].m_uiStrings[0] = "Blur zoom, zoom in = ";
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR].m_uiStrings[1] =
        " (true=zoom in, false = zoom out), saturate colors = ";

    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_JUMP].m_internalName = "CAMERA_MOTION_BLUR_JUMP";
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_JUMP].m_uiName =
        "Camera_/Fade Effects/Motion blur zoom with jump cut.";
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_JUMP].m_numParameters = 2;
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_JUMP].m_parameters[0] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_JUMP].m_parameters[1] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_JUMP].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_JUMP].m_uiStrings[0] =
        "Blur zoom, zoom in at current location, zoom out at ";
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_JUMP].m_uiStrings[1] = ", saturate colors = ";

    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_FOLLOW].m_internalName = "CAMERA_MOTION_BLUR_FOLLOW";
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_FOLLOW].m_uiName =
        "Camera_/Fade Effects/Start motion blur as the camera moves.";
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_FOLLOW].m_numParameters = 1;
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_FOLLOW].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_FOLLOW].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_FOLLOW].m_uiStrings[0] =
        "Start motion blur as the camera moves, amount= ";
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_FOLLOW].m_uiStrings[1] = " (start with 30 and adjust up or down). ";

    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_END_FOLLOW].m_internalName = "CAMERA_MOTION_BLUR_END_FOLLOW";
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_END_FOLLOW].m_uiName =
        "Camera_/Fade Effects/End motion blur as the camera moves.";
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_END_FOLLOW].m_numParameters = 0;
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_END_FOLLOW].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::CAMERA_MOTION_BLUR_END_FOLLOW].m_uiStrings[0] = "End motion blur as the camera moves.";

    m_actionTemplates[ScriptAction::DRAW_SKYBOX_BEGIN].m_internalName = "DRAW_SKYBOX_BEGIN";
    m_actionTemplates[ScriptAction::DRAW_SKYBOX_BEGIN].m_uiName = "Camera_/Skybox/Start skybox mode.";
    m_actionTemplates[ScriptAction::DRAW_SKYBOX_BEGIN].m_numParameters = 0;
    m_actionTemplates[ScriptAction::DRAW_SKYBOX_BEGIN].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::DRAW_SKYBOX_BEGIN].m_uiStrings[0] = "Start skybox mode (draw sky background).";

    m_actionTemplates[ScriptAction::DRAW_SKYBOX_END].m_internalName = "DRAW_SKYBOX_END";
    m_actionTemplates[ScriptAction::DRAW_SKYBOX_END].m_uiName = "Camera_/Skybox/End skybox mode.";
    m_actionTemplates[ScriptAction::DRAW_SKYBOX_END].m_numParameters = 0;
    m_actionTemplates[ScriptAction::DRAW_SKYBOX_END].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::DRAW_SKYBOX_END].m_uiStrings[0] = "End skybox mode (draw black background).";

    m_actionTemplates[ScriptAction::FREEZE_TIME].m_internalName = "FREEZE_TIME";
    m_actionTemplates[ScriptAction::FREEZE_TIME].m_uiName = "Scripting_/Time/Freeze time.";
    m_actionTemplates[ScriptAction::FREEZE_TIME].m_numParameters = 0;
    m_actionTemplates[ScriptAction::FREEZE_TIME].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::FREEZE_TIME].m_uiStrings[0] = "Freeze time.";

    m_actionTemplates[ScriptAction::UNFREEZE_TIME].m_internalName = "UNFREEZE_TIME";
    m_actionTemplates[ScriptAction::UNFREEZE_TIME].m_uiName = "Scripting_/Time/Unfreeze time.";
    m_actionTemplates[ScriptAction::UNFREEZE_TIME].m_numParameters = 0;
    m_actionTemplates[ScriptAction::UNFREEZE_TIME].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::UNFREEZE_TIME].m_uiStrings[0] = "Unfreeze time.";

    m_actionTemplates[ScriptAction::SHOW_MILITARY_CAPTION].m_internalName = "SHOW_MILITARY_CAPTION";
    m_actionTemplates[ScriptAction::SHOW_MILITARY_CAPTION].m_uiName = "Scripting_/Briefing/Show military briefing caption.";
    m_actionTemplates[ScriptAction::SHOW_MILITARY_CAPTION].m_numParameters = 2;
    m_actionTemplates[ScriptAction::SHOW_MILITARY_CAPTION].m_parameters[0] = Parameter::TEXT_STRING;
    m_actionTemplates[ScriptAction::SHOW_MILITARY_CAPTION].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::SHOW_MILITARY_CAPTION].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::SHOW_MILITARY_CAPTION].m_uiStrings[0] = "Show military briefing ";
    m_actionTemplates[ScriptAction::SHOW_MILITARY_CAPTION].m_uiStrings[1] = " for ";
    m_actionTemplates[ScriptAction::SHOW_MILITARY_CAPTION].m_uiStrings[2] = " milliseconds.";

    m_actionTemplates[ScriptAction::CAMERA_SET_AUDIBLE_DISTANCE].m_internalName = "CAMERA_SET_AUDIBLE_DISTANCE";
    m_actionTemplates[ScriptAction::CAMERA_SET_AUDIBLE_DISTANCE].m_uiName =
        "Camera_/Sounds/Set the audible distance for camera-up shots.";
    m_actionTemplates[ScriptAction::CAMERA_SET_AUDIBLE_DISTANCE].m_numParameters = 1;
    m_actionTemplates[ScriptAction::CAMERA_SET_AUDIBLE_DISTANCE].m_parameters[0] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_SET_AUDIBLE_DISTANCE].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::CAMERA_SET_AUDIBLE_DISTANCE].m_uiStrings[0] =
        "Set the audible range during camera-up shots to ";

    m_actionTemplates[ScriptAction::NAMED_SET_HELD].m_internalName = "NAMED_SET_HELD";
    m_actionTemplates[ScriptAction::NAMED_SET_HELD].m_uiName =
        "Unit_/Move/Set unit to be held in place, ignoring Physics, Locomotors, etc.";
    m_actionTemplates[ScriptAction::NAMED_SET_HELD].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_SET_HELD].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_SET_HELD].m_parameters[1] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::NAMED_SET_HELD].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::NAMED_SET_HELD].m_uiStrings[0] = "Set Held status for ";
    m_actionTemplates[ScriptAction::NAMED_SET_HELD].m_uiStrings[1] = " to ";
    m_actionTemplates[ScriptAction::NAMED_SET_HELD].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::NAMED_SET_STOPPING_DISTANCE].m_internalName = "NAMED_SET_STOPPING_DISTANCE";
    m_actionTemplates[ScriptAction::NAMED_SET_STOPPING_DISTANCE].m_uiName =
        "Unit_/Move/Set stopping distance for current locomotor.";
    m_actionTemplates[ScriptAction::NAMED_SET_STOPPING_DISTANCE].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_SET_STOPPING_DISTANCE].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_SET_STOPPING_DISTANCE].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::NAMED_SET_STOPPING_DISTANCE].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::NAMED_SET_STOPPING_DISTANCE].m_uiStrings[0] = "Set stopping distance for ";
    m_actionTemplates[ScriptAction::NAMED_SET_STOPPING_DISTANCE].m_uiStrings[1] = " to ";
    m_actionTemplates[ScriptAction::NAMED_SET_STOPPING_DISTANCE].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::SET_STOPPING_DISTANCE].m_internalName = "SET_STOPPING_DISTANCE";
    m_actionTemplates[ScriptAction::SET_STOPPING_DISTANCE].m_uiName =
        "Team_/Move/Set stopping distance for each unit's current locomotor.";
    m_actionTemplates[ScriptAction::SET_STOPPING_DISTANCE].m_numParameters = 2;
    m_actionTemplates[ScriptAction::SET_STOPPING_DISTANCE].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::SET_STOPPING_DISTANCE].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::SET_STOPPING_DISTANCE].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::SET_STOPPING_DISTANCE].m_uiStrings[0] = "Set stopping distances for ";
    m_actionTemplates[ScriptAction::SET_STOPPING_DISTANCE].m_uiStrings[1] = " to ";
    m_actionTemplates[ScriptAction::SET_STOPPING_DISTANCE].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::SET_FPS_LIMIT].m_internalName = "SET_FPS_LIMIT";
    m_actionTemplates[ScriptAction::SET_FPS_LIMIT].m_uiName = "Scripting_/ Set max frames per second.";
    m_actionTemplates[ScriptAction::SET_FPS_LIMIT].m_numParameters = 1;
    m_actionTemplates[ScriptAction::SET_FPS_LIMIT].m_parameters[0] = Parameter::INT;
    m_actionTemplates[ScriptAction::SET_FPS_LIMIT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::SET_FPS_LIMIT].m_uiStrings[0] = "Set max FPS to ";
    m_actionTemplates[ScriptAction::SET_FPS_LIMIT].m_uiStrings[1] = ".  (0 sets to default.)";

    m_actionTemplates[ScriptAction::DISABLE_SPECIAL_POWER_DISPLAY].m_internalName = "DISABLE_SPECIAL_POWER_DISPLAY";
    m_actionTemplates[ScriptAction::DISABLE_SPECIAL_POWER_DISPLAY].m_uiName =
        "Scripting_/ Special power countdown display -- disable.";
    m_actionTemplates[ScriptAction::DISABLE_SPECIAL_POWER_DISPLAY].m_numParameters = 0;
    m_actionTemplates[ScriptAction::DISABLE_SPECIAL_POWER_DISPLAY].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::DISABLE_SPECIAL_POWER_DISPLAY].m_uiStrings[0] =
        "Disables special power countdown display.";

    m_actionTemplates[ScriptAction::ENABLE_SPECIAL_POWER_DISPLAY].m_internalName = "ENABLE_SPECIAL_POWER_DISPLAY";
    m_actionTemplates[ScriptAction::ENABLE_SPECIAL_POWER_DISPLAY].m_uiName =
        "Scripting_/ Special power countdown display -- enable.";
    m_actionTemplates[ScriptAction::ENABLE_SPECIAL_POWER_DISPLAY].m_numParameters = 0;
    m_actionTemplates[ScriptAction::ENABLE_SPECIAL_POWER_DISPLAY].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::ENABLE_SPECIAL_POWER_DISPLAY].m_uiStrings[0] =
        "Enables special power countdown display.";

    m_actionTemplates[ScriptAction::NAMED_HIDE_SPECIAL_POWER_DISPLAY].m_internalName = "NAMED_HIDE_SPECIAL_POWER_DISPLAY";
    m_actionTemplates[ScriptAction::NAMED_HIDE_SPECIAL_POWER_DISPLAY].m_uiName =
        "Unit_/ Special power countdown timer -- hide.";
    m_actionTemplates[ScriptAction::NAMED_HIDE_SPECIAL_POWER_DISPLAY].m_numParameters = 1;
    m_actionTemplates[ScriptAction::NAMED_HIDE_SPECIAL_POWER_DISPLAY].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_HIDE_SPECIAL_POWER_DISPLAY].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_HIDE_SPECIAL_POWER_DISPLAY].m_uiStrings[0] = "Hides special power countdowns for ";
    m_actionTemplates[ScriptAction::NAMED_HIDE_SPECIAL_POWER_DISPLAY].m_uiStrings[1] = ".";

    m_actionTemplates[ScriptAction::NAMED_SHOW_SPECIAL_POWER_DISPLAY].m_internalName = "NAMED_SHOW_SPECIAL_POWER_DISPLAY";
    m_actionTemplates[ScriptAction::NAMED_SHOW_SPECIAL_POWER_DISPLAY].m_uiName =
        "Unit_/ Special power countdown timer -- display.";
    m_actionTemplates[ScriptAction::NAMED_SHOW_SPECIAL_POWER_DISPLAY].m_numParameters = 1;
    m_actionTemplates[ScriptAction::NAMED_SHOW_SPECIAL_POWER_DISPLAY].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_SHOW_SPECIAL_POWER_DISPLAY].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_SHOW_SPECIAL_POWER_DISPLAY].m_uiStrings[0] = "Shows special power countdowns for ";
    m_actionTemplates[ScriptAction::NAMED_SHOW_SPECIAL_POWER_DISPLAY].m_uiStrings[1] = ".";

    m_actionTemplates[ScriptAction::MUSIC_SET_VOLUME].m_internalName = "MUSIC_SET_VOLUME";
    m_actionTemplates[ScriptAction::MUSIC_SET_VOLUME].m_uiName = "Multimedia_/ Set the current music volume.";
    m_actionTemplates[ScriptAction::MUSIC_SET_VOLUME].m_numParameters = 1;
    m_actionTemplates[ScriptAction::MUSIC_SET_VOLUME].m_parameters[0] = Parameter::REAL;
    m_actionTemplates[ScriptAction::MUSIC_SET_VOLUME].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::MUSIC_SET_VOLUME].m_uiStrings[0] = "Set the desired music volume to ";
    m_actionTemplates[ScriptAction::MUSIC_SET_VOLUME].m_uiStrings[1] = "%. (0-100)";

    m_actionTemplates[ScriptAction::TEAM_TRANSFER_TO_PLAYER].m_internalName = "TEAM_TRANSFER_TO_PLAYER";
    m_actionTemplates[ScriptAction::TEAM_TRANSFER_TO_PLAYER].m_uiName = "Team_/ Transfer control of a team to a player.";
    m_actionTemplates[ScriptAction::TEAM_TRANSFER_TO_PLAYER].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_TRANSFER_TO_PLAYER].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_TRANSFER_TO_PLAYER].m_parameters[1] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::TEAM_TRANSFER_TO_PLAYER].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_TRANSFER_TO_PLAYER].m_uiStrings[0] = "Control of ";
    m_actionTemplates[ScriptAction::TEAM_TRANSFER_TO_PLAYER].m_uiStrings[1] = " transfers to ";

    m_actionTemplates[ScriptAction::PLAYER_SET_MONEY].m_internalName = "PLAYER_SET_MONEY";
    m_actionTemplates[ScriptAction::PLAYER_SET_MONEY].m_uiName = "Player_/ Set player's money.";
    m_actionTemplates[ScriptAction::PLAYER_SET_MONEY].m_numParameters = 2;
    m_actionTemplates[ScriptAction::PLAYER_SET_MONEY].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_SET_MONEY].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::PLAYER_SET_MONEY].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::PLAYER_SET_MONEY].m_uiStrings[0] = "Set ";
    m_actionTemplates[ScriptAction::PLAYER_SET_MONEY].m_uiStrings[1] = "'s money to $";

    m_actionTemplates[ScriptAction::PLAYER_GIVE_MONEY].m_internalName = "PLAYER_GIVE_MONEY";
    m_actionTemplates[ScriptAction::PLAYER_GIVE_MONEY].m_uiName = "Player_/ Gives/takes from player's money.";
    m_actionTemplates[ScriptAction::PLAYER_GIVE_MONEY].m_numParameters = 2;
    m_actionTemplates[ScriptAction::PLAYER_GIVE_MONEY].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_GIVE_MONEY].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::PLAYER_GIVE_MONEY].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::PLAYER_GIVE_MONEY].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_GIVE_MONEY].m_uiStrings[1] = " gets $";

    m_actionTemplates[ScriptAction::DISPLAY_COUNTER].m_internalName = "DISPLAY_COUNTER";
    m_actionTemplates[ScriptAction::DISPLAY_COUNTER].m_uiName =
        "Scripting_/ Counter -- display an individual counter to the user.";
    m_actionTemplates[ScriptAction::DISPLAY_COUNTER].m_numParameters = 2;
    m_actionTemplates[ScriptAction::DISPLAY_COUNTER].m_parameters[0] = Parameter::COUNTER;
    m_actionTemplates[ScriptAction::DISPLAY_COUNTER].m_parameters[1] = Parameter::LOCALIZED_TEXT;
    m_actionTemplates[ScriptAction::DISPLAY_COUNTER].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::DISPLAY_COUNTER].m_uiStrings[0] = "Show ";
    m_actionTemplates[ScriptAction::DISPLAY_COUNTER].m_uiStrings[1] = " with text ";

    m_actionTemplates[ScriptAction::HIDE_COUNTER].m_internalName = "HIDE_COUNTER";
    m_actionTemplates[ScriptAction::HIDE_COUNTER].m_uiName =
        "Scripting_/ Counter -- hides an individual counter from the user.";
    m_actionTemplates[ScriptAction::HIDE_COUNTER].m_numParameters = 1;
    m_actionTemplates[ScriptAction::HIDE_COUNTER].m_parameters[0] = Parameter::COUNTER;
    m_actionTemplates[ScriptAction::HIDE_COUNTER].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::HIDE_COUNTER].m_uiStrings[0] = "Hide ";

    m_actionTemplates[ScriptAction::DISPLAY_COUNTDOWN_TIMER].m_internalName = "DISPLAY_COUNTDOWN_TIMER";
    m_actionTemplates[ScriptAction::DISPLAY_COUNTDOWN_TIMER].m_uiName =
        "Scripting_/ Timer -- display an individual timer to the user.";
    m_actionTemplates[ScriptAction::DISPLAY_COUNTDOWN_TIMER].m_numParameters = 2;
    m_actionTemplates[ScriptAction::DISPLAY_COUNTDOWN_TIMER].m_parameters[0] = Parameter::COUNTER;
    m_actionTemplates[ScriptAction::DISPLAY_COUNTDOWN_TIMER].m_parameters[1] = Parameter::LOCALIZED_TEXT;
    m_actionTemplates[ScriptAction::DISPLAY_COUNTDOWN_TIMER].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::DISPLAY_COUNTDOWN_TIMER].m_uiStrings[0] = "Show ";
    m_actionTemplates[ScriptAction::DISPLAY_COUNTDOWN_TIMER].m_uiStrings[1] = " with text ";

    m_actionTemplates[ScriptAction::HIDE_COUNTDOWN_TIMER].m_internalName = "HIDE_COUNTDOWN_TIMER";
    m_actionTemplates[ScriptAction::HIDE_COUNTDOWN_TIMER].m_uiName =
        "Scripting_/ Timer -- hides an individual timer from the user.";
    m_actionTemplates[ScriptAction::HIDE_COUNTDOWN_TIMER].m_numParameters = 1;
    m_actionTemplates[ScriptAction::HIDE_COUNTDOWN_TIMER].m_parameters[0] = Parameter::COUNTER;
    m_actionTemplates[ScriptAction::HIDE_COUNTDOWN_TIMER].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::HIDE_COUNTDOWN_TIMER].m_uiStrings[0] = "Hide ";

    m_actionTemplates[ScriptAction::DISABLE_COUNTDOWN_TIMER_DISPLAY].m_internalName = "DISABLE_COUNTDOWN_TIMER_DISPLAY";
    m_actionTemplates[ScriptAction::DISABLE_COUNTDOWN_TIMER_DISPLAY].m_uiName =
        "Scripting_/ Timer -- hide all timers from the user.";
    m_actionTemplates[ScriptAction::DISABLE_COUNTDOWN_TIMER_DISPLAY].m_numParameters = 0;
    m_actionTemplates[ScriptAction::DISABLE_COUNTDOWN_TIMER_DISPLAY].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::DISABLE_COUNTDOWN_TIMER_DISPLAY].m_uiStrings[0] = "Disables timer display.";

    m_actionTemplates[ScriptAction::ENABLE_COUNTDOWN_TIMER_DISPLAY].m_internalName = "ENABLE_COUNTDOWN_TIMER_DISPLAY";
    m_actionTemplates[ScriptAction::ENABLE_COUNTDOWN_TIMER_DISPLAY].m_uiName =
        "Scripting_/ Timer -- display all timers to the user.";
    m_actionTemplates[ScriptAction::ENABLE_COUNTDOWN_TIMER_DISPLAY].m_numParameters = 0;
    m_actionTemplates[ScriptAction::ENABLE_COUNTDOWN_TIMER_DISPLAY].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::ENABLE_COUNTDOWN_TIMER_DISPLAY].m_uiStrings[0] = "Enables timer display.";

    m_actionTemplates[ScriptAction::NAMED_STOP_SPECIAL_POWER_COUNTDOWN].m_internalName =
        "NAMED_STOP_SPECIAL_POWER_COUNTDOWN";
    m_actionTemplates[ScriptAction::NAMED_STOP_SPECIAL_POWER_COUNTDOWN].m_uiName =
        "Unit_/ Special power countdown timer -- pause.";
    m_actionTemplates[ScriptAction::NAMED_STOP_SPECIAL_POWER_COUNTDOWN].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_STOP_SPECIAL_POWER_COUNTDOWN].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_STOP_SPECIAL_POWER_COUNTDOWN].m_parameters[1] = Parameter::SPECIAL_POWER;
    m_actionTemplates[ScriptAction::NAMED_STOP_SPECIAL_POWER_COUNTDOWN].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::NAMED_STOP_SPECIAL_POWER_COUNTDOWN].m_uiStrings[0] = "Pause ";
    m_actionTemplates[ScriptAction::NAMED_STOP_SPECIAL_POWER_COUNTDOWN].m_uiStrings[1] = "'s ";
    m_actionTemplates[ScriptAction::NAMED_STOP_SPECIAL_POWER_COUNTDOWN].m_uiStrings[2] = " countdown.";

    m_actionTemplates[ScriptAction::NAMED_START_SPECIAL_POWER_COUNTDOWN].m_internalName =
        "NAMED_START_SPECIAL_POWER_COUNTDOWN";
    m_actionTemplates[ScriptAction::NAMED_START_SPECIAL_POWER_COUNTDOWN].m_uiName =
        "Unit_/ Special power countdown timer -- resume.";
    m_actionTemplates[ScriptAction::NAMED_START_SPECIAL_POWER_COUNTDOWN].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_START_SPECIAL_POWER_COUNTDOWN].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_START_SPECIAL_POWER_COUNTDOWN].m_parameters[1] = Parameter::SPECIAL_POWER;
    m_actionTemplates[ScriptAction::NAMED_START_SPECIAL_POWER_COUNTDOWN].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::NAMED_START_SPECIAL_POWER_COUNTDOWN].m_uiStrings[0] = "Resume ";
    m_actionTemplates[ScriptAction::NAMED_START_SPECIAL_POWER_COUNTDOWN].m_uiStrings[1] = "'s ";
    m_actionTemplates[ScriptAction::NAMED_START_SPECIAL_POWER_COUNTDOWN].m_uiStrings[2] = " countdown.";

    m_actionTemplates[ScriptAction::NAMED_SET_SPECIAL_POWER_COUNTDOWN].m_internalName = "NAMED_SET_SPECIAL_POWER_COUNTDOWN";
    m_actionTemplates[ScriptAction::NAMED_SET_SPECIAL_POWER_COUNTDOWN].m_uiName =
        "Unit_/ Special power countdown timer -- set.";
    m_actionTemplates[ScriptAction::NAMED_SET_SPECIAL_POWER_COUNTDOWN].m_numParameters = 3;
    m_actionTemplates[ScriptAction::NAMED_SET_SPECIAL_POWER_COUNTDOWN].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_SET_SPECIAL_POWER_COUNTDOWN].m_parameters[1] = Parameter::SPECIAL_POWER;
    m_actionTemplates[ScriptAction::NAMED_SET_SPECIAL_POWER_COUNTDOWN].m_parameters[2] = Parameter::INT;
    m_actionTemplates[ScriptAction::NAMED_SET_SPECIAL_POWER_COUNTDOWN].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::NAMED_SET_SPECIAL_POWER_COUNTDOWN].m_uiStrings[0] = "Set ";
    m_actionTemplates[ScriptAction::NAMED_SET_SPECIAL_POWER_COUNTDOWN].m_uiStrings[1] = "'s ";
    m_actionTemplates[ScriptAction::NAMED_SET_SPECIAL_POWER_COUNTDOWN].m_uiStrings[2] = " to ";
    m_actionTemplates[ScriptAction::NAMED_SET_SPECIAL_POWER_COUNTDOWN].m_uiStrings[3] = " seconds.";

    m_actionTemplates[ScriptAction::NAMED_ADD_SPECIAL_POWER_COUNTDOWN].m_internalName = "NAMED_ADD_SPECIAL_POWER_COUNTDOWN";
    m_actionTemplates[ScriptAction::NAMED_ADD_SPECIAL_POWER_COUNTDOWN].m_uiName =
        "Unit_/ Special power countdown timer -- add seconds.";
    m_actionTemplates[ScriptAction::NAMED_ADD_SPECIAL_POWER_COUNTDOWN].m_numParameters = 3;
    m_actionTemplates[ScriptAction::NAMED_ADD_SPECIAL_POWER_COUNTDOWN].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_ADD_SPECIAL_POWER_COUNTDOWN].m_parameters[1] = Parameter::SPECIAL_POWER;
    m_actionTemplates[ScriptAction::NAMED_ADD_SPECIAL_POWER_COUNTDOWN].m_parameters[2] = Parameter::INT;
    m_actionTemplates[ScriptAction::NAMED_ADD_SPECIAL_POWER_COUNTDOWN].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::NAMED_ADD_SPECIAL_POWER_COUNTDOWN].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_ADD_SPECIAL_POWER_COUNTDOWN].m_uiStrings[1] = "'s ";
    m_actionTemplates[ScriptAction::NAMED_ADD_SPECIAL_POWER_COUNTDOWN].m_uiStrings[2] = " has ";
    m_actionTemplates[ScriptAction::NAMED_ADD_SPECIAL_POWER_COUNTDOWN].m_uiStrings[3] = " seconds added to it.";

    m_actionTemplates[ScriptAction::SKIRMISH_FIRE_SPECIAL_POWER_AT_MOST_COST].m_internalName =
        "SKIRMISH_FIRE_SPECIAL_POWER_AT_MOST_COST";
    m_actionTemplates[ScriptAction::SKIRMISH_FIRE_SPECIAL_POWER_AT_MOST_COST].m_uiName =
        "Skirmish_/ Special power -- fire at enemy's highest cost area.";
    m_actionTemplates[ScriptAction::SKIRMISH_FIRE_SPECIAL_POWER_AT_MOST_COST].m_numParameters = 2;
    m_actionTemplates[ScriptAction::SKIRMISH_FIRE_SPECIAL_POWER_AT_MOST_COST].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::SKIRMISH_FIRE_SPECIAL_POWER_AT_MOST_COST].m_parameters[1] = Parameter::SPECIAL_POWER;
    m_actionTemplates[ScriptAction::SKIRMISH_FIRE_SPECIAL_POWER_AT_MOST_COST].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::SKIRMISH_FIRE_SPECIAL_POWER_AT_MOST_COST].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::SKIRMISH_FIRE_SPECIAL_POWER_AT_MOST_COST].m_uiStrings[1] = " fire ";
    m_actionTemplates[ScriptAction::SKIRMISH_FIRE_SPECIAL_POWER_AT_MOST_COST].m_uiStrings[2] =
        " at enemy's most costly area.";

    m_actionTemplates[ScriptAction::PLAYER_REPAIR_NAMED_STRUCTURE].m_internalName = "PLAYER_REPAIR_NAMED_STRUCTURE";
    m_actionTemplates[ScriptAction::PLAYER_REPAIR_NAMED_STRUCTURE].m_uiName = "Player_/ Repair named bridge or structure.";
    m_actionTemplates[ScriptAction::PLAYER_REPAIR_NAMED_STRUCTURE].m_numParameters = 2;
    m_actionTemplates[ScriptAction::PLAYER_REPAIR_NAMED_STRUCTURE].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_REPAIR_NAMED_STRUCTURE].m_parameters[1] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::PLAYER_REPAIR_NAMED_STRUCTURE].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::PLAYER_REPAIR_NAMED_STRUCTURE].m_uiStrings[0] = "Have ";
    m_actionTemplates[ScriptAction::PLAYER_REPAIR_NAMED_STRUCTURE].m_uiStrings[1] = " repair ";
    m_actionTemplates[ScriptAction::PLAYER_REPAIR_NAMED_STRUCTURE].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_WAYPOINT].m_internalName =
        "NAMED_FIRE_SPECIAL_POWER_AT_WAYPOINT";
    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_WAYPOINT].m_uiName =
        "Unit_/ Special power -- fire at location.";
    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_WAYPOINT].m_numParameters = 3;
    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_WAYPOINT].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_WAYPOINT].m_parameters[1] = Parameter::SPECIAL_POWER;
    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_WAYPOINT].m_parameters[2] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_WAYPOINT].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_WAYPOINT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_WAYPOINT].m_uiStrings[1] = " fires ";
    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_WAYPOINT].m_uiStrings[2] = " at ";
    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_WAYPOINT].m_uiStrings[3] = ".";

    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_NAMED].m_internalName = "NAMED_FIRE_SPECIAL_POWER_AT_NAMED";
    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_NAMED].m_uiName = "Unit_/ Special power -- fire at unit.";
    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_NAMED].m_numParameters = 3;
    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_NAMED].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_NAMED].m_parameters[1] = Parameter::SPECIAL_POWER;
    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_NAMED].m_parameters[2] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_NAMED].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_NAMED].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_NAMED].m_uiStrings[1] = " fires ";
    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_NAMED].m_uiStrings[2] = " at ";
    m_actionTemplates[ScriptAction::NAMED_FIRE_SPECIAL_POWER_AT_NAMED].m_uiStrings[3] = ".";

    m_actionTemplates[ScriptAction::REFRESH_RADAR].m_internalName = "REFRESH_RADAR";
    m_actionTemplates[ScriptAction::REFRESH_RADAR].m_uiName = "Scripting_/ Refresh radar terrain.";
    m_actionTemplates[ScriptAction::REFRESH_RADAR].m_numParameters = 0;
    m_actionTemplates[ScriptAction::REFRESH_RADAR].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::REFRESH_RADAR].m_uiStrings[0] = "Refresh radar terrain.";

    m_actionTemplates[ScriptAction::NAMED_STOP].m_internalName = "NAMED_STOP";
    m_actionTemplates[ScriptAction::NAMED_STOP].m_uiName = "Unit_/ Set a specific unit to stop.";
    m_actionTemplates[ScriptAction::NAMED_STOP].m_numParameters = 1;
    m_actionTemplates[ScriptAction::NAMED_STOP].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_STOP].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_STOP].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_STOP].m_uiStrings[1] = " stops.";

    m_actionTemplates[ScriptAction::TEAM_STOP].m_internalName = "TEAM_STOP";
    m_actionTemplates[ScriptAction::TEAM_STOP].m_uiName = "Team_/ Set to stop.";
    m_actionTemplates[ScriptAction::TEAM_STOP].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_STOP].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_STOP].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_STOP].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_STOP].m_uiStrings[1] = " stops.";

    m_actionTemplates[ScriptAction::TEAM_STOP_AND_DISBAND].m_internalName = "TEAM_STOP_AND_DISBAND";
    m_actionTemplates[ScriptAction::TEAM_STOP_AND_DISBAND].m_uiName = "Team_/ Set to stop, then disband.";
    m_actionTemplates[ScriptAction::TEAM_STOP_AND_DISBAND].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_STOP_AND_DISBAND].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_STOP_AND_DISBAND].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_STOP_AND_DISBAND].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_STOP_AND_DISBAND].m_uiStrings[1] = " stops, then disbands.";

    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_TEAM].m_internalName =
        "TEAM_SET_OVERRIDE_RELATION_TO_TEAM";
    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_TEAM].m_uiName =
        "Team_/ Override a team's relationship to another team.";
    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_TEAM].m_numParameters = 3;
    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_TEAM].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_TEAM].m_parameters[1] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_TEAM].m_parameters[2] = Parameter::RELATION;
    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_TEAM].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_TEAM].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_TEAM].m_uiStrings[1] = " considers ";
    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_TEAM].m_uiStrings[2] = " to be ";
    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_TEAM].m_uiStrings[3] =
        " (rather than using the the player relationship).";

    m_actionTemplates[ScriptAction::TEAM_REMOVE_OVERRIDE_RELATION_TO_TEAM].m_internalName =
        "TEAM_REMOVE_OVERRIDE_RELATION_TO_TEAM";
    m_actionTemplates[ScriptAction::TEAM_REMOVE_OVERRIDE_RELATION_TO_TEAM].m_uiName =
        "Team_/ Remove an override to a team's relationship to another team.";
    m_actionTemplates[ScriptAction::TEAM_REMOVE_OVERRIDE_RELATION_TO_TEAM].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_REMOVE_OVERRIDE_RELATION_TO_TEAM].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_REMOVE_OVERRIDE_RELATION_TO_TEAM].m_parameters[1] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_REMOVE_OVERRIDE_RELATION_TO_TEAM].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_REMOVE_OVERRIDE_RELATION_TO_TEAM].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_REMOVE_OVERRIDE_RELATION_TO_TEAM].m_uiStrings[1] =
        " uses the player relationship to ";

    m_actionTemplates[ScriptAction::TEAM_REMOVE_ALL_OVERRIDE_RELATIONS].m_internalName =
        "TEAM_REMOVE_ALL_OVERRIDE_RELATIONS";
    m_actionTemplates[ScriptAction::TEAM_REMOVE_ALL_OVERRIDE_RELATIONS].m_uiName =
        "Team_/ Remove all overrides to team's relationship to teams and/or players.";
    m_actionTemplates[ScriptAction::TEAM_REMOVE_ALL_OVERRIDE_RELATIONS].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_REMOVE_ALL_OVERRIDE_RELATIONS].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_REMOVE_ALL_OVERRIDE_RELATIONS].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_REMOVE_ALL_OVERRIDE_RELATIONS].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_REMOVE_ALL_OVERRIDE_RELATIONS].m_uiStrings[1] =
        " uses the player relationship to all other teams and players.";

    m_actionTemplates[ScriptAction::CAMERA_TETHER_NAMED].m_internalName = "CAMERA_TETHER_NAMED";
    m_actionTemplates[ScriptAction::CAMERA_TETHER_NAMED].m_uiName = "Camera_/ Tether camera to a specific unit.";
    m_actionTemplates[ScriptAction::CAMERA_TETHER_NAMED].m_numParameters = 3;
    m_actionTemplates[ScriptAction::CAMERA_TETHER_NAMED].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::CAMERA_TETHER_NAMED].m_parameters[1] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::CAMERA_TETHER_NAMED].m_parameters[2] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_TETHER_NAMED].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::CAMERA_TETHER_NAMED].m_uiStrings[0] = "Have the camera tethered to ";
    m_actionTemplates[ScriptAction::CAMERA_TETHER_NAMED].m_uiStrings[1] = ".  Snap camera to object is ";
    m_actionTemplates[ScriptAction::CAMERA_TETHER_NAMED].m_uiStrings[2] = ".  Amount of play is ";
    m_actionTemplates[ScriptAction::CAMERA_TETHER_NAMED].m_uiStrings[3] = ".";

    m_actionTemplates[ScriptAction::CAMERA_STOP_TETHER_NAMED].m_internalName = "CAMERA_STOP_TETHER_NAMED";
    m_actionTemplates[ScriptAction::CAMERA_STOP_TETHER_NAMED].m_uiName = "Camera_/ Stop tether to any units.";
    m_actionTemplates[ScriptAction::CAMERA_STOP_TETHER_NAMED].m_numParameters = 0;
    m_actionTemplates[ScriptAction::CAMERA_STOP_TETHER_NAMED].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::CAMERA_STOP_TETHER_NAMED].m_uiStrings[0] = "Stop tether to any units.";

    m_actionTemplates[ScriptAction::CAMERA_SET_DEFAULT].m_internalName = "CAMERA_SET_DEFAULT";
    m_actionTemplates[ScriptAction::CAMERA_SET_DEFAULT].m_uiName = "Camera_/ Set default camera.";
    m_actionTemplates[ScriptAction::CAMERA_SET_DEFAULT].m_numParameters = 3;
    m_actionTemplates[ScriptAction::CAMERA_SET_DEFAULT].m_parameters[0] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_SET_DEFAULT].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_SET_DEFAULT].m_parameters[2] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_SET_DEFAULT].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::CAMERA_SET_DEFAULT].m_uiStrings[0] = "Camera Pitch = ";
    m_actionTemplates[ScriptAction::CAMERA_SET_DEFAULT].m_uiStrings[1] = "(0.0==default), angle = ";
    m_actionTemplates[ScriptAction::CAMERA_SET_DEFAULT].m_uiStrings[2] = "(0.0 is N, 90.0 is W, etc), height = ";
    m_actionTemplates[ScriptAction::CAMERA_SET_DEFAULT].m_uiStrings[3] = "(1.0==default).";

    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_OBJECT].m_internalName = "CAMERA_LOOK_TOWARD_OBJECT";
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_OBJECT].m_uiName = "Camera (R)_/ Rotate toward unit.";
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_OBJECT].m_numParameters = 5;
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_OBJECT].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_OBJECT].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_OBJECT].m_parameters[2] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_OBJECT].m_parameters[3] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_OBJECT].m_parameters[4] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_OBJECT].m_numUiStrings = 6;
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_OBJECT].m_uiStrings[0] = "Rotate toward ";
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_OBJECT].m_uiStrings[1] = ", taking ";
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_OBJECT].m_uiStrings[2] = " seconds and holding ";
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_OBJECT].m_uiStrings[3] = " seconds, ease-in ";
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_OBJECT].m_uiStrings[4] = " seconds, ease-out ";
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_OBJECT].m_uiStrings[5] = " seconds.";

    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_WAYPOINT].m_internalName = "CAMERA_LOOK_TOWARD_WAYPOINT";
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_WAYPOINT].m_uiName = "Camera (R)_/ Rotate to look at a waypoint.";
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_WAYPOINT].m_numParameters = 5;
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_WAYPOINT].m_parameters[0] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_WAYPOINT].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_WAYPOINT].m_parameters[2] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_WAYPOINT].m_parameters[3] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_WAYPOINT].m_parameters[4] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_WAYPOINT].m_numUiStrings = 6;
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_WAYPOINT].m_uiStrings[0] = "Rotate to look at ";
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_WAYPOINT].m_uiStrings[1] = ", taking ";
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_WAYPOINT].m_uiStrings[2] = " seconds, ease-in ";
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_WAYPOINT].m_uiStrings[3] = " seconds, ease-out ";
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_WAYPOINT].m_uiStrings[4] = " seconds, reverse rotation ";
    m_actionTemplates[ScriptAction::CAMERA_LOOK_TOWARD_WAYPOINT].m_uiStrings[5] = ".";

    m_actionTemplates[ScriptAction::UNIT_DESTROY_ALL_CONTAINED].m_internalName = "UNIT_DESTROY_ALL_CONTAINED";
    m_actionTemplates[ScriptAction::UNIT_DESTROY_ALL_CONTAINED].m_uiName =
        "Unit_/ Kill all units contained within a specific transport or structure.";
    m_actionTemplates[ScriptAction::UNIT_DESTROY_ALL_CONTAINED].m_numParameters = 1;
    m_actionTemplates[ScriptAction::UNIT_DESTROY_ALL_CONTAINED].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::UNIT_DESTROY_ALL_CONTAINED].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::UNIT_DESTROY_ALL_CONTAINED].m_uiStrings[0] = "All units inside ";
    m_actionTemplates[ScriptAction::UNIT_DESTROY_ALL_CONTAINED].m_uiStrings[1] = " are killed.";

    m_actionTemplates[ScriptAction::NAMED_FIRE_WEAPON_FOLLOWING_WAYPOINT_PATH].m_internalName =
        "NAMED_FIRE_WEAPON_FOLLOWING_WAYPOINT_PATH";
    m_actionTemplates[ScriptAction::NAMED_FIRE_WEAPON_FOLLOWING_WAYPOINT_PATH].m_uiName =
        "Unit_/ Fire waypoint-weapon following waypoint path.";
    m_actionTemplates[ScriptAction::NAMED_FIRE_WEAPON_FOLLOWING_WAYPOINT_PATH].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_FIRE_WEAPON_FOLLOWING_WAYPOINT_PATH].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_FIRE_WEAPON_FOLLOWING_WAYPOINT_PATH].m_parameters[1] = Parameter::WAYPOINT_PATH;
    m_actionTemplates[ScriptAction::NAMED_FIRE_WEAPON_FOLLOWING_WAYPOINT_PATH].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::NAMED_FIRE_WEAPON_FOLLOWING_WAYPOINT_PATH].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_FIRE_WEAPON_FOLLOWING_WAYPOINT_PATH].m_uiStrings[1] =
        " fire waypoint-weapon following waypoints starting at ";
    m_actionTemplates[ScriptAction::NAMED_FIRE_WEAPON_FOLLOWING_WAYPOINT_PATH].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_PLAYER].m_internalName =
        "TEAM_SET_OVERRIDE_RELATION_TO_PLAYER";
    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_PLAYER].m_uiName =
        "Team_/ Override a team's relationship to another player.";
    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_PLAYER].m_numParameters = 3;
    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_PLAYER].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_PLAYER].m_parameters[1] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_PLAYER].m_parameters[2] = Parameter::RELATION;
    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_PLAYER].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_PLAYER].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_PLAYER].m_uiStrings[1] = " considers ";
    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_PLAYER].m_uiStrings[2] = " to be ";
    m_actionTemplates[ScriptAction::TEAM_SET_OVERRIDE_RELATION_TO_PLAYER].m_uiStrings[3] =
        " (rather than using the the player relationship).";

    m_actionTemplates[ScriptAction::TEAM_REMOVE_OVERRIDE_RELATION_TO_PLAYER].m_internalName =
        "TEAM_REMOVE_OVERRIDE_RELATION_TO_PLAYER";
    m_actionTemplates[ScriptAction::TEAM_REMOVE_OVERRIDE_RELATION_TO_PLAYER].m_uiName =
        "Team_/ Remove an override to a team's relationship to another player.";
    m_actionTemplates[ScriptAction::TEAM_REMOVE_OVERRIDE_RELATION_TO_PLAYER].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_REMOVE_OVERRIDE_RELATION_TO_PLAYER].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_REMOVE_OVERRIDE_RELATION_TO_PLAYER].m_parameters[1] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::TEAM_REMOVE_OVERRIDE_RELATION_TO_PLAYER].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_REMOVE_OVERRIDE_RELATION_TO_PLAYER].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_REMOVE_OVERRIDE_RELATION_TO_PLAYER].m_uiStrings[1] =
        " uses the player relationship to ";

    m_actionTemplates[ScriptAction::PLAYER_SET_OVERRIDE_RELATION_TO_TEAM].m_internalName =
        "PLAYER_SET_OVERRIDE_RELATION_TO_TEAM";
    m_actionTemplates[ScriptAction::PLAYER_SET_OVERRIDE_RELATION_TO_TEAM].m_uiName =
        "Player_/ Override a player's relationship to another team.";
    m_actionTemplates[ScriptAction::PLAYER_SET_OVERRIDE_RELATION_TO_TEAM].m_numParameters = 3;
    m_actionTemplates[ScriptAction::PLAYER_SET_OVERRIDE_RELATION_TO_TEAM].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_SET_OVERRIDE_RELATION_TO_TEAM].m_parameters[1] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::PLAYER_SET_OVERRIDE_RELATION_TO_TEAM].m_parameters[2] = Parameter::RELATION;
    m_actionTemplates[ScriptAction::PLAYER_SET_OVERRIDE_RELATION_TO_TEAM].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::PLAYER_SET_OVERRIDE_RELATION_TO_TEAM].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_SET_OVERRIDE_RELATION_TO_TEAM].m_uiStrings[1] = " considers ";
    m_actionTemplates[ScriptAction::PLAYER_SET_OVERRIDE_RELATION_TO_TEAM].m_uiStrings[2] = " to be ";
    m_actionTemplates[ScriptAction::PLAYER_SET_OVERRIDE_RELATION_TO_TEAM].m_uiStrings[3] =
        " (rather than using the the player relationship).";

    m_actionTemplates[ScriptAction::PLAYER_REMOVE_OVERRIDE_RELATION_TO_TEAM].m_internalName =
        "PLAYER_REMOVE_OVERRIDE_RELATION_TO_TEAM";
    m_actionTemplates[ScriptAction::PLAYER_REMOVE_OVERRIDE_RELATION_TO_TEAM].m_uiName =
        "Player_/ Remove an override to a player's relationship to another team.";
    m_actionTemplates[ScriptAction::PLAYER_REMOVE_OVERRIDE_RELATION_TO_TEAM].m_numParameters = 2;
    m_actionTemplates[ScriptAction::PLAYER_REMOVE_OVERRIDE_RELATION_TO_TEAM].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_REMOVE_OVERRIDE_RELATION_TO_TEAM].m_parameters[1] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::PLAYER_REMOVE_OVERRIDE_RELATION_TO_TEAM].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::PLAYER_REMOVE_OVERRIDE_RELATION_TO_TEAM].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_REMOVE_OVERRIDE_RELATION_TO_TEAM].m_uiStrings[1] =
        " uses the player relationship to ";

    m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT].m_internalName = "UNIT_EXECUTE_SEQUENTIAL_SCRIPT";
    m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT].m_uiName =
        "Unit_/ Set a specific unit to execute a script sequentially.";
    m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT].m_numParameters = 2;
    m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT].m_parameters[1] = Parameter::SCRIPT;
    m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT].m_uiStrings[1] = " executes ";
    m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT].m_uiStrings[2] = " sequentially.";

    m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_internalName =
        "UNIT_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING";
    m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_uiName =
        "Unit_/ Set a specific unit to execute a looping sequential script.";
    m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_numParameters = 3;
    m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_parameters[1] = Parameter::SCRIPT;
    m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_parameters[2] = Parameter::INT;
    m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_uiStrings[1] = " executes ";
    m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_uiStrings[2] = " sequentially, ";
    m_actionTemplates[ScriptAction::UNIT_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_uiStrings[3] = " times. (0=forever)";

    m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT].m_internalName = "TEAM_EXECUTE_SEQUENTIAL_SCRIPT";
    m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT].m_uiName =
        "Team_/ Execute script sequentially -- start.";
    m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT].m_parameters[1] = Parameter::SCRIPT;
    m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT].m_uiStrings[1] = " executes ";
    m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT].m_uiStrings[2] = " sequentially.";

    m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_internalName =
        "TEAM_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING";
    m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_uiName =
        "Team_/ Execute script sequentially -- looping.";
    m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_numParameters = 3;
    m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_parameters[1] = Parameter::SCRIPT;
    m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_parameters[2] = Parameter::INT;
    m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_uiStrings[1] = " executes ";
    m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_uiStrings[2] = " sequentially, ";
    m_actionTemplates[ScriptAction::TEAM_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING].m_uiStrings[3] = " times. (0=forever)";

    m_actionTemplates[ScriptAction::UNIT_STOP_SEQUENTIAL_SCRIPT].m_internalName = "UNIT_STOP_SEQUENTIAL_SCRIPT";
    m_actionTemplates[ScriptAction::UNIT_STOP_SEQUENTIAL_SCRIPT].m_uiName =
        "Unit_/ Set a specific unit to stop executing a sequential script.";
    m_actionTemplates[ScriptAction::UNIT_STOP_SEQUENTIAL_SCRIPT].m_numParameters = 1;
    m_actionTemplates[ScriptAction::UNIT_STOP_SEQUENTIAL_SCRIPT].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::UNIT_STOP_SEQUENTIAL_SCRIPT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::UNIT_STOP_SEQUENTIAL_SCRIPT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::UNIT_STOP_SEQUENTIAL_SCRIPT].m_uiStrings[1] = " stops executing.";

    m_actionTemplates[ScriptAction::TEAM_STOP_SEQUENTIAL_SCRIPT].m_internalName = "TEAM_STOP_SEQUENTIAL_SCRIPT";
    m_actionTemplates[ScriptAction::TEAM_STOP_SEQUENTIAL_SCRIPT].m_uiName = "Team_/ Execute script sequentially -- stop.";
    m_actionTemplates[ScriptAction::TEAM_STOP_SEQUENTIAL_SCRIPT].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_STOP_SEQUENTIAL_SCRIPT].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_STOP_SEQUENTIAL_SCRIPT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_STOP_SEQUENTIAL_SCRIPT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_STOP_SEQUENTIAL_SCRIPT].m_uiStrings[1] = " stops executing.";

    m_actionTemplates[ScriptAction::UNIT_GUARD_FOR_FRAMECOUNT].m_internalName = "UNIT_GUARD_FOR_FRAMECOUNT";
    m_actionTemplates[ScriptAction::UNIT_GUARD_FOR_FRAMECOUNT].m_uiName = "Unit_/ Set to guard for some number of frames.";
    m_actionTemplates[ScriptAction::UNIT_GUARD_FOR_FRAMECOUNT].m_numParameters = 2;
    m_actionTemplates[ScriptAction::UNIT_GUARD_FOR_FRAMECOUNT].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::UNIT_GUARD_FOR_FRAMECOUNT].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::UNIT_GUARD_FOR_FRAMECOUNT].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::UNIT_GUARD_FOR_FRAMECOUNT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::UNIT_GUARD_FOR_FRAMECOUNT].m_uiStrings[1] = " guards for ";
    m_actionTemplates[ScriptAction::UNIT_GUARD_FOR_FRAMECOUNT].m_uiStrings[2] = " frames.";

    m_actionTemplates[ScriptAction::UNIT_IDLE_FOR_FRAMECOUNT].m_internalName = "UNIT_IDLE_FOR_FRAMECOUNT";
    m_actionTemplates[ScriptAction::UNIT_IDLE_FOR_FRAMECOUNT].m_uiName = "Unit_/ Set to idle for some number of frames.";
    m_actionTemplates[ScriptAction::UNIT_IDLE_FOR_FRAMECOUNT].m_numParameters = 2;
    m_actionTemplates[ScriptAction::UNIT_IDLE_FOR_FRAMECOUNT].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::UNIT_IDLE_FOR_FRAMECOUNT].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::UNIT_IDLE_FOR_FRAMECOUNT].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::UNIT_IDLE_FOR_FRAMECOUNT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::UNIT_IDLE_FOR_FRAMECOUNT].m_uiStrings[1] = " idles for ";
    m_actionTemplates[ScriptAction::UNIT_IDLE_FOR_FRAMECOUNT].m_uiStrings[2] = " frames.";

    m_actionTemplates[ScriptAction::TEAM_GUARD_FOR_FRAMECOUNT].m_internalName = "TEAM_GUARD_FOR_FRAMECOUNT";
    m_actionTemplates[ScriptAction::TEAM_GUARD_FOR_FRAMECOUNT].m_uiName = "Team_/ Set to guard -- number of frames.";
    m_actionTemplates[ScriptAction::TEAM_GUARD_FOR_FRAMECOUNT].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_GUARD_FOR_FRAMECOUNT].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_GUARD_FOR_FRAMECOUNT].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::TEAM_GUARD_FOR_FRAMECOUNT].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_GUARD_FOR_FRAMECOUNT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_GUARD_FOR_FRAMECOUNT].m_uiStrings[1] = " guards for ";
    m_actionTemplates[ScriptAction::TEAM_GUARD_FOR_FRAMECOUNT].m_uiStrings[2] = " frames.";

    m_actionTemplates[ScriptAction::TEAM_IDLE_FOR_FRAMECOUNT].m_internalName = "TEAM_IDLE_FOR_FRAMECOUNT";
    m_actionTemplates[ScriptAction::TEAM_IDLE_FOR_FRAMECOUNT].m_uiName = "Team_/ Set to idle for some number of frames.";
    m_actionTemplates[ScriptAction::TEAM_IDLE_FOR_FRAMECOUNT].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_IDLE_FOR_FRAMECOUNT].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_IDLE_FOR_FRAMECOUNT].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::TEAM_IDLE_FOR_FRAMECOUNT].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_IDLE_FOR_FRAMECOUNT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_IDLE_FOR_FRAMECOUNT].m_uiStrings[1] = " idles for ";
    m_actionTemplates[ScriptAction::TEAM_IDLE_FOR_FRAMECOUNT].m_uiStrings[2] = " frames.";

    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT].m_internalName = "WATER_CHANGE_HEIGHT";
    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT].m_uiName = "Map_/ Adjust water height to a new level";
    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT].m_numParameters = 2;
    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT].m_parameters[0] = Parameter::TRIGGER_AREA;
    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT].m_uiStrings[1] = " changes altitude to ";

    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT_OVER_TIME].m_internalName = "WATER_CHANGE_HEIGHT_OVER_TIME";
    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT_OVER_TIME].m_uiName =
        "Map_/ Adjust water height to a new level with damage over time";
    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT_OVER_TIME].m_numParameters = 4;
    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT_OVER_TIME].m_parameters[0] = Parameter::TRIGGER_AREA;
    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT_OVER_TIME].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT_OVER_TIME].m_parameters[2] = Parameter::REAL;
    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT_OVER_TIME].m_parameters[3] = Parameter::REAL;
    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT_OVER_TIME].m_numUiStrings = 5;
    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT_OVER_TIME].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT_OVER_TIME].m_uiStrings[1] = " changes altitude to ";
    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT_OVER_TIME].m_uiStrings[2] = " in ";
    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT_OVER_TIME].m_uiStrings[3] = " seconds doing ";
    m_actionTemplates[ScriptAction::WATER_CHANGE_HEIGHT_OVER_TIME].m_uiStrings[4] = " dam_/sec.";

    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY].m_internalName = "NAMED_USE_COMMANDBUTTON_ABILITY";
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY].m_uiName = "Unit_/ Use commandbutton ability.";
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY].m_parameters[1] = Parameter::COMMANDBUTTON_ABILITY;
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY].m_uiStrings[1] = " use ";
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_internalName =
        "NAMED_USE_COMMANDBUTTON_ABILITY_ON_NAMED";
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_uiName =
        "Unit_/ Use commandbutton ability on an object.";
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_numParameters = 3;
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_parameters[1] =
        Parameter::COMMANDBUTTON_ABILITY;
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_parameters[2] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_uiStrings[1] = " use ";
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_uiStrings[2] = " on ";
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_uiStrings[3] = ".";

    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_internalName =
        "NAMED_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT";
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_uiName =
        "Unit_/ Use commandbutton ability at a waypoint.";
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_numParameters = 3;
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_parameters[1] =
        Parameter::COMMANDBUTTON_ABILITY;
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_parameters[2] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_uiStrings[1] = " use ";
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_uiStrings[2] = " at ";
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_uiStrings[3] = ".";

    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_USING_WAYPOINT_PATH].m_internalName =
        "NAMED_USE_COMMANDBUTTON_ABILITY_USING_WAYPOINT_PATH";
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_USING_WAYPOINT_PATH].m_uiName =
        "Unit_/ Use commandbutton ability using a waypoint path.";
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_USING_WAYPOINT_PATH].m_numParameters = 3;
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_USING_WAYPOINT_PATH].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_USING_WAYPOINT_PATH].m_parameters[1] =
        Parameter::COMMANDBUTTON_ABILITY;
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_USING_WAYPOINT_PATH].m_parameters[2] =
        Parameter::WAYPOINT_PATH;
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_USING_WAYPOINT_PATH].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_USING_WAYPOINT_PATH].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_USING_WAYPOINT_PATH].m_uiStrings[1] = " use ";
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_USING_WAYPOINT_PATH].m_uiStrings[2] = " to follow ";
    m_actionTemplates[ScriptAction::NAMED_USE_COMMANDBUTTON_ABILITY_USING_WAYPOINT_PATH].m_uiStrings[3] = " path.";

    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY].m_internalName = "TEAM_USE_COMMANDBUTTON_ABILITY";
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY].m_uiName = "Team_/ Use commandbutton ability.";
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY].m_parameters[1] = Parameter::TEAM_COMMANDBUTTON_ABILITY;
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY].m_uiStrings[1] = " use ";
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_internalName =
        "TEAM_USE_COMMANDBUTTON_ABILITY_ON_NAMED";
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_uiName =
        "Team_/ Use commandbutton ability on an object.";
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_numParameters = 3;
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_parameters[1] =
        Parameter::TEAM_COMMANDBUTTON_ABILITY;
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_parameters[2] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_uiStrings[1] = " use ";
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_uiStrings[2] = " on ";
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_ON_NAMED].m_uiStrings[3] = ".";

    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_internalName =
        "TEAM_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT";
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_uiName =
        "Team_/ Use commandbutton ability at a waypoint.";
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_numParameters = 3;
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_parameters[1] =
        Parameter::TEAM_COMMANDBUTTON_ABILITY;
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_parameters[2] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_uiStrings[1] = " use ";
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_uiStrings[2] = " at ";
    m_actionTemplates[ScriptAction::TEAM_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT].m_uiStrings[3] = ".";

    m_actionTemplates[ScriptAction::MAP_SWITCH_BORDER].m_internalName = "MAP_SWITCH_BORDER";
    m_actionTemplates[ScriptAction::MAP_SWITCH_BORDER].m_uiName = "Map_/ Change the active boundary.";
    m_actionTemplates[ScriptAction::MAP_SWITCH_BORDER].m_numParameters = 1;
    m_actionTemplates[ScriptAction::MAP_SWITCH_BORDER].m_parameters[0] = Parameter::BOUNDARY;
    m_actionTemplates[ScriptAction::MAP_SWITCH_BORDER].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::MAP_SWITCH_BORDER].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::MAP_SWITCH_BORDER].m_uiStrings[1] = " becomes the active border.";

    m_actionTemplates[ScriptAction::OBJECT_FORCE_SELECT].m_internalName = "OBJECT_FORCE_SELECT";
    m_actionTemplates[ScriptAction::OBJECT_FORCE_SELECT].m_uiName = "Scripting_/ Select the first object type on a team.";
    m_actionTemplates[ScriptAction::OBJECT_FORCE_SELECT].m_numParameters = 4;
    m_actionTemplates[ScriptAction::OBJECT_FORCE_SELECT].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::OBJECT_FORCE_SELECT].m_parameters[1] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::OBJECT_FORCE_SELECT].m_parameters[2] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::OBJECT_FORCE_SELECT].m_parameters[3] = Parameter::DIALOG;
    m_actionTemplates[ScriptAction::OBJECT_FORCE_SELECT].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::OBJECT_FORCE_SELECT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::OBJECT_FORCE_SELECT].m_uiStrings[1] = " 's first ";
    m_actionTemplates[ScriptAction::OBJECT_FORCE_SELECT].m_uiStrings[2] = ", centers in view (";
    m_actionTemplates[ScriptAction::OBJECT_FORCE_SELECT].m_uiStrings[3] = ") while playing ";

    m_actionTemplates[ScriptAction::RADAR_FORCE_ENABLE].m_internalName = "RADAR_FORCE_ENABLE";
    m_actionTemplates[ScriptAction::RADAR_FORCE_ENABLE].m_uiName = "Radar_/ Force enable the radar.";
    m_actionTemplates[ScriptAction::RADAR_FORCE_ENABLE].m_numParameters = 0;
    m_actionTemplates[ScriptAction::RADAR_FORCE_ENABLE].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::RADAR_FORCE_ENABLE].m_uiStrings[0] = "The radar is now forced to be enabled.";

    m_actionTemplates[ScriptAction::RADAR_REVERT_TO_NORMAL].m_internalName = "RADAR_REVERT_TO_NORMAL";
    m_actionTemplates[ScriptAction::RADAR_REVERT_TO_NORMAL].m_uiName = "Radar_/ Revert radar to normal behavior.";
    m_actionTemplates[ScriptAction::RADAR_REVERT_TO_NORMAL].m_numParameters = 0;
    m_actionTemplates[ScriptAction::RADAR_REVERT_TO_NORMAL].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::RADAR_REVERT_TO_NORMAL].m_uiStrings[0] =
        "The radar is now reverting to its normal behavior.";

    m_actionTemplates[ScriptAction::SCREEN_SHAKE].m_internalName = "SCREEN_SHAKE";
    m_actionTemplates[ScriptAction::SCREEN_SHAKE].m_uiName = "Camera_/ Shake Screen.";
    m_actionTemplates[ScriptAction::SCREEN_SHAKE].m_numParameters = 1;
    m_actionTemplates[ScriptAction::SCREEN_SHAKE].m_parameters[0] = Parameter::SHAKE_INTENSITY;
    m_actionTemplates[ScriptAction::SCREEN_SHAKE].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::SCREEN_SHAKE].m_uiStrings[0] = "The screen will shake with ";

    m_actionTemplates[ScriptAction::TECHTREE_MODIFY_BUILDABILITY_OBJECT].m_internalName =
        "TECHTREE_MODIFY_BUILDABILITY_OBJECT";
    m_actionTemplates[ScriptAction::TECHTREE_MODIFY_BUILDABILITY_OBJECT].m_uiName =
        "Map_/ Adjust the tech tree for a specific object type.";
    m_actionTemplates[ScriptAction::TECHTREE_MODIFY_BUILDABILITY_OBJECT].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TECHTREE_MODIFY_BUILDABILITY_OBJECT].m_parameters[0] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::TECHTREE_MODIFY_BUILDABILITY_OBJECT].m_parameters[1] = Parameter::BUILDABLE;
    m_actionTemplates[ScriptAction::TECHTREE_MODIFY_BUILDABILITY_OBJECT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TECHTREE_MODIFY_BUILDABILITY_OBJECT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TECHTREE_MODIFY_BUILDABILITY_OBJECT].m_uiStrings[1] = " becomes ";

    m_actionTemplates[ScriptAction::SET_CAVE_INDEX].m_internalName = "SET_CAVE_INDEX";
    m_actionTemplates[ScriptAction::SET_CAVE_INDEX].m_uiName = "Unit_/ Set Cave connectivity index.";
    m_actionTemplates[ScriptAction::SET_CAVE_INDEX].m_numParameters = 2;
    m_actionTemplates[ScriptAction::SET_CAVE_INDEX].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::SET_CAVE_INDEX].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::SET_CAVE_INDEX].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::SET_CAVE_INDEX].m_uiStrings[0] = "Cave named ";
    m_actionTemplates[ScriptAction::SET_CAVE_INDEX].m_uiStrings[1] = " is set to being connected to all caves of index ";
    m_actionTemplates[ScriptAction::SET_CAVE_INDEX].m_uiStrings[2] = ", but only if both Cave listings have no occupants. ";

    m_actionTemplates[ScriptAction::WAREHOUSE_SET_VALUE].m_internalName = "WAREHOUSE_SET_VALUE";
    m_actionTemplates[ScriptAction::WAREHOUSE_SET_VALUE].m_uiName = "Unit_/ Set cash value of Warehouse.";
    m_actionTemplates[ScriptAction::WAREHOUSE_SET_VALUE].m_numParameters = 2;
    m_actionTemplates[ScriptAction::WAREHOUSE_SET_VALUE].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::WAREHOUSE_SET_VALUE].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::WAREHOUSE_SET_VALUE].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::WAREHOUSE_SET_VALUE].m_uiStrings[0] = "Warehouse named ";
    m_actionTemplates[ScriptAction::WAREHOUSE_SET_VALUE].m_uiStrings[1] = " is set to having ";
    m_actionTemplates[ScriptAction::WAREHOUSE_SET_VALUE].m_uiStrings[2] = " dollars worth of boxes. ";

    m_actionTemplates[ScriptAction::SOUND_DISABLE_TYPE].m_internalName = "SOUND_DISABLE_TYPE";
    m_actionTemplates[ScriptAction::SOUND_DISABLE_TYPE].m_uiName = "Multimedia_/ Sound Events -- disable type.";
    m_actionTemplates[ScriptAction::SOUND_DISABLE_TYPE].m_numParameters = 1;
    m_actionTemplates[ScriptAction::SOUND_DISABLE_TYPE].m_parameters[0] = Parameter::SOUND;
    m_actionTemplates[ScriptAction::SOUND_DISABLE_TYPE].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::SOUND_DISABLE_TYPE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::SOUND_DISABLE_TYPE].m_uiStrings[1] = " is disabled.";

    m_actionTemplates[ScriptAction::SOUND_ENABLE_TYPE].m_internalName = "SOUND_ENABLE_TYPE";
    m_actionTemplates[ScriptAction::SOUND_ENABLE_TYPE].m_uiName = "Multimedia_/ Sound Events -- enable type.";
    m_actionTemplates[ScriptAction::SOUND_ENABLE_TYPE].m_numParameters = 1;
    m_actionTemplates[ScriptAction::SOUND_ENABLE_TYPE].m_parameters[0] = Parameter::SOUND;
    m_actionTemplates[ScriptAction::SOUND_ENABLE_TYPE].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::SOUND_ENABLE_TYPE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::SOUND_ENABLE_TYPE].m_uiStrings[1] = " is enabled.";

    m_actionTemplates[ScriptAction::SOUND_REMOVE_TYPE].m_internalName = "SOUND_REMOVE_TYPE";
    m_actionTemplates[ScriptAction::SOUND_REMOVE_TYPE].m_uiName = "Multimedia_/ Sound Events -- remove type.";
    m_actionTemplates[ScriptAction::SOUND_REMOVE_TYPE].m_numParameters = 1;
    m_actionTemplates[ScriptAction::SOUND_REMOVE_TYPE].m_parameters[0] = Parameter::SOUND;
    m_actionTemplates[ScriptAction::SOUND_REMOVE_TYPE].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::SOUND_REMOVE_TYPE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::SOUND_REMOVE_TYPE].m_uiStrings[1] = " is removed.";

    m_actionTemplates[ScriptAction::SOUND_REMOVE_ALL_DISABLED].m_internalName = "SOUND_REMOVE_ALL_DISABLED";
    m_actionTemplates[ScriptAction::SOUND_REMOVE_ALL_DISABLED].m_uiName =
        "Multimedia_/ Sound Events -- remove all disabled.";
    m_actionTemplates[ScriptAction::SOUND_REMOVE_ALL_DISABLED].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::SOUND_REMOVE_ALL_DISABLED].m_uiStrings[0] = "Remove all disabled sound events.";

    m_actionTemplates[ScriptAction::SOUND_ENABLE_ALL].m_internalName = "SOUND_ENABLE_ALL";
    m_actionTemplates[ScriptAction::SOUND_ENABLE_ALL].m_uiName = "Multimedia_/ Sound Events -- enable all.";
    m_actionTemplates[ScriptAction::SOUND_ENABLE_ALL].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::SOUND_ENABLE_ALL].m_uiStrings[0] = "Enable all sound events.";

    m_actionTemplates[ScriptAction::AUDIO_OVERRIDE_VOLUME_TYPE].m_internalName = "AUDIO_OVERRIDE_VOLUME_TYPE";
    m_actionTemplates[ScriptAction::AUDIO_OVERRIDE_VOLUME_TYPE].m_uiName =
        "Multimedia_/ Sound Events -- override volume -- type.";
    m_actionTemplates[ScriptAction::AUDIO_OVERRIDE_VOLUME_TYPE].m_numParameters = 2;
    m_actionTemplates[ScriptAction::AUDIO_OVERRIDE_VOLUME_TYPE].m_parameters[0] = Parameter::SOUND;
    m_actionTemplates[ScriptAction::AUDIO_OVERRIDE_VOLUME_TYPE].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::AUDIO_OVERRIDE_VOLUME_TYPE].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::AUDIO_OVERRIDE_VOLUME_TYPE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::AUDIO_OVERRIDE_VOLUME_TYPE].m_uiStrings[1] = " play at ";
    m_actionTemplates[ScriptAction::AUDIO_OVERRIDE_VOLUME_TYPE].m_uiStrings[2] = "% of full volume.";

    m_actionTemplates[ScriptAction::AUDIO_RESTORE_VOLUME_TYPE].m_internalName = "AUDIO_RESTORE_VOLUME_TYPE";
    m_actionTemplates[ScriptAction::AUDIO_RESTORE_VOLUME_TYPE].m_uiName =
        "Multimedia_/ Sound Events -- restore volume -- type.";
    m_actionTemplates[ScriptAction::AUDIO_RESTORE_VOLUME_TYPE].m_numParameters = 1;
    m_actionTemplates[ScriptAction::AUDIO_RESTORE_VOLUME_TYPE].m_parameters[0] = Parameter::SOUND;
    m_actionTemplates[ScriptAction::AUDIO_RESTORE_VOLUME_TYPE].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::AUDIO_RESTORE_VOLUME_TYPE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::AUDIO_RESTORE_VOLUME_TYPE].m_uiStrings[1] = " play at normal volume.";

    m_actionTemplates[ScriptAction::AUDIO_RESTORE_VOLUME_ALL_TYPE].m_internalName = "AUDIO_RESTORE_VOLUME_ALL_TYPE";
    m_actionTemplates[ScriptAction::AUDIO_RESTORE_VOLUME_ALL_TYPE].m_uiName =
        "Multimedia_/ Sound Events -- restore volume -- all.";
    m_actionTemplates[ScriptAction::AUDIO_RESTORE_VOLUME_ALL_TYPE].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::AUDIO_RESTORE_VOLUME_ALL_TYPE].m_uiStrings[0] =
        "All sound events play at normal volume.";

    m_actionTemplates[ScriptAction::NAMED_SET_TOPPLE_DIRECTION].m_internalName = "NAMED_SET_TOPPLE_DIRECTION";
    m_actionTemplates[ScriptAction::NAMED_SET_TOPPLE_DIRECTION].m_uiName = "Unit_/ Set topple direction.";
    m_actionTemplates[ScriptAction::NAMED_SET_TOPPLE_DIRECTION].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_SET_TOPPLE_DIRECTION].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_SET_TOPPLE_DIRECTION].m_parameters[1] = Parameter::COORD3D;
    m_actionTemplates[ScriptAction::NAMED_SET_TOPPLE_DIRECTION].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::NAMED_SET_TOPPLE_DIRECTION].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_SET_TOPPLE_DIRECTION].m_uiStrings[1] = " will topple towards ";
    m_actionTemplates[ScriptAction::NAMED_SET_TOPPLE_DIRECTION].m_uiStrings[2] = " if destroyed.";

    m_actionTemplates[ScriptAction::UNIT_MOVE_TOWARDS_NEAREST_OBJECT_TYPE].m_internalName =
        "UNIT_MOVE_TOWARDS_NEAREST_OBJECT_TYPE";
    m_actionTemplates[ScriptAction::UNIT_MOVE_TOWARDS_NEAREST_OBJECT_TYPE].m_uiName =
        "Unit_/ Move unit towards the nearest object of a specific type.";
    m_actionTemplates[ScriptAction::UNIT_MOVE_TOWARDS_NEAREST_OBJECT_TYPE].m_numParameters = 3;
    m_actionTemplates[ScriptAction::UNIT_MOVE_TOWARDS_NEAREST_OBJECT_TYPE].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::UNIT_MOVE_TOWARDS_NEAREST_OBJECT_TYPE].m_parameters[1] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::UNIT_MOVE_TOWARDS_NEAREST_OBJECT_TYPE].m_parameters[2] = Parameter::TRIGGER_AREA;
    m_actionTemplates[ScriptAction::UNIT_MOVE_TOWARDS_NEAREST_OBJECT_TYPE].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::UNIT_MOVE_TOWARDS_NEAREST_OBJECT_TYPE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::UNIT_MOVE_TOWARDS_NEAREST_OBJECT_TYPE].m_uiStrings[1] =
        " will move towards the nearest ";
    m_actionTemplates[ScriptAction::UNIT_MOVE_TOWARDS_NEAREST_OBJECT_TYPE].m_uiStrings[2] = " within ";

    m_actionTemplates[ScriptAction::TEAM_MOVE_TOWARDS_NEAREST_OBJECT_TYPE].m_internalName =
        "TEAM_MOVE_TOWARDS_NEAREST_OBJECT_TYPE";
    m_actionTemplates[ScriptAction::TEAM_MOVE_TOWARDS_NEAREST_OBJECT_TYPE].m_uiName =
        "Team_/ Move team towards the nearest object of a specific type.";
    m_actionTemplates[ScriptAction::TEAM_MOVE_TOWARDS_NEAREST_OBJECT_TYPE].m_numParameters = 3;
    m_actionTemplates[ScriptAction::TEAM_MOVE_TOWARDS_NEAREST_OBJECT_TYPE].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_MOVE_TOWARDS_NEAREST_OBJECT_TYPE].m_parameters[1] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::TEAM_MOVE_TOWARDS_NEAREST_OBJECT_TYPE].m_parameters[2] = Parameter::TRIGGER_AREA;
    m_actionTemplates[ScriptAction::TEAM_MOVE_TOWARDS_NEAREST_OBJECT_TYPE].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_MOVE_TOWARDS_NEAREST_OBJECT_TYPE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_MOVE_TOWARDS_NEAREST_OBJECT_TYPE].m_uiStrings[1] =
        " will move towards the nearest ";
    m_actionTemplates[ScriptAction::TEAM_MOVE_TOWARDS_NEAREST_OBJECT_TYPE].m_uiStrings[2] = " within ";

    m_actionTemplates[ScriptAction::SKIRMISH_ATTACK_NEAREST_GROUP_WITH_VALUE].m_internalName =
        "SKIRMISH_ATTACK_NEAREST_GROUP_WITH_VALUE";
    m_actionTemplates[ScriptAction::SKIRMISH_ATTACK_NEAREST_GROUP_WITH_VALUE].m_uiName =
        "Skirmish_/ Team attacks nearest group matching value comparison.";
    m_actionTemplates[ScriptAction::SKIRMISH_ATTACK_NEAREST_GROUP_WITH_VALUE].m_numParameters = 3;
    m_actionTemplates[ScriptAction::SKIRMISH_ATTACK_NEAREST_GROUP_WITH_VALUE].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::SKIRMISH_ATTACK_NEAREST_GROUP_WITH_VALUE].m_parameters[1] = Parameter::COMPARISON;
    m_actionTemplates[ScriptAction::SKIRMISH_ATTACK_NEAREST_GROUP_WITH_VALUE].m_parameters[2] = Parameter::INT;
    m_actionTemplates[ScriptAction::SKIRMISH_ATTACK_NEAREST_GROUP_WITH_VALUE].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::SKIRMISH_ATTACK_NEAREST_GROUP_WITH_VALUE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::SKIRMISH_ATTACK_NEAREST_GROUP_WITH_VALUE].m_uiStrings[1] =
        " attacks nearest group worth ";
    m_actionTemplates[ScriptAction::SKIRMISH_ATTACK_NEAREST_GROUP_WITH_VALUE].m_uiStrings[2] = " ";

    m_actionTemplates[ScriptAction::SKIRMISH_PERFORM_COMMANDBUTTON_ON_MOST_VALUABLE_OBJECT].m_internalName =
        "SKIRMISH_PERFORM_COMMANDBUTTON_ON_MOST_VALUABLE_OBJECT";
    m_actionTemplates[ScriptAction::SKIRMISH_PERFORM_COMMANDBUTTON_ON_MOST_VALUABLE_OBJECT].m_uiName =
        "Skirmish_/ Team performs command ability on most valuable object.";
    m_actionTemplates[ScriptAction::SKIRMISH_PERFORM_COMMANDBUTTON_ON_MOST_VALUABLE_OBJECT].m_numParameters = 4;
    m_actionTemplates[ScriptAction::SKIRMISH_PERFORM_COMMANDBUTTON_ON_MOST_VALUABLE_OBJECT].m_parameters[0] =
        Parameter::TEAM;
    m_actionTemplates[ScriptAction::SKIRMISH_PERFORM_COMMANDBUTTON_ON_MOST_VALUABLE_OBJECT].m_parameters[1] =
        Parameter::TEAM_COMMANDBUTTON_ABILITY;
    m_actionTemplates[ScriptAction::SKIRMISH_PERFORM_COMMANDBUTTON_ON_MOST_VALUABLE_OBJECT].m_parameters[2] =
        Parameter::REAL;
    m_actionTemplates[ScriptAction::SKIRMISH_PERFORM_COMMANDBUTTON_ON_MOST_VALUABLE_OBJECT].m_parameters[3] =
        Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::SKIRMISH_PERFORM_COMMANDBUTTON_ON_MOST_VALUABLE_OBJECT].m_numUiStrings = 5;
    m_actionTemplates[ScriptAction::SKIRMISH_PERFORM_COMMANDBUTTON_ON_MOST_VALUABLE_OBJECT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::SKIRMISH_PERFORM_COMMANDBUTTON_ON_MOST_VALUABLE_OBJECT].m_uiStrings[1] = " performs ";
    m_actionTemplates[ScriptAction::SKIRMISH_PERFORM_COMMANDBUTTON_ON_MOST_VALUABLE_OBJECT].m_uiStrings[2] =
        " on most expensive object within ";
    m_actionTemplates[ScriptAction::SKIRMISH_PERFORM_COMMANDBUTTON_ON_MOST_VALUABLE_OBJECT].m_uiStrings[3] = " ";
    m_actionTemplates[ScriptAction::SKIRMISH_PERFORM_COMMANDBUTTON_ON_MOST_VALUABLE_OBJECT].m_uiStrings[4] =
        " (true = all valid sources, false = first valid source).";

    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_ALL].m_internalName =
        "SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_ALL";
    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_ALL].m_uiName =
        "Skirmish_/ Delay a sequential script until the specified command ability"
        " is ready - all.";
    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_ALL].m_numParameters = 3;
    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_ALL].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_ALL].m_parameters[1] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_ALL].m_parameters[2] =
        Parameter::TEAM_COMMANDBUTTON_ABILITY;
    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_ALL].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_ALL].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_ALL].m_uiStrings[1] = " 's ";
    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_ALL].m_uiStrings[2] = " all wait until ";
    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_ALL].m_uiStrings[3] = " is ready.";

    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_PARTIAL].m_internalName =
        "SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_PARTIAL";
    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_PARTIAL].m_uiName =
        "Skirmish_/ Delay a sequential script until the specified command ability"
        " is ready - partial.";
    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_PARTIAL].m_numParameters = 3;
    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_PARTIAL].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_PARTIAL].m_parameters[1] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_PARTIAL].m_parameters[2] =
        Parameter::TEAM_COMMANDBUTTON_ABILITY;
    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_PARTIAL].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_PARTIAL].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_PARTIAL].m_uiStrings[1] = " 's ";
    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_PARTIAL].m_uiStrings[2] =
        " wait until at least one member is ";
    m_actionTemplates[ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_PARTIAL].m_uiStrings[3] = " ready.";

    m_actionTemplates[ScriptAction::TEAM_SPIN_FOR_FRAMECOUNT].m_internalName = "TEAM_SPIN_FOR_FRAMECOUNT";
    m_actionTemplates[ScriptAction::TEAM_SPIN_FOR_FRAMECOUNT].m_uiName =
        "Team_/ Set to continue current action for some number of frames.";
    m_actionTemplates[ScriptAction::TEAM_SPIN_FOR_FRAMECOUNT].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_SPIN_FOR_FRAMECOUNT].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_SPIN_FOR_FRAMECOUNT].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::TEAM_SPIN_FOR_FRAMECOUNT].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_SPIN_FOR_FRAMECOUNT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_SPIN_FOR_FRAMECOUNT].m_uiStrings[1] =
        " continue their current action for at least ";
    m_actionTemplates[ScriptAction::TEAM_SPIN_FOR_FRAMECOUNT].m_uiStrings[2] = " frames.";

    m_actionTemplates[ScriptAction::CAMERA_FADE_MULTIPLY].m_internalName = "CAMERA_FADE_MULTIPLY";
    m_actionTemplates[ScriptAction::CAMERA_FADE_MULTIPLY].m_uiName =
        "Camera_/Fade Effects/Fade using a multiply blend to black.";
    m_actionTemplates[ScriptAction::CAMERA_ENABLE_SLAVE_MODE].m_internalName = "CAMERA_ENABLE_SLAVE_MODE";
    m_actionTemplates[ScriptAction::CAMERA_ENABLE_SLAVE_MODE].m_uiName =
        "Camera_/Enable 3DSMax Camera Animation Playback mode.";
    m_actionTemplates[ScriptAction::CAMERA_ENABLE_SLAVE_MODE].m_numParameters = 2;
    m_actionTemplates[ScriptAction::CAMERA_ENABLE_SLAVE_MODE].m_parameters[0] = Parameter::TEXT_STRING;
    m_actionTemplates[ScriptAction::CAMERA_ENABLE_SLAVE_MODE].m_parameters[1] = Parameter::TEXT_STRING;
    m_actionTemplates[ScriptAction::CAMERA_ENABLE_SLAVE_MODE].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::CAMERA_ENABLE_SLAVE_MODE].m_uiStrings[0] =
        "Enable 3DSMax Camera playback of animation with thing name ";
    m_actionTemplates[ScriptAction::CAMERA_ENABLE_SLAVE_MODE].m_uiStrings[1] = " containing bone name ";

    m_actionTemplates[ScriptAction::CAMERA_DISABLE_SLAVE_MODE].m_internalName = "CAMERA_DISABLE_SLAVE_MODE";
    m_actionTemplates[ScriptAction::CAMERA_DISABLE_SLAVE_MODE].m_uiName =
        "Camera_/Disable 3DSMax Camera Animation Playback mode.";
    m_actionTemplates[ScriptAction::CAMERA_DISABLE_SLAVE_MODE].m_numParameters = 0;
    m_actionTemplates[ScriptAction::CAMERA_DISABLE_SLAVE_MODE].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::CAMERA_DISABLE_SLAVE_MODE].m_uiStrings[0] = "Disable camera playback mode.";

    m_actionTemplates[ScriptAction::CAMERA_ADD_SHAKER_AT].m_internalName = "CAMERA_ADD_SHAKER_AT";
    m_actionTemplates[ScriptAction::CAMERA_ADD_SHAKER_AT].m_uiName = "Camera_/Add Camera Shaker Effect at.";
    m_actionTemplates[ScriptAction::CAMERA_ADD_SHAKER_AT].m_numParameters = 4;
    m_actionTemplates[ScriptAction::CAMERA_ADD_SHAKER_AT].m_parameters[0] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::CAMERA_ADD_SHAKER_AT].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_ADD_SHAKER_AT].m_parameters[2] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_ADD_SHAKER_AT].m_parameters[3] = Parameter::REAL;
    m_actionTemplates[ScriptAction::CAMERA_ADD_SHAKER_AT].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::CAMERA_ADD_SHAKER_AT].m_uiStrings[0] = "Add Camera Shaker Effect at waypoint ";
    m_actionTemplates[ScriptAction::CAMERA_ADD_SHAKER_AT].m_uiStrings[1] = " with Amplitude ";
    m_actionTemplates[ScriptAction::CAMERA_ADD_SHAKER_AT].m_uiStrings[2] = " Duration (seconds) ";
    m_actionTemplates[ScriptAction::CAMERA_ADD_SHAKER_AT].m_uiStrings[3] = " Radius.";

    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NAMED].m_internalName =
        "TEAM_ALL_USE_COMMANDBUTTON_ON_NAMED";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NAMED].m_uiName =
        "Team_/ Use command ability -- all -- named enemy";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NAMED].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NAMED].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NAMED].m_parameters[1] =
        Parameter::TEAM_COMMANDBUTTON_ABILITY;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NAMED].m_parameters[2] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NAMED].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NAMED].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NAMED].m_uiStrings[1] = " use ";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NAMED].m_uiStrings[2] = "  on ";

    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_UNIT].m_internalName =
        "TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_UNIT";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_UNIT].m_uiName =
        "Team_/ Use command ability -- all -- nearest enemy unit";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_UNIT].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_UNIT].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_UNIT].m_parameters[1] =
        Parameter::TEAM_COMMANDBUTTON_ABILITY;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_UNIT].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_UNIT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_UNIT].m_uiStrings[1] = " use ";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_UNIT].m_uiStrings[2] =
        "  on nearest enemy unit.";

    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_GARRISONED_BUILDING].m_internalName =
        "TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_GARRISONED_BUILDING";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_GARRISONED_BUILDING].m_uiName =
        "Team_/ Use command ability -- all -- nearest enemy garrisoned building.";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_GARRISONED_BUILDING].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_GARRISONED_BUILDING].m_parameters[0] =
        Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_GARRISONED_BUILDING].m_parameters[1] =
        Parameter::TEAM_COMMANDBUTTON_ABILITY;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_GARRISONED_BUILDING].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_GARRISONED_BUILDING].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_GARRISONED_BUILDING].m_uiStrings[1] = " use ";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_GARRISONED_BUILDING].m_uiStrings[2] =
        "  on nearest enemy garrisoned building.";

    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_KINDOF].m_internalName =
        "TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_KINDOF";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_KINDOF].m_uiName =
        "Team_/ Use command ability -- all -- nearest enemy object with kind of.";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_KINDOF].m_numParameters = 3;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_KINDOF].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_KINDOF].m_parameters[1] =
        Parameter::TEAM_COMMANDBUTTON_ABILITY;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_KINDOF].m_parameters[2] = Parameter::KIND_OF_PARAM;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_KINDOF].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_KINDOF].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_KINDOF].m_uiStrings[1] = " use ";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_KINDOF].m_uiStrings[2] =
        "  on nearest enemy with ";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_KINDOF].m_uiStrings[4] = ".";

    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING].m_internalName =
        "TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING].m_uiName =
        "Team_/ Use command ability -- all -- nearest enemy building.";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING].m_parameters[1] =
        Parameter::TEAM_COMMANDBUTTON_ABILITY;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING].m_uiStrings[1] = " use ";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING].m_uiStrings[2] =
        "  on nearest enemy building.";

    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING_CLASS].m_internalName =
        "TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING_CLASS";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING_CLASS].m_uiName =
        "Team_/ Use command ability -- all -- nearest enemy building kindof.";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING_CLASS].m_numParameters = 3;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING_CLASS].m_parameters[0] =
        Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING_CLASS].m_parameters[1] =
        Parameter::TEAM_COMMANDBUTTON_ABILITY;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING_CLASS].m_parameters[2] =
        Parameter::KIND_OF_PARAM;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING_CLASS].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING_CLASS].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING_CLASS].m_uiStrings[1] = " use ";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING_CLASS].m_uiStrings[2] =
        "  on nearest enemy building with ";

    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_OBJECTTYPE].m_internalName =
        "TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_OBJECTTYPE";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_OBJECTTYPE].m_uiName =
        "Team_/ Use command ability -- all -- nearest object type.";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_OBJECTTYPE].m_numParameters = 3;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_OBJECTTYPE].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_OBJECTTYPE].m_parameters[1] =
        Parameter::TEAM_COMMANDBUTTON_ABILITY;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_OBJECTTYPE].m_parameters[2] =
        Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_OBJECTTYPE].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_OBJECTTYPE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_OBJECTTYPE].m_uiStrings[1] = " use ";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_OBJECTTYPE].m_uiStrings[2] =
        " on nearest object of type ";
    m_actionTemplates[ScriptAction::TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_OBJECTTYPE].m_uiStrings[3] = ".";

    m_actionTemplates[ScriptAction::TEAM_PARTIAL_USE_COMMANDBUTTON].m_internalName = "TEAM_PARTIAL_USE_COMMANDBUTTON";
    m_actionTemplates[ScriptAction::TEAM_PARTIAL_USE_COMMANDBUTTON].m_uiName =
        "Team_/ Use command ability -- partial -- self.";
    m_actionTemplates[ScriptAction::TEAM_PARTIAL_USE_COMMANDBUTTON].m_numParameters = 3;
    m_actionTemplates[ScriptAction::TEAM_PARTIAL_USE_COMMANDBUTTON].m_parameters[0] = Parameter::REAL;
    m_actionTemplates[ScriptAction::TEAM_PARTIAL_USE_COMMANDBUTTON].m_parameters[1] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_PARTIAL_USE_COMMANDBUTTON].m_parameters[2] = Parameter::TEAM_COMMANDBUTTON_ABILITY;
    m_actionTemplates[ScriptAction::TEAM_PARTIAL_USE_COMMANDBUTTON].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::TEAM_PARTIAL_USE_COMMANDBUTTON].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_PARTIAL_USE_COMMANDBUTTON].m_uiStrings[1] = "% of ";
    m_actionTemplates[ScriptAction::TEAM_PARTIAL_USE_COMMANDBUTTON].m_uiStrings[2] = " perform ";
    m_actionTemplates[ScriptAction::TEAM_PARTIAL_USE_COMMANDBUTTON].m_uiStrings[3] = ".";

    m_actionTemplates[ScriptAction::TEAM_CAPTURE_NEAREST_UNOWNED_FACTION_UNIT].m_internalName =
        "TEAM_CAPTURE_NEAREST_UNOWNED_FACTION_UNIT";
    m_actionTemplates[ScriptAction::TEAM_CAPTURE_NEAREST_UNOWNED_FACTION_UNIT].m_uiName =
        "Team_/ Capture unowned faction unit -- nearest.";
    m_actionTemplates[ScriptAction::TEAM_CAPTURE_NEAREST_UNOWNED_FACTION_UNIT].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_CAPTURE_NEAREST_UNOWNED_FACTION_UNIT].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_CAPTURE_NEAREST_UNOWNED_FACTION_UNIT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_CAPTURE_NEAREST_UNOWNED_FACTION_UNIT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_CAPTURE_NEAREST_UNOWNED_FACTION_UNIT].m_uiStrings[1] =
        " capture the nearest unowned faction unit.";

    m_actionTemplates[ScriptAction::PLAYER_CREATE_TEAM_FROM_CAPTURED_UNITS].m_internalName =
        "PLAYER_CREATE_TEAM_FROM_CAPTURED_UNITS";
    m_actionTemplates[ScriptAction::PLAYER_CREATE_TEAM_FROM_CAPTURED_UNITS].m_uiName =
        "Player_/ Create team from all captured units.";
    m_actionTemplates[ScriptAction::PLAYER_CREATE_TEAM_FROM_CAPTURED_UNITS].m_numParameters = 2;
    m_actionTemplates[ScriptAction::PLAYER_CREATE_TEAM_FROM_CAPTURED_UNITS].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_CREATE_TEAM_FROM_CAPTURED_UNITS].m_parameters[1] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::PLAYER_CREATE_TEAM_FROM_CAPTURED_UNITS].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::PLAYER_CREATE_TEAM_FROM_CAPTURED_UNITS].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_CREATE_TEAM_FROM_CAPTURED_UNITS].m_uiStrings[1] = " creates a new ";
    m_actionTemplates[ScriptAction::PLAYER_CREATE_TEAM_FROM_CAPTURED_UNITS].m_uiStrings[2] =
        " from units it has captured. (There's nothing quite like being assaulted"
        " by your own captured units!)";

    m_actionTemplates[ScriptAction::TEAM_WAIT_FOR_NOT_CONTAINED_ALL].m_internalName = "TEAM_WAIT_FOR_NOT_CONTAINED_ALL";
    m_actionTemplates[ScriptAction::TEAM_WAIT_FOR_NOT_CONTAINED_ALL].m_uiName =
        "Team_/ Delay a sequential script until the team is no longer contained - all";
    m_actionTemplates[ScriptAction::TEAM_WAIT_FOR_NOT_CONTAINED_ALL].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_WAIT_FOR_NOT_CONTAINED_ALL].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_WAIT_FOR_NOT_CONTAINED_ALL].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_WAIT_FOR_NOT_CONTAINED_ALL].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_WAIT_FOR_NOT_CONTAINED_ALL].m_uiStrings[1] =
        " all delay until they are no longer contained.";

    m_actionTemplates[ScriptAction::TEAM_WAIT_FOR_NOT_CONTAINED_PARTIAL].m_internalName =
        "TEAM_WAIT_FOR_NOT_CONTAINED_PARTIAL";
    m_actionTemplates[ScriptAction::TEAM_WAIT_FOR_NOT_CONTAINED_PARTIAL].m_uiName =
        "Team_/ Delay a sequential script until the team is no longer contained - partial";
    m_actionTemplates[ScriptAction::TEAM_WAIT_FOR_NOT_CONTAINED_PARTIAL].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_WAIT_FOR_NOT_CONTAINED_PARTIAL].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_WAIT_FOR_NOT_CONTAINED_PARTIAL].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_WAIT_FOR_NOT_CONTAINED_PARTIAL].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_WAIT_FOR_NOT_CONTAINED_PARTIAL].m_uiStrings[1] =
        " delay until at least one of them is no longer contained.";

    m_actionTemplates[ScriptAction::TEAM_SET_EMOTICON].m_internalName = "TEAM_SET_EMOTICON";
    m_actionTemplates[ScriptAction::TEAM_SET_EMOTICON].m_uiName =
        "Team_/ Set emoticon for duration (-1.0 permanent, otherwise duration in sec).";
    m_actionTemplates[ScriptAction::TEAM_SET_EMOTICON].m_numParameters = 3;
    m_actionTemplates[ScriptAction::TEAM_SET_EMOTICON].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_SET_EMOTICON].m_parameters[1] = Parameter::EMOTICON;
    m_actionTemplates[ScriptAction::TEAM_SET_EMOTICON].m_parameters[2] = Parameter::REAL;
    m_actionTemplates[ScriptAction::TEAM_SET_EMOTICON].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::TEAM_SET_EMOTICON].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_SET_EMOTICON].m_uiStrings[1] = " use ";
    m_actionTemplates[ScriptAction::TEAM_SET_EMOTICON].m_uiStrings[2] = " emoticon for ";
    m_actionTemplates[ScriptAction::TEAM_SET_EMOTICON].m_uiStrings[3] = " seconds.";

    m_actionTemplates[ScriptAction::NAMED_SET_EMOTICON].m_internalName = "NAMED_SET_EMOTICON";
    m_actionTemplates[ScriptAction::NAMED_SET_EMOTICON].m_uiName =
        "Unit_/ Set emoticon for duration (-1.0 permanent, otherwise duration in sec).";
    m_actionTemplates[ScriptAction::NAMED_SET_EMOTICON].m_numParameters = 3;
    m_actionTemplates[ScriptAction::NAMED_SET_EMOTICON].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_SET_EMOTICON].m_parameters[1] = Parameter::EMOTICON;
    m_actionTemplates[ScriptAction::NAMED_SET_EMOTICON].m_parameters[2] = Parameter::REAL;
    m_actionTemplates[ScriptAction::NAMED_SET_EMOTICON].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::NAMED_SET_EMOTICON].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_SET_EMOTICON].m_uiStrings[1] = " use ";
    m_actionTemplates[ScriptAction::NAMED_SET_EMOTICON].m_uiStrings[2] = " emoticon for ";
    m_actionTemplates[ScriptAction::NAMED_SET_EMOTICON].m_uiStrings[3] = " seconds.";

    m_actionTemplates[ScriptAction::OBJECTLIST_ADDOBJECTTYPE].m_internalName = "OBJECTLIST_ADDOBJECTTYPE";
    m_actionTemplates[ScriptAction::OBJECTLIST_ADDOBJECTTYPE].m_uiName = "Scripting_/ Object Type List -- Add Object Type.";
    m_actionTemplates[ScriptAction::OBJECTLIST_ADDOBJECTTYPE].m_numParameters = 2;
    m_actionTemplates[ScriptAction::OBJECTLIST_ADDOBJECTTYPE].m_parameters[0] = Parameter::OBJECT_TYPE_LIST;
    m_actionTemplates[ScriptAction::OBJECTLIST_ADDOBJECTTYPE].m_parameters[1] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::OBJECTLIST_ADDOBJECTTYPE].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::OBJECTLIST_ADDOBJECTTYPE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::OBJECTLIST_ADDOBJECTTYPE].m_uiStrings[1] = " : add ";

    m_actionTemplates[ScriptAction::OBJECTLIST_REMOVEOBJECTTYPE].m_internalName = "OBJECTLIST_REMOVEOBJECTTYPE";
    m_actionTemplates[ScriptAction::OBJECTLIST_REMOVEOBJECTTYPE].m_uiName =
        "Scripting_/ Object Type List -- Remove Object Type.";
    m_actionTemplates[ScriptAction::OBJECTLIST_REMOVEOBJECTTYPE].m_numParameters = 2;
    m_actionTemplates[ScriptAction::OBJECTLIST_REMOVEOBJECTTYPE].m_parameters[0] = Parameter::OBJECT_TYPE_LIST;
    m_actionTemplates[ScriptAction::OBJECTLIST_REMOVEOBJECTTYPE].m_parameters[1] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::OBJECTLIST_REMOVEOBJECTTYPE].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::OBJECTLIST_REMOVEOBJECTTYPE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::OBJECTLIST_REMOVEOBJECTTYPE].m_uiStrings[1] = " : remove ";

    m_actionTemplates[ScriptAction::MAP_REVEAL_PERMANENTLY_AT_WAYPOINT].m_internalName =
        "MAP_REVEAL_PERMANENTLY_AT_WAYPOINT";
    m_actionTemplates[ScriptAction::MAP_REVEAL_PERMANENTLY_AT_WAYPOINT].m_uiName =
        "Map_/ Reveal map at waypoint -- permanently.";
    m_actionTemplates[ScriptAction::MAP_REVEAL_PERMANENTLY_AT_WAYPOINT].m_numParameters = 4;
    m_actionTemplates[ScriptAction::MAP_REVEAL_PERMANENTLY_AT_WAYPOINT].m_parameters[0] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::MAP_REVEAL_PERMANENTLY_AT_WAYPOINT].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::MAP_REVEAL_PERMANENTLY_AT_WAYPOINT].m_parameters[2] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::MAP_REVEAL_PERMANENTLY_AT_WAYPOINT].m_parameters[3] = Parameter::REVEAL_NAME;
    m_actionTemplates[ScriptAction::MAP_REVEAL_PERMANENTLY_AT_WAYPOINT].m_numUiStrings = 5;
    m_actionTemplates[ScriptAction::MAP_REVEAL_PERMANENTLY_AT_WAYPOINT].m_uiStrings[0] =
        "The map is permanently revealed at ";
    m_actionTemplates[ScriptAction::MAP_REVEAL_PERMANENTLY_AT_WAYPOINT].m_uiStrings[1] = " with a radius of ";
    m_actionTemplates[ScriptAction::MAP_REVEAL_PERMANENTLY_AT_WAYPOINT].m_uiStrings[2] = " for ";
    m_actionTemplates[ScriptAction::MAP_REVEAL_PERMANENTLY_AT_WAYPOINT].m_uiStrings[3] = ". (Afterwards referred to as ";
    m_actionTemplates[ScriptAction::MAP_REVEAL_PERMANENTLY_AT_WAYPOINT].m_uiStrings[4] = ").";

    m_actionTemplates[ScriptAction::MAP_UNDO_REVEAL_PERMANENTLY_AT_WAYPOINT].m_internalName =
        "MAP_UNDO_REVEAL_PERMANENTLY_AT_WAYPOINT";
    m_actionTemplates[ScriptAction::MAP_UNDO_REVEAL_PERMANENTLY_AT_WAYPOINT].m_uiName =
        "Map_/ Reveal map at waypoint -- undo permanently.";
    m_actionTemplates[ScriptAction::MAP_UNDO_REVEAL_PERMANENTLY_AT_WAYPOINT].m_numParameters = 1;
    m_actionTemplates[ScriptAction::MAP_UNDO_REVEAL_PERMANENTLY_AT_WAYPOINT].m_parameters[0] = Parameter::REVEAL_NAME;
    m_actionTemplates[ScriptAction::MAP_UNDO_REVEAL_PERMANENTLY_AT_WAYPOINT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::MAP_UNDO_REVEAL_PERMANENTLY_AT_WAYPOINT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::MAP_UNDO_REVEAL_PERMANENTLY_AT_WAYPOINT].m_uiStrings[1] = " is undone.";

    m_actionTemplates[ScriptAction::EVA_SET_ENABLED_DISABLED].m_internalName = "EVA_SET_ENABLED_DISABLED";
    m_actionTemplates[ScriptAction::EVA_SET_ENABLED_DISABLED].m_uiName = "Scripting_/ Enable or Disable EVA.";
    m_actionTemplates[ScriptAction::EVA_SET_ENABLED_DISABLED].m_numParameters = 1;
    m_actionTemplates[ScriptAction::EVA_SET_ENABLED_DISABLED].m_parameters[0] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::EVA_SET_ENABLED_DISABLED].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::EVA_SET_ENABLED_DISABLED].m_uiStrings[0] = "Set EVA to be enabled ";
    m_actionTemplates[ScriptAction::EVA_SET_ENABLED_DISABLED].m_uiStrings[1] = " (False to disable.)";

    m_actionTemplates[ScriptAction::OPTIONS_SET_OCCLUSION_MODE].m_internalName = "OPTIONS_SET_OCCLUSION_MODE";
    m_actionTemplates[ScriptAction::OPTIONS_SET_OCCLUSION_MODE].m_uiName =
        "Scripting_/ Enable or Disable Occlusion (Drawing Behind Buildings).";
    m_actionTemplates[ScriptAction::OPTIONS_SET_OCCLUSION_MODE].m_numParameters = 1;
    m_actionTemplates[ScriptAction::OPTIONS_SET_OCCLUSION_MODE].m_parameters[0] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::OPTIONS_SET_OCCLUSION_MODE].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::OPTIONS_SET_OCCLUSION_MODE].m_uiStrings[0] = "Set Occlusion to be enabled ";
    m_actionTemplates[ScriptAction::OPTIONS_SET_OCCLUSION_MODE].m_uiStrings[1] = " (False to disable.)";

    m_actionTemplates[ScriptAction::OPTIONS_SET_DRAWICON_UI_MODE].m_internalName = "OPTIONS_SET_DRAWICON_UI_MODE";
    m_actionTemplates[ScriptAction::OPTIONS_SET_DRAWICON_UI_MODE].m_uiName = "Scripting_/ Enable or Disable Draw-icon UI.";
    m_actionTemplates[ScriptAction::OPTIONS_SET_DRAWICON_UI_MODE].m_numParameters = 1;
    m_actionTemplates[ScriptAction::OPTIONS_SET_DRAWICON_UI_MODE].m_parameters[0] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::OPTIONS_SET_DRAWICON_UI_MODE].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::OPTIONS_SET_DRAWICON_UI_MODE].m_uiStrings[0] = "Set Draw-icon UI to be enabled ";
    m_actionTemplates[ScriptAction::OPTIONS_SET_DRAWICON_UI_MODE].m_uiStrings[1] = " (False to disable.)";

    m_actionTemplates[ScriptAction::OPTIONS_SET_PARTICLE_CAP_MODE].m_internalName = "OPTIONS_SET_PARTICLE_CAP_MODE";
    m_actionTemplates[ScriptAction::OPTIONS_SET_PARTICLE_CAP_MODE].m_uiName = "Scripting_/ Enable or Disable Particle Cap.";
    m_actionTemplates[ScriptAction::OPTIONS_SET_PARTICLE_CAP_MODE].m_numParameters = 1;
    m_actionTemplates[ScriptAction::OPTIONS_SET_PARTICLE_CAP_MODE].m_parameters[0] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::OPTIONS_SET_PARTICLE_CAP_MODE].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::OPTIONS_SET_PARTICLE_CAP_MODE].m_uiStrings[0] = "Set Particle Cap to be enabled ";
    m_actionTemplates[ScriptAction::OPTIONS_SET_PARTICLE_CAP_MODE].m_uiStrings[1] = " (False to disable.)";

    m_actionTemplates[ScriptAction::UNIT_AFFECT_OBJECT_PANEL_FLAGS].m_internalName = "UNIT_AFFECT_OBJECT_PANEL_FLAGS";
    m_actionTemplates[ScriptAction::UNIT_AFFECT_OBJECT_PANEL_FLAGS].m_uiName = "Unit_/ Affect flags set on object panel.";
    m_actionTemplates[ScriptAction::UNIT_AFFECT_OBJECT_PANEL_FLAGS].m_numParameters = 3;
    m_actionTemplates[ScriptAction::UNIT_AFFECT_OBJECT_PANEL_FLAGS].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::UNIT_AFFECT_OBJECT_PANEL_FLAGS].m_parameters[1] = Parameter::OBJECT_FLAG;
    m_actionTemplates[ScriptAction::UNIT_AFFECT_OBJECT_PANEL_FLAGS].m_parameters[2] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::UNIT_AFFECT_OBJECT_PANEL_FLAGS].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::UNIT_AFFECT_OBJECT_PANEL_FLAGS].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::UNIT_AFFECT_OBJECT_PANEL_FLAGS].m_uiStrings[1] = " changes the value of flag ";
    m_actionTemplates[ScriptAction::UNIT_AFFECT_OBJECT_PANEL_FLAGS].m_uiStrings[2] = " to ";
    m_actionTemplates[ScriptAction::UNIT_AFFECT_OBJECT_PANEL_FLAGS].m_uiStrings[3] = ".";

    m_actionTemplates[ScriptAction::TEAM_AFFECT_OBJECT_PANEL_FLAGS].m_internalName = "TEAM_AFFECT_OBJECT_PANEL_FLAGS";
    m_actionTemplates[ScriptAction::TEAM_AFFECT_OBJECT_PANEL_FLAGS].m_uiName =
        "Team_/ Affect flags set on object panel - all.";
    m_actionTemplates[ScriptAction::TEAM_AFFECT_OBJECT_PANEL_FLAGS].m_numParameters = 3;
    m_actionTemplates[ScriptAction::TEAM_AFFECT_OBJECT_PANEL_FLAGS].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_AFFECT_OBJECT_PANEL_FLAGS].m_parameters[1] = Parameter::OBJECT_FLAG;
    m_actionTemplates[ScriptAction::TEAM_AFFECT_OBJECT_PANEL_FLAGS].m_parameters[2] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::TEAM_AFFECT_OBJECT_PANEL_FLAGS].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::TEAM_AFFECT_OBJECT_PANEL_FLAGS].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_AFFECT_OBJECT_PANEL_FLAGS].m_uiStrings[1] = " change the value of flag ";
    m_actionTemplates[ScriptAction::TEAM_AFFECT_OBJECT_PANEL_FLAGS].m_uiStrings[2] = " to ";
    m_actionTemplates[ScriptAction::TEAM_AFFECT_OBJECT_PANEL_FLAGS].m_uiStrings[3] = ".";

    m_actionTemplates[ScriptAction::PLAYER_SELECT_SKILLSET].m_internalName = "PLAYER_SELECT_SKILLSET";
    m_actionTemplates[ScriptAction::PLAYER_SELECT_SKILLSET].m_uiName = "Player_/ Set the skillset for a computer player.";
    m_actionTemplates[ScriptAction::PLAYER_SELECT_SKILLSET].m_numParameters = 2;
    m_actionTemplates[ScriptAction::PLAYER_SELECT_SKILLSET].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_SELECT_SKILLSET].m_parameters[1] = Parameter::INT;
    m_actionTemplates[ScriptAction::PLAYER_SELECT_SKILLSET].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::PLAYER_SELECT_SKILLSET].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_SELECT_SKILLSET].m_uiStrings[1] = " uses skillset number ";
    m_actionTemplates[ScriptAction::PLAYER_SELECT_SKILLSET].m_uiStrings[2] = " (1-5).";

    m_actionTemplates[ScriptAction::SCRIPTING_OVERRIDE_HULK_LIFETIME].m_internalName = "SCRIPTING_OVERRIDE_HULK_LIFETIME";
    m_actionTemplates[ScriptAction::SCRIPTING_OVERRIDE_HULK_LIFETIME].m_uiName = "Scripting_/ Hulk set override lifetime.";
    m_actionTemplates[ScriptAction::SCRIPTING_OVERRIDE_HULK_LIFETIME].m_numParameters = 1;
    m_actionTemplates[ScriptAction::SCRIPTING_OVERRIDE_HULK_LIFETIME].m_parameters[0] = Parameter::REAL;
    m_actionTemplates[ScriptAction::SCRIPTING_OVERRIDE_HULK_LIFETIME].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::SCRIPTING_OVERRIDE_HULK_LIFETIME].m_uiStrings[0] = "Override hulk lifetime to ";
    m_actionTemplates[ScriptAction::SCRIPTING_OVERRIDE_HULK_LIFETIME].m_uiStrings[1] =
        " seconds. Negative value reverts to normal behavior.";

    m_actionTemplates[ScriptAction::NAMED_FACE_NAMED].m_internalName = "NAMED_FACE_NAMED";
    m_actionTemplates[ScriptAction::NAMED_FACE_NAMED].m_uiName = "Unit_/ Set unit to face another unit.";
    m_actionTemplates[ScriptAction::NAMED_FACE_NAMED].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_FACE_NAMED].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_FACE_NAMED].m_parameters[1] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_FACE_NAMED].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_FACE_NAMED].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_FACE_NAMED].m_uiStrings[1] = " begin facing ";

    m_actionTemplates[ScriptAction::NAMED_FACE_WAYPOINT].m_internalName = "NAMED_FACE_WAYPOINT";
    m_actionTemplates[ScriptAction::NAMED_FACE_WAYPOINT].m_uiName = "Unit_/ Set unit to face a waypoint.";
    m_actionTemplates[ScriptAction::NAMED_FACE_WAYPOINT].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_FACE_WAYPOINT].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_FACE_WAYPOINT].m_parameters[1] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::NAMED_FACE_WAYPOINT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_FACE_WAYPOINT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_FACE_WAYPOINT].m_uiStrings[1] = " begin facing ";

    m_actionTemplates[ScriptAction::TEAM_FACE_NAMED].m_internalName = "TEAM_FACE_NAMED";
    m_actionTemplates[ScriptAction::TEAM_FACE_NAMED].m_uiName = "Team_/ Set team to face another unit.";
    m_actionTemplates[ScriptAction::TEAM_FACE_NAMED].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_FACE_NAMED].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_FACE_NAMED].m_parameters[1] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::TEAM_FACE_NAMED].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_FACE_NAMED].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_FACE_NAMED].m_uiStrings[1] = " begin facing ";

    m_actionTemplates[ScriptAction::TEAM_FACE_WAYPOINT].m_internalName = "TEAM_FACE_WAYPOINT";
    m_actionTemplates[ScriptAction::TEAM_FACE_WAYPOINT].m_uiName = "Team_/ Set team to face a waypoint.";
    m_actionTemplates[ScriptAction::TEAM_FACE_WAYPOINT].m_numParameters = 2;
    m_actionTemplates[ScriptAction::TEAM_FACE_WAYPOINT].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_FACE_WAYPOINT].m_parameters[1] = Parameter::WAYPOINT;
    m_actionTemplates[ScriptAction::TEAM_FACE_WAYPOINT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_FACE_WAYPOINT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_FACE_WAYPOINT].m_uiStrings[1] = " begin facing ";

    m_actionTemplates[ScriptAction::COMMANDBAR_REMOVE_BUTTON_OBJECTTYPE].m_internalName =
        "COMMANDBAR_REMOVE_BUTTON_OBJECTTYPE";
    m_actionTemplates[ScriptAction::COMMANDBAR_REMOVE_BUTTON_OBJECTTYPE].m_uiName =
        "Scripting_/ Remove a command button from an object type.";
    m_actionTemplates[ScriptAction::COMMANDBAR_REMOVE_BUTTON_OBJECTTYPE].m_numParameters = 2;
    m_actionTemplates[ScriptAction::COMMANDBAR_REMOVE_BUTTON_OBJECTTYPE].m_parameters[0] = Parameter::COMMANDBUTTON;
    m_actionTemplates[ScriptAction::COMMANDBAR_REMOVE_BUTTON_OBJECTTYPE].m_parameters[1] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::COMMANDBAR_REMOVE_BUTTON_OBJECTTYPE].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::COMMANDBAR_REMOVE_BUTTON_OBJECTTYPE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::COMMANDBAR_REMOVE_BUTTON_OBJECTTYPE].m_uiStrings[1] =
        " is removed from all objects of type ";
    m_actionTemplates[ScriptAction::COMMANDBAR_REMOVE_BUTTON_OBJECTTYPE].m_uiStrings[2] = ".";

    m_actionTemplates[ScriptAction::COMMANDBAR_ADD_BUTTON_OBJECTTYPE_SLOT].m_internalName =
        "COMMANDBAR_ADD_BUTTON_OBJECTTYPE_SLOT";
    m_actionTemplates[ScriptAction::COMMANDBAR_ADD_BUTTON_OBJECTTYPE_SLOT].m_uiName =
        "Scripting_/ Add a command button to an object type.";
    m_actionTemplates[ScriptAction::COMMANDBAR_ADD_BUTTON_OBJECTTYPE_SLOT].m_numParameters = 3;
    m_actionTemplates[ScriptAction::COMMANDBAR_ADD_BUTTON_OBJECTTYPE_SLOT].m_parameters[0] = Parameter::COMMANDBUTTON;
    m_actionTemplates[ScriptAction::COMMANDBAR_ADD_BUTTON_OBJECTTYPE_SLOT].m_parameters[1] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::COMMANDBAR_ADD_BUTTON_OBJECTTYPE_SLOT].m_parameters[2] = Parameter::INT;
    m_actionTemplates[ScriptAction::COMMANDBAR_ADD_BUTTON_OBJECTTYPE_SLOT].m_numUiStrings = 4;
    m_actionTemplates[ScriptAction::COMMANDBAR_ADD_BUTTON_OBJECTTYPE_SLOT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::COMMANDBAR_ADD_BUTTON_OBJECTTYPE_SLOT].m_uiStrings[1] =
        " is added to all objects of type ";
    m_actionTemplates[ScriptAction::COMMANDBAR_ADD_BUTTON_OBJECTTYPE_SLOT].m_uiStrings[2] = " in slot number ";
    m_actionTemplates[ScriptAction::COMMANDBAR_ADD_BUTTON_OBJECTTYPE_SLOT].m_uiStrings[3] = " (1-12).";

    m_actionTemplates[ScriptAction::UNIT_SPAWN_NAMED_LOCATION_ORIENTATION].m_internalName =
        "UNIT_SPAWN_NAMED_LOCATION_ORIENTATION";
    m_actionTemplates[ScriptAction::UNIT_SPAWN_NAMED_LOCATION_ORIENTATION].m_uiName =
        "Unit_/ Spawn -- named unit on a team at a position with an orientation.";
    m_actionTemplates[ScriptAction::UNIT_SPAWN_NAMED_LOCATION_ORIENTATION].m_numParameters = 5;
    m_actionTemplates[ScriptAction::UNIT_SPAWN_NAMED_LOCATION_ORIENTATION].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::UNIT_SPAWN_NAMED_LOCATION_ORIENTATION].m_parameters[1] = Parameter::OBJECT_TYPE;
    m_actionTemplates[ScriptAction::UNIT_SPAWN_NAMED_LOCATION_ORIENTATION].m_parameters[2] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::UNIT_SPAWN_NAMED_LOCATION_ORIENTATION].m_parameters[3] = Parameter::COORD3D;
    m_actionTemplates[ScriptAction::UNIT_SPAWN_NAMED_LOCATION_ORIENTATION].m_parameters[4] = Parameter::ANGLE;
    m_actionTemplates[ScriptAction::UNIT_SPAWN_NAMED_LOCATION_ORIENTATION].m_numUiStrings = 6;
    m_actionTemplates[ScriptAction::UNIT_SPAWN_NAMED_LOCATION_ORIENTATION].m_uiStrings[0] = "Spawn ";
    m_actionTemplates[ScriptAction::UNIT_SPAWN_NAMED_LOCATION_ORIENTATION].m_uiStrings[1] = " of type ";
    m_actionTemplates[ScriptAction::UNIT_SPAWN_NAMED_LOCATION_ORIENTATION].m_uiStrings[2] = " on team ";
    m_actionTemplates[ScriptAction::UNIT_SPAWN_NAMED_LOCATION_ORIENTATION].m_uiStrings[3] = " at position (";
    m_actionTemplates[ScriptAction::UNIT_SPAWN_NAMED_LOCATION_ORIENTATION].m_uiStrings[4] = "), rotated ";
    m_actionTemplates[ScriptAction::UNIT_SPAWN_NAMED_LOCATION_ORIENTATION].m_uiStrings[5] = " .";

    m_actionTemplates[ScriptAction::PLAYER_AFFECT_RECEIVING_EXPERIENCE].m_internalName =
        "PLAYER_AFFECT_RECEIVING_EXPERIENCE";
    m_actionTemplates[ScriptAction::PLAYER_AFFECT_RECEIVING_EXPERIENCE].m_uiName =
        "Player_/ Change the modifier to generals experience that a player receives.";
    m_actionTemplates[ScriptAction::PLAYER_AFFECT_RECEIVING_EXPERIENCE].m_numParameters = 2;
    m_actionTemplates[ScriptAction::PLAYER_AFFECT_RECEIVING_EXPERIENCE].m_parameters[0] = Parameter::SIDE;
    m_actionTemplates[ScriptAction::PLAYER_AFFECT_RECEIVING_EXPERIENCE].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::PLAYER_AFFECT_RECEIVING_EXPERIENCE].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::PLAYER_AFFECT_RECEIVING_EXPERIENCE].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::PLAYER_AFFECT_RECEIVING_EXPERIENCE].m_uiStrings[1] = " gains experience at ";
    m_actionTemplates[ScriptAction::PLAYER_AFFECT_RECEIVING_EXPERIENCE].m_uiStrings[2] =
        " times the usual rate (0.0 for no gain, 1.0 for normal rate)";

    m_actionTemplates[ScriptAction::SOUND_SET_VOLUME].m_internalName = "SOUND_SET_VOLUME";
    m_actionTemplates[ScriptAction::SOUND_SET_VOLUME].m_uiName = "Multimedia_/ Set the current sound volume.";
    m_actionTemplates[ScriptAction::SOUND_SET_VOLUME].m_numParameters = 1;
    m_actionTemplates[ScriptAction::SOUND_SET_VOLUME].m_parameters[0] = Parameter::REAL;
    m_actionTemplates[ScriptAction::SOUND_SET_VOLUME].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::SOUND_SET_VOLUME].m_uiStrings[0] = "Set the desired sound volume to ";
    m_actionTemplates[ScriptAction::SOUND_SET_VOLUME].m_uiStrings[1] = "%. (0-100)";

    m_actionTemplates[ScriptAction::SPEECH_SET_VOLUME].m_internalName = "SPEECH_SET_VOLUME";
    m_actionTemplates[ScriptAction::SPEECH_SET_VOLUME].m_uiName = "Multimedia_/ Set the current speech volume.";
    m_actionTemplates[ScriptAction::SPEECH_SET_VOLUME].m_numParameters = 1;
    m_actionTemplates[ScriptAction::SPEECH_SET_VOLUME].m_parameters[0] = Parameter::REAL;
    m_actionTemplates[ScriptAction::SPEECH_SET_VOLUME].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::SPEECH_SET_VOLUME].m_uiStrings[0] = "Set the desired speech volume to ";
    m_actionTemplates[ScriptAction::SPEECH_SET_VOLUME].m_uiStrings[1] = "%. (0-100)";

    m_actionTemplates[ScriptAction::OBJECT_ALLOW_BONUSES].m_internalName = "OBJECT_ALLOW_BONUSES";
    m_actionTemplates[ScriptAction::OBJECT_ALLOW_BONUSES].m_uiName = "Map_/ Adjust Object Bonuses based on difficulty.";
    m_actionTemplates[ScriptAction::OBJECT_ALLOW_BONUSES].m_numParameters = 1;
    m_actionTemplates[ScriptAction::OBJECT_ALLOW_BONUSES].m_parameters[0] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::OBJECT_ALLOW_BONUSES].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::OBJECT_ALLOW_BONUSES].m_uiStrings[0] = "Enable Object Bonuses based on difficulty ";
    m_actionTemplates[ScriptAction::OBJECT_ALLOW_BONUSES].m_uiStrings[1] = " (true to enable, false to disable).";

    m_actionTemplates[ScriptAction::TEAM_GUARD_IN_TUNNEL_NETWORK].m_internalName = "TEAM_GUARD_IN_TUNNEL_NETWORK";
    m_actionTemplates[ScriptAction::TEAM_GUARD_IN_TUNNEL_NETWORK].m_uiName =
        "Team_/ Set to guard - from inside tunnel network.";
    m_actionTemplates[ScriptAction::TEAM_GUARD_IN_TUNNEL_NETWORK].m_numParameters = 1;
    m_actionTemplates[ScriptAction::TEAM_GUARD_IN_TUNNEL_NETWORK].m_parameters[0] = Parameter::TEAM;
    m_actionTemplates[ScriptAction::TEAM_GUARD_IN_TUNNEL_NETWORK].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::TEAM_GUARD_IN_TUNNEL_NETWORK].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::TEAM_GUARD_IN_TUNNEL_NETWORK].m_uiStrings[1] = " Enter and guard from tunnel network.";

    m_actionTemplates[ScriptAction::LOCALDEFEAT].m_internalName = "LOCALDEFEAT";
    m_actionTemplates[ScriptAction::LOCALDEFEAT].m_uiName = "Multiplayer_/ Announce local defeat.";
    m_actionTemplates[ScriptAction::LOCALDEFEAT].m_numParameters = 0;
    m_actionTemplates[ScriptAction::LOCALDEFEAT].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::LOCALDEFEAT].m_uiStrings[0] = "Show 'Game Over' window";

    m_actionTemplates[ScriptAction::VICTORY].m_internalName = "VICTORY";
    m_actionTemplates[ScriptAction::VICTORY].m_uiName = "Multiplayer_/ Announce victory.";
    m_actionTemplates[ScriptAction::VICTORY].m_numParameters = 0;
    m_actionTemplates[ScriptAction::VICTORY].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::VICTORY].m_uiStrings[0] = "Show 'Victorious' window and end game";

    m_actionTemplates[ScriptAction::DEFEAT].m_internalName = "DEFEAT";
    m_actionTemplates[ScriptAction::DEFEAT].m_uiName = "Multiplayer_/ Announce defeat.";
    m_actionTemplates[ScriptAction::DEFEAT].m_numParameters = 0;
    m_actionTemplates[ScriptAction::DEFEAT].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::DEFEAT].m_uiStrings[0] = "Show 'Defeated' window and end game";

    m_actionTemplates[ScriptAction::RESIZE_VIEW_GUARDBAND].m_internalName = "RESIZE_VIEW_GUARDBAND";
    m_actionTemplates[ScriptAction::RESIZE_VIEW_GUARDBAND].m_uiName = "Map_/ Resize view guardband.";
    m_actionTemplates[ScriptAction::RESIZE_VIEW_GUARDBAND].m_numParameters = 2;
    m_actionTemplates[ScriptAction::RESIZE_VIEW_GUARDBAND].m_parameters[0] = Parameter::REAL;
    m_actionTemplates[ScriptAction::RESIZE_VIEW_GUARDBAND].m_parameters[1] = Parameter::REAL;
    m_actionTemplates[ScriptAction::RESIZE_VIEW_GUARDBAND].m_numUiStrings = 3;
    m_actionTemplates[ScriptAction::RESIZE_VIEW_GUARDBAND].m_uiStrings[0] =
        "Allow bigger objects to be perceived as onscreen near the edge (";
    m_actionTemplates[ScriptAction::RESIZE_VIEW_GUARDBAND].m_uiStrings[1], " = ";
    m_actionTemplates[ScriptAction::RESIZE_VIEW_GUARDBAND].m_uiStrings[2] = ") Width then height, in world units.";

    m_actionTemplates[ScriptAction::DELETE_ALL_UNMANNED].m_internalName = "DELETE_ALL_UNMANNED";
    m_actionTemplates[ScriptAction::DELETE_ALL_UNMANNED].m_uiName = "Scripting_/ Delete all unmanned (sniped) vehicles.";
    m_actionTemplates[ScriptAction::DELETE_ALL_UNMANNED].m_numParameters = 0;
    m_actionTemplates[ScriptAction::DELETE_ALL_UNMANNED].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::DELETE_ALL_UNMANNED].m_uiStrings[0] = "Delete all unmanned (sniped) vehicles.";

    m_actionTemplates[ScriptAction::CHOOSE_VICTIM_ALWAYS_USES_NORMAL].m_internalName = "CHOOSE_VICTIM_ALWAYS_USES_NORMAL";
    m_actionTemplates[ScriptAction::CHOOSE_VICTIM_ALWAYS_USES_NORMAL].m_uiName =
        "Map_/ Force ChooseVictim to ignore game difficulty and always use Normal setting.";
    m_actionTemplates[ScriptAction::CHOOSE_VICTIM_ALWAYS_USES_NORMAL].m_numParameters = 1;
    m_actionTemplates[ScriptAction::CHOOSE_VICTIM_ALWAYS_USES_NORMAL].m_parameters[0] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::CHOOSE_VICTIM_ALWAYS_USES_NORMAL].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::CHOOSE_VICTIM_ALWAYS_USES_NORMAL].m_uiStrings[0] =
        "Force ChooseVictim to ignore game difficulty and always use Normal setting ";
    m_actionTemplates[ScriptAction::CHOOSE_VICTIM_ALWAYS_USES_NORMAL].m_uiStrings[1] =
        " (true to enable, false to disable).";

    m_actionTemplates[ScriptAction::SET_TRAIN_HELD].m_internalName = "SET_TRAIN_HELD";
    m_actionTemplates[ScriptAction::SET_TRAIN_HELD].m_uiName =
        "Unit/ Set a train to stay at a station. TRUE = stay. FALSE = go-ahead.";
    m_actionTemplates[ScriptAction::SET_TRAIN_HELD].m_numParameters = 2;
    m_actionTemplates[ScriptAction::SET_TRAIN_HELD].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::SET_TRAIN_HELD].m_parameters[1] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::SET_TRAIN_HELD].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::SET_TRAIN_HELD].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::SET_TRAIN_HELD].m_uiStrings[1] = " sets its held status to ";

    m_actionTemplates[ScriptAction::NAMED_SET_EVAC_LEFT_OR_RIGHT].m_internalName = "NAMED_SET_EVAC_LEFT_OR_RIGHT";
    m_actionTemplates[ScriptAction::NAMED_SET_EVAC_LEFT_OR_RIGHT].m_uiName =
        "Unit/ Set which side of a container (likely a train) you want the riders to exit on.";
    m_actionTemplates[ScriptAction::NAMED_SET_EVAC_LEFT_OR_RIGHT].m_numParameters = 2;
    m_actionTemplates[ScriptAction::NAMED_SET_EVAC_LEFT_OR_RIGHT].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::NAMED_SET_EVAC_LEFT_OR_RIGHT].m_parameters[1] = Parameter::EVACUATE_CONTAINER_SIDE;
    m_actionTemplates[ScriptAction::NAMED_SET_EVAC_LEFT_OR_RIGHT].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::NAMED_SET_EVAC_LEFT_OR_RIGHT].m_uiStrings[0] = " ";
    m_actionTemplates[ScriptAction::NAMED_SET_EVAC_LEFT_OR_RIGHT].m_uiStrings[1] = " will exit its riders on its ";

    m_actionTemplates[ScriptAction::ENABLE_OBJECT_SOUND].m_internalName = "ENABLE_OBJECT_SOUND";
    m_actionTemplates[ScriptAction::ENABLE_OBJECT_SOUND].m_uiName = "Multimedia_/Sound Effect/Enable object's ambient sound";
    m_actionTemplates[ScriptAction::ENABLE_OBJECT_SOUND].m_numParameters = 1;
    m_actionTemplates[ScriptAction::ENABLE_OBJECT_SOUND].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::ENABLE_OBJECT_SOUND].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::ENABLE_OBJECT_SOUND].m_uiStrings[0] = "Enable (or trigger) ";
    m_actionTemplates[ScriptAction::ENABLE_OBJECT_SOUND].m_uiStrings[1] = "'s ambient sound.";

    m_actionTemplates[ScriptAction::DISABLE_OBJECT_SOUND].m_internalName = "DISABLE_OBJECT_SOUND";
    m_actionTemplates[ScriptAction::DISABLE_OBJECT_SOUND].m_uiName =
        "Multimedia_/Sound Effect/Disable object's ambient sound";
    m_actionTemplates[ScriptAction::DISABLE_OBJECT_SOUND].m_numParameters = 1;
    m_actionTemplates[ScriptAction::DISABLE_OBJECT_SOUND].m_parameters[0] = Parameter::UNIT;
    m_actionTemplates[ScriptAction::DISABLE_OBJECT_SOUND].m_numUiStrings = 2;
    m_actionTemplates[ScriptAction::DISABLE_OBJECT_SOUND].m_uiStrings[0] = "Disable ";
    m_actionTemplates[ScriptAction::DISABLE_OBJECT_SOUND].m_uiStrings[1] = "'s ambient sound.";

    m_actionTemplates[ScriptAction::SHOW_WEATHER].m_internalName = "SHOW_WEATHER";
    m_actionTemplates[ScriptAction::SHOW_WEATHER].m_uiName = "Map/Environment/Show Weather.";
    m_actionTemplates[ScriptAction::SHOW_WEATHER].m_numParameters = 1;
    m_actionTemplates[ScriptAction::SHOW_WEATHER].m_parameters[0] = Parameter::BOOLEAN;
    m_actionTemplates[ScriptAction::SHOW_WEATHER].m_numUiStrings = 1;
    m_actionTemplates[ScriptAction::SHOW_WEATHER].m_uiStrings[0] = "Show Weather = ";

    m_conditionTemplates[Condition::CONDITION_FALSE].m_internalName = "CONDITION_FALSE";
    m_conditionTemplates[Condition::CONDITION_FALSE].m_uiName = "Scripting_/ False.";
    m_conditionTemplates[Condition::CONDITION_FALSE].m_numParameters = 0;
    m_conditionTemplates[Condition::CONDITION_FALSE].m_numUiStrings = 1;
    m_conditionTemplates[Condition::CONDITION_FALSE].m_uiStrings[0] = "False.";

    m_conditionTemplates[Condition::COUNTER].m_internalName = "COUNTER";
    m_conditionTemplates[Condition::COUNTER].m_uiName = "Scripting_/ Counter compared to a value.";
    m_conditionTemplates[Condition::COUNTER].m_numParameters = 3;
    m_conditionTemplates[Condition::COUNTER].m_parameters[0] = Parameter::COUNTER;
    m_conditionTemplates[Condition::COUNTER].m_parameters[1] = Parameter::COMPARISON;
    m_conditionTemplates[Condition::COUNTER].m_parameters[2] = Parameter::INT;
    m_conditionTemplates[Condition::COUNTER].m_numUiStrings = 3;
    m_conditionTemplates[Condition::COUNTER].m_uiStrings[0] = "Counter ";
    m_conditionTemplates[Condition::COUNTER].m_uiStrings[1] = " IS ";
    m_conditionTemplates[Condition::COUNTER].m_uiStrings[2] = " ";

    m_conditionTemplates[Condition::UNIT_HEALTH].m_internalName = "UNIT_HEALTH";
    m_conditionTemplates[Condition::UNIT_HEALTH].m_uiName = "Unit_/ Unit health % compared to a value.";
    m_conditionTemplates[Condition::UNIT_HEALTH].m_numParameters = 3;
    m_conditionTemplates[Condition::UNIT_HEALTH].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::UNIT_HEALTH].m_parameters[1] = Parameter::COMPARISON;
    m_conditionTemplates[Condition::UNIT_HEALTH].m_parameters[2] = Parameter::INT;
    m_conditionTemplates[Condition::UNIT_HEALTH].m_numUiStrings = 4;
    m_conditionTemplates[Condition::UNIT_HEALTH].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::UNIT_HEALTH].m_uiStrings[1] = " Health IS ";
    m_conditionTemplates[Condition::UNIT_HEALTH].m_uiStrings[2] = " ";
    m_conditionTemplates[Condition::UNIT_HEALTH].m_uiStrings[3] = " percent.";

    m_conditionTemplates[Condition::FLAG].m_internalName = "FLAG";
    m_conditionTemplates[Condition::FLAG].m_uiName = "Scripting_/ Flag compared to a value.";
    m_conditionTemplates[Condition::FLAG].m_numParameters = 2;
    m_conditionTemplates[Condition::FLAG].m_parameters[0] = Parameter::FLAG;
    m_conditionTemplates[Condition::FLAG].m_parameters[1] = Parameter::BOOLEAN;
    m_conditionTemplates[Condition::FLAG].m_numUiStrings = 2;
    m_conditionTemplates[Condition::FLAG].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::FLAG].m_uiStrings[1] = " IS ";

    m_conditionTemplates[Condition::TEAM_STATE_IS].m_internalName = "TEAM_STATE_IS";
    m_conditionTemplates[Condition::TEAM_STATE_IS].m_uiName = "Team_/ Team state is.";
    m_conditionTemplates[Condition::TEAM_STATE_IS].m_numParameters = 2;
    m_conditionTemplates[Condition::TEAM_STATE_IS].m_parameters[0] = Parameter::TEAM;
    m_conditionTemplates[Condition::TEAM_STATE_IS].m_parameters[1] = Parameter::TEAM_STATE;
    m_conditionTemplates[Condition::TEAM_STATE_IS].m_numUiStrings = 2;
    m_conditionTemplates[Condition::TEAM_STATE_IS].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::TEAM_STATE_IS].m_uiStrings[1] = " state IS ";

    m_conditionTemplates[Condition::TEAM_STATE_IS_NOT].m_internalName = "TEAM_STATE_IS_NOT";
    m_conditionTemplates[Condition::TEAM_STATE_IS_NOT].m_uiName = "Team_/ Team state is not.";
    m_conditionTemplates[Condition::TEAM_STATE_IS_NOT].m_numParameters = 2;
    m_conditionTemplates[Condition::TEAM_STATE_IS_NOT].m_parameters[0] = Parameter::TEAM;
    m_conditionTemplates[Condition::TEAM_STATE_IS_NOT].m_parameters[1] = Parameter::TEAM_STATE;
    m_conditionTemplates[Condition::TEAM_STATE_IS_NOT].m_numUiStrings = 2;
    m_conditionTemplates[Condition::TEAM_STATE_IS_NOT].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::TEAM_STATE_IS_NOT].m_uiStrings[1] = " state IS NOT ";

    m_conditionTemplates[Condition::CONDITION_TRUE].m_internalName = "CONDITION_TRUE";
    m_conditionTemplates[Condition::CONDITION_TRUE].m_uiName = "Scripting_/ True.";
    m_conditionTemplates[Condition::CONDITION_TRUE].m_numParameters = 0;
    m_conditionTemplates[Condition::CONDITION_TRUE].m_numUiStrings = 1;
    m_conditionTemplates[Condition::CONDITION_TRUE].m_uiStrings[0] = "True.";

    m_conditionTemplates[Condition::TIMER_EXPIRED].m_internalName = "TIMER_EXPIRED";
    m_conditionTemplates[Condition::TIMER_EXPIRED].m_uiName = "Scripting_/ Timer expired.";
    m_conditionTemplates[Condition::TIMER_EXPIRED].m_numParameters = 1;
    m_conditionTemplates[Condition::TIMER_EXPIRED].m_parameters[0] = Parameter::COUNTER;
    m_conditionTemplates[Condition::TIMER_EXPIRED].m_numUiStrings = 2;
    m_conditionTemplates[Condition::TIMER_EXPIRED].m_uiStrings[0] = "Timer ";
    m_conditionTemplates[Condition::TIMER_EXPIRED].m_uiStrings[1] = " has expired.";

    m_conditionTemplates[Condition::PLAYER_ALL_DESTROYED].m_internalName = "PLAYER_ALL_DESTROYED";
    m_conditionTemplates[Condition::PLAYER_ALL_DESTROYED].m_uiName = "Player_/ All destroyed.";
    m_conditionTemplates[Condition::PLAYER_ALL_DESTROYED].m_numParameters = 1;
    m_conditionTemplates[Condition::PLAYER_ALL_DESTROYED].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_ALL_DESTROYED].m_numUiStrings = 2;
    m_conditionTemplates[Condition::PLAYER_ALL_DESTROYED].m_uiStrings[0] = "Everything belonging to  ";
    m_conditionTemplates[Condition::PLAYER_ALL_DESTROYED].m_uiStrings[1] = " has been destroyed.";

    m_conditionTemplates[Condition::PLAYER_ALL_BUILDFACILITIES_DESTROYED].m_internalName =
        "PLAYER_ALL_BUILDFACILITIES_DESTROYED";
    m_conditionTemplates[Condition::PLAYER_ALL_BUILDFACILITIES_DESTROYED].m_uiName = "Player_/ All factories destroyed.";
    m_conditionTemplates[Condition::PLAYER_ALL_BUILDFACILITIES_DESTROYED].m_numParameters = 1;
    m_conditionTemplates[Condition::PLAYER_ALL_BUILDFACILITIES_DESTROYED].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_ALL_BUILDFACILITIES_DESTROYED].m_numUiStrings = 2;
    m_conditionTemplates[Condition::PLAYER_ALL_BUILDFACILITIES_DESTROYED].m_uiStrings[0] = "All factories belonging to  ";
    m_conditionTemplates[Condition::PLAYER_ALL_BUILDFACILITIES_DESTROYED].m_uiStrings[1] = " have been destroyed.";

    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_PARTIALLY].m_internalName = "TEAM_INSIDE_AREA_PARTIALLY";
    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_PARTIALLY].m_uiName = "Team_/ Team has units in an area.";
    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_PARTIALLY].m_numParameters = 3;
    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_PARTIALLY].m_parameters[0] = Parameter::TEAM;
    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_PARTIALLY].m_parameters[1] = Parameter::TRIGGER_AREA;
    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_PARTIALLY].m_parameters[2] = Parameter::SURFACES_ALLOWED;
    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_PARTIALLY].m_numUiStrings = 4;
    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_PARTIALLY].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_PARTIALLY].m_uiStrings[1] = " has one or more units in ";
    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_PARTIALLY].m_uiStrings[2] = " (";
    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_PARTIALLY].m_uiStrings[3] = ").";

    m_conditionTemplates[Condition::NAMED_INSIDE_AREA].m_internalName = "NAMED_INSIDE_AREA";
    m_conditionTemplates[Condition::NAMED_INSIDE_AREA].m_uiName = "Unit_/ Unit entered area.";
    m_conditionTemplates[Condition::NAMED_INSIDE_AREA].m_numParameters = 2;
    m_conditionTemplates[Condition::NAMED_INSIDE_AREA].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::NAMED_INSIDE_AREA].m_parameters[1] = Parameter::TRIGGER_AREA;
    m_conditionTemplates[Condition::NAMED_INSIDE_AREA].m_numUiStrings = 4;
    m_conditionTemplates[Condition::NAMED_INSIDE_AREA].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::NAMED_INSIDE_AREA].m_uiStrings[1] = " is in ";
    m_conditionTemplates[Condition::NAMED_INSIDE_AREA].m_uiStrings[2] = " (";
    m_conditionTemplates[Condition::NAMED_INSIDE_AREA].m_uiStrings[3] = ").";

    m_conditionTemplates[Condition::TEAM_DESTROYED].m_internalName = "TEAM_DESTROYED";
    m_conditionTemplates[Condition::TEAM_DESTROYED].m_uiName = "Team_/ Team is destroyed.";
    m_conditionTemplates[Condition::TEAM_DESTROYED].m_numParameters = 1;
    m_conditionTemplates[Condition::TEAM_DESTROYED].m_parameters[0] = Parameter::TEAM;
    m_conditionTemplates[Condition::TEAM_DESTROYED].m_numUiStrings = 2;
    m_conditionTemplates[Condition::TEAM_DESTROYED].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::TEAM_DESTROYED].m_uiStrings[1] = " has been destroyed.";

    m_conditionTemplates[Condition::NAMED_DESTROYED].m_internalName = "NAMED_DESTROYED";
    m_conditionTemplates[Condition::NAMED_DESTROYED].m_uiName = "Unit_/ Unit is destroyed.";
    m_conditionTemplates[Condition::NAMED_DESTROYED].m_numParameters = 1;
    m_conditionTemplates[Condition::NAMED_DESTROYED].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::NAMED_DESTROYED].m_numUiStrings = 2;
    m_conditionTemplates[Condition::NAMED_DESTROYED].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::NAMED_DESTROYED].m_uiStrings[1] = " has been destroyed.";

    m_conditionTemplates[Condition::NAMED_DYING].m_internalName = "NAMED_DYING";
    m_conditionTemplates[Condition::NAMED_DYING].m_uiName = "Unit_/ Unit is dying.";
    m_conditionTemplates[Condition::NAMED_DYING].m_numParameters = 1;
    m_conditionTemplates[Condition::NAMED_DYING].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::NAMED_DYING].m_numUiStrings = 2;
    m_conditionTemplates[Condition::NAMED_DYING].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::NAMED_DYING].m_uiStrings[1] = " has been killed, but still on screen.";

    m_conditionTemplates[Condition::NAMED_TOTALLY_DEAD].m_internalName = "NAMED_TOTALLY_DEAD";
    m_conditionTemplates[Condition::NAMED_TOTALLY_DEAD].m_uiName = "Unit_/ Unit is finished dying.";
    m_conditionTemplates[Condition::NAMED_TOTALLY_DEAD].m_numParameters = 1;
    m_conditionTemplates[Condition::NAMED_TOTALLY_DEAD].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::NAMED_TOTALLY_DEAD].m_numUiStrings = 2;
    m_conditionTemplates[Condition::NAMED_TOTALLY_DEAD].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::NAMED_TOTALLY_DEAD].m_uiStrings[1] = " has been killed, and is finished dying.";

    m_conditionTemplates[Condition::BRIDGE_BROKEN].m_internalName = "BRIDGE_BROKEN";
    m_conditionTemplates[Condition::BRIDGE_BROKEN].m_uiName = "Unit_/ Bridge is broken.";
    m_conditionTemplates[Condition::BRIDGE_BROKEN].m_numParameters = 1;
    m_conditionTemplates[Condition::BRIDGE_BROKEN].m_parameters[0] = Parameter::BRIDGE;
    m_conditionTemplates[Condition::BRIDGE_BROKEN].m_numUiStrings = 2;
    m_conditionTemplates[Condition::BRIDGE_BROKEN].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::BRIDGE_BROKEN].m_uiStrings[1] = " has been broken.";

    m_conditionTemplates[Condition::BRIDGE_REPAIRED].m_internalName = "BRIDGE_REPAIRED";
    m_conditionTemplates[Condition::BRIDGE_REPAIRED].m_uiName = "Unit_/ Bridge is repaired.";
    m_conditionTemplates[Condition::BRIDGE_REPAIRED].m_numParameters = 1;
    m_conditionTemplates[Condition::BRIDGE_REPAIRED].m_parameters[0] = Parameter::BRIDGE;
    m_conditionTemplates[Condition::BRIDGE_REPAIRED].m_numUiStrings = 2;
    m_conditionTemplates[Condition::BRIDGE_REPAIRED].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::BRIDGE_REPAIRED].m_uiStrings[1] = " has been repaired.";

    m_conditionTemplates[Condition::NAMED_NOT_DESTROYED].m_internalName = "NAMED_NOT_DESTROYED";
    m_conditionTemplates[Condition::NAMED_NOT_DESTROYED].m_uiName = "Unit_/ Unit exists and is alive.";
    m_conditionTemplates[Condition::NAMED_NOT_DESTROYED].m_numParameters = 1;
    m_conditionTemplates[Condition::NAMED_NOT_DESTROYED].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::NAMED_NOT_DESTROYED].m_numUiStrings = 2;
    m_conditionTemplates[Condition::NAMED_NOT_DESTROYED].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::NAMED_NOT_DESTROYED].m_uiStrings[1] = " exists and is alive.";

    m_conditionTemplates[Condition::TEAM_HAS_UNITS].m_internalName = "TEAM_HAS_UNITS";
    m_conditionTemplates[Condition::TEAM_HAS_UNITS].m_uiName = "Team_/ Team has units.";
    m_conditionTemplates[Condition::TEAM_HAS_UNITS].m_numParameters = 1;
    m_conditionTemplates[Condition::TEAM_HAS_UNITS].m_parameters[0] = Parameter::TEAM;
    m_conditionTemplates[Condition::TEAM_HAS_UNITS].m_numUiStrings = 2;
    m_conditionTemplates[Condition::TEAM_HAS_UNITS].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::TEAM_HAS_UNITS].m_uiStrings[1] = " has one or more units.";

    m_conditionTemplates[Condition::CAMERA_MOVEMENT_FINISHED].m_internalName = "CAMERA_MOVEMENT_FINISHED";
    m_conditionTemplates[Condition::CAMERA_MOVEMENT_FINISHED].m_uiName = "Camera_/ Camera movement finished.";
    m_conditionTemplates[Condition::CAMERA_MOVEMENT_FINISHED].m_numParameters = 0;
    m_conditionTemplates[Condition::CAMERA_MOVEMENT_FINISHED].m_numUiStrings = 1;
    m_conditionTemplates[Condition::CAMERA_MOVEMENT_FINISHED].m_uiStrings[0] = "The camera movement has finished.";

    m_conditionTemplates[Condition::NAMED_INSIDE_AREA].m_internalName = "NAMED_INSIDE_AREA";
    m_conditionTemplates[Condition::NAMED_INSIDE_AREA].m_uiName = "Unit_/ Unit inside an area.";
    m_conditionTemplates[Condition::NAMED_INSIDE_AREA].m_numParameters = 2;
    m_conditionTemplates[Condition::NAMED_INSIDE_AREA].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::NAMED_INSIDE_AREA].m_parameters[1] = Parameter::TRIGGER_AREA;
    m_conditionTemplates[Condition::NAMED_INSIDE_AREA].m_numUiStrings = 2;
    m_conditionTemplates[Condition::NAMED_INSIDE_AREA].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::NAMED_INSIDE_AREA].m_uiStrings[1] = " is inside ";

    m_conditionTemplates[Condition::NAMED_OUTSIDE_AREA].m_internalName = "NAMED_OUTSIDE_AREA";
    m_conditionTemplates[Condition::NAMED_OUTSIDE_AREA].m_uiName = "Unit_/ Unit outside an area.";
    m_conditionTemplates[Condition::NAMED_OUTSIDE_AREA].m_numParameters = 2;
    m_conditionTemplates[Condition::NAMED_OUTSIDE_AREA].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::NAMED_OUTSIDE_AREA].m_parameters[1] = Parameter::TRIGGER_AREA;
    m_conditionTemplates[Condition::NAMED_OUTSIDE_AREA].m_numUiStrings = 2;
    m_conditionTemplates[Condition::NAMED_OUTSIDE_AREA].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::NAMED_OUTSIDE_AREA].m_uiStrings[1] = " is outside ";

    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_ENTIRELY].m_internalName = "TEAM_INSIDE_AREA_ENTIRELY";
    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_ENTIRELY].m_uiName = "Team_/ Team completely inside an area.";
    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_ENTIRELY].m_numParameters = 3;
    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_ENTIRELY].m_parameters[0] = Parameter::TEAM;
    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_ENTIRELY].m_parameters[1] = Parameter::TRIGGER_AREA;
    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_ENTIRELY].m_parameters[2] = Parameter::SURFACES_ALLOWED;
    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_ENTIRELY].m_numUiStrings = 4;
    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_ENTIRELY].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_ENTIRELY].m_uiStrings[1] = " is all inside ";
    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_ENTIRELY].m_uiStrings[2] = " (";
    m_conditionTemplates[Condition::TEAM_INSIDE_AREA_ENTIRELY].m_uiStrings[3] = ").";

    m_conditionTemplates[Condition::TEAM_OUTSIDE_AREA_ENTIRELY].m_internalName = "TEAM_OUTSIDE_AREA_ENTIRELY";
    m_conditionTemplates[Condition::TEAM_OUTSIDE_AREA_ENTIRELY].m_uiName = "Team_/ Team is completely outside an area.";
    m_conditionTemplates[Condition::TEAM_OUTSIDE_AREA_ENTIRELY].m_numParameters = 3;
    m_conditionTemplates[Condition::TEAM_OUTSIDE_AREA_ENTIRELY].m_parameters[0] = Parameter::TEAM;
    m_conditionTemplates[Condition::TEAM_OUTSIDE_AREA_ENTIRELY].m_parameters[1] = Parameter::TRIGGER_AREA;
    m_conditionTemplates[Condition::TEAM_OUTSIDE_AREA_ENTIRELY].m_parameters[2] = Parameter::SURFACES_ALLOWED;
    m_conditionTemplates[Condition::TEAM_OUTSIDE_AREA_ENTIRELY].m_numUiStrings = 4;
    m_conditionTemplates[Condition::TEAM_OUTSIDE_AREA_ENTIRELY].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::TEAM_OUTSIDE_AREA_ENTIRELY].m_uiStrings[1] = " is completely outside ";
    m_conditionTemplates[Condition::TEAM_OUTSIDE_AREA_ENTIRELY].m_uiStrings[2] = " (";
    m_conditionTemplates[Condition::TEAM_OUTSIDE_AREA_ENTIRELY].m_uiStrings[3] = ").";

    m_conditionTemplates[Condition::NAMED_ATTACKED_BY_OBJECTTYPE].m_internalName = "NAMED_ATTACKED_BY_OBJECTTYPE";
    m_conditionTemplates[Condition::NAMED_ATTACKED_BY_OBJECTTYPE].m_uiName =
        "Unit_/ Unit is attacked by a specific unit type.";
    m_conditionTemplates[Condition::NAMED_ATTACKED_BY_OBJECTTYPE].m_numParameters = 2;
    m_conditionTemplates[Condition::NAMED_ATTACKED_BY_OBJECTTYPE].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::NAMED_ATTACKED_BY_OBJECTTYPE].m_parameters[1] = Parameter::OBJECT_TYPE;
    m_conditionTemplates[Condition::NAMED_ATTACKED_BY_OBJECTTYPE].m_numUiStrings = 2;
    m_conditionTemplates[Condition::NAMED_ATTACKED_BY_OBJECTTYPE].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::NAMED_ATTACKED_BY_OBJECTTYPE].m_uiStrings[1] = " has been attacked by a(n) ";

    m_conditionTemplates[Condition::TEAM_ATTACKED_BY_OBJECTTYPE].m_internalName = "TEAM_ATTACKED_BY_OBJECTTYPE";
    m_conditionTemplates[Condition::TEAM_ATTACKED_BY_OBJECTTYPE].m_uiName =
        "Team_/ Team is attacked by a specific unit type.";
    m_conditionTemplates[Condition::TEAM_ATTACKED_BY_OBJECTTYPE].m_numParameters = 2;
    m_conditionTemplates[Condition::TEAM_ATTACKED_BY_OBJECTTYPE].m_parameters[0] = Parameter::TEAM;
    m_conditionTemplates[Condition::TEAM_ATTACKED_BY_OBJECTTYPE].m_parameters[1] = Parameter::OBJECT_TYPE;
    m_conditionTemplates[Condition::TEAM_ATTACKED_BY_OBJECTTYPE].m_numUiStrings = 2;
    m_conditionTemplates[Condition::TEAM_ATTACKED_BY_OBJECTTYPE].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::TEAM_ATTACKED_BY_OBJECTTYPE].m_uiStrings[1] = " has been attacked by a(n) ";

    m_conditionTemplates[Condition::NAMED_ATTACKED_BY_PLAYER].m_internalName = "NAMED_ATTACKED_BY_PLAYER";
    m_conditionTemplates[Condition::NAMED_ATTACKED_BY_PLAYER].m_uiName = "Unit_/ Unit has been attacked by a player.";
    m_conditionTemplates[Condition::NAMED_ATTACKED_BY_PLAYER].m_numParameters = 2;
    m_conditionTemplates[Condition::NAMED_ATTACKED_BY_PLAYER].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::NAMED_ATTACKED_BY_PLAYER].m_parameters[1] = Parameter::SIDE;
    m_conditionTemplates[Condition::NAMED_ATTACKED_BY_PLAYER].m_numUiStrings = 2;
    m_conditionTemplates[Condition::NAMED_ATTACKED_BY_PLAYER].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::NAMED_ATTACKED_BY_PLAYER].m_uiStrings[1] = " has been attacked by ";

    m_conditionTemplates[Condition::TEAM_ATTACKED_BY_PLAYER].m_internalName = "TEAM_ATTACKED_BY_PLAYER";
    m_conditionTemplates[Condition::TEAM_ATTACKED_BY_PLAYER].m_uiName = "Team_/ Team has been attacked by a player.";
    m_conditionTemplates[Condition::TEAM_ATTACKED_BY_PLAYER].m_numParameters = 2;
    m_conditionTemplates[Condition::TEAM_ATTACKED_BY_PLAYER].m_parameters[0] = Parameter::TEAM;
    m_conditionTemplates[Condition::TEAM_ATTACKED_BY_PLAYER].m_parameters[1] = Parameter::SIDE;
    m_conditionTemplates[Condition::TEAM_ATTACKED_BY_PLAYER].m_numUiStrings = 2;
    m_conditionTemplates[Condition::TEAM_ATTACKED_BY_PLAYER].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::TEAM_ATTACKED_BY_PLAYER].m_uiStrings[1] = " has been attacked by ";

    m_conditionTemplates[Condition::BUILT_BY_PLAYER].m_internalName = "BUILT_BY_PLAYER";
    m_conditionTemplates[Condition::BUILT_BY_PLAYER].m_uiName = "Player_/ Player has built an object type.";
    m_conditionTemplates[Condition::BUILT_BY_PLAYER].m_numParameters = 2;
    m_conditionTemplates[Condition::BUILT_BY_PLAYER].m_parameters[0] = Parameter::OBJECT_TYPE;
    m_conditionTemplates[Condition::BUILT_BY_PLAYER].m_parameters[1] = Parameter::SIDE;
    m_conditionTemplates[Condition::BUILT_BY_PLAYER].m_numUiStrings = 2;
    m_conditionTemplates[Condition::BUILT_BY_PLAYER].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::BUILT_BY_PLAYER].m_uiStrings[1] = " has been built by ";

    m_conditionTemplates[Condition::NAMED_CREATED].m_internalName = "NAMED_CREATED";
    m_conditionTemplates[Condition::NAMED_CREATED].m_uiName = "Unit_/ Unit has been created.";
    m_conditionTemplates[Condition::NAMED_CREATED].m_numParameters = 1;
    m_conditionTemplates[Condition::NAMED_CREATED].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::NAMED_CREATED].m_numUiStrings = 2;
    m_conditionTemplates[Condition::NAMED_CREATED].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::NAMED_CREATED].m_uiStrings[1] = " has been created.";

    m_conditionTemplates[Condition::TEAM_CREATED].m_internalName = "TEAM_CREATED";
    m_conditionTemplates[Condition::TEAM_CREATED].m_uiName = "Team_/ Team has been created.";
    m_conditionTemplates[Condition::TEAM_CREATED].m_numParameters = 1;
    m_conditionTemplates[Condition::TEAM_CREATED].m_parameters[0] = Parameter::TEAM;
    m_conditionTemplates[Condition::TEAM_CREATED].m_numUiStrings = 2;
    m_conditionTemplates[Condition::TEAM_CREATED].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::TEAM_CREATED].m_uiStrings[1] = " has been created.";

    m_conditionTemplates[Condition::PLAYER_HAS_CREDITS].m_internalName = "PLAYER_HAS_CREDITS";
    m_conditionTemplates[Condition::PLAYER_HAS_CREDITS].m_uiName =
        "Player_/ Player has (comparison) to a number of credits.";
    m_conditionTemplates[Condition::PLAYER_HAS_CREDITS].m_numParameters = 3;
    m_conditionTemplates[Condition::PLAYER_HAS_CREDITS].m_parameters[0] = Parameter::INT;
    m_conditionTemplates[Condition::PLAYER_HAS_CREDITS].m_parameters[1] = Parameter::COMPARISON;
    m_conditionTemplates[Condition::PLAYER_HAS_CREDITS].m_parameters[2] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_HAS_CREDITS].m_numUiStrings = 3;
    m_conditionTemplates[Condition::PLAYER_HAS_CREDITS].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::PLAYER_HAS_CREDITS].m_uiStrings[1] = " is ";
    m_conditionTemplates[Condition::PLAYER_HAS_CREDITS].m_uiStrings[2] = " the number of credits possessed by ";

    m_conditionTemplates[Condition::NAMED_DISCOVERED].m_internalName = "NAMED_DISCOVERED";
    m_conditionTemplates[Condition::NAMED_DISCOVERED].m_uiName = "Player_/ Player has discovered a specific unit.";
    m_conditionTemplates[Condition::NAMED_DISCOVERED].m_numParameters = 2;
    m_conditionTemplates[Condition::NAMED_DISCOVERED].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::NAMED_DISCOVERED].m_parameters[1] = Parameter::SIDE;
    m_conditionTemplates[Condition::NAMED_DISCOVERED].m_numUiStrings = 2;
    m_conditionTemplates[Condition::NAMED_DISCOVERED].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::NAMED_DISCOVERED].m_uiStrings[1] = " has been discovered by ";

    m_conditionTemplates[Condition::NAMED_BUILDING_IS_EMPTY].m_internalName = "NAMED_BUILDING_IS_EMPTY";
    m_conditionTemplates[Condition::NAMED_BUILDING_IS_EMPTY].m_uiName = "Unit_/ A specific building is empty.";
    m_conditionTemplates[Condition::NAMED_BUILDING_IS_EMPTY].m_numParameters = 1;
    m_conditionTemplates[Condition::NAMED_BUILDING_IS_EMPTY].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::NAMED_BUILDING_IS_EMPTY].m_numUiStrings = 2;
    m_conditionTemplates[Condition::NAMED_BUILDING_IS_EMPTY].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::NAMED_BUILDING_IS_EMPTY].m_uiStrings[1] = " is empty.";

    m_conditionTemplates[Condition::BUILDING_ENTERED_BY_PLAYER].m_internalName = "BUILDING_ENTERED_BY_PLAYER";
    m_conditionTemplates[Condition::BUILDING_ENTERED_BY_PLAYER].m_uiName =
        "Player_/ Player has entered a specific building.";
    m_conditionTemplates[Condition::BUILDING_ENTERED_BY_PLAYER].m_numParameters = 2;
    m_conditionTemplates[Condition::BUILDING_ENTERED_BY_PLAYER].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::BUILDING_ENTERED_BY_PLAYER].m_parameters[1] = Parameter::UNIT;
    m_conditionTemplates[Condition::BUILDING_ENTERED_BY_PLAYER].m_numUiStrings = 2;
    m_conditionTemplates[Condition::BUILDING_ENTERED_BY_PLAYER].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::BUILDING_ENTERED_BY_PLAYER].m_uiStrings[1] = " has entered building named ";

    m_conditionTemplates[Condition::ENEMY_SIGHTED].m_internalName = "ENEMY_SIGHTED";
    m_conditionTemplates[Condition::ENEMY_SIGHTED].m_uiName =
        "Unit_/ Unit has sighted a(n) friendly/neutral/enemy unit belonging to a side.";
    m_conditionTemplates[Condition::ENEMY_SIGHTED].m_numParameters = 3;
    m_conditionTemplates[Condition::ENEMY_SIGHTED].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::ENEMY_SIGHTED].m_parameters[1] = Parameter::RELATION;
    m_conditionTemplates[Condition::ENEMY_SIGHTED].m_parameters[2] = Parameter::SIDE;
    m_conditionTemplates[Condition::ENEMY_SIGHTED].m_numUiStrings = 4;
    m_conditionTemplates[Condition::ENEMY_SIGHTED].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::ENEMY_SIGHTED].m_uiStrings[1] = " sees a(n) ";
    m_conditionTemplates[Condition::ENEMY_SIGHTED].m_uiStrings[2] = " unit belonging to ";
    m_conditionTemplates[Condition::ENEMY_SIGHTED].m_uiStrings[3] = ".";

    m_conditionTemplates[Condition::TYPE_SIGHTED].m_internalName = "TYPE_SIGHTED";
    m_conditionTemplates[Condition::TYPE_SIGHTED].m_uiName = "Unit_/ Unit has sighted a type of unit belonging to a side.";
    m_conditionTemplates[Condition::TYPE_SIGHTED].m_numParameters = 3;
    m_conditionTemplates[Condition::TYPE_SIGHTED].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::TYPE_SIGHTED].m_parameters[1] = Parameter::OBJECT_TYPE;
    m_conditionTemplates[Condition::TYPE_SIGHTED].m_parameters[2] = Parameter::SIDE;
    m_conditionTemplates[Condition::TYPE_SIGHTED].m_numUiStrings = 4;
    m_conditionTemplates[Condition::TYPE_SIGHTED].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::TYPE_SIGHTED].m_uiStrings[1] = " sees a(n) ";
    m_conditionTemplates[Condition::TYPE_SIGHTED].m_uiStrings[2] = " belonging to ";
    m_conditionTemplates[Condition::TYPE_SIGHTED].m_uiStrings[3] = ".";

    m_conditionTemplates[Condition::TEAM_DISCOVERED].m_internalName = "TEAM_DISCOVERED";
    m_conditionTemplates[Condition::TEAM_DISCOVERED].m_uiName = "Player_/ Player has discovered a team.";
    m_conditionTemplates[Condition::TEAM_DISCOVERED].m_numParameters = 2;
    m_conditionTemplates[Condition::TEAM_DISCOVERED].m_parameters[0] = Parameter::TEAM;
    m_conditionTemplates[Condition::TEAM_DISCOVERED].m_parameters[1] = Parameter::SIDE;
    m_conditionTemplates[Condition::TEAM_DISCOVERED].m_numUiStrings = 2;
    m_conditionTemplates[Condition::TEAM_DISCOVERED].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::TEAM_DISCOVERED].m_uiStrings[1] = " has been discovered by ";

    m_conditionTemplates[Condition::MISSION_ATTEMPTS].m_internalName = "MISSION_ATTEMPTS";
    m_conditionTemplates[Condition::MISSION_ATTEMPTS].m_uiName =
        "Player_/ Player has attempted the mission a number of times.";
    m_conditionTemplates[Condition::MISSION_ATTEMPTS].m_numParameters = 3;
    m_conditionTemplates[Condition::MISSION_ATTEMPTS].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::MISSION_ATTEMPTS].m_parameters[1] = Parameter::COMPARISON;
    m_conditionTemplates[Condition::MISSION_ATTEMPTS].m_parameters[2] = Parameter::INT;
    m_conditionTemplates[Condition::MISSION_ATTEMPTS].m_numUiStrings = 4;
    m_conditionTemplates[Condition::MISSION_ATTEMPTS].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::MISSION_ATTEMPTS].m_uiStrings[1] = " has attempted the mission ";
    m_conditionTemplates[Condition::MISSION_ATTEMPTS].m_uiStrings[2] = " ";
    m_conditionTemplates[Condition::MISSION_ATTEMPTS].m_uiStrings[3] = " times.";

    m_conditionTemplates[Condition::NAMED_OWNED_BY_PLAYER].m_internalName = "NAMED_OWNED_BY_PLAYER";
    m_conditionTemplates[Condition::NAMED_OWNED_BY_PLAYER].m_uiName = "Player_/ Player owns the specific Unit.";
    m_conditionTemplates[Condition::NAMED_OWNED_BY_PLAYER].m_numParameters = 2;
    m_conditionTemplates[Condition::NAMED_OWNED_BY_PLAYER].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::NAMED_OWNED_BY_PLAYER].m_parameters[1] = Parameter::SIDE;
    m_conditionTemplates[Condition::NAMED_OWNED_BY_PLAYER].m_numUiStrings = 2;
    m_conditionTemplates[Condition::NAMED_OWNED_BY_PLAYER].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::NAMED_OWNED_BY_PLAYER].m_uiStrings[1] = " is owned by ";

    m_conditionTemplates[Condition::TEAM_OWNED_BY_PLAYER].m_internalName = "TEAM_OWNED_BY_PLAYER";
    m_conditionTemplates[Condition::TEAM_OWNED_BY_PLAYER].m_uiName = "Player_/ Player owns a specific team.";
    m_conditionTemplates[Condition::TEAM_OWNED_BY_PLAYER].m_numParameters = 2;
    m_conditionTemplates[Condition::TEAM_OWNED_BY_PLAYER].m_parameters[0] = Parameter::TEAM;
    m_conditionTemplates[Condition::TEAM_OWNED_BY_PLAYER].m_parameters[1] = Parameter::SIDE;
    m_conditionTemplates[Condition::TEAM_OWNED_BY_PLAYER].m_numUiStrings = 2;
    m_conditionTemplates[Condition::TEAM_OWNED_BY_PLAYER].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::TEAM_OWNED_BY_PLAYER].m_uiStrings[1] = " is owned by ";

    m_conditionTemplates[Condition::PLAYER_HAS_N_OR_FEWER_BUILDINGS].m_internalName = "PLAYER_HAS_N_OR_FEWER_BUILDINGS";
    m_conditionTemplates[Condition::PLAYER_HAS_N_OR_FEWER_BUILDINGS].m_uiName =
        "Player_/ Player currently owns N or fewer buildings.";
    m_conditionTemplates[Condition::PLAYER_HAS_N_OR_FEWER_BUILDINGS].m_numParameters = 2;
    m_conditionTemplates[Condition::PLAYER_HAS_N_OR_FEWER_BUILDINGS].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_HAS_N_OR_FEWER_BUILDINGS].m_parameters[1] = Parameter::INT;
    m_conditionTemplates[Condition::PLAYER_HAS_N_OR_FEWER_BUILDINGS].m_numUiStrings = 3;
    m_conditionTemplates[Condition::PLAYER_HAS_N_OR_FEWER_BUILDINGS].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::PLAYER_HAS_N_OR_FEWER_BUILDINGS].m_uiStrings[1] = " currently owns ";
    m_conditionTemplates[Condition::PLAYER_HAS_N_OR_FEWER_BUILDINGS].m_uiStrings[2] = " or fewer buildings.";

    m_conditionTemplates[Condition::PLAYER_HAS_N_OR_FEWER_FACTION_BUILDINGS].m_internalName =
        "PLAYER_HAS_N_OR_FEWER_FACTION_BUILDINGS";
    m_conditionTemplates[Condition::PLAYER_HAS_N_OR_FEWER_FACTION_BUILDINGS].m_uiName =
        "Player_/ Player currently owns N or fewer faction buildings.";
    m_conditionTemplates[Condition::PLAYER_HAS_N_OR_FEWER_FACTION_BUILDINGS].m_numParameters = 2;
    m_conditionTemplates[Condition::PLAYER_HAS_N_OR_FEWER_FACTION_BUILDINGS].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_HAS_N_OR_FEWER_FACTION_BUILDINGS].m_parameters[1] = Parameter::INT;
    m_conditionTemplates[Condition::PLAYER_HAS_N_OR_FEWER_FACTION_BUILDINGS].m_numUiStrings = 3;
    m_conditionTemplates[Condition::PLAYER_HAS_N_OR_FEWER_FACTION_BUILDINGS].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::PLAYER_HAS_N_OR_FEWER_FACTION_BUILDINGS].m_uiStrings[1] = " currently owns ";
    m_conditionTemplates[Condition::PLAYER_HAS_N_OR_FEWER_FACTION_BUILDINGS].m_uiStrings[2] = " or fewer faction buildings.";

    m_conditionTemplates[Condition::PLAYER_HAS_POWER].m_internalName = "PLAYER_HAS_POWER";
    m_conditionTemplates[Condition::PLAYER_HAS_POWER].m_uiName = "Player_/ Player's base currently has power.";
    m_conditionTemplates[Condition::PLAYER_HAS_POWER].m_numParameters = 1;
    m_conditionTemplates[Condition::PLAYER_HAS_POWER].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_HAS_POWER].m_numUiStrings = 2;
    m_conditionTemplates[Condition::PLAYER_HAS_POWER].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::PLAYER_HAS_POWER].m_uiStrings[1] = " buildings are powered.";

    m_conditionTemplates[Condition::PLAYER_HAS_NO_POWER].m_internalName = "PLAYER_HAS_NO_POWER";
    m_conditionTemplates[Condition::PLAYER_HAS_NO_POWER].m_uiName = "Player_/ Player's base currently has no power.";
    m_conditionTemplates[Condition::PLAYER_HAS_NO_POWER].m_numParameters = 1;
    m_conditionTemplates[Condition::PLAYER_HAS_NO_POWER].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_HAS_NO_POWER].m_numUiStrings = 2;
    m_conditionTemplates[Condition::PLAYER_HAS_NO_POWER].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::PLAYER_HAS_NO_POWER].m_uiStrings[1] = " buildings are not powered.";

    m_conditionTemplates[Condition::NAMED_REACHED_WAYPOINTS_END].m_internalName = "NAMED_REACHED_WAYPOINTS_END";
    m_conditionTemplates[Condition::NAMED_REACHED_WAYPOINTS_END].m_uiName =
        "Unit_/ Unit has reached the end of a specific waypoint path.";
    m_conditionTemplates[Condition::NAMED_REACHED_WAYPOINTS_END].m_numParameters = 2;
    m_conditionTemplates[Condition::NAMED_REACHED_WAYPOINTS_END].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::NAMED_REACHED_WAYPOINTS_END].m_parameters[1] = Parameter::WAYPOINT_PATH;
    m_conditionTemplates[Condition::NAMED_REACHED_WAYPOINTS_END].m_numUiStrings = 2;
    m_conditionTemplates[Condition::NAMED_REACHED_WAYPOINTS_END].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::NAMED_REACHED_WAYPOINTS_END].m_uiStrings[1] = " has reached the end of ";

    m_conditionTemplates[Condition::TEAM_REACHED_WAYPOINTS_END].m_internalName = "TEAM_REACHED_WAYPOINTS_END";
    m_conditionTemplates[Condition::TEAM_REACHED_WAYPOINTS_END].m_uiName =
        "Team_/ Team has reached the end of a specific waypoint path.";
    m_conditionTemplates[Condition::TEAM_REACHED_WAYPOINTS_END].m_numParameters = 2;
    m_conditionTemplates[Condition::TEAM_REACHED_WAYPOINTS_END].m_parameters[0] = Parameter::TEAM;
    m_conditionTemplates[Condition::TEAM_REACHED_WAYPOINTS_END].m_parameters[1] = Parameter::WAYPOINT_PATH;
    m_conditionTemplates[Condition::TEAM_REACHED_WAYPOINTS_END].m_numUiStrings = 2;
    m_conditionTemplates[Condition::TEAM_REACHED_WAYPOINTS_END].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::TEAM_REACHED_WAYPOINTS_END].m_uiStrings[1] = " has reached the end of ";

    m_conditionTemplates[Condition::NAMED_SELECTED].m_internalName = "NAMED_SELECTED";
    m_conditionTemplates[Condition::NAMED_SELECTED].m_uiName = "Unit_/ Unit currently selected.";
    m_conditionTemplates[Condition::NAMED_SELECTED].m_numParameters = 1;
    m_conditionTemplates[Condition::NAMED_SELECTED].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::NAMED_SELECTED].m_numUiStrings = 2;
    m_conditionTemplates[Condition::NAMED_SELECTED].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::NAMED_SELECTED].m_uiStrings[1] = " is currently selected.";

    m_conditionTemplates[Condition::NAMED_ENTERED_AREA].m_internalName = "NAMED_ENTERED_AREA";
    m_conditionTemplates[Condition::NAMED_ENTERED_AREA].m_uiName = "Unit_/ Unit enters an area.";
    m_conditionTemplates[Condition::NAMED_ENTERED_AREA].m_numParameters = 2;
    m_conditionTemplates[Condition::NAMED_ENTERED_AREA].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::NAMED_ENTERED_AREA].m_parameters[1] = Parameter::TRIGGER_AREA;
    m_conditionTemplates[Condition::NAMED_ENTERED_AREA].m_numUiStrings = 2;
    m_conditionTemplates[Condition::NAMED_ENTERED_AREA].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::NAMED_ENTERED_AREA].m_uiStrings[1] = " enters ";

    m_conditionTemplates[Condition::NAMED_EXITED_AREA].m_internalName = "NAMED_EXITED_AREA";
    m_conditionTemplates[Condition::NAMED_EXITED_AREA].m_uiName = "Unit_/ Unit exits an area.";
    m_conditionTemplates[Condition::NAMED_EXITED_AREA].m_numParameters = 2;
    m_conditionTemplates[Condition::NAMED_EXITED_AREA].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::NAMED_EXITED_AREA].m_parameters[1] = Parameter::TRIGGER_AREA;
    m_conditionTemplates[Condition::NAMED_EXITED_AREA].m_numUiStrings = 2;
    m_conditionTemplates[Condition::NAMED_EXITED_AREA].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::NAMED_EXITED_AREA].m_uiStrings[1] = " exits ";

    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_ENTIRELY].m_internalName = "TEAM_ENTERED_AREA_ENTIRELY";
    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_ENTIRELY].m_uiName = "Team_/ Team entirely enters an area.";
    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_ENTIRELY].m_numParameters = 3;
    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_ENTIRELY].m_parameters[0] = Parameter::TEAM;
    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_ENTIRELY].m_parameters[1] = Parameter::TRIGGER_AREA;
    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_ENTIRELY].m_parameters[2] = Parameter::SURFACES_ALLOWED;
    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_ENTIRELY].m_numUiStrings = 4;
    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_ENTIRELY].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_ENTIRELY].m_uiStrings[1] = " all enter ";
    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_ENTIRELY].m_uiStrings[2] = " (";
    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_ENTIRELY].m_uiStrings[3] = ").";

    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_PARTIALLY].m_internalName = "TEAM_ENTERED_AREA_PARTIALLY";
    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_PARTIALLY].m_uiName = "Team_/ One unit enters an area.";
    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_PARTIALLY].m_numParameters = 3;
    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_PARTIALLY].m_parameters[0] = Parameter::TEAM;
    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_PARTIALLY].m_parameters[1] = Parameter::TRIGGER_AREA;
    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_PARTIALLY].m_parameters[2] = Parameter::SURFACES_ALLOWED;
    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_PARTIALLY].m_numUiStrings = 4;
    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_PARTIALLY].m_uiStrings[0] = "One unit from ";
    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_PARTIALLY].m_uiStrings[1] = " enters ";
    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_PARTIALLY].m_uiStrings[2] = " (";
    m_conditionTemplates[Condition::TEAM_ENTERED_AREA_PARTIALLY].m_uiStrings[3] = ").";

    m_conditionTemplates[Condition::TEAM_EXITED_AREA_ENTIRELY].m_internalName = "TEAM_EXITED_AREA_ENTIRELY";
    m_conditionTemplates[Condition::TEAM_EXITED_AREA_ENTIRELY].m_uiName = "Team_/ Team entirely exits an area.";
    m_conditionTemplates[Condition::TEAM_EXITED_AREA_ENTIRELY].m_numParameters = 3;
    m_conditionTemplates[Condition::TEAM_EXITED_AREA_ENTIRELY].m_parameters[0] = Parameter::TEAM;
    m_conditionTemplates[Condition::TEAM_EXITED_AREA_ENTIRELY].m_parameters[1] = Parameter::TRIGGER_AREA;
    m_conditionTemplates[Condition::TEAM_EXITED_AREA_ENTIRELY].m_parameters[2] = Parameter::SURFACES_ALLOWED;
    m_conditionTemplates[Condition::TEAM_EXITED_AREA_ENTIRELY].m_numUiStrings = 4;
    m_conditionTemplates[Condition::TEAM_EXITED_AREA_ENTIRELY].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::TEAM_EXITED_AREA_ENTIRELY].m_uiStrings[1] = " all exit ";
    m_conditionTemplates[Condition::TEAM_EXITED_AREA_ENTIRELY].m_uiStrings[2] = " (";
    m_conditionTemplates[Condition::TEAM_EXITED_AREA_ENTIRELY].m_uiStrings[3] = ").";

    m_conditionTemplates[Condition::TEAM_EXITED_AREA_PARTIALLY].m_internalName = "TEAM_EXITED_AREA_PARTIALLY";
    m_conditionTemplates[Condition::TEAM_EXITED_AREA_PARTIALLY].m_uiName = "Team_/ One unit exits an area.";
    m_conditionTemplates[Condition::TEAM_EXITED_AREA_PARTIALLY].m_numParameters = 3;
    m_conditionTemplates[Condition::TEAM_EXITED_AREA_PARTIALLY].m_parameters[0] = Parameter::TEAM;
    m_conditionTemplates[Condition::TEAM_EXITED_AREA_PARTIALLY].m_parameters[1] = Parameter::TRIGGER_AREA;
    m_conditionTemplates[Condition::TEAM_EXITED_AREA_PARTIALLY].m_parameters[2] = Parameter::SURFACES_ALLOWED;
    m_conditionTemplates[Condition::TEAM_EXITED_AREA_PARTIALLY].m_numUiStrings = 4;
    m_conditionTemplates[Condition::TEAM_EXITED_AREA_PARTIALLY].m_uiStrings[0] = "One unit from ";
    m_conditionTemplates[Condition::TEAM_EXITED_AREA_PARTIALLY].m_uiStrings[1] = " exits ";
    m_conditionTemplates[Condition::TEAM_EXITED_AREA_PARTIALLY].m_uiStrings[2] = " (";
    m_conditionTemplates[Condition::TEAM_EXITED_AREA_PARTIALLY].m_uiStrings[3] = ").";

    m_conditionTemplates[Condition::MULTIPLAYER_ALLIED_VICTORY].m_internalName = "MULTIPLAYER_ALLIED_VICTORY";
    m_conditionTemplates[Condition::MULTIPLAYER_ALLIED_VICTORY].m_uiName = "Multiplayer_/ Multiplayer allied victory.";
    m_conditionTemplates[Condition::MULTIPLAYER_ALLIED_VICTORY].m_numParameters = 0;
    m_conditionTemplates[Condition::MULTIPLAYER_ALLIED_VICTORY].m_numUiStrings = 1;
    m_conditionTemplates[Condition::MULTIPLAYER_ALLIED_VICTORY].m_uiStrings[0] =
        "The multiplayer game has ended in victory for the local player and his allies.";

    m_conditionTemplates[Condition::MULTIPLAYER_ALLIED_DEFEAT].m_internalName = "MULTIPLAYER_ALLIED_DEFEAT";
    m_conditionTemplates[Condition::MULTIPLAYER_ALLIED_DEFEAT].m_uiName = "Multiplayer_/ Multiplayer allied defeat.";
    m_conditionTemplates[Condition::MULTIPLAYER_ALLIED_DEFEAT].m_numParameters = 0;
    m_conditionTemplates[Condition::MULTIPLAYER_ALLIED_DEFEAT].m_numUiStrings = 1;
    m_conditionTemplates[Condition::MULTIPLAYER_ALLIED_DEFEAT].m_uiStrings[0] =
        "The multiplayer game has ended in defeat for the local player and his allies.";

    m_conditionTemplates[Condition::MULTIPLAYER_PLAYER_DEFEAT].m_internalName = "MULTIPLAYER_PLAYER_DEFEAT";
    m_conditionTemplates[Condition::MULTIPLAYER_PLAYER_DEFEAT].m_uiName =
        "Multiplayer_/ Multiplayer local player defeat check.";
    m_conditionTemplates[Condition::MULTIPLAYER_PLAYER_DEFEAT].m_numParameters = 0;
    m_conditionTemplates[Condition::MULTIPLAYER_PLAYER_DEFEAT].m_numUiStrings = 1;
    m_conditionTemplates[Condition::MULTIPLAYER_PLAYER_DEFEAT].m_uiStrings[0] =
        "Everything belonging to the local player has been destroyed, but his all"
        "ies may or may not have been defeated.";

    m_conditionTemplates[Condition::HAS_FINISHED_VIDEO].m_internalName = "HAS_FINISHED_VIDEO";
    m_conditionTemplates[Condition::HAS_FINISHED_VIDEO].m_uiName = "Multimedia_/ Video has completed playing.";
    m_conditionTemplates[Condition::HAS_FINISHED_VIDEO].m_numParameters = 1;
    m_conditionTemplates[Condition::HAS_FINISHED_VIDEO].m_parameters[0] = Parameter::MOVIE;
    m_conditionTemplates[Condition::HAS_FINISHED_VIDEO].m_numUiStrings = 2;
    m_conditionTemplates[Condition::HAS_FINISHED_VIDEO].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::HAS_FINISHED_VIDEO].m_uiStrings[1] = " has completed playing.";

    m_conditionTemplates[Condition::HAS_FINISHED_SPEECH].m_internalName = "HAS_FINISHED_SPEECH";
    m_conditionTemplates[Condition::HAS_FINISHED_SPEECH].m_uiName = "Multimedia_/ Speech has completed playing.";
    m_conditionTemplates[Condition::HAS_FINISHED_SPEECH].m_numParameters = 1;
    m_conditionTemplates[Condition::HAS_FINISHED_SPEECH].m_parameters[0] = Parameter::DIALOG;
    m_conditionTemplates[Condition::HAS_FINISHED_SPEECH].m_numUiStrings = 2;
    m_conditionTemplates[Condition::HAS_FINISHED_SPEECH].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::HAS_FINISHED_SPEECH].m_uiStrings[1] = " has completed playing.";

    m_conditionTemplates[Condition::HAS_FINISHED_AUDIO].m_internalName = "HAS_FINISHED_AUDIO";
    m_conditionTemplates[Condition::HAS_FINISHED_AUDIO].m_uiName = "Multimedia_/ Sound has completed playing.";
    m_conditionTemplates[Condition::HAS_FINISHED_AUDIO].m_numParameters = 1;
    m_conditionTemplates[Condition::HAS_FINISHED_AUDIO].m_parameters[0] = Parameter::SOUND;
    m_conditionTemplates[Condition::HAS_FINISHED_AUDIO].m_numUiStrings = 2;
    m_conditionTemplates[Condition::HAS_FINISHED_AUDIO].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::HAS_FINISHED_AUDIO].m_uiStrings[1] = " has completed playing.";

    m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER].m_internalName = "PLAYER_TRIGGERED_SPECIAL_POWER";
    m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER].m_uiName =
        "Player_/ Player starts using a special power.";
    m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER].m_numParameters = 2;
    m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER].m_parameters[1] = Parameter::SPECIAL_POWER;
    m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER].m_numUiStrings = 3;
    m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER].m_uiStrings[0] = "Player ";
    m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER].m_uiStrings[1] = " starts using ";
    m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER].m_uiStrings[2] = ".";

    m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER_FROM_NAMED].m_internalName =
        "PLAYER_TRIGGERED_SPECIAL_POWER_FROM_NAMED";
    m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER_FROM_NAMED].m_uiName =
        "Player_/ Player start using a special power from a named unit.";
    m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER_FROM_NAMED].m_numParameters = 3;
    m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER_FROM_NAMED].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER_FROM_NAMED].m_parameters[1] = Parameter::SPECIAL_POWER;
    m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER_FROM_NAMED].m_parameters[2] = Parameter::UNIT;
    m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER_FROM_NAMED].m_numUiStrings = 4;
    m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER_FROM_NAMED].m_uiStrings[0] = "Player ";
    m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER_FROM_NAMED].m_uiStrings[1] = " starts using ";
    m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER_FROM_NAMED].m_uiStrings[2] = " from ";
    m_conditionTemplates[Condition::PLAYER_TRIGGERED_SPECIAL_POWER_FROM_NAMED].m_uiStrings[3] = ".";

    m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER].m_internalName = "PLAYER_MIDWAY_SPECIAL_POWER";
    m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER].m_uiName =
        "Player_/ Player is midway through using a special power.";
    m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER].m_numParameters = 2;
    m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER].m_parameters[1] = Parameter::SPECIAL_POWER;
    m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER].m_numUiStrings = 3;
    m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER].m_uiStrings[0] = "Player ";
    m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER].m_uiStrings[1] = " is midway using ";
    m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER].m_uiStrings[2] = ".";

    m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER_FROM_NAMED].m_internalName =
        "PLAYER_MIDWAY_SPECIAL_POWER_FROM_NAMED";
    m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER_FROM_NAMED].m_uiName =
        "Player_/ Player is midway through using a special power from a named unit.";
    m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER_FROM_NAMED].m_numParameters = 3;
    m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER_FROM_NAMED].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER_FROM_NAMED].m_parameters[1] = Parameter::SPECIAL_POWER;
    m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER_FROM_NAMED].m_parameters[2] = Parameter::UNIT;
    m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER_FROM_NAMED].m_numUiStrings = 4;
    m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER_FROM_NAMED].m_uiStrings[0] = "Player ";
    m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER_FROM_NAMED].m_uiStrings[1] = " is midway using ";
    m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER_FROM_NAMED].m_uiStrings[2] = " from ";
    m_conditionTemplates[Condition::PLAYER_MIDWAY_SPECIAL_POWER_FROM_NAMED].m_uiStrings[3] = ".";

    m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER].m_internalName = "PLAYER_COMPLETED_SPECIAL_POWER";
    m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER].m_uiName =
        "Player_/ Player completed using a special power.";
    m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER].m_numParameters = 2;
    m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER].m_parameters[1] = Parameter::SPECIAL_POWER;
    m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER].m_numUiStrings = 3;
    m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER].m_uiStrings[0] = "Player ";
    m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER].m_uiStrings[1] = " completed using ";
    m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER].m_uiStrings[2] = ".";

    m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER_FROM_NAMED].m_internalName =
        "PLAYER_COMPLETED_SPECIAL_POWER_FROM_NAMED";
    m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER_FROM_NAMED].m_uiName =
        "Player_/ Player completed using a special power from a named unit.";
    m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER_FROM_NAMED].m_numParameters = 3;
    m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER_FROM_NAMED].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER_FROM_NAMED].m_parameters[1] = Parameter::SPECIAL_POWER;
    m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER_FROM_NAMED].m_parameters[2] = Parameter::UNIT;
    m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER_FROM_NAMED].m_numUiStrings = 4;
    m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER_FROM_NAMED].m_uiStrings[0] = "Player ";
    m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER_FROM_NAMED].m_uiStrings[1] = " completed using ";
    m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER_FROM_NAMED].m_uiStrings[2] = " from ";
    m_conditionTemplates[Condition::PLAYER_COMPLETED_SPECIAL_POWER_FROM_NAMED].m_uiStrings[3] = ".";

    m_conditionTemplates[Condition::PLAYER_ACQUIRED_SCIENCE].m_internalName = "PLAYER_ACQUIRED_SCIENCE";
    m_conditionTemplates[Condition::PLAYER_ACQUIRED_SCIENCE].m_uiName = "Player_/ Player acquired a Science.";
    m_conditionTemplates[Condition::PLAYER_ACQUIRED_SCIENCE].m_numParameters = 2;
    m_conditionTemplates[Condition::PLAYER_ACQUIRED_SCIENCE].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_ACQUIRED_SCIENCE].m_parameters[1] = Parameter::SCIENCE;
    m_conditionTemplates[Condition::PLAYER_ACQUIRED_SCIENCE].m_numUiStrings = 3;
    m_conditionTemplates[Condition::PLAYER_ACQUIRED_SCIENCE].m_uiStrings[0] = "Player ";
    m_conditionTemplates[Condition::PLAYER_ACQUIRED_SCIENCE].m_uiStrings[1] = " acquired ";
    m_conditionTemplates[Condition::PLAYER_ACQUIRED_SCIENCE].m_uiStrings[2] = ".";

    m_conditionTemplates[Condition::PLAYER_CAN_PURCHASE_SCIENCE].m_internalName = "PLAYER_CAN_PURCHASE_SCIENCE";
    m_conditionTemplates[Condition::PLAYER_CAN_PURCHASE_SCIENCE].m_uiName =
        "Player_/ Player can purchase a particular Science (has all prereqs & points).";
    m_conditionTemplates[Condition::PLAYER_CAN_PURCHASE_SCIENCE].m_numParameters = 2;
    m_conditionTemplates[Condition::PLAYER_CAN_PURCHASE_SCIENCE].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_CAN_PURCHASE_SCIENCE].m_parameters[1] = Parameter::SCIENCE;
    m_conditionTemplates[Condition::PLAYER_CAN_PURCHASE_SCIENCE].m_numUiStrings = 3;
    m_conditionTemplates[Condition::PLAYER_CAN_PURCHASE_SCIENCE].m_uiStrings[0] = "Player ";
    m_conditionTemplates[Condition::PLAYER_CAN_PURCHASE_SCIENCE].m_uiStrings[1] = " can purchase ";
    m_conditionTemplates[Condition::PLAYER_CAN_PURCHASE_SCIENCE].m_uiStrings[2] = ".";

    m_conditionTemplates[Condition::PLAYER_HAS_SCIENCEPURCHASEPOINTS].m_internalName = "PLAYER_HAS_SCIENCEPURCHASEPOINTS";
    m_conditionTemplates[Condition::PLAYER_HAS_SCIENCEPURCHASEPOINTS].m_uiName =
        "Player_/ Player has a certain number of Science Purchase Points available.";
    m_conditionTemplates[Condition::PLAYER_HAS_SCIENCEPURCHASEPOINTS].m_numParameters = 2;
    m_conditionTemplates[Condition::PLAYER_HAS_SCIENCEPURCHASEPOINTS].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_HAS_SCIENCEPURCHASEPOINTS].m_parameters[1] = Parameter::INT;
    m_conditionTemplates[Condition::PLAYER_HAS_SCIENCEPURCHASEPOINTS].m_numUiStrings = 3;
    m_conditionTemplates[Condition::PLAYER_HAS_SCIENCEPURCHASEPOINTS].m_uiStrings[0] = "Player ";
    m_conditionTemplates[Condition::PLAYER_HAS_SCIENCEPURCHASEPOINTS].m_uiStrings[1] = " has at least ";
    m_conditionTemplates[Condition::PLAYER_HAS_SCIENCEPURCHASEPOINTS].m_uiStrings[2] = " Science Purchase Points available.";

    m_conditionTemplates[Condition::NAMED_HAS_FREE_CONTAINER_SLOTS].m_internalName = "NAMED_HAS_FREE_CONTAINER_SLOTS";
    m_conditionTemplates[Condition::NAMED_HAS_FREE_CONTAINER_SLOTS].m_uiName = "Unit_/ Unit has free container slots.";
    m_conditionTemplates[Condition::NAMED_HAS_FREE_CONTAINER_SLOTS].m_numParameters = 1;
    m_conditionTemplates[Condition::NAMED_HAS_FREE_CONTAINER_SLOTS].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::NAMED_HAS_FREE_CONTAINER_SLOTS].m_numUiStrings = 2;
    m_conditionTemplates[Condition::NAMED_HAS_FREE_CONTAINER_SLOTS].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::NAMED_HAS_FREE_CONTAINER_SLOTS].m_uiStrings[1] = " has free container slots.";

    m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE].m_internalName = "PLAYER_BUILT_UPGRADE";
    m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE].m_uiName = "Player_/ Player built an upgrade.";
    m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE].m_numParameters = 2;
    m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE].m_parameters[1] = Parameter::UPGRADE;
    m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE].m_numUiStrings = 3;
    m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE].m_uiStrings[0] = "Player ";
    m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE].m_uiStrings[1] = " built ";
    m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE].m_uiStrings[2] = ".";

    m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE_FROM_NAMED].m_internalName = "PLAYER_BUILT_UPGRADE_FROM_NAMED";
    m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE_FROM_NAMED].m_uiName =
        "Player_/ Player built an upgrade from a named unit.";
    m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE_FROM_NAMED].m_numParameters = 3;
    m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE_FROM_NAMED].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE_FROM_NAMED].m_parameters[1] = Parameter::UPGRADE;
    m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE_FROM_NAMED].m_parameters[2] = Parameter::UNIT;
    m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE_FROM_NAMED].m_numUiStrings = 4;
    m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE_FROM_NAMED].m_uiStrings[0] = "Player ";
    m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE_FROM_NAMED].m_uiStrings[1] = " built ";
    m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE_FROM_NAMED].m_uiStrings[2] = " from ";
    m_conditionTemplates[Condition::PLAYER_BUILT_UPGRADE_FROM_NAMED].m_uiStrings[3] = ".";

    m_conditionTemplates[Condition::PLAYER_DESTROYED_N_BUILDINGS_PLAYER].m_internalName =
        "PLAYER_DESTROYED_N_BUILDINGS_PLAYER";
    m_conditionTemplates[Condition::PLAYER_DESTROYED_N_BUILDINGS_PLAYER].m_uiName =
        "Player_/ Player destroyed N or more of an opponent's buildings.";
    m_conditionTemplates[Condition::PLAYER_DESTROYED_N_BUILDINGS_PLAYER].m_numParameters = 3;
    m_conditionTemplates[Condition::PLAYER_DESTROYED_N_BUILDINGS_PLAYER].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_DESTROYED_N_BUILDINGS_PLAYER].m_parameters[1] = Parameter::INT;
    m_conditionTemplates[Condition::PLAYER_DESTROYED_N_BUILDINGS_PLAYER].m_parameters[2] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_DESTROYED_N_BUILDINGS_PLAYER].m_numUiStrings = 4;
    m_conditionTemplates[Condition::PLAYER_DESTROYED_N_BUILDINGS_PLAYER].m_uiStrings[0] = "Player ";
    m_conditionTemplates[Condition::PLAYER_DESTROYED_N_BUILDINGS_PLAYER].m_uiStrings[1] = " destroyed ";
    m_conditionTemplates[Condition::PLAYER_DESTROYED_N_BUILDINGS_PLAYER].m_uiStrings[2] = " or more buildings owned by ";
    m_conditionTemplates[Condition::PLAYER_DESTROYED_N_BUILDINGS_PLAYER].m_uiStrings[3] = ".";

    m_conditionTemplates[Condition::PLAYER_HAS_OBJECT_COMPARISON].m_internalName = "PLAYER_HAS_OBJECT_COMPARISON";
    m_conditionTemplates[Condition::PLAYER_HAS_OBJECT_COMPARISON].m_uiName = "Player_/ Player has (comparison) unit type.";
    m_conditionTemplates[Condition::PLAYER_HAS_OBJECT_COMPARISON].m_numParameters = 4;
    m_conditionTemplates[Condition::PLAYER_HAS_OBJECT_COMPARISON].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_HAS_OBJECT_COMPARISON].m_parameters[1] = Parameter::COMPARISON;
    m_conditionTemplates[Condition::PLAYER_HAS_OBJECT_COMPARISON].m_parameters[2] = Parameter::INT;
    m_conditionTemplates[Condition::PLAYER_HAS_OBJECT_COMPARISON].m_parameters[3] = Parameter::OBJECT_TYPE;
    m_conditionTemplates[Condition::PLAYER_HAS_OBJECT_COMPARISON].m_numUiStrings = 4;
    m_conditionTemplates[Condition::PLAYER_HAS_OBJECT_COMPARISON].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::PLAYER_HAS_OBJECT_COMPARISON].m_uiStrings[1] = " has ";
    m_conditionTemplates[Condition::PLAYER_HAS_OBJECT_COMPARISON].m_uiStrings[2] = " ";
    m_conditionTemplates[Condition::PLAYER_HAS_OBJECT_COMPARISON].m_uiStrings[3] = " unit or structure of type ";

    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_TYPE_IN_TRIGGER_AREA].m_internalName =
        "PLAYER_HAS_COMPARISON_UNIT_TYPE_IN_TRIGGER_AREA";
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_TYPE_IN_TRIGGER_AREA].m_uiName =
        "Player_/ Player has (comparison) unit type in an area.";
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_TYPE_IN_TRIGGER_AREA].m_numParameters = 5;
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_TYPE_IN_TRIGGER_AREA].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_TYPE_IN_TRIGGER_AREA].m_parameters[1] = Parameter::COMPARISON;
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_TYPE_IN_TRIGGER_AREA].m_parameters[2] = Parameter::INT;
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_TYPE_IN_TRIGGER_AREA].m_parameters[3] =
        Parameter::OBJECT_TYPE;
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_TYPE_IN_TRIGGER_AREA].m_parameters[4] =
        Parameter::TRIGGER_AREA;
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_TYPE_IN_TRIGGER_AREA].m_numUiStrings = 5;
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_TYPE_IN_TRIGGER_AREA].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_TYPE_IN_TRIGGER_AREA].m_uiStrings[1] = " has ";
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_TYPE_IN_TRIGGER_AREA].m_uiStrings[2] = " ";
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_TYPE_IN_TRIGGER_AREA].m_uiStrings[3] =
        " unit or structure of type ";
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_TYPE_IN_TRIGGER_AREA].m_uiStrings[4] = " in the ";

    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_KIND_IN_TRIGGER_AREA].m_internalName =
        "PLAYER_HAS_COMPARISON_UNIT_KIND_IN_TRIGGER_AREA";
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_KIND_IN_TRIGGER_AREA].m_uiName =
        "Player_/ Player has (comparison) kind of unit or structure in an area.";
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_KIND_IN_TRIGGER_AREA].m_numParameters = 5;
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_KIND_IN_TRIGGER_AREA].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_KIND_IN_TRIGGER_AREA].m_parameters[1] = Parameter::COMPARISON;
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_KIND_IN_TRIGGER_AREA].m_parameters[2] = Parameter::INT;
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_KIND_IN_TRIGGER_AREA].m_parameters[3] =
        Parameter::KIND_OF_PARAM;
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_KIND_IN_TRIGGER_AREA].m_parameters[4] =
        Parameter::TRIGGER_AREA;
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_KIND_IN_TRIGGER_AREA].m_numUiStrings = 5;
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_KIND_IN_TRIGGER_AREA].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_KIND_IN_TRIGGER_AREA].m_uiStrings[1] = " has ";
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_KIND_IN_TRIGGER_AREA].m_uiStrings[2] = " ";
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_KIND_IN_TRIGGER_AREA].m_uiStrings[3] =
        " unit or structure with ";
    m_conditionTemplates[Condition::PLAYER_HAS_COMPARISON_UNIT_KIND_IN_TRIGGER_AREA].m_uiStrings[4] = " in the ";

    m_conditionTemplates[Condition::PLAYER_POWER_COMPARE_PERCENT].m_internalName = "PLAYER_POWER_COMPARE_PERCENT";
    m_conditionTemplates[Condition::PLAYER_POWER_COMPARE_PERCENT].m_uiName =
        "Player_/ Player has (comparison) percent power supply to consumption.";
    m_conditionTemplates[Condition::PLAYER_POWER_COMPARE_PERCENT].m_numParameters = 3;
    m_conditionTemplates[Condition::PLAYER_POWER_COMPARE_PERCENT].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_POWER_COMPARE_PERCENT].m_parameters[1] = Parameter::COMPARISON;
    m_conditionTemplates[Condition::PLAYER_POWER_COMPARE_PERCENT].m_parameters[2] = Parameter::INT;
    m_conditionTemplates[Condition::PLAYER_POWER_COMPARE_PERCENT].m_numUiStrings = 4;
    m_conditionTemplates[Condition::PLAYER_POWER_COMPARE_PERCENT].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::PLAYER_POWER_COMPARE_PERCENT].m_uiStrings[1] = " has ";
    m_conditionTemplates[Condition::PLAYER_POWER_COMPARE_PERCENT].m_uiStrings[2] = " ";
    m_conditionTemplates[Condition::PLAYER_POWER_COMPARE_PERCENT].m_uiStrings[3] = " percent power supply ratio.";

    m_conditionTemplates[Condition::PLAYER_EXCESS_POWER_COMPARE_VALUE].m_internalName = "PLAYER_EXCESS_POWER_COMPARE_VALUE";
    m_conditionTemplates[Condition::PLAYER_EXCESS_POWER_COMPARE_VALUE].m_uiName =
        "Player_/ Player has (comparison) kilowatts excess power supply.";
    m_conditionTemplates[Condition::PLAYER_EXCESS_POWER_COMPARE_VALUE].m_numParameters = 3;
    m_conditionTemplates[Condition::PLAYER_EXCESS_POWER_COMPARE_VALUE].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_EXCESS_POWER_COMPARE_VALUE].m_parameters[1] = Parameter::COMPARISON;
    m_conditionTemplates[Condition::PLAYER_EXCESS_POWER_COMPARE_VALUE].m_parameters[2] = Parameter::INT;
    m_conditionTemplates[Condition::PLAYER_EXCESS_POWER_COMPARE_VALUE].m_numUiStrings = 4;
    m_conditionTemplates[Condition::PLAYER_EXCESS_POWER_COMPARE_VALUE].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::PLAYER_EXCESS_POWER_COMPARE_VALUE].m_uiStrings[1] = " has ";
    m_conditionTemplates[Condition::PLAYER_EXCESS_POWER_COMPARE_VALUE].m_uiStrings[2] = " ";
    m_conditionTemplates[Condition::PLAYER_EXCESS_POWER_COMPARE_VALUE].m_uiStrings[3] = " excess kilowatts power supply.";

    m_conditionTemplates[Condition::UNIT_EMPTIED].m_internalName = "UNIT_EMPTIED";
    m_conditionTemplates[Condition::UNIT_EMPTIED].m_uiName = "Unit_/ Unit has emptied its contents.";
    m_conditionTemplates[Condition::UNIT_EMPTIED].m_numParameters = 1;
    m_conditionTemplates[Condition::UNIT_EMPTIED].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::UNIT_EMPTIED].m_numUiStrings = 2;
    m_conditionTemplates[Condition::UNIT_EMPTIED].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::UNIT_EMPTIED].m_uiStrings[1] = " emptied its contents.";

    m_conditionTemplates[Condition::SKIRMISH_SPECIAL_POWER_READY].m_internalName = "SKIRMISH_SPECIAL_POWER_READY";
    m_conditionTemplates[Condition::SKIRMISH_SPECIAL_POWER_READY].m_uiName =
        "Skirmish_/ Player's special power is ready to fire.";
    m_conditionTemplates[Condition::SKIRMISH_SPECIAL_POWER_READY].m_numParameters = 2;
    m_conditionTemplates[Condition::SKIRMISH_SPECIAL_POWER_READY].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::SKIRMISH_SPECIAL_POWER_READY].m_parameters[1] = Parameter::SPECIAL_POWER;
    m_conditionTemplates[Condition::SKIRMISH_SPECIAL_POWER_READY].m_numUiStrings = 3;
    m_conditionTemplates[Condition::SKIRMISH_SPECIAL_POWER_READY].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::SKIRMISH_SPECIAL_POWER_READY].m_uiStrings[1] = " is ready to fire ";
    m_conditionTemplates[Condition::SKIRMISH_SPECIAL_POWER_READY].m_uiStrings[2] = ".";

    m_conditionTemplates[Condition::UNIT_HAS_OBJECT_STATUS].m_internalName = "UNIT_HAS_OBJECT_STATUS";
    m_conditionTemplates[Condition::UNIT_HAS_OBJECT_STATUS].m_uiName = "Unit_/ Unit has object status.";
    m_conditionTemplates[Condition::UNIT_HAS_OBJECT_STATUS].m_numParameters = 2;
    m_conditionTemplates[Condition::UNIT_HAS_OBJECT_STATUS].m_parameters[0] = Parameter::UNIT;
    m_conditionTemplates[Condition::UNIT_HAS_OBJECT_STATUS].m_parameters[1] = Parameter::OBJECT_STATUS;
    m_conditionTemplates[Condition::UNIT_HAS_OBJECT_STATUS].m_numUiStrings = 2;
    m_conditionTemplates[Condition::UNIT_HAS_OBJECT_STATUS].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::UNIT_HAS_OBJECT_STATUS].m_uiStrings[1] = " has ";

    m_conditionTemplates[Condition::TEAM_ALL_HAS_OBJECT_STATUS].m_internalName = "TEAM_ALL_HAS_OBJECT_STATUS";
    m_conditionTemplates[Condition::TEAM_ALL_HAS_OBJECT_STATUS].m_uiName = "Team_/ Team has object status - all.";
    m_conditionTemplates[Condition::TEAM_ALL_HAS_OBJECT_STATUS].m_numParameters = 2;
    m_conditionTemplates[Condition::TEAM_ALL_HAS_OBJECT_STATUS].m_parameters[0] = Parameter::TEAM;
    m_conditionTemplates[Condition::TEAM_ALL_HAS_OBJECT_STATUS].m_parameters[1] = Parameter::OBJECT_STATUS;
    m_conditionTemplates[Condition::TEAM_ALL_HAS_OBJECT_STATUS].m_numUiStrings = 2;
    m_conditionTemplates[Condition::TEAM_ALL_HAS_OBJECT_STATUS].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::TEAM_ALL_HAS_OBJECT_STATUS].m_uiStrings[1] = " has ";

    m_conditionTemplates[Condition::TEAM_SOME_HAVE_OBJECT_STATUS].m_internalName = "TEAM_SOME_HAVE_OBJECT_STATUS";
    m_conditionTemplates[Condition::TEAM_SOME_HAVE_OBJECT_STATUS].m_uiName = "Team_/ Team has object status - partial.";
    m_conditionTemplates[Condition::TEAM_SOME_HAVE_OBJECT_STATUS].m_numParameters = 2;
    m_conditionTemplates[Condition::TEAM_SOME_HAVE_OBJECT_STATUS].m_parameters[0] = Parameter::TEAM;
    m_conditionTemplates[Condition::TEAM_SOME_HAVE_OBJECT_STATUS].m_parameters[1] = Parameter::OBJECT_STATUS;
    m_conditionTemplates[Condition::TEAM_SOME_HAVE_OBJECT_STATUS].m_numUiStrings = 2;
    m_conditionTemplates[Condition::TEAM_SOME_HAVE_OBJECT_STATUS].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::TEAM_SOME_HAVE_OBJECT_STATUS].m_uiStrings[1] = " has ";

    m_conditionTemplates[Condition::SKIRMISH_VALUE_IN_AREA].m_internalName = "SKIRMISH_VALUE_IN_AREA";
    m_conditionTemplates[Condition::SKIRMISH_VALUE_IN_AREA].m_uiName = "Skirmish Only_/ Player has total value in area.";
    m_conditionTemplates[Condition::SKIRMISH_VALUE_IN_AREA].m_numParameters = 4;
    m_conditionTemplates[Condition::SKIRMISH_VALUE_IN_AREA].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::SKIRMISH_VALUE_IN_AREA].m_parameters[1] = Parameter::COMPARISON;
    m_conditionTemplates[Condition::SKIRMISH_VALUE_IN_AREA].m_parameters[2] = Parameter::INT;
    m_conditionTemplates[Condition::SKIRMISH_VALUE_IN_AREA].m_parameters[3] = Parameter::TRIGGER_AREA;
    m_conditionTemplates[Condition::SKIRMISH_VALUE_IN_AREA].m_numUiStrings = 4;
    m_conditionTemplates[Condition::SKIRMISH_VALUE_IN_AREA].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::SKIRMISH_VALUE_IN_AREA].m_uiStrings[1] = " has ";
    m_conditionTemplates[Condition::SKIRMISH_VALUE_IN_AREA].m_uiStrings[2] = " ";
    m_conditionTemplates[Condition::SKIRMISH_VALUE_IN_AREA].m_uiStrings[3] = " within area ";

    m_conditionTemplates[Condition::SKIRMISH_PLAYER_FACTION].m_internalName = "SKIRMISH_PLAYER_FACTION";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_FACTION].m_uiName = "Skirmish_/ Player is faction. - untested";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_FACTION].m_numParameters = 2;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_FACTION].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_FACTION].m_parameters[1] = Parameter::FACTION_NAME;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_FACTION].m_numUiStrings = 2;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_FACTION].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_FACTION].m_uiStrings[1] = " is ";

    m_conditionTemplates[Condition::SKIRMISH_SUPPLIES_VALUE_WITHIN_DISTANCE].m_internalName =
        "SKIRMISH_SUPPLIES_VALUE_WITHIN_DISTANCE";
    m_conditionTemplates[Condition::SKIRMISH_SUPPLIES_VALUE_WITHIN_DISTANCE].m_uiName =
        "Skirmish Only_/ Supplies are within specified distance.";
    m_conditionTemplates[Condition::SKIRMISH_SUPPLIES_VALUE_WITHIN_DISTANCE].m_numParameters = 4;
    m_conditionTemplates[Condition::SKIRMISH_SUPPLIES_VALUE_WITHIN_DISTANCE].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::SKIRMISH_SUPPLIES_VALUE_WITHIN_DISTANCE].m_parameters[1] = Parameter::REAL;
    m_conditionTemplates[Condition::SKIRMISH_SUPPLIES_VALUE_WITHIN_DISTANCE].m_parameters[2] = Parameter::TRIGGER_AREA;
    m_conditionTemplates[Condition::SKIRMISH_SUPPLIES_VALUE_WITHIN_DISTANCE].m_parameters[3] = Parameter::REAL;
    m_conditionTemplates[Condition::SKIRMISH_SUPPLIES_VALUE_WITHIN_DISTANCE].m_numUiStrings = 4;
    m_conditionTemplates[Condition::SKIRMISH_SUPPLIES_VALUE_WITHIN_DISTANCE].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::SKIRMISH_SUPPLIES_VALUE_WITHIN_DISTANCE].m_uiStrings[1] = " has supplies within ";
    m_conditionTemplates[Condition::SKIRMISH_SUPPLIES_VALUE_WITHIN_DISTANCE].m_uiStrings[2] = " of ";
    m_conditionTemplates[Condition::SKIRMISH_SUPPLIES_VALUE_WITHIN_DISTANCE].m_uiStrings[3] = " worth at least ";

    m_conditionTemplates[Condition::SKIRMISH_TECH_BUILDING_WITHIN_DISTANCE].m_internalName =
        "SKIRMISH_TECH_BUILDING_WITHIN_DISTANCE";
    m_conditionTemplates[Condition::SKIRMISH_TECH_BUILDING_WITHIN_DISTANCE].m_uiName =
        "Skirmish Only_/ Tech building is within specified distance.";
    m_conditionTemplates[Condition::SKIRMISH_TECH_BUILDING_WITHIN_DISTANCE].m_numParameters = 3;
    m_conditionTemplates[Condition::SKIRMISH_TECH_BUILDING_WITHIN_DISTANCE].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::SKIRMISH_TECH_BUILDING_WITHIN_DISTANCE].m_parameters[1] = Parameter::REAL;
    m_conditionTemplates[Condition::SKIRMISH_TECH_BUILDING_WITHIN_DISTANCE].m_parameters[2] = Parameter::TRIGGER_AREA;
    m_conditionTemplates[Condition::SKIRMISH_TECH_BUILDING_WITHIN_DISTANCE].m_numUiStrings = 3;
    m_conditionTemplates[Condition::SKIRMISH_TECH_BUILDING_WITHIN_DISTANCE].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::SKIRMISH_TECH_BUILDING_WITHIN_DISTANCE].m_uiStrings[1] = " has a tech building within ";
    m_conditionTemplates[Condition::SKIRMISH_TECH_BUILDING_WITHIN_DISTANCE].m_uiStrings[2] = " of ";

    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_ALL].m_internalName = "SKIRMISH_COMMAND_BUTTON_READY_ALL";
    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_ALL].m_uiName =
        "Skirmish_/ Command Ability is ready - all.";
    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_ALL].m_numParameters = 3;
    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_ALL].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_ALL].m_parameters[1] = Parameter::TEAM;
    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_ALL].m_parameters[2] =
        Parameter::TEAM_COMMANDBUTTON_ABILITY;
    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_ALL].m_numUiStrings = 4;
    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_ALL].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_ALL].m_uiStrings[1] = "'s ";
    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_ALL].m_uiStrings[2] = " are ready to use ";
    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_ALL].m_uiStrings[3] = " (all applicable members).";

    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_PARTIAL].m_internalName =
        "SKIRMISH_COMMAND_BUTTON_READY_PARTIAL";
    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_PARTIAL].m_uiName =
        "Skirmish_/ Command Ability is ready - partial";
    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_PARTIAL].m_numParameters = 3;
    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_PARTIAL].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_PARTIAL].m_parameters[1] = Parameter::TEAM;
    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_PARTIAL].m_parameters[2] =
        Parameter::TEAM_COMMANDBUTTON_ABILITY;
    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_PARTIAL].m_numUiStrings = 4;
    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_PARTIAL].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_PARTIAL].m_uiStrings[1] = "'s ";
    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_PARTIAL].m_uiStrings[2] = " are ready to use ";
    m_conditionTemplates[Condition::SKIRMISH_COMMAND_BUTTON_READY_PARTIAL].m_uiStrings[3] = " (at least one member).";

    m_conditionTemplates[Condition::SKIRMISH_UNOWNED_FACTION_UNIT_EXISTS].m_internalName =
        "SKIRMISH_UNOWNED_FACTION_UNIT_EXISTS";
    m_conditionTemplates[Condition::SKIRMISH_UNOWNED_FACTION_UNIT_EXISTS].m_uiName =
        "Skirmish_/ Unowned faction unit -- comparison.";
    m_conditionTemplates[Condition::SKIRMISH_UNOWNED_FACTION_UNIT_EXISTS].m_numParameters = 3;
    m_conditionTemplates[Condition::SKIRMISH_UNOWNED_FACTION_UNIT_EXISTS].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::SKIRMISH_UNOWNED_FACTION_UNIT_EXISTS].m_parameters[1] = Parameter::COMPARISON;
    m_conditionTemplates[Condition::SKIRMISH_UNOWNED_FACTION_UNIT_EXISTS].m_parameters[2] = Parameter::INT;
    m_conditionTemplates[Condition::SKIRMISH_UNOWNED_FACTION_UNIT_EXISTS].m_numUiStrings = 4;
    m_conditionTemplates[Condition::SKIRMISH_UNOWNED_FACTION_UNIT_EXISTS].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::SKIRMISH_UNOWNED_FACTION_UNIT_EXISTS].m_uiStrings[1] = ". There are ";
    m_conditionTemplates[Condition::SKIRMISH_UNOWNED_FACTION_UNIT_EXISTS].m_uiStrings[2] = " ";
    m_conditionTemplates[Condition::SKIRMISH_UNOWNED_FACTION_UNIT_EXISTS].m_uiStrings[3] = " unowned faction units.";

    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_PREREQUISITE_TO_BUILD].m_internalName =
        "SKIRMISH_PLAYER_HAS_PREREQUISITE_TO_BUILD";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_PREREQUISITE_TO_BUILD].m_uiName =
        "Skirmish_/ Player has prerequisites to build an object type.";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_PREREQUISITE_TO_BUILD].m_numParameters = 2;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_PREREQUISITE_TO_BUILD].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_PREREQUISITE_TO_BUILD].m_parameters[1] = Parameter::OBJECT_TYPE;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_PREREQUISITE_TO_BUILD].m_numUiStrings = 3;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_PREREQUISITE_TO_BUILD].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_PREREQUISITE_TO_BUILD].m_uiStrings[1] = " can build ";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_PREREQUISITE_TO_BUILD].m_uiStrings[2] = ".";

    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_GARRISONED].m_internalName =
        "SKIRMISH_PLAYER_HAS_COMPARISON_GARRISONED";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_GARRISONED].m_uiName =
        "Skirmish_/ Player has garrisoned buildings -- comparison.";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_GARRISONED].m_numParameters = 3;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_GARRISONED].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_GARRISONED].m_parameters[1] = Parameter::COMPARISON;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_GARRISONED].m_parameters[2] = Parameter::INT;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_GARRISONED].m_numUiStrings = 4;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_GARRISONED].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_GARRISONED].m_uiStrings[1] = " has ";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_GARRISONED].m_uiStrings[2] = " ";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_GARRISONED].m_uiStrings[3] = " garrisoned buildings.";

    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_CAPTURED_UNITS].m_internalName =
        "SKIRMISH_PLAYER_HAS_COMPARISON_CAPTURED_UNITS";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_CAPTURED_UNITS].m_uiName =
        "Skirmish_/ Player has captured units -- comparison";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_CAPTURED_UNITS].m_numParameters = 3;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_CAPTURED_UNITS].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_CAPTURED_UNITS].m_parameters[1] = Parameter::COMPARISON;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_CAPTURED_UNITS].m_parameters[2] = Parameter::INT;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_CAPTURED_UNITS].m_numUiStrings = 4;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_CAPTURED_UNITS].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_CAPTURED_UNITS].m_uiStrings[1] = " has captured ";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_CAPTURED_UNITS].m_uiStrings[2] = " ";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_COMPARISON_CAPTURED_UNITS].m_uiStrings[3] = " units.";

    m_conditionTemplates[Condition::SKIRMISH_NAMED_AREA_EXIST].m_internalName = "SKIRMISH_NAMED_AREA_EXIST";
    m_conditionTemplates[Condition::SKIRMISH_NAMED_AREA_EXIST].m_uiName = "Skirmish_/ Area exists.";
    m_conditionTemplates[Condition::SKIRMISH_NAMED_AREA_EXIST].m_numParameters = 2;
    m_conditionTemplates[Condition::SKIRMISH_NAMED_AREA_EXIST].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::SKIRMISH_NAMED_AREA_EXIST].m_parameters[1] = Parameter::TRIGGER_AREA;
    m_conditionTemplates[Condition::SKIRMISH_NAMED_AREA_EXIST].m_numUiStrings = 3;
    m_conditionTemplates[Condition::SKIRMISH_NAMED_AREA_EXIST].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::SKIRMISH_NAMED_AREA_EXIST].m_uiStrings[1] = ". ";
    m_conditionTemplates[Condition::SKIRMISH_NAMED_AREA_EXIST].m_uiStrings[2] = " exists.";

    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_UNITS_IN_AREA].m_internalName = "SKIRMISH_PLAYER_HAS_UNITS_IN_AREA";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_UNITS_IN_AREA].m_uiName = "Skirmish_/ Player has units in an area";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_UNITS_IN_AREA].m_numParameters = 2;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_UNITS_IN_AREA].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_UNITS_IN_AREA].m_parameters[1] = Parameter::TRIGGER_AREA;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_UNITS_IN_AREA].m_numUiStrings = 3;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_UNITS_IN_AREA].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_UNITS_IN_AREA].m_uiStrings[1] = " has units in ";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_UNITS_IN_AREA].m_uiStrings[2] = ".";

    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_BEEN_ATTACKED_BY_PLAYER].m_internalName =
        "SKIRMISH_PLAYER_HAS_BEEN_ATTACKED_BY_PLAYER";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_BEEN_ATTACKED_BY_PLAYER].m_uiName =
        "Skirmish_/ Player has been attacked by player.";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_BEEN_ATTACKED_BY_PLAYER].m_numParameters = 2;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_BEEN_ATTACKED_BY_PLAYER].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_BEEN_ATTACKED_BY_PLAYER].m_parameters[1] = Parameter::SIDE;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_BEEN_ATTACKED_BY_PLAYER].m_numUiStrings = 3;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_BEEN_ATTACKED_BY_PLAYER].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_BEEN_ATTACKED_BY_PLAYER].m_uiStrings[1] = " has been attacked by ";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_BEEN_ATTACKED_BY_PLAYER].m_uiStrings[2] = ".";

    m_conditionTemplates[Condition::SKIRMISH_PLAYER_IS_OUTSIDE_AREA].m_internalName = "SKIRMISH_PLAYER_IS_OUTSIDE_AREA";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_IS_OUTSIDE_AREA].m_uiName =
        "Skirmish_/ Player doesn't have units in an area.";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_IS_OUTSIDE_AREA].m_numParameters = 2;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_IS_OUTSIDE_AREA].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_IS_OUTSIDE_AREA].m_parameters[1] = Parameter::TRIGGER_AREA;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_IS_OUTSIDE_AREA].m_numUiStrings = 3;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_IS_OUTSIDE_AREA].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_IS_OUTSIDE_AREA].m_uiStrings[1] = " has doesn't have units in ";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_IS_OUTSIDE_AREA].m_uiStrings[2] = ".";

    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_DISCOVERED_PLAYER].m_internalName =
        "SKIRMISH_PLAYER_HAS_DISCOVERED_PLAYER";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_DISCOVERED_PLAYER].m_uiName =
        "Skirmish_/ Player has discovered another player.";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_DISCOVERED_PLAYER].m_numParameters = 2;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_DISCOVERED_PLAYER].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_DISCOVERED_PLAYER].m_parameters[1] = Parameter::SIDE;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_DISCOVERED_PLAYER].m_numUiStrings = 3;
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_DISCOVERED_PLAYER].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_DISCOVERED_PLAYER].m_uiStrings[1] = " has discovered ";
    m_conditionTemplates[Condition::SKIRMISH_PLAYER_HAS_DISCOVERED_PLAYER].m_uiStrings[2] = ".";

    m_conditionTemplates[Condition::MUSIC_TRACK_HAS_COMPLETED].m_internalName = "MUSIC_TRACK_HAS_COMPLETED";
    m_conditionTemplates[Condition::MUSIC_TRACK_HAS_COMPLETED].m_uiName =
        "Multimedia_/ Music track has completed some number of times.";
    m_conditionTemplates[Condition::MUSIC_TRACK_HAS_COMPLETED].m_numParameters = 2;
    m_conditionTemplates[Condition::MUSIC_TRACK_HAS_COMPLETED].m_parameters[0] = Parameter::MUSIC;
    m_conditionTemplates[Condition::MUSIC_TRACK_HAS_COMPLETED].m_parameters[1] = Parameter::INT;
    m_conditionTemplates[Condition::MUSIC_TRACK_HAS_COMPLETED].m_numUiStrings = 3;
    m_conditionTemplates[Condition::MUSIC_TRACK_HAS_COMPLETED].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::MUSIC_TRACK_HAS_COMPLETED].m_uiStrings[1] = " has completed at least ";
    m_conditionTemplates[Condition::MUSIC_TRACK_HAS_COMPLETED].m_uiStrings[2] =
        " times. (NOTE: This can only be used to start other music. USING THIS SC"
        "RIPT IN ANY OTHER WAY WILL CAUSE REPLAYS TO NOT WORK.)";

    m_conditionTemplates[Condition::SUPPLY_SOURCE_SAFE].m_internalName = "SUPPLY_SOURCE_SAFE";
    m_conditionTemplates[Condition::SUPPLY_SOURCE_SAFE].m_uiName = "Skirmish_/ Supply source is safe.";
    m_conditionTemplates[Condition::SUPPLY_SOURCE_SAFE].m_numParameters = 2;
    m_conditionTemplates[Condition::SUPPLY_SOURCE_SAFE].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::SUPPLY_SOURCE_SAFE].m_parameters[1] = Parameter::INT;
    m_conditionTemplates[Condition::SUPPLY_SOURCE_SAFE].m_numUiStrings = 3;
    m_conditionTemplates[Condition::SUPPLY_SOURCE_SAFE].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::SUPPLY_SOURCE_SAFE].m_uiStrings[1] = " closest supply src with at least ";
    m_conditionTemplates[Condition::SUPPLY_SOURCE_SAFE].m_uiStrings[2] =
        " available resources is SAFE from enemy influence.";

    m_conditionTemplates[Condition::SUPPLY_SOURCE_ATTACKED].m_internalName = "SUPPLY_SOURCE_ATTACKED";
    m_conditionTemplates[Condition::SUPPLY_SOURCE_ATTACKED].m_uiName = "Skirmish_/ Supply source is attacked.";
    m_conditionTemplates[Condition::SUPPLY_SOURCE_ATTACKED].m_numParameters = 1;
    m_conditionTemplates[Condition::SUPPLY_SOURCE_ATTACKED].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::SUPPLY_SOURCE_ATTACKED].m_numUiStrings = 2;
    m_conditionTemplates[Condition::SUPPLY_SOURCE_ATTACKED].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::SUPPLY_SOURCE_ATTACKED].m_uiStrings[1] = " supply source is under attack.";

    m_conditionTemplates[Condition::START_POSITION_IS].m_internalName = "START_POSITION_IS";
    m_conditionTemplates[Condition::START_POSITION_IS].m_uiName = "Skirmish_/ Start position.";
    m_conditionTemplates[Condition::START_POSITION_IS].m_numParameters = 2;
    m_conditionTemplates[Condition::START_POSITION_IS].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::START_POSITION_IS].m_parameters[1] = Parameter::INT;
    m_conditionTemplates[Condition::START_POSITION_IS].m_numUiStrings = 3;
    m_conditionTemplates[Condition::START_POSITION_IS].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::START_POSITION_IS].m_uiStrings[1] = " starting position is ";
    m_conditionTemplates[Condition::START_POSITION_IS].m_uiStrings[2] = " .";

    m_conditionTemplates[Condition::PLAYER_LOST_OBJECT_TYPE].m_internalName = "PLAYER_LOST_OBJECT_TYPE";
    m_conditionTemplates[Condition::PLAYER_LOST_OBJECT_TYPE].m_uiName = "Player_/ Player has lost an object of type.";
    m_conditionTemplates[Condition::PLAYER_LOST_OBJECT_TYPE].m_numParameters = 2;
    m_conditionTemplates[Condition::PLAYER_LOST_OBJECT_TYPE].m_parameters[0] = Parameter::SIDE;
    m_conditionTemplates[Condition::PLAYER_LOST_OBJECT_TYPE].m_parameters[1] = Parameter::OBJECT_TYPE;
    m_conditionTemplates[Condition::PLAYER_LOST_OBJECT_TYPE].m_numUiStrings = 3;
    m_conditionTemplates[Condition::PLAYER_LOST_OBJECT_TYPE].m_uiStrings[0] = " ";
    m_conditionTemplates[Condition::PLAYER_LOST_OBJECT_TYPE].m_uiStrings[1] = " has lost an object of type ";
    m_conditionTemplates[Condition::PLAYER_LOST_OBJECT_TYPE].m_uiStrings[2] = " (can be an object type list).";

    for (int i = 0; i < Condition::CONDITION_COUNT; i++) {
        Utf8String cnd_num;
        cnd_num.Format("[%d]", i);
        m_conditionTemplates[i].m_uiName += cnd_num;
        m_conditionTemplates[i].m_nameKey = g_theNameKeyGenerator->Name_To_Key(m_conditionTemplates[i].m_internalName.Str());
    }

    for (int i = 0; i < ScriptAction::ACTION_COUNT; i++) {
        Utf8String cnd_num;
        cnd_num.Format("[%d]", i);
        m_actionTemplates[i].m_uiName += cnd_num;
        m_actionTemplates[i].m_nameKey = g_theNameKeyGenerator->Name_To_Key(m_actionTemplates[i].m_internalName.Str());
    }

    Reset();
}

void ScriptEngine::Reset()
{
    if (g_theGameEngine != nullptr && g_theWriteableGlobalData != nullptr) {
        g_theGameEngine->Set_FPS_Limit(g_theWriteableGlobalData->m_framesPerSecondLimit);
    }

    if (g_theScriptActions != nullptr) {
        g_theScriptActions->Reset();
    }

    if (g_theScriptConditions != nullptr) {
        g_theScriptConditions->Reset();
    }

    m_numCounters = 1;
    m_numAttackInfo = 1;
    m_numFlags = 1;
    m_endGameTimer = -1;
    m_closeWindowTimer = -1;
    m_callingTeam = nullptr;
    m_callingObject = nullptr;
    m_conditionTeam = nullptr;
    m_conditionObject = nullptr;
    m_currentPlayer = nullptr;
    m_unkInt1 = 0;
    m_objectCreationDestructionFrame = 0;
    m_hasShownMPLocalDefeatWindow = 0;

    for (int i = 0; i < MAX_COUNTERS; i++) {
        m_counters[i].value = 0;
        m_counters[i].is_countdown_timer = false;
        m_counters[i].name.Clear();
    }

    for (int i = 0; i < MAX_FLAGS; i++) {
        m_flags[i].value = false;
        m_flags[i].name.Clear();
    }

    m_breezeInfo.direction = 1.0471976f;
    m_breezeInfo.sway_direction.x = GameMath::Sin(m_breezeInfo.direction);
    m_breezeInfo.sway_direction.y = GameMath::Cos(m_breezeInfo.direction);
    m_breezeInfo.intensity = 0.054977871f;
    m_breezeInfo.lean = 0.054977871f;
    m_breezeInfo.period = 150;
    m_breezeInfo.randomness = 0.2f;
    m_breezeInfo.version = 0;
    m_freezeByScript = false;
    m_useObjectDifficultyBonuses = true;
    m_chooseVictimAlwaysUsesNormal = false;

    // No point processing this section of code unless log levels can be debug or higher.
#if LOGLEVEL_DEBUG <= LOGGING_LEVEL
#ifdef GAME_DEBUG_STRUCTS
    if (m_numFrames > 1.0) {
        captainslog_debug("\n***SCRIPT ENGINE STATS %.0f frames:", m_numFrames);
        captainslog_debug("Avg time to update %.3f milisec", 1000.0 * m_totalUpdateTime / m_numFrames);
        captainslog_debug("  Max time to update %.3f miliseconds.", m_maxUpdateTime * 1000.0);
    }

    m_numFrames = 0.0;
    m_totalUpdateTime = 0.0;
    m_maxUpdateTime = 0.0;
#endif

    if (g_theSidesList != nullptr) {
        // Evaluate the MAX_DEBUG_SCRIPTS slowest scripts to report on.
        for (int j = 0; j < MAX_DEBUG_SCRIPTS; j++) {
            float longest_exec_time = 0.0f;
            Script *longest_script = nullptr;

            for (int side_idx = 0; side_idx < g_theSidesList->Get_Num_Sides(); side_idx++) {
                ScriptList *script_list = g_theSidesList->Get_Side_Info(side_idx)->Get_Script_List();

                if (script_list != nullptr) {
                    for (Script *script = script_list->Get_Script(); script != nullptr; script = script->Get_Next()) {
                        if (script->Get_Total_Exec_Time() > longest_exec_time) {
                            longest_exec_time = script->Get_Total_Exec_Time();
                            longest_script = script;
                        }
                    }

                    for (ScriptGroup *group = script_list->Get_Script_Group(); group != nullptr; group = group->Get_Next()) {
                        for (Script *script = group->Get_Script(); script != nullptr; script = script->Get_Next()) {
                            if (script->Get_Total_Exec_Time() > longest_exec_time) {
                                longest_exec_time = script->Get_Total_Exec_Time();
                                longest_script = script;
                            }
                        }
                    }
                }
            }

            if (longest_script != nullptr) {
                captainslog_debug("   SCRIPT %s total time %f seconds,\n        evaluated %d times, avg execution %2.3f "
                                  "msec (Goal less than 0.05)",
                    longest_script->Get_Name().Str(),
                    longest_script->Get_Total_Exec_Time(),
                    longest_script->Get_Eval_Count(),
                    longest_script->Get_Total_Exec_Time() * 1000.0f / longest_script->Get_Eval_Count());
                longest_script->Update_Exec_Time(-2.0f * longest_exec_time);
            }
        }

        captainslog_debug("***");
    }
#endif

    Update_Current_Particle_Cap();

    for (auto it = m_sequentialScripts.begin(); it != m_sequentialScripts.end(); Cleanup_Sequential_Script(it, true)) {
    }

    for (auto it = m_allObjectTypeLists.begin(); it != m_allObjectTypeLists.end(); it = m_allObjectTypeLists.begin()) {
        if (*it != nullptr) {
            Remove_Object_Types(*it);
        } else {
            m_allObjectTypeLists.erase(it);
        }
    }

    captainslog_dbgassert(
        m_allObjectTypeLists.empty(), "ScriptEngine::Reset - m_allObjectTypeLists should be empty but is not!");

    m_namedReveals.clear();
    m_namedObjects.clear();
    m_completedVideo.clear();
    m_completedSpeech.clear();
    m_completedAudio.clear();
    m_uiInteraction.clear();

    for (int player_idx = 0; player_idx < MAX_PLAYER_COUNT; player_idx++) {
        m_triggeredSpecialPowers[player_idx].clear();
        m_midwaySpecialPowers[player_idx].clear();
        m_finishedSpecialPowers[player_idx].clear();
        m_acquiredSciences[player_idx].clear();
        m_completedUpgrades[player_idx].clear();
    }

    ScriptList::Reset();

    for (int i = 0; i < MAX_ATTACK_PRIORITIES; i++) {
        m_attackPriorityInfo[i].Reset();
    }

    for (int player_idx = 0; player_idx < MAX_PLAYER_COUNT; player_idx++) {
        m_playerObjectCounts[player_idx].clear();
    }

    m_toppleDirections.clear();
}

void ScriptEngine::New_Map()
{
    m_numCounters = 1;

    for (int i = 0; i < MAX_COUNTERS; i++) {
        m_counters[i].value = 0;
        m_counters[i].is_countdown_timer = false;
        m_counters[i].name.Clear();
    }

    m_numFlags = 1;

    for (int i = 0; i < MAX_FLAGS; i++) {
        m_flags[i].value = 0;
        m_flags[i].name.Clear();
    }

    m_endGameTimer = -1;
    m_closeWindowTimer = -1;
#ifdef GAME_DEBUG_STRUCTS
    m_numFrames = 0.0f;
    m_totalUpdateTime = 0.0f;
    m_maxUpdateTime = 0.0f;
#endif
    m_completedVideo.clear();
    m_completedSpeech.clear();
    m_completedAudio.clear();
    m_uiInteraction.clear();

    for (int player_idx = 0; player_idx < MAX_PLAYER_COUNT; player_idx++) {
        m_triggeredSpecialPowers[player_idx].clear();
        m_midwaySpecialPowers[player_idx].clear();
        m_finishedSpecialPowers[player_idx].clear();
        m_acquiredSciences[player_idx].clear();
        m_completedUpgrades[player_idx].clear();
    }

    for (int side_idx = 0; side_idx < g_theSidesList->Get_Num_Sides(); side_idx++) {
        ScriptList *list = g_theSidesList->Get_Side_Info(side_idx)->Get_Script_List();

        if (list != nullptr) {
            for (Script *script = list->Get_Script(); script != nullptr; script = script->Get_Next()) {
                Check_Conditions_For_Team_Names(script);
            }

            for (ScriptGroup *group = list->Get_Script_Group(); group != nullptr; group = group->Get_Next()) {
                for (Script *script = group->Get_Script(); script != nullptr; script = script->Get_Next()) {
                    Check_Conditions_For_Team_Names(script);
                }
            }
        }
    }

    m_firstUpdate = true;
    m_fade = FADE_MULTIPLY;
    m_curFadeFrame = 0;
    m_minFade = 1.0f;
    m_maxFade = 0.0f;
    m_fadeFramesIncrease = 0;
    m_fadeFramesHold = 0;
    m_fadeFramesDecrease = 33;
    m_curFadeValue = 0.0f;
}

void ScriptEngine::Update()
{
#ifdef GAME_DEBUG_STRUCTS
#ifdef PLATFORM_WINDOWS
    double time = 0.0f;
    LARGE_INTEGER frequency;
    LARGE_INTEGER perf_count;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&perf_count);
#endif
#endif

    if (m_firstUpdate) {
        Create_Named_Cache();
        Particle_Editor_Update();
        m_firstUpdate = false;
    } else {
        Particle_Editor_Update();
    }

    if (m_closeWindowTimer > 0 && --m_closeWindowTimer < 1) {
        g_theScriptActions->Close_Windows(false);
    }

    if (m_endGameTimer > 0 && --m_endGameTimer < 1) {
        g_theMessageStream->Append_Message(GameMessage::MSG_CLEAR_GAME_DATA);
    }

    Update_Frame_Number();

    if (Is_Time_Frozen_Debug()) {
        s_lastFrame = s_currentFrame - 1;
    } else {
        if (m_fade != FADE_NONE) {
            Update_Fades();
        }

        if (m_endGameTimer < 0) {
            if (g_theScriptActions != nullptr) {
                g_theScriptActions->Update();
            }

            if (g_theScriptConditions != nullptr) {
                g_theScriptConditions->Update();
            }

            for (int i = 1; i < m_numCounters; i++) {
                if (m_counters[i].is_countdown_timer) {
                    if (m_counters[i].value >= 0) {
                        m_counters[i].value--;
                    }
                }
            }

            for (int sides_idx = 0; sides_idx < g_theSidesList->Get_Num_Sides(); sides_idx++) {
                m_currentPlayer = g_thePlayerList->Get_Nth_Player(sides_idx);
                ScriptList *list = g_theSidesList->Get_Side_Info(sides_idx)->Get_Script_List();

                if (list != nullptr) {
                    Execute_Scripts(list->Get_Script());

                    for (ScriptGroup *group = list->Get_Script_Group(); group != nullptr; group = group->Get_Next()) {
                        if (group->Is_Active() && !group->Is_Subroutine()) {
                            Execute_Scripts(group->Get_Script());
                        }
                    }

                    m_currentPlayer = nullptr;
                }
            }

            g_thePlayerList->Update_Team_States();
            m_uiInteraction.clear();
            Evaluate_And_Progress_All_Sequential_Scripts();
            s_currentFrame++;

#ifdef PLATFORM_WINDOWS
            if (s_debugDll != nullptr) {
                for (int i = 1; i < m_numCounters; i++) {
                    Adjust_Variable(m_counters[i].name, m_counters[i].value, false);
                }

                for (int i = 1; i < m_numFlags; i++) {
                    Adjust_Variable(m_flags[i].name, m_flags[i].value, false);
                }
            }

#ifdef GAME_DEBUG_STRUCTS
            LARGE_INTEGER new_perf_count;
            QueryPerformanceCounter(&new_perf_count);
            time = (double)(new_perf_count.QuadPart - perf_count.QuadPart) / (double)frequency.QuadPart;
            m_numFrames += 1.0f;
            m_totalUpdateTime += time;

            if (time > m_maxUpdateTime) {
                m_maxUpdateTime = time;
            }

            m_frameUpdateTime = time;
            // vtune code removed
#endif
#endif
        }
    }
}

Utf8String ScriptEngine::Get_Stats(float *slowest_scripts, float *time_last_frame, float *time)
{
#ifdef GAME_DEBUG_STRUCTS
    *slowest_scripts = 0.0f;
    *time_last_frame = 0.0f;
    *time = 0.0f;
    Utf8String str("Script Engine Profiling disabled.");
    str = "#1-";
    *slowest_scripts = m_frameUpdateTime;

    if (g_theSidesList != nullptr) {
        for (int i = 0; i < 2; i++) {
            float new_time = 0.0f;
            Script *slowest_script = nullptr;

            for (int j = 0; j < g_theSidesList->Get_Num_Sides(); j++) {
                ScriptList *script_list = g_theSidesList->Get_Side_Info(i)->Get_Script_List();

                if (script_list != nullptr) {
                    for (Script *script = script_list->Get_Script(); script != nullptr; script = script->Get_Next()) {
                        if (script->Get_Script_Timing() > new_time) {
                            new_time = script->Get_Script_Timing();
                            slowest_script = script;
                        }
                    }

                    for (ScriptGroup *group = script_list->Get_Script_Group(); group != nullptr; group = group->Get_Next()) {
                        for (Script *script = group->Get_Script(); script != nullptr; script = script->Get_Next()) {
                            if (script->Get_Script_Timing() > new_time) {
                                new_time = script->Get_Script_Timing();
                                slowest_script = script;
                            }
                        }
                    }
                }
            }

            if (slowest_script != nullptr) {
                if (i != 0) {
                    *time = new_time;
                    str.Concat(", #2-");
                } else {
                    *time_last_frame = new_time;
                }

                str += slowest_script->Get_Name();
                slowest_script->Set_Script_Timing(0.0f);
            }
        }
    }

    return str;
#else
    return "";
#endif
}

void ScriptEngine::Start_Quick_End_Game_Timer()
{
    m_endGameTimer = 1;
}

void ScriptEngine::Start_End_Game_Timer()
{
    m_endGameTimer = 120;
}

void ScriptEngine::Start_Close_Window_Timer()
{
    m_closeWindowTimer = 120;
}

void ScriptEngine::Update_Fades()
{
    int fade = ++m_curFadeFrame;

    if (fade <= m_fadeFramesIncrease) {
        m_curFadeValue = (m_maxFade - m_minFade) * ((float)m_curFadeFrame / (float)m_fadeFramesIncrease) + m_minFade;
    } else {
        fade -= m_fadeFramesIncrease;

        if (fade <= m_fadeFramesHold) {
            m_curFadeValue = m_maxFade;
        } else {
            fade -= m_fadeFramesHold;

            if (fade <= m_fadeFramesDecrease) {
                int divisor = m_fadeFramesDecrease + 1;

                if (m_fadeFramesDecrease == -1) {
                    divisor = 1;
                }

                m_curFadeValue = (m_minFade - m_maxFade) * ((float)fade / (float)divisor) + m_maxFade;
            } else {
                m_fade = FADE_NONE;
            }
        }
    }
}

Player *ScriptEngine::Get_Current_Player()
{
    if (m_currentPlayer == nullptr) {
        Append_Debug_Message("***Unexpected NULL player:***", false);
    }

    return m_currentPlayer;
}

void ScriptEngine::Clear_Flag(const Utf8String &flag)
{
    for (int player_idx = 0; player_idx < MAX_PLAYER_COUNT; player_idx++) {
        Utf8String str;
        str.Format("%s%d", flag.Str(), player_idx);

        for (int flag_idx = 1; flag_idx < m_numFlags; flag_idx++) {
            if (str == m_flags[flag_idx].name) {
                m_flags[flag_idx].value = false;
            }
        }
    }
}

void ScriptEngine::Clear_Team_Flags()
{
    Clear_Flag("USA Team is Building");
    Clear_Flag("USA Air Team Is Building");
    Clear_Flag("USA Inf Team Is Building");
    Clear_Flag("China Team is Building");
    Clear_Flag("China Air Team Is Building");
    Clear_Flag("China Inf Team Is Building");
    Clear_Flag("GLA Team is Building");
    Clear_Flag("GLA Inf Team is Building");
}

Player *ScriptEngine::Get_Skirmish_Enemy_Player()
{
    bool is_challenge_campaign = g_theCampaignManager->Get_Current_Campaign() != nullptr
        && g_theCampaignManager->Get_Current_Campaign()->m_isChallengeCampaign;

    if (m_currentPlayer == nullptr) {
        captainslog_dbgassert(false, "No enemy found.  Unexpected but not fatal.");
        return nullptr;
    } else {
        Player *enemy = m_currentPlayer->Get_Current_Enemy();

        if (enemy == nullptr) {
            for (int player_idx = 0; player_idx < g_thePlayerList->Get_Player_Count(); player_idx++) {
                enemy = g_thePlayerList->Get_Nth_Player(player_idx);

                if (enemy->Get_Player_Type() == Player::PLAYER_COMPUTER) {
                    enemy = nullptr;
                } else {
                    if (!is_challenge_campaign) {
                        return enemy;
                    }

                    if (enemy->Get_Player_NameKey() == g_theNameKeyGenerator->Name_To_Key("ThePlayer")) {
                        return enemy;
                    }
                }
            }
        }

        return enemy;
    }
}

Player *ScriptEngine::Get_Player_From_AsciiString(const Utf8String &player_name)
{
    bool is_challenge_campaign = g_theCampaignManager->Get_Current_Campaign() != nullptr
        && g_theCampaignManager->Get_Current_Campaign()->m_isChallengeCampaign;

    if (player_name == "<Local Player>" || (player_name == "ThePlayer" && is_challenge_campaign)) {
        return g_thePlayerList->Get_Local_Player();
    }

    if (player_name == "<This Player>") {
        return Get_Current_Player();
    }

    if (player_name == "<This Player's Enemy>") {
        return Get_Skirmish_Enemy_Player();
    }

    Player *player = g_thePlayerList->Find_Player_With_NameKey(g_theNameKeyGenerator->Name_To_Key(player_name.Str()));

    if (player != nullptr) {
        return player;
    }

    Append_Debug_Message("***Invalid Player name:***", false);
    return nullptr;
}

ObjectTypes *ScriptEngine::Get_Object_Types(const Utf8String &list_name)
{
    for (auto it = m_allObjectTypeLists.begin(); it != m_allObjectTypeLists.end(); it++) {
        if (*it != nullptr) {
            if ((*it)->Get_List_Name() == list_name) {
                return *it;
            }
        } else {
            captainslog_dbgassert(false, "NULL object type list was unexpected.");
        }
    }

    return nullptr;
}

void ScriptEngine::Do_ObjectType_List_Maintenance(const Utf8String &list_name, const Utf8String &type_name, bool add_type)
{
    ObjectTypes *types = Get_Object_Types(list_name);

    if (types == nullptr) {
        types = new ObjectTypes(list_name);
        m_allObjectTypeLists.push_back(types);
    }

    if (add_type) {
        types->Add_Object_Type(type_name);
    } else {
        types->Remove_Object_Type(type_name);
    }

    if (types->Get_List_Count() == 0) {
        Remove_Object_Types(types);
    }
}

PolygonTrigger *ScriptEngine::Get_Qualified_Trigger_Area_By_Name(Utf8String trigger_area)
{
    if (trigger_area == "[Skirmish]MyInnerPerimeter" || trigger_area == "[Skirmish]MyOuterPerimeter") {
        if (m_currentPlayer == nullptr) {
            return nullptr;
        }

        int start_index = m_currentPlayer->Get_Multiplayer_Start_Index() + 1;

        if (trigger_area == "[Skirmish]MyInnerPerimeter") {
            trigger_area.Format("%s%d", "InnerPerimeter", start_index);
        } else {
            trigger_area.Format("%s%d", "OuterPerimeter", start_index);
        }
    } else if (trigger_area == "[Skirmish]EnemyInnerPerimeter" || trigger_area == "[Skirmish]EnemyOuterPerimeter") {
        int start_index = -1;

        if (m_currentPlayer != nullptr) {
            Player *enemy = Get_Current_Player()->Get_Current_Enemy();

            if (enemy != nullptr) {
                start_index = enemy->Get_Multiplayer_Start_Index() + 1;
            }
        }

        if (trigger_area == "[Skirmish]EnemyInnerPerimeter") {
            trigger_area.Format("%s%d", "InnerPerimeter", start_index);
        } else {
            trigger_area.Format("%s%d", "OuterPerimeter", start_index);
        }
    }

    PolygonTrigger *trigger = g_theTerrainLogic->Get_Trigger_Area_By_Name(trigger_area);

    if (trigger == nullptr) {
        Utf8String str("!!!WARNING!!! Trigger area '");
        str += trigger_area;
        str.Concat("' not found.");
        Append_Debug_Message(str, true);
    }

    return trigger;
}

Team *ScriptEngine::Get_Team_Named(const Utf8String &team_name)
{
    bool is_challenge_campaign = g_theCampaignManager->Get_Current_Campaign() != nullptr
        && g_theCampaignManager->Get_Current_Campaign()->m_isChallengeCampaign;

    if (team_name == "teamThePlayer" && is_challenge_campaign) {
        return g_thePlayerList->Get_Local_Player()->Get_Default_Team();
    } else if (team_name == "<This Team>") {
        if (m_callingTeam != nullptr) {
            return m_callingTeam;
        } else {
            return m_conditionTeam;
        }
    } else if (m_callingTeam != nullptr && m_callingTeam->Get_Name() == team_name) {
        return m_callingTeam;
    } else if (m_conditionTeam != nullptr && m_conditionTeam->Get_Name() == team_name) {
        return m_conditionTeam;
    } else {
        TeamPrototype *prototype = g_theTeamFactory->Find_Team_Prototype(team_name);

        if (prototype != nullptr) {
            if (prototype->Get_Singleton()) {
                Team *first_team = prototype->Get_First_Item_In_Team_Instance_List();

                if (first_team != nullptr && first_team->Is_Active()) {
                    return first_team;
                } else {
                    return nullptr;
                }
            } else {
                static int warnCount = 0;
                if (prototype->Count_Team_Instances() > 1 && warnCount < 10) {
                    warnCount++;
                    Append_Debug_Message("***Referencing multiple team by unspecific instance:***", false);
                    Append_Debug_Message(team_name, false);
                }

                return prototype->Get_First_Item_In_Team_Instance_List();
            }
        } else {
            return nullptr;
        }
    }
}

Object *ScriptEngine::Get_Unit_Named(const Utf8String &unit_name)
{
    if (unit_name == "<This Object>") {
        if (m_callingObject != nullptr) {
            return m_callingObject;
        } else {
            return m_conditionObject;
        }
    } else {
        for (auto it = m_namedObjects.begin(); it != m_namedObjects.end(); it++) {
            if (unit_name == it->first) {
                return it->second;
            }
        }

        return nullptr;
    }
}

bool ScriptEngine::Did_Unit_Exist(const Utf8String &unit_name)
{
    for (auto it = m_namedObjects.begin(); it != m_namedObjects.end(); it++) {
        if (unit_name == it->first) {
            return it->second == nullptr;
        }
    }

    return false;
}

void ScriptEngine::Run_Script(const Utf8String &script_name, Team *team)
{
    if (!script_name.Is_Empty() && !(script_name == "<none>")) {
        Player *player = m_currentPlayer;
        LatchRestore<Team *> latch(&m_callingTeam, &team);
        m_conditionTeam = nullptr;
        m_currentPlayer = nullptr;

        if (m_callingTeam != nullptr) {
            m_currentPlayer = m_callingTeam->Get_Controlling_Player();
        }

        Script *script = nullptr;
        ScriptGroup *group = Find_Group(script_name);

        if (group != nullptr) {
            if (group->Is_Subroutine()) {
                if (group->Is_Active()) {
                    Execute_Scripts(group->Get_Script());
                }
            } else {
                Append_Debug_Message("***Attempting to call script that is not a subroutine:***", false);
                Append_Debug_Message(script_name, false);
                captainslog_debug("Attempting to call script '%s' that is not a subroutine.", script_name.Str());
            }
        } else {
            script = Find_Script(script_name);

            if (script != nullptr) {
                if (script->Is_Subroutine()) {
                    Execute_Script(script);
                } else {
                    Append_Debug_Message("***Attempting to call script that is not a subroutine:***", false);
                    Append_Debug_Message(script_name, false);
                    captainslog_debug("Attempting to call script '%s' that is not a subroutine.", script_name.Str());
                }
            } else {
                Append_Debug_Message("***Script not defined:***", false);
                Append_Debug_Message(script_name, false);
                captainslog_debug("WARNING: Script '%s' not defined.", script_name.Str());
            }
        }

        m_currentPlayer = player;
    }
}

void ScriptEngine::Run_Object_Script(const Utf8String &script_name, Object *obj)
{
    if (!script_name.Is_Empty() && !(script_name == "<none>")) {
        Object *calling_object = m_callingObject;
        m_callingObject = obj;
        Script *script = nullptr;
        ScriptGroup *group = Find_Group(script_name);

        if (group != nullptr) {
            if (group->Is_Subroutine()) {
                if (group->Is_Active()) {
                    Execute_Scripts(group->Get_Script());
                }
            } else {
                Append_Debug_Message("***Attempting to call script that is not a subroutine:***", false);
                Append_Debug_Message(script_name, false);
                captainslog_debug("Attempting to call script '%s' that is not a subroutine.", script_name.Str());
            }
        } else {
            script = Find_Script(script_name);

            if (script != nullptr) {
                if (script->Is_Subroutine()) {
                    Execute_Script(script);
                } else {
                    Append_Debug_Message("***Attempting to call script that is not a subroutine:***", false);
                    Append_Debug_Message(script_name, false);
                    captainslog_debug("Attempting to call script '%s' that is not a subroutine.", script_name.Str());
                }
            } else {
                Append_Debug_Message("***Script not defined:***", false);
                Append_Debug_Message(script_name, false);
                captainslog_debug("WARNING: Script '%s' not defined.", script_name.Str());
            }
        }

        m_callingObject = calling_object;
    }
}

int ScriptEngine::Allocate_Counter(const Utf8String &counter)
{
    for (int i = 1; i < m_numCounters; i++) {
        if (counter == m_counters[i].name) {
            return i;
        }
    }

    captainslog_dbgassert(m_numCounters < MAX_COUNTERS, "Too many counters, failed to make '%s'.", counter.Str());

    if (m_numCounters >= MAX_COUNTERS) {
        return 0;
    }

    m_counters[m_numCounters].name = counter;
    return m_numCounters++;
}

const TCounter *ScriptEngine::Get_Counter(const Utf8String &counter)
{
    for (int i = 1; i < m_numCounters; i++) {
        if (counter == m_counters[i].name) {
            return &m_counters[i];
        }
    }

    return nullptr;
}

void ScriptEngine::Create_Named_Map_Reveal(
    const Utf8String &reveal, const Utf8String &waypoint, float radius, const Utf8String &player)
{
    for (auto it = m_namedReveals.begin(); it != m_namedReveals.end(); it++) {
        if (it->reveal_name == reveal) {
            captainslog_dbgassert(false,
                "ScriptEngine::createNamedMapReveal: Attempted to redefine named Reveal '%s', so I won't change it.",
                reveal.Str());
            return;
        }
    }

    NamedReveal new_reveal;
    new_reveal.player = player;
    new_reveal.radius = radius;
    new_reveal.reveal_name = reveal;
    new_reveal.waypoint_name = waypoint;
    m_namedReveals.push_back(new_reveal);
}

void ScriptEngine::Do_Named_Map_Reveal(const Utf8String &reveal)
{
    NamedReveal *do_reveal = nullptr;

    for (auto it = m_namedReveals.begin(); it != m_namedReveals.end(); it++) {
        if (it->reveal_name == reveal) {
            do_reveal = &*it;
            break;
        }
    }

    if (do_reveal != nullptr) {
        Waypoint *waypoint = g_theTerrainLogic->Get_Waypoint_By_Name(do_reveal->waypoint_name);

        if (waypoint != nullptr) {
            Player *player = Get_Player_From_AsciiString(do_reveal->player);

            if (player != nullptr) {
                const Coord3D *loc = waypoint->Get_Location();
                g_thePartitionManager->Do_Shroud_Reveal(loc->x, loc->y, do_reveal->radius, player->Get_Player_Mask());
            }
        }
    }
}

void ScriptEngine::Undo_Named_Map_Reveal(const Utf8String &reveal)
{
    NamedReveal *do_reveal = nullptr;

    for (auto it = m_namedReveals.begin(); it != m_namedReveals.end(); it++) {
        if (it->reveal_name == reveal) {
            do_reveal = &*it;
            break;
        }
    }

    if (do_reveal != nullptr) {
        Waypoint *waypoint = g_theTerrainLogic->Get_Waypoint_By_Name(do_reveal->waypoint_name);

        if (waypoint != nullptr) {
            Player *player = Get_Player_From_AsciiString(do_reveal->player);

            if (player != nullptr) {
                const Coord3D *loc = waypoint->Get_Location();
                g_thePartitionManager->Undo_Shroud_Reveal(loc->x, loc->y, do_reveal->radius, player->Get_Player_Mask());
            }
        }
    }
}

void ScriptEngine::Remove_Named_Map_Reveal(const Utf8String &reveal)
{
    for (auto it = m_namedReveals.begin(); it != m_namedReveals.end(); it++) {
        if (it->reveal_name == reveal) {
            m_namedReveals.erase(it);
            return;
        }
    }
}

int ScriptEngine::Allocate_Flag(const Utf8String &flag)
{
    for (int i = 1; i < m_numFlags; i++) {
        if (flag == m_flags[i].name) {
            return i;
        }
    }

    captainslog_dbgassert(m_numFlags < MAX_FLAGS, "Too many flags, failed to make '%s'.", flag.Str());

    if (m_numFlags >= MAX_FLAGS) {
        return 0;
    }

    m_flags[m_numFlags].name = flag;
    return m_numFlags++;
}

ScriptGroup *ScriptEngine::Find_Group(const Utf8String &group)
{
    for (int sides_idx = 0; sides_idx < g_theSidesList->Get_Num_Sides(); sides_idx++) {
        ScriptList *list = g_theSidesList->Get_Side_Info(sides_idx)->Get_Script_List();

        if (list != nullptr) {
            for (ScriptGroup *script = list->Get_Script_Group(); script != nullptr; script = script->Get_Next()) {
                if (script->Get_Name() == group) {
                    return script;
                }
            }
        }
    }

    return nullptr;
}

Script *ScriptEngine::Find_Script(const Utf8String &script)
{
    for (int sides_idx = 0; sides_idx < g_theSidesList->Get_Num_Sides(); sides_idx++) {
        ScriptList *script_list = g_theSidesList->Get_Side_Info(sides_idx)->Get_Script_List();

        if (script_list != nullptr) {
            for (Script *scr = script_list->Get_Script(); scr != nullptr; scr = scr->Get_Next()) {
                if (scr->Get_Name() == script) {
                    return scr;
                }
            }

            for (ScriptGroup *group = script_list->Get_Script_Group(); group != nullptr; group = group->Get_Next()) {
                for (Script *scr = group->Get_Script(); scr != nullptr; scr = scr->Get_Next()) {
                    if (scr->Get_Name() == script) {
                        return scr;
                    }
                }
            }
        }
    }

    return nullptr;
}

bool ScriptEngine::Evaluate_Counter(Condition *condition)
{
    captainslog_dbgassert(condition->Get_Num_Parameters() >= 3, "Not enough parameters.");
    captainslog_dbgassert(condition->Get_Condition_Type() == Condition::COUNTER, "Wrong condition.");

    int counter = condition->Get_Parameter(0)->Get_Int();

    if (counter == 0) {
        counter = Allocate_Counter(condition->Get_Parameter(0)->Get_String());
        condition->Get_Parameter(0)->Set_Int(counter);
    }

    int value = condition->Get_Parameter(2)->Get_Int();
    bool ret;

    switch (condition->Get_Parameter(1)->Get_Int()) {
        case 0:
            ret = m_counters[counter].value < value;
            break;
        case 1:
            ret = m_counters[counter].value <= value;
            break;
        case 2:
            ret = m_counters[counter].value == value;
            break;
        case 3:
            ret = m_counters[counter].value >= value;
            break;
        case 4:
            ret = m_counters[counter].value > value;
            break;
        case 5:
            ret = m_counters[counter].value != value;
            break;
        default:
            ret = false;
            break;
    }

    return ret;
}

void ScriptEngine::Set_Counter(ScriptAction *action)
{
    captainslog_dbgassert(action->Get_Num_Parameters() >= 2, "Not enough parameters.");

    int counter = action->Get_Parameter(0)->Get_Int();

    if (counter == 0) {
        counter = Allocate_Counter(action->Get_Parameter(0)->Get_String());
        action->Get_Parameter(0)->Set_Int(counter);
    }

    m_counters[counter].value = action->Get_Parameter(1)->Get_Int();
}

void ScriptEngine::Set_Fade(ScriptAction *action)
{
#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData->m_disableCameraFade) {
        m_fade = FADE_NONE;
    } else
#endif
    {
        captainslog_dbgassert(action->Get_Num_Parameters() >= 5, "Not enough parameters.");

        switch (action->Get_Action_Type()) {
            case ScriptAction::CAMERA_FADE_ADD:
                m_fade = FADE_ADD;
                goto l1;
            case ScriptAction::CAMERA_FADE_SUBTRACT:
                m_fade = FADE_SUBTRACT;
                goto l1;
            case ScriptAction::CAMERA_FADE_SATURATE:
                m_fade = FADE_SATURATE;
                goto l1;
            case ScriptAction::CAMERA_FADE_MULTIPLY:
                m_fade = FADE_MULTIPLY;
            l1:
                m_curFadeFrame = 0;
                m_minFade = action->Get_Parameter(0)->Get_Real();
                m_maxFade = action->Get_Parameter(1)->Get_Real();
                m_fadeFramesIncrease = action->Get_Parameter(2)->Get_Int();
                m_fadeFramesHold = action->Get_Parameter(3)->Get_Int();
                m_fadeFramesDecrease = action->Get_Parameter(4)->Get_Int();
                m_curFadeValue = m_minFade;

                if (m_fadeFramesIncrease == 0) {
                    Update_Fades();
                }
                break;
            default:
                m_fade = FADE_NONE;
                break;
        }
    }
}

void ScriptEngine::Set_Sway(ScriptAction *action)
{
    captainslog_dbgassert(action->Get_Num_Parameters() >= 5, "Not enough parameters.");
    m_breezeInfo.version++;
    m_breezeInfo.direction = action->Get_Parameter(0)->Get_Real();
    m_breezeInfo.sway_direction.x = GameMath::Sin(m_breezeInfo.direction);
    m_breezeInfo.sway_direction.y = GameMath::Cos(m_breezeInfo.direction);
    m_breezeInfo.intensity = action->Get_Parameter(1)->Get_Real();
    m_breezeInfo.lean = action->Get_Parameter(2)->Get_Real();
    m_breezeInfo.period = action->Get_Parameter(3)->Get_Int();

    if (m_breezeInfo.period < 1) {
        m_breezeInfo.period = 1;
    }

    m_breezeInfo.randomness = action->Get_Parameter(4)->Get_Real();
}

void ScriptEngine::Add_Counter(ScriptAction *action)
{
    captainslog_dbgassert(action->Get_Num_Parameters() >= 2, "Not enough parameters.");
    int value = action->Get_Parameter(0)->Get_Int();
    int counter = action->Get_Parameter(1)->Get_Int();

    if (counter == 0) {
        counter = Allocate_Counter(action->Get_Parameter(1)->Get_String());
        action->Get_Parameter(1)->Set_Int(counter);
    }

    m_counters[counter].value += value;
}

void ScriptEngine::Sub_Counter(ScriptAction *action)
{
    captainslog_dbgassert(action->Get_Num_Parameters() >= 2, "Not enough parameters.");
    int value = action->Get_Parameter(0)->Get_Int();
    int counter = action->Get_Parameter(1)->Get_Int();

    if (counter == 0) {
        counter = Allocate_Counter(action->Get_Parameter(1)->Get_String());
        action->Get_Parameter(1)->Set_Int(counter);
    }

    m_counters[counter].value -= value;
}

bool ScriptEngine::Evaluate_Flag(Condition *condition)
{
    captainslog_dbgassert(condition->Get_Num_Parameters() >= 2, "Not enough parameters.");
    captainslog_dbgassert(condition->Get_Condition_Type() == Condition::FLAG, "Wrong condition.");

    int flag = condition->Get_Parameter(0)->Get_Int();

    if (flag == 0) {
        flag = Allocate_Flag(condition->Get_Parameter(0)->Get_String());
        condition->Get_Parameter(0)->Set_Int(flag);
    }

    if ((condition->Get_Parameter(1)->Get_Int() != 0) == (m_flags[flag].value != 0)) {
        return true;
    }

    for (auto it = m_uiInteraction.begin(); it != m_uiInteraction.end(); it++) {
        if (it->Compare(condition->Get_Parameter(0)->Get_String()) == 0) {
            return true;
        }
    }

    return false;
}

void ScriptEngine::Set_Flag(ScriptAction *action)
{
    captainslog_dbgassert(action->Get_Num_Parameters() >= 2, "Not enough parameters.");
    int flag = action->Get_Parameter(0)->Get_Int();

    if (flag == 0) {
        flag = Allocate_Flag(action->Get_Parameter(0)->Get_String());
        action->Get_Parameter(0)->Set_Int(flag);
    }

    m_flags[flag].value = action->Get_Parameter(1)->Get_Int() != 0;
}

AttackPriorityInfo *ScriptEngine::Find_Attack_Info(const Utf8String &name, bool add_if_not_found)
{
    for (int i = 0; i < m_numAttackInfo; i++) {
        if (m_attackPriorityInfo[i].Get_Name() == name) {
            return &m_attackPriorityInfo[i];
        }
    }

    if (!add_if_not_found || m_numAttackInfo >= MAX_ATTACK_PRIORITIES) {
        return nullptr;
    }

    m_attackPriorityInfo[m_numAttackInfo].Set_Name(name);
    return &m_attackPriorityInfo[m_numAttackInfo++];
}

const AttackPriorityInfo *ScriptEngine::Get_Default_Attack_Info()
{
    return m_attackPriorityInfo;
}

const AttackPriorityInfo *ScriptEngine::Get_Attack_Info(Utf8String const &name)
{
    for (int i = 0; i < m_numAttackInfo; i++) {
        if (m_attackPriorityInfo[i].Get_Name() == name) {
            return &m_attackPriorityInfo[i];
        }
    }

    return &m_attackPriorityInfo[0];
}

void ScriptEngine::Set_Priority_Thing(ScriptAction *action)
{
    captainslog_dbgassert(action->Get_Num_Parameters() >= 3, "Not enough parameters.");
    Utf8String name(action->Get_Parameter(1)->Get_String());
    ObjectTypes *types = Get_Object_Types(name);

    if (types != nullptr) {
        for (unsigned int i = 0; i < types->Get_List_Count(); i++) {
            Utf8String str = types->Get_Nth_In_List(i);
            ThingTemplate *tmplate = g_theThingFactory->Find_Template(str, true);

            if (tmplate == nullptr) {
                Append_Debug_Message("***Attempting to set attack priority on an invalid thing:***", false);
                Append_Debug_Message(action->Get_Parameter(0)->Get_String(), false);
                return;
            }

            AttackPriorityInfo *info = Find_Attack_Info(action->Get_Parameter(0)->Get_String(), true);

            if (info == nullptr) {
                Append_Debug_Message("***Error allocating attack priority set - fix or raise limit. ***", false);
                return;
            }

            info->Set_Priority(tmplate, action->Get_Parameter(2)->Get_Int());
        }
    } else {
        ThingTemplate *tmplate = g_theThingFactory->Find_Template(name, true);

        if (tmplate != nullptr) {
            AttackPriorityInfo *info = Find_Attack_Info(action->Get_Parameter(0)->Get_String(), true);

            if (info != nullptr) {
                info->Set_Priority(tmplate, action->Get_Parameter(2)->Get_Int());
            } else {
                Append_Debug_Message("***Error allocating attack priority set - fix or raise limit. ***", false);
            }
        } else {
            Append_Debug_Message("***Attempting to set attack priority on an invalid thing:***", false);
            Append_Debug_Message(action->Get_Parameter(0)->Get_String(), false);
        }
    }
}

void ScriptEngine::Set_Priority_Kind(ScriptAction *action)
{
    captainslog_dbgassert(action->Get_Num_Parameters() >= 3, "Not enough parameters.");
    AttackPriorityInfo *info = Find_Attack_Info(action->Get_Parameter(0)->Get_String(), true);

    if (info != nullptr) {
        int kind = action->Get_Parameter(1)->Get_Int();
        int priority = action->Get_Parameter(2)->Get_Int();

        for (ThingTemplate *tmplate = g_theThingFactory->First_Template(); tmplate != nullptr;
             tmplate = tmplate->Friend_Get_Next_Template()) {
            if (tmplate->Is_KindOf(static_cast<KindOfType>(kind))) {
                info->Set_Priority(tmplate, priority);
            }
        }
    } else {
        Append_Debug_Message("***Error allocating attack priority set - fix or raise limit. ***", false);
    }
}

void ScriptEngine::Set_Priority_Default(ScriptAction *action)
{
    captainslog_dbgassert(action->Get_Num_Parameters() >= 2, "Not enough parameters.");
    AttackPriorityInfo *info = Find_Attack_Info(action->Get_Parameter(0)->Get_String(), true);

    if (info != nullptr) {
        info->Set_Default_Priority(action->Get_Parameter(1)->Get_Int());
    } else {
        Append_Debug_Message("***Error allocating attack priority set - fix or raise limit. ***", false);
    }
}

int ScriptEngine::Get_Object_Count(int player, const Utf8String &name) const
{
    if (!g_thePlayerList->Get_Nth_Player(player)->Is_Player_Active()) {
        return 0;
    }

    auto it = m_playerObjectCounts[player].find(name);

    if (it == m_playerObjectCounts[player].end()) {
        return 0;
    } else {
        return it->second;
    }
}

void ScriptEngine::Set_Object_Count(int player, const Utf8String &name, int count)
{
    m_playerObjectCounts[player][name] = count;
}

void ScriptEngine::Remove_Object_Types(ObjectTypes *obj)
{
    auto it = std::find(m_allObjectTypeLists.begin(), m_allObjectTypeLists.end(), obj);

    if (it != m_allObjectTypeLists.end()) {
        obj->Delete_Instance();
        m_allObjectTypeLists.erase(it);
    }
}

bool ScriptEngine::Evaluate_Timer(Condition *condition)
{
    captainslog_dbgassert(condition->Get_Num_Parameters() >= 1, "Not enough parameters.");
    captainslog_dbgassert(condition->Get_Condition_Type() == Condition::TIMER_EXPIRED, "Wrong condition.");

    int counter = condition->Get_Parameter(0)->Get_Int();

    if (counter == 0) {
        counter = Allocate_Counter(condition->Get_Parameter(0)->Get_String());
        condition->Get_Parameter(0)->Set_Int(counter);
    }

    if (m_counters[counter].is_countdown_timer) {
        return m_counters[counter].value < 1;
    } else {
        return false;
    }
}

const float _DURATION_MULT = 0.03f;

void ScriptEngine::Set_Timer(ScriptAction *action, bool millisecond_timer, bool random)
{
    captainslog_dbgassert(action->Get_Num_Parameters() >= 2, "Not enough parameters.");
    int counter = action->Get_Parameter(0)->Get_Int();

    if (counter == 0) {
        counter = Allocate_Counter(action->Get_Parameter(0)->Get_String());
        action->Get_Parameter(0)->Set_Int(counter);
    }

    if (millisecond_timer) {
        float value = action->Get_Parameter(1)->Get_Real();

        if (random) {
            value = Get_Logic_Random_Value(value, action->Get_Parameter(2)->Get_Real());
        }

        m_counters[counter].value = GameMath::Fast_To_Int_Ceil(value * 1000.0f * _DURATION_MULT);
    } else {
        int value = action->Get_Parameter(1)->Get_Int();

        if (random) {
            value = Get_Logic_Random_Value(value, action->Get_Parameter(2)->Get_Int());
        }

        m_counters[counter].value = value;
    }

    m_counters[counter].is_countdown_timer = true;
}

void ScriptEngine::Pause_Timer(ScriptAction *action)
{
    captainslog_dbgassert(action->Get_Num_Parameters() >= 1, "Not enough parameters.");
    int counter = action->Get_Parameter(0)->Get_Int();

    if (counter == 0) {
        counter = Allocate_Counter(action->Get_Parameter(0)->Get_String());
        action->Get_Parameter(0)->Set_Int(counter);
    }

    m_counters[counter].is_countdown_timer = false;
}

void ScriptEngine::Restart_Timer(ScriptAction *action)
{
    captainslog_dbgassert(action->Get_Num_Parameters() >= 1, "Not enough parameters.");
    int counter = action->Get_Parameter(0)->Get_Int();

    if (counter == 0) {
        counter = Allocate_Counter(action->Get_Parameter(0)->Get_String());
        action->Get_Parameter(0)->Set_Int(counter);
    }

    if (m_counters[counter].value > 0) {
        m_counters[counter].is_countdown_timer = true;
    }
}

void ScriptEngine::Adjust_Timer(ScriptAction *action, bool millisecond_timer, bool add)
{
    captainslog_dbgassert(action->Get_Num_Parameters() >= 2, "Not enough parameters.");
    int counter = action->Get_Parameter(0)->Get_Int();

    if (counter == 0) {
        counter = Allocate_Counter(action->Get_Parameter(0)->Get_String());
        action->Get_Parameter(0)->Set_Int(counter);
    }

    if (millisecond_timer) {
        float value = action->Get_Parameter(1)->Get_Real();

        if (!add) {
            value = -value;
        }

        m_counters[counter].value += GameMath::Fast_To_Int_Ceil(value * 1000.0f * _DURATION_MULT);
    } else {
        int value = action->Get_Parameter(1)->Get_Int();

        if (!add) {
            value = -value;
        }

        m_counters[counter].value += value;
    }
}

void ScriptEngine::Enable_Script(ScriptAction *action)
{
    captainslog_dbgassert(action->Get_Num_Parameters() >= 1, "Not enough parameters.");
    ScriptGroup *group = Find_Group(action->Get_Parameter(0)->Get_String());

    if (group != nullptr) {
        group->Set_Active(true);
    }

    Script *script = Find_Script(action->Get_Parameter(0)->Get_String());

    if (script != nullptr) {
        script->Set_Active(true);
    }
}

void ScriptEngine::Disable_Script(ScriptAction *action)
{
    captainslog_dbgassert(action->Get_Num_Parameters() >= 1, "Not enough parameters.");
    ScriptGroup *group = Find_Group(action->Get_Parameter(0)->Get_String());

    if (group != nullptr) {
        group->Set_Active(false);
    }

    Script *script = Find_Script(action->Get_Parameter(0)->Get_String());

    if (script != nullptr) {
        script->Set_Active(false);
    }
}

void ScriptEngine::Call_Subroutine(ScriptAction *action)
{
    captainslog_dbgassert(action->Get_Num_Parameters() >= 1, "Not enough parameters.");
    Utf8String name = action->Get_Parameter(0)->Get_String();

    ScriptGroup *group = Find_Group(name);

    if (group != nullptr) {
        if (group->Is_Subroutine()) {
            if (group->Is_Active()) {
                Execute_Scripts(group->Get_Script());
            }
        } else {
            Append_Debug_Message("***Attempting to call script that is not a subroutine:***", false);
            Append_Debug_Message(name, false);
            captainslog_debug("Attempting to call script '%s' that is not a subroutine.", name.Str());
        }
    } else {
        Script *script = Find_Script(name);

        if (script != nullptr) {
            if (script->Is_Subroutine()) {
                Execute_Script(script);
                Append_Debug_Message("***Attempting to call script that is not a subroutine:***", false);
                Append_Debug_Message(name, false);
                captainslog_debug("Attempting to call script '%s' that is not a subroutine.", name.Str());
            }
        } else {
            Append_Debug_Message("***Script not defined:***", false);
            Append_Debug_Message(name, false);
            captainslog_debug("WARNING: Script '%s' not defined.", name.Str());
        }
    }
}

void ScriptEngine::Check_Conditions_For_Team_Names(Script *script)
{
    Utf8String singleton_team_name;
    Utf8String non_singleton_team_name;

    if (script->Get_Evaluation_Interval() <= 0) {
        script->Set_Evaluation_Frame(0);
    } else {
        script->Set_Evaluation_Frame(Get_Logic_Random_Value(0, 60));
    }

    Utf8String script_name = script->Get_Name();

    for (OrCondition *or_condition = script->Get_Or_Condition(); or_condition != nullptr;
         or_condition = or_condition->Get_Next_Or_Condition()) {
        for (Condition *and_condition = or_condition->Get_First_And_Condition(); and_condition != nullptr;
             and_condition = and_condition->Get_Next()) {
            for (int i = 0; i < and_condition->Get_Num_Parameters(); i++) {
                if (and_condition->Get_Parameter(i)->Get_Parameter_Type() == Parameter::TEAM) {
                    Utf8String team_name = and_condition->Get_Parameter(i)->Get_String();
                    TeamPrototype *prototype = g_theTeamFactory->Find_Team_Prototype(team_name);

                    if (prototype != nullptr) {
                        bool is_singleton = prototype->Get_Singleton();

                        if (prototype->Get_Template_Info()->m_maxInstances < 2) {
                            is_singleton = true;
                        }

                        if (is_singleton) {
                            singleton_team_name = team_name;
                        } else if (non_singleton_team_name.Is_Empty()) {
                            non_singleton_team_name = team_name;
                        } else if (non_singleton_team_name != team_name) {
                            Append_Debug_Message(
                                "***WARNING: Script contains multiple non-singleton team conditions::***", false);
                            Append_Debug_Message(script_name, false);
                            Append_Debug_Message(non_singleton_team_name, false);
                            Append_Debug_Message(team_name, false);
                            captainslog_debug(
                                "WARNING: Script '%s' contains multiple non-singleton team conditions: %s & %s.",
                                script_name.Str(),
                                non_singleton_team_name.Str(),
                                team_name.Str());
                        }
                    }
                }
            }
        }
    }

    if (non_singleton_team_name.Is_Empty()) {
        if (!singleton_team_name.Is_Empty()) {
            script->Set_Condition_Team_Name(singleton_team_name);
        }
    } else {
        script->Set_Condition_Team_Name(non_singleton_team_name);
    }
}

void ScriptEngine::Execute_Script(Script *script)
{
    script->Set_Script_Timing(0.0f);

    if (script->Is_Active()) {
        GameDifficulty difficulty = Get_Difficulty();

        if (m_currentPlayer != nullptr) {
            difficulty = m_currentPlayer->Get_Player_Difficulty();
        }

        if (difficulty == DIFFICULTY_EASY) {
            if (!script->Is_Easy()) {
                return;
            }
        } else if (difficulty == DIFFICULTY_NORMAL) {
            if (!script->Is_Normal()) {
                return;
            }
        } else if (difficulty == DIFFICULTY_HARD) {
            if (!script->Is_Hard()) {
                return;
            }
        }

        if (g_theGameLogic->Get_Frame() >= script->Get_Evaluation_Frame()) {
            int interval = script->Get_Evaluation_Interval();

            if (interval > 0) {
                script->Set_Evaluation_Frame(30 * interval + g_theGameLogic->Get_Frame());
            }

#ifdef PLATFORM_WINDOWS
            double time = 0.0f;
            LARGE_INTEGER frequency;
            LARGE_INTEGER perf_count;
            QueryPerformanceFrequency(&frequency);
            QueryPerformanceCounter(&perf_count);
#endif
            Team *condition_team = m_conditionTeam;
            TeamPrototype *prototype = nullptr;

            if (!script->Get_Condition_Team_Name().Is_Empty()) {
                prototype = g_theTeamFactory->Find_Team_Prototype(script->Get_Condition_Team_Name());
            }

            if (prototype != nullptr && prototype->Count_Team_Instances() > 0) {
                DLINK_ITERATOR<Team> it = prototype->Iterate_Team_Instance_List();

                while (!it.Done()) {
                    m_conditionTeam = it.Cur();

                    if (Evaluate_Conditions(script, nullptr, nullptr)) {
                        if (script->Get_Action() != nullptr) {
                            Append_Message(script->Get_Name(), true, false);
                            Execute_Actions(script->Get_Action());
                        }

                        if (script->Is_One_Shot()) {
                            script->Set_Active(false);
                        }
                    } else if (script->Get_False_Action()) {
                        Append_Message(script->Get_Name(), true, false);
                        Execute_Actions(script->Get_False_Action());
                    }

                    it.Advance();
                }
            } else {
                m_conditionTeam = nullptr;

                if (Evaluate_Conditions(script, nullptr, nullptr)) {
                    if (script->Get_Action() != nullptr) {
                        Append_Message(script->Get_Name(), true, false);
                        Execute_Actions(script->Get_Action());
                    }

                    if (script->Is_One_Shot()) {
                        script->Set_Active(false);
                    }
                } else if (script->Get_False_Action()) {
                    Append_Message(script->Get_Name(), true, false);
                    Execute_Actions(script->Get_False_Action());
                }
            }

#ifdef PLATFORM_WINDOWS
            LARGE_INTEGER new_perf_count;
            QueryPerformanceCounter(&new_perf_count);
            time = (double)(new_perf_count.QuadPart - perf_count.QuadPart) / (double)frequency.QuadPart;
            script->Set_Script_Timing(time);
#endif
            m_conditionTeam = condition_team;
        }
    }
}

bool ScriptEngine::Evaluate_Condition(Condition *condition)
{
    bool ret;

    switch (condition->Get_Condition_Type()) {
        case Condition::CONDITION_FALSE:
            ret = false;
            break;
        case Condition::COUNTER:
            ret = Evaluate_Counter(condition);
            break;
        case Condition::FLAG:
            ret = Evaluate_Flag(condition);
            break;
        case Condition::CONDITION_TRUE:
            ret = true;
            break;
        case Condition::TIMER_EXPIRED:
            ret = Evaluate_Timer(condition);
            break;
        default:
            ret = g_theScriptConditions->Evaluate_Condition(condition);
            break;
    }

    return ret;
}

void ScriptEngine::Friend_Execute_Action(ScriptAction *action, Team *team)
{
    Team *calling_team = m_callingTeam;
    Player *player = m_currentPlayer;
    m_callingTeam = team;
    m_currentPlayer = nullptr;

    if (team != nullptr) {
        m_currentPlayer = team->Get_Controlling_Player();
    }

    Execute_Actions(action);
    m_callingTeam = calling_team;
    m_currentPlayer = player;
}

void ScriptEngine::Add_Object_To_Cache(Object *obj)
{
    if (obj != nullptr) {
        Utf8String name = obj->Get_Name();

        if (!(name == Utf8String::s_emptyString)) {
            auto it = m_namedObjects.begin();

            for (;; it++) {
                if (it == m_namedObjects.end()) {
                    std::pair<Utf8String, Object *> pair;
                    pair.first = name;
                    pair.second = obj;
                    m_namedObjects.push_back(pair);
                    return;
                }

                if (it->first == name) {
                    break;
                }

                if (it->second == obj) {
                    it->first = name;
                    return;
                }
            }

            if (it->second == nullptr) {
                Utf8String message;
                message.Format("Reassigning dead object's name '%s' to object (%d) of type '%s'\n",
                    name.Str(),
                    obj->Get_ID(),
                    obj->Get_Template()->Get_Name().Str());
                Append_Debug_Message(message, false);
                captainslog_debug(message.Str());
                it->second = obj;
                return;
            }

            captainslog_dbgassert(false,
                "Attempting to assign the name '%s' to object (%d) of type '%s', but object (%d) of type '%s' already has "
                "that name",
                name.Str(),
                obj->Get_ID(),
                obj->Get_Template()->Get_Name().Str(),
                it->second->Get_ID(),
                it->second->Get_Template()->Get_Name().Str());
        }
    }
}

void ScriptEngine::Remove_Object_From_Cache(Object *obj)
{
    for (auto it = m_namedObjects.begin(); it != m_namedObjects.end(); it++) {
        if (obj == it->second) {
            it->second = nullptr;
        }
    }
}

void ScriptEngine::Transfer_Object_Name(const Utf8String &obj_name, Object *obj)
{
    if (obj != nullptr && obj_name.Get_Length() != 0) {
        if (obj->Get_Name().Is_Not_Empty()) {
            Remove_Object_From_Cache(obj);
        }

        obj->Set_Name(obj_name);

        for (auto it = m_namedObjects.begin(); it != m_namedObjects.end(); it++) {
            if (obj_name.Compare(it->first) == 0) {
                Object *cached_obj = it->second;

                if (cached_obj != nullptr) {
                    if (cached_obj->Has_Custom_Indicator_Color()) {
                        obj->Set_Custom_Indicator_Color(cached_obj->Get_Indicator_Color());
                    } else {
                        obj->Remove_Custom_Indicator_Color();
                    }
                }

                it->second = obj;
                return;
            }
        }
    }
}

void ScriptEngine::Notify_Of_Object_Destruction(Object *obj)
{
    if (!obj->Get_Name().Is_Empty()) {
        Remove_Object_From_Cache(obj);
    }

    if (m_conditionObject == obj) {
        m_conditionObject = nullptr;
    }

    if (m_callingObject == obj) {
        m_callingObject = nullptr;
    }
}

void ScriptEngine::Notify_Of_Completed_Video(const Utf8String &video_name)
{
    m_completedVideo.push_back(video_name);
}

void ScriptEngine::Notify_Of_Triggered_Special_Power(int player_idx, const Utf8String &power_name, ObjectID source)
{
    m_triggeredSpecialPowers[player_idx].push_back({ power_name, source });
}

void ScriptEngine::Notify_Of_Midway_Special_Power(int player_idx, const Utf8String &power_name, ObjectID source)
{
    m_midwaySpecialPowers[player_idx].push_back({ power_name, source });
}

void ScriptEngine::Notify_Of_Completed_Special_Power(int player_idx, const Utf8String &power_name, ObjectID source)
{
    m_finishedSpecialPowers[player_idx].push_back({ power_name, source });
}

void ScriptEngine::Notify_Of_Completed_Upgrade(int player_idx, const Utf8String &upgrade_name, ObjectID source)
{
    m_completedUpgrades[player_idx].push_back({ upgrade_name, source });
}

void ScriptEngine::Notify_Of_Acquired_Science(int player_idx, ScienceType science)
{
    m_acquiredSciences[player_idx].push_back(science);
}

void ScriptEngine::Signal_UI_Interact(const Utf8String &hook_name)
{
    m_uiInteraction.push_back(hook_name);
    Append_Debug_Message(hook_name, false);
}

bool ScriptEngine::Is_Video_Complete(const Utf8String &video_name, bool remove_from_list)
{
    auto it = std::find(m_completedVideo.begin(), m_completedVideo.end(), video_name);

    if (it != m_completedVideo.end()) {
        if (remove_from_list) {
            m_completedVideo.erase(it);
        }

        return true;
    }

    return false;
}

bool ScriptEngine::Is_Speech_Complete(const Utf8String &speech_name, bool remove_from_list)
{
    auto it = m_completedSpeech.begin();
    for (; it != m_completedSpeech.end(); it++) {
        if (it->first == speech_name) {
            break;
        }
    }

    if (it == m_completedSpeech.end()) {
        std::pair<Utf8String, unsigned int> pair;
        AudioEventRTS audio(speech_name);
        float length = g_theAudio->Get_Audio_Length_MS(&audio);
        pair.first = speech_name;
        pair.second = g_theGameLogic->Get_Frame() + GameMath::Fast_To_Int_Truncate(length / 33.333332f);
        m_completedSpeech.push_front(pair);
        it = m_completedSpeech.begin();
    }

    if (g_theGameLogic->Get_Frame() < it->second) {
        return false;
    }

    if (remove_from_list) {
        m_completedSpeech.erase(it);
    }

    return true;
}

bool ScriptEngine::Is_Audio_Complete(const Utf8String &audio_name, bool remove_from_list)
{
    auto it = m_completedAudio.begin();
    for (; it != m_completedAudio.end(); it++) {
        if (it->first == audio_name) {
            break;
        }
    }

    if (it == m_completedAudio.end()) {
        std::pair<Utf8String, unsigned int> pair;
        AudioEventRTS audio(audio_name);
        float length = g_theAudio->Get_Audio_Length_MS(&audio);
        pair.first = audio_name;
        pair.second = g_theGameLogic->Get_Frame() + GameMath::Fast_To_Int_Truncate(length / 33.333332f);
        m_completedAudio.push_front(pair);
        it = m_completedAudio.begin();
    }

    if (g_theGameLogic->Get_Frame() < it->second) {
        return false;
    }

    if (remove_from_list) {
        m_completedAudio.erase(it);
    }

    return true;
}

bool ScriptEngine::Is_Special_Power_Triggered(
    int player_idx, const Utf8String &power_name, bool remove_from_list, ObjectID source)
{
    if (player_idx >= MAX_PLAYER_COUNT) {
        return false;
    }

    for (auto it = m_triggeredSpecialPowers[player_idx].begin(); it != m_triggeredSpecialPowers[player_idx].end(); it++) {
        if (it->first == power_name && (source == INVALID_OBJECT_ID || source == it->second)) {
            if (remove_from_list) {
                m_triggeredSpecialPowers[player_idx].erase(it);
            }

            return true;
        }
    }

    return false;
}

bool ScriptEngine::Is_Special_Power_Midway(
    int player_idx, const Utf8String &power_name, bool remove_from_list, ObjectID source)
{
    if (player_idx >= MAX_PLAYER_COUNT) {
        return false;
    }

    for (auto it = m_midwaySpecialPowers[player_idx].begin(); it != m_midwaySpecialPowers[player_idx].end(); it++) {
        if (it->first == power_name && (source == INVALID_OBJECT_ID || source == it->second)) {
            if (remove_from_list) {
                m_midwaySpecialPowers[player_idx].erase(it);
            }

            return true;
        }
    }

    return false;
}

bool ScriptEngine::Is_Special_Power_Complete(
    int player_idx, const Utf8String &power_name, bool remove_from_list, ObjectID source)
{
    if (player_idx >= MAX_PLAYER_COUNT) {
        return false;
    }

    for (auto it = m_finishedSpecialPowers[player_idx].begin(); it != m_finishedSpecialPowers[player_idx].end(); it++) {
        if (it->first == power_name && (source == INVALID_OBJECT_ID || source == it->second)) {
            if (remove_from_list) {
                m_finishedSpecialPowers[player_idx].erase(it);
            }

            return true;
        }
    }

    return false;
}

bool ScriptEngine::Is_Upgrade_Complete(
    int player_idx, const Utf8String &upgrade_name, bool remove_from_list, ObjectID source)
{
    if (player_idx >= MAX_PLAYER_COUNT) {
        return false;
    }

    for (auto it = m_completedUpgrades[player_idx].begin(); it != m_completedUpgrades[player_idx].end(); it++) {
        if (it->first == upgrade_name && (source == INVALID_OBJECT_ID || source == it->second)) {
            if (remove_from_list) {
                m_completedUpgrades[player_idx].erase(it);
            }

            return true;
        }
    }

    return false;
}

bool ScriptEngine::Is_Science_Acquired(int player_idx, ScienceType science, bool remove_from_list)
{
    if (player_idx >= MAX_PLAYER_COUNT) {
        return false;
    }

    for (auto it = m_acquiredSciences[player_idx].begin(); it != m_acquiredSciences[player_idx].end(); it++) {
        if (*it == science) {
            if (remove_from_list) {
                m_acquiredSciences[player_idx].erase(it);
            }

            return true;
        }
    }

    return false;
}

void ScriptEngine::Set_Topple_Direction(const Utf8String &name, const Coord3D *direction)
{
    if (!name.Is_Empty()) {
        auto it = m_toppleDirections.begin();

        for (;; it++) {
            if (!(it != m_toppleDirections.end())) {
                std::pair<Utf8String, Coord3D> pair;
                pair.first = name;
                pair.second = *direction;
                m_toppleDirections.push_back(pair);
                return;
            }

            if (it->first == name) {
                break;
            }
        }

        if (direction != nullptr) {
            it->second = *direction;
        } else {
            m_toppleDirections.erase(it);
        }
    }
}

void ScriptEngine::Adjust_Topple_Direction(Object *obj, Coord2D *direction)
{
    if (obj != nullptr) {
        if (direction != nullptr) {
            Coord3D new_direction;
            new_direction.x = direction->x;
            new_direction.y = direction->y;
            Adjust_Topple_Direction(obj, &new_direction);
            direction->x = new_direction.x;
            direction->y = new_direction.y;
        }
    }
}

void ScriptEngine::Adjust_Topple_Direction(Object *obj, Coord3D *direction)
{
    Utf8String name = obj->Get_Name();

    if (!name.Is_Empty() && direction != nullptr) {
        for (auto it = m_toppleDirections.begin(); it != m_toppleDirections.end(); it++) {
            if (it->first == name) {
                *direction = it->second;
                direction->Normalize();
            }
        }
    }
}

bool ScriptEngine::Evaluate_Conditions(Script *script, Team *team, Player *player)
{
    LatchRestore<Team *> team_latch(&m_callingTeam, &team);

    if (team != nullptr) {
        player = team->Get_Controlling_Player();
    }

    if (player == nullptr) {
        player = m_currentPlayer;
    }

    LatchRestore<Player *> player_latch(&m_currentPlayer, &player);
    bool ret = false;

#ifdef PLATFORM_WINDOWS
    LARGE_INTEGER frequency;
    LARGE_INTEGER perf_count;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&perf_count);
#endif

    for (OrCondition *or_condition = script->Get_Or_Condition(); or_condition != nullptr;
         or_condition = or_condition->Get_Next_Or_Condition()) {
        Condition *and_condition = or_condition->Get_First_And_Condition();

        if (and_condition != nullptr) {
            bool b = true;

            while (and_condition != nullptr) {
                if (!Evaluate_Condition(and_condition)) {
                    b = false;
                    break;
                }

                and_condition = and_condition->Get_Next();
            }

            if (b) {
                ret = true;
                break;
            }
        }
    }

    script->Inc_Eval_Count();

#ifdef PLATFORM_WINDOWS
    LARGE_INTEGER new_perf_count;
    QueryPerformanceCounter(&new_perf_count);
    double time = (double)(new_perf_count.QuadPart - perf_count.QuadPart) / (double)frequency.QuadPart;
    script->Update_Exec_Time(time);
#endif

    return ret;
}

void ScriptEngine::Execute_Actions(ScriptAction *action)
{
    for (ScriptAction *act = action; act != nullptr; act = act->Get_Next()) {
        switch (act->Get_Action_Type()) {
            case ScriptAction::SET_FLAG:
                Set_Flag(act);
                break;
            case ScriptAction::SET_COUNTER:
                Set_Counter(act);
                break;
            case ScriptAction::NO_OP:
                continue;
            case ScriptAction::SET_TIMER:
                Set_Timer(act, false, false);
                break;
            case ScriptAction::ENABLE_SCRIPT:
                Enable_Script(act);
                break;
            case ScriptAction::DISABLE_SCRIPT:
                Disable_Script(act);
                break;
            case ScriptAction::CALL_SUBROUTINE:
                Call_Subroutine(act);
                break;
            case ScriptAction::INCREMENT_COUNTER:
                Add_Counter(act);
                break;
            case ScriptAction::DECREMENT_COUNTER:
                Sub_Counter(act);
                break;
            case ScriptAction::SET_MILLISECOND_TIMER:
                Set_Timer(act, true, false);
                break;
            case ScriptAction::SET_TREE_SWAY:
                Set_Sway(act);
                break;
            case ScriptAction::CAMERA_FADE_ADD:
            case ScriptAction::CAMERA_FADE_SUBTRACT:
            case ScriptAction::CAMERA_FADE_SATURATE:
            case ScriptAction::CAMERA_FADE_MULTIPLY:
                Set_Fade(act);
                break;
            case ScriptAction::SET_ATTACK_PRIORITY_THING:
                Set_Priority_Thing(act);
                break;
            case ScriptAction::SET_ATTACK_PRIORITY_KIND_OF:
                Set_Priority_Kind(act);
                break;
            case ScriptAction::SET_DEFAULT_ATTACK_PRIORITY:
                Set_Priority_Default(act);
                break;
            case ScriptAction::SET_RANDOM_TIMER:
                Set_Timer(act, false, true);
                break;
            case ScriptAction::SET_RANDOM_MSEC_TIMER:
                Set_Timer(act, true, true);
                break;
            case ScriptAction::STOP_TIMER:
                Pause_Timer(act);
                break;
            case ScriptAction::RESTART_TIMER:
                Restart_Timer(act);
                break;
            case ScriptAction::ADD_TO_MSEC_TIMER:
                Adjust_Timer(act, true, true);
                break;
            case ScriptAction::SUB_FROM_MSEC_TIMER:
                Adjust_Timer(act, true, false);
                break;
            default:
                if (g_theScriptActions != nullptr) {
                    g_theScriptActions->Execute_Action(act);
                }
                break;
        }
    }
}

void ScriptEngine::Execute_Scripts(Script *script)
{
    while (script != nullptr) {
        if (!script->Is_Subroutine()) {
            Execute_Script(script);
        }

        script = script->Get_Next();
    }
}

ActionTemplate *ScriptEngine::Get_Action_Template(int index)
{
    captainslog_dbgassert(index < ScriptAction::ACTION_COUNT, "Out of range.");

    if (index >= ScriptAction::ACTION_COUNT) {
        index = ScriptAction::DEBUG_MESSAGE_BOX;
    }

    captainslog_dbgassert(!m_actionTemplates[index].Get_UI_Name().Is_Empty(), "Need to initialize action enum=%d.", index);
    return &m_actionTemplates[index];
}

ConditionTemplate *ScriptEngine::Get_Condition_Template(int index)
{
    captainslog_dbgassert(index < Condition::CONDITION_COUNT, "Out of range.");

    if (index >= Condition::CONDITION_COUNT) {
        index = Condition::CONDITION_FALSE;
    }

    captainslog_dbgassert(
        !m_conditionTemplates[index].Get_UI_Name().Is_Empty(), "Need to initialize Condition enum=%d.", index);
    return &m_conditionTemplates[index];
}

void ScriptEngine::Create_Named_Cache()
{
    m_namedObjects.clear();

    if (g_theGameLogic != nullptr) {
        for (Object *obj = g_theGameLogic->Get_First_Object(); obj != nullptr; obj = obj->Get_Next_Object()) {
            if (!obj->Get_Name().Is_Empty()) {
                std::pair<Utf8String, Object *> pair;
                pair.first = obj->Get_Name();
                pair.second = obj;
                m_namedObjects.push_back(pair);
            }
        }
    }
}

void ScriptEngine::Append_Sequential_Script(const SequentialScript *script)
{
    SequentialScript *new_script = new SequentialScript();
    *new_script = *script;
    new_script->m_nextScriptInSequence = nullptr;
    new_script->m_currentInstruction = -1;
    bool found = false;

    for (auto it = m_sequentialScripts.begin(); it != m_sequentialScripts.end(); it++) {
        SequentialScript *sequential = *it;

        if (sequential != nullptr
            && (script->m_objectID != INVALID_OBJECT_ID && sequential->m_objectID == script->m_objectID
                || script->m_teamToExecOn != nullptr && script->m_teamToExecOn == sequential->m_teamToExecOn)) {
            found = true;

            while (sequential->m_nextScriptInSequence != nullptr) {
                sequential = sequential->m_nextScriptInSequence;
            }

            sequential->m_nextScriptInSequence = new_script;
            break;
        }
    }

    if (!found) {
        m_sequentialScripts.push_back(new_script);
    }
}

void ScriptEngine::Remove_Sequential_Script(SequentialScript *script)
{
    // function empty in original
}

void ScriptEngine::Remove_All_Sequential_Scripts(Object *obj)
{
    if (obj != nullptr) {
        ObjectID id = obj->Get_ID();

        auto it = m_sequentialScripts.begin();

        while (it != m_sequentialScripts.end()) {
            if (*it != nullptr) {
                if ((*it)->m_objectID == id) {
                    it = Cleanup_Sequential_Script(it, true);
                } else {
                    it++;
                }
            } else {
                it++;
            }
        }
    }
}

void ScriptEngine::Remove_All_Sequential_Scripts(Team *team)
{
    Notify_Of_Team_Destruction(team);
}

void ScriptEngine::Notify_Of_Team_Destruction(Team *team_destroyed)
{
    if (team_destroyed != nullptr) {
        for (auto it = m_sequentialScripts.begin(); it != m_sequentialScripts.end();) {
            if (*it != nullptr) {
                if ((*it)->m_teamToExecOn == team_destroyed) {
                    it = Cleanup_Sequential_Script(it, true);
                } else {
                    it++;
                }
            }
        }

        if (m_callingTeam == team_destroyed) {
            m_callingTeam = nullptr;
        }

        if (m_conditionTeam == team_destroyed) {
            m_conditionTeam = nullptr;
        }
    }
}

void ScriptEngine::Notify_Of_Object_Creation_Or_Destruction()
{
    m_objectCreationDestructionFrame = g_theGameLogic->Get_Frame();
}

void ScriptEngine::Set_Sequential_Timer(Object *obj, int timer)
{
    if (obj != nullptr) {
        ObjectID id = obj->Get_ID();

        for (auto it = m_sequentialScripts.begin(); it != m_sequentialScripts.end(); it++) {
            if (*it != nullptr && (*it)->m_objectID == id) {
                (*it)->m_framesToWait = timer;
            }
        }
    }
}

void ScriptEngine::Set_Sequential_Timer(Team *team, int timer)
{
    if (team != nullptr) {
        for (auto it = m_sequentialScripts.begin(); it != m_sequentialScripts.end(); it++) {
            if (*it != nullptr && (*it)->m_teamToExecOn == team) {
                (*it)->m_framesToWait = timer;
            }
        }
    }
}

void ScriptEngine::Evaluate_And_Progress_All_Sequential_Scripts()
{
    auto prev_script = m_sequentialScripts.end();
    int repeat_count = 0;
    auto cur_script = m_sequentialScripts.begin();

    for (;;) {
        if (cur_script == m_sequentialScripts.end()) {
            break;
        }

        if (cur_script == prev_script) {
            repeat_count++;
        } else {
            repeat_count = 0;
        }

        if (repeat_count > 20) {
            SequentialScript *script = *cur_script;

            if (script != nullptr) {
                captainslog_debug("Sequential script %s appears to be in an infinite loop.",
                    script->m_scriptToExecuteSequentially->Get_Name().Str());
            }

            cur_script++;
        } else {
            prev_script = cur_script;
            bool b = false;
            SequentialScript *script = *cur_script;

            if (script != nullptr) {
                Team *team = script->m_teamToExecOn;
                Object *obj = g_theGameLogic->Find_Object_By_ID(script->m_objectID);

                if (obj != nullptr || team != nullptr) {
                    m_currentPlayer = nullptr;

                    if (obj != nullptr) {
                        m_currentPlayer = obj->Get_Controlling_Player();
                    } else if (team) {
                        m_currentPlayer = team->Get_Controlling_Player();
                    }

                    if (m_currentPlayer != nullptr && !m_currentPlayer->Is_Skirmish_AI_Player()) {
                        m_currentPlayer = nullptr;
                    }

                    AIUpdateInterface *update;

                    if (obj != nullptr) {
                        update = obj->Get_AI_Update_Interface();
                    } else {
                        update = nullptr;
                    }

                    AIGroup *group;

                    if (team != nullptr) {
                        group = g_theAI->Create_Group();
                    } else {
                        group = nullptr;
                    }

                    if (group != nullptr) {
                        team->Get_Team_As_AI_Group(group);
                    }

                    if (update != nullptr || group != nullptr) {
                        if ((((update == nullptr || !update->Is_Idle()) && (group == nullptr || !group->Is_Idle()))
                                || script->m_framesToWait >= 1)
                            && script->m_framesToWait != 0) {
                            if (script->m_framesToWait > 0) {
                                script->m_framesToWait--;
                            }

                            goto l1;
                        }

                        bool append_debug_message = true;

                        if (script->m_unkbool1) {
                            script->m_unkbool1 = false;
                            append_debug_message = false;
                        } else {
                            script->m_currentInstruction++;
                        }

                        Utf8String message("Advancing SeqScript \'");
                        message += script->m_scriptToExecuteSequentially->Get_Name();
                        message.Concat("\' on ");
                        Utf8String owner_name;

                        if (team != nullptr) {
                            owner_name = team->Get_Name();
                        }

                        if (obj != nullptr) {
                            owner_name = obj->Get_Name();
                        }

                        message += owner_name;
                        message.Concat(" -- ");

                        int instruction = script->m_currentInstruction;
                        ScriptAction *action;

                        for (action = script->m_scriptToExecuteSequentially->Get_Action();
                             action != nullptr && instruction != 0;
                             action = action->Get_Next()) {
                            instruction--;
                        }

                        if (action == nullptr) {
                            if (script->m_timesToLoop != 0) {
                                if (script->m_timesToLoop != -1) {
                                    script->m_timesToLoop--;
                                }

                                script->m_framesToWait = -1;
                                Append_Sequential_Script(script);
                            }

                            cur_script = Cleanup_Sequential_Script(cur_script, false);
                            b = true;
                            goto l1;
                        }

                        m_conditionTeam = team;
                        m_conditionObject = obj;
                        script->m_framesToWait = -1;
                        ScriptAction *next_action = action->Get_Next();
                        action->Set_Next_Action(nullptr);

                        if (action->Get_Action_Type() == ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_ALL) {
                            if (!g_theScriptConditions->Evaluate_Skirmish_Command_Button_Is_Ready(
                                    nullptr, action->Get_Parameter(1), action->Get_Parameter(2), true)) {
                                script->m_unkbool1 = true;
                            }
                        } else if (action->Get_Action_Type()
                            == ScriptAction::SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_PARTIAL) {
                            if (!g_theScriptConditions->Evaluate_Skirmish_Command_Button_Is_Ready(
                                    nullptr, action->Get_Parameter(1), action->Get_Parameter(2), false)) {
                                script->m_unkbool1 = true;
                            }
                        } else if (action->Get_Action_Type() == ScriptAction::TEAM_WAIT_FOR_NOT_CONTAINED_ALL) {
                            if (g_theScriptConditions->Evaluate_Team_Is_Contained(action->Get_Parameter(0), true)) {
                                script->m_unkbool1 = true;
                            }
                        } else if (action->Get_Action_Type() == ScriptAction::TEAM_WAIT_FOR_NOT_CONTAINED_PARTIAL) {
                            if (g_theScriptConditions->Evaluate_Team_Is_Contained(action->Get_Parameter(0), false)) {
                                script->m_unkbool1 = true;
                            }
                        } else {
                            Execute_Actions(action);
                        }

                        if (append_debug_message) {
                            message += action->Get_UI_Text();
                            Append_Debug_Message(message, false);
                        } else {
                            message.Clear();
                        }

                        action->Set_Next_Action(next_action);

                        if (script->m_unkbool1) {
                            cur_script++;
                            b = true;
                        } else {
                            if (update != nullptr && update->Is_Idle()) {
                                b = true;
                            } else if (team != nullptr) {
                                group = g_theAI->Create_Group();
                                team->Get_Team_As_AI_Group(group);
                            }

                            if (group != nullptr && group->Is_Idle()) {
                                b = true;
                            }

                            if (!b
                                || (obj == nullptr || !obj->Is_Effectively_Dead())
                                    && (group == nullptr || !group->Is_Group_AI_Dead())) {
                                goto l1;
                            }

                            cur_script = Cleanup_Sequential_Script(cur_script, true);
                        }
                    } else {
                    l1:
                        if (!b) {
                            cur_script++;
                        }
                    }
                } else {
                    cur_script = Cleanup_Sequential_Script(cur_script, false);
                    b = true;
                }
            } else {
                cur_script = Cleanup_Sequential_Script(cur_script, false);
            }
        }
    }

    m_currentPlayer = nullptr;
}

std::vector<SequentialScript *>::iterator ScriptEngine::Cleanup_Sequential_Script(
    std::vector<SequentialScript *>::iterator it, bool delete_sequence)
{
    SequentialScript *script = *it;

    if (*it == nullptr) {
        return it;
    }

    if (delete_sequence) {
        while (script != nullptr) {
            SequentialScript *to_delete = script;
            script = script->m_nextScriptInSequence;
            to_delete->Delete_Instance();
        }

        *it = nullptr;
    } else {
        *it = script->m_nextScriptInSequence;
        script->Delete_Instance();
    }

    return *it != nullptr ? it : m_sequentialScripts.erase(it);
}

bool ScriptEngine::Has_Unit_Completed_Sequential_Script(Object *obj, const Utf8String &name)
{
    return false;
}

bool ScriptEngine::Has_Team_Completed_Sequential_Script(Team *team, const Utf8String &name)
{
    return false;
}

bool ScriptEngine::Get_Enable_Vtune() const
{
    // vtune code removed
    return false;
}

void ScriptEngine::Set_Enable_Vtune(bool set)
{
    // vtune code removed
}

void ScriptEngine::Particle_Editor_Update()
{
#ifdef PLATFORM_WINDOWS
    if (s_particleDll != nullptr) {
        Update_Current_Particle_Count();
        bool keeplooping = false;

        do {
            if (m_firstUpdate) {
                Append_All_Particle_Systems();
                Append_All_Thing_Templates();
            } else {
                int behavior = Get_Editor_Behavior();

                if (behavior > 254) {
                    if (behavior == 255) {
                        keeplooping = false;
                    }
                } else if (behavior == 254) {
                    keeplooping = true;
                } else {
                    switch (behavior) {
                        case 0:
                            keeplooping = false;
                            return;
                        case 1:
                        case 2:
                            Update_And_Set_Current_System();
                            keeplooping = false;
                            break;
                        case 3: {
                            Utf8String name = Get_Particle_System_Name();
                            Add_Updated_Particle_System(name);
                            ParticleSystemTemplate *tmplate = g_theParticleSystemManager->Find_Template(name);

                            if (tmplate != nullptr) {
                                Update_AsciiString_Parms_To_System(tmplate);
                            }

                            Write_Out_INI();
                            keeplooping = false;
                            break;
                        }
                        case 4:
                            Reload_Particle_System_From_INI(Get_Particle_System_Name());
                            keeplooping = false;
                            return;
                        case 5: {
                            int cap = Get_New_Current_Particle_Cap();

                            if (cap >= 0) {
                                g_theWriteableGlobalData->m_maxParticleCount = cap;
                            }

                            Reload_Textures();
                            keeplooping = false;
                        } break;
                        case 6:
                            Reload_Textures();
                            keeplooping = false;
                            break;
                        case 7:
                            g_theParticleSystemManager->Reset();
                            Update_Current_Particle_Count();
                            keeplooping = true;
                            break;
                        default:
                            break;
                    }
                }
            }
        } while (keeplooping);
    }
#endif
}

bool ScriptEngine::Is_Time_Frozen_Debug()
{
#ifdef PLATFORM_WINDOWS
    if (s_debugDll == nullptr) {
        return false;
    }

    if (s_lastFrame != s_currentFrame) {
        s_lastFrame = s_currentFrame;

        bool (*CanAppContinue)(void) = reinterpret_cast<bool (*)(void)>(GetProcAddress(s_debugDll, "CanAppContinue"));

        if (CanAppContinue != nullptr) {
            s_canAppContinue = CanAppContinue();
        }
    }

    return s_canAppContinue == false;
#else
    return false;
#endif
}

bool ScriptEngine::Is_Time_Fast()
{
#ifdef PLATFORM_WINDOWS
    if (s_debugDll == nullptr) {
        return false;
    }

    bool (*ForceAppContinue)(void) = reinterpret_cast<bool (*)(void)>(GetProcAddress(s_debugDll, "ForceAppContinue"));

    if (ForceAppContinue != nullptr && ForceAppContinue()) {
        s_appIsFast = true;
    } else if (s_appIsFast) {
        s_appIsFast = false;
    }

    return s_appIsFast && g_theGameLogic->Get_Frame() % 10 != 0;
#else
    return false;
#endif
}

void ScriptEngine::Force_Unfreeze_Time()
{
#ifdef PLATFORM_WINDOWS
    if (s_debugDll != nullptr) {
        void (*CanAppContinue)(void) = reinterpret_cast<void (*)(void)>(GetProcAddress(s_debugDll, "CanAppContinue"));

        if (CanAppContinue) {
            CanAppContinue();
        }
    }
#endif
}

void ScriptEngine::Append_Debug_Message(const Utf8String &message, bool should_pause)
{
#ifdef PLATFORM_WINDOWS
    if (s_debugDll != nullptr) {
        void (*AppendMessage)(const char *);

        if (should_pause) {
            AppendMessage = reinterpret_cast<void (*)(const char *)>(GetProcAddress(s_debugDll, "AppendMessageAndPause"));
        } else {
            AppendMessage = reinterpret_cast<void (*)(const char *)>(GetProcAddress(s_debugDll, "AppendMessage"));
        }

        if (AppendMessage != nullptr) {
            Utf8String str;
            str.Format("%d ", g_theGameLogic->Get_Frame());
            str += message;
            AppendMessage(str.Str());
        }
    }
#endif
}

void ScriptEngine::Adjust_Debug_Variable_Data(const Utf8String &variable, int value, bool pause)
{
    Adjust_Variable(variable, value, pause);
}

void ScriptEngine::Set_Global_Difficulty(GameDifficulty diff)
{
    captainslog_debug("ScriptEngine::setGlobalDifficulty(%d)", diff);
    m_gameDifficulty = diff;
}

void Xfer_List_AsciiString(Xfer *xfer, std::list<Utf8String> *list)
{
    captainslog_dbgassert(list != nullptr, "xferListAsciiString - Invalid parameters");
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    unsigned short size = static_cast<unsigned short>(list->size());
    xfer->xferUnsignedShort(&size);
    Utf8String str;

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (auto it = list->begin(); it != list->end(); it++) {
            str = *it;
            xfer->xferAsciiString(&str);
        }
    } else {
        captainslog_relassert(list->empty(), 6, "xferListAsciiString - list should be empty upon loading but is not");

        for (unsigned short i = 0; i < size; i++) {
            xfer->xferAsciiString(&str);
            list->push_back(str);
        }
    }
}

void Xfer_List_AsciiString_Uint(Xfer *xfer, std::list<std::pair<Utf8String, unsigned int>> *list)
{
    captainslog_dbgassert(list != nullptr, "xferListAsciiStringUINT - Invalid parameters");
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    unsigned short size = static_cast<unsigned short>(list->size());
    xfer->xferUnsignedShort(&size);
    Utf8String str;

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (auto it = list->begin(); it != list->end(); it++) {
            str = it->first;
            xfer->xferAsciiString(&str);
            unsigned int value = it->second;
            xfer->xferUnsignedInt(&value);
        }
    } else {
        captainslog_relassert(list->empty(), 6, "xferListAsciiStringUINT - list should be empty upon loading but is not");
        std::pair<Utf8String, unsigned int> pair;

        for (unsigned short i = 0; i < size; i++) {
            xfer->xferAsciiString(&str);
            unsigned int value;
            xfer->xferUnsignedInt(&value);
            pair.first = str;
            pair.second = value;
            list->push_back(pair);
        }
    }
}

void Xfer_List_AsciiString_ObjectID(Xfer *xfer, std::list<std::pair<Utf8String, ObjectID>> *list)
{
    captainslog_dbgassert(list != nullptr, "xferListAsciiStringObjectID - Invalid parameters");
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    unsigned short size = static_cast<unsigned short>(list->size());
    xfer->xferUnsignedShort(&size);
    Utf8String str;

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (auto it = list->begin(); it != list->end(); it++) {
            str = it->first;
            xfer->xferAsciiString(&str);
            ObjectID value = it->second;
            xfer->xferObjectID(&value);
        }
    } else {
        captainslog_relassert(
            list->empty(), 6, "xferListAsciiStringObjectID - list should be empty upon loading but is not");
        std::pair<Utf8String, ObjectID> pair;

        for (unsigned short i = 0; i < size; i++) {
            xfer->xferAsciiString(&str);
            ObjectID value;
            xfer->xferObjectID(&value);
            pair.first = str;
            pair.second = value;
            list->push_back(pair);
        }
    }
}

void Xfer_List_AsciiString_Coord3D(Xfer *xfer, std::list<std::pair<Utf8String, Coord3D>> *list)
{
    captainslog_dbgassert(list != nullptr, "xferListAsciiStringCoord3D - Invalid parameters");
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    unsigned short size = static_cast<unsigned short>(list->size());
    xfer->xferUnsignedShort(&size);
    Utf8String str;

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (auto it = list->begin(); it != list->end(); it++) {
            str = it->first;
            xfer->xferAsciiString(&str);
            Coord3D value = it->second;
            xfer->xferCoord3D(&value);
        }
    } else {
        captainslog_relassert(list->empty(), 6, "xferListAsciiStringCoord3D - list should be empty upon loading but is not");
        std::pair<Utf8String, Coord3D> pair;

        for (unsigned short i = 0; i < size; i++) {
            xfer->xferAsciiString(&str);
            Coord3D value;
            xfer->xferCoord3D(&value);
            pair.first = str;
            pair.second = value;
            list->push_back(pair);
        }
    }
}

void ScriptEngine::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 5;
    xfer->xferVersion(&version, 5);
    unsigned short sequential_script_count = static_cast<unsigned short>(m_sequentialScripts.size());
    xfer->xferUnsignedShort(&sequential_script_count);

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (auto it = m_sequentialScripts.begin(); it != m_sequentialScripts.end(); it++) {
            xfer->xferSnapshot(*it);
        }
    } else {
        captainslog_relassert(
            m_sequentialScripts.size() == 0, 6, "ScriptEngine::xfer - m_sequentialScripts should be empty but is not");

        for (unsigned short i = 0; i < sequential_script_count; i++) {
            SequentialScript *script = new SequentialScript();
            m_sequentialScripts.push_back(script);
            xfer->xferSnapshot(script);
        }
    }

    unsigned short counter_count = m_numCounters;
    xfer->xferUnsignedShort(&counter_count);
    captainslog_relassert(
        counter_count <= MAX_COUNTERS, 6, "ScriptEngine::xfer - MAX_COUNTERS has changed size, need to version this");

    for (unsigned int j = 0; j < counter_count; j++) {
        xfer->xferInt(&m_counters[j].value);
        xfer->xferAsciiString(&m_counters[j].name);
        xfer->xferBool(&m_counters[j].is_countdown_timer);
    }

    xfer->xferInt(&m_numCounters);

    unsigned short flag_count = m_numFlags;
    xfer->xferUnsignedShort(&flag_count);
    captainslog_relassert(
        flag_count <= MAX_FLAGS, 6, "ScriptEngine::xfer - MAX_FLAGS has changed size, need to version this");

    for (unsigned int j = 0; j < flag_count; j++) {
        xfer->xferBool(&m_flags[j].value);
        xfer->xferAsciiString(&m_flags[j].name);
    }

    xfer->xferInt(&m_numFlags);

    unsigned short attack_info_count = m_numAttackInfo;
    xfer->xferUnsignedShort(&attack_info_count);
    captainslog_relassert(attack_info_count <= MAX_ATTACK_PRIORITIES,
        6,
        "ScriptEngine::xfer - MAX_ATTACK_PRIORITIES has changed size, need to version this");

    for (unsigned int j = 0; j < attack_info_count; j++) {
        xfer->xferSnapshot(&m_attackPriorityInfo[j]);
    }

    xfer->xferInt(&m_numAttackInfo);
    xfer->xferInt(&m_endGameTimer);
    xfer->xferInt(&m_closeWindowTimer);
    unsigned short named_objects_count = static_cast<unsigned short>(m_namedObjects.size());
    xfer->xferUnsignedShort(&named_objects_count);
    Utf8String name;

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (auto it = m_namedObjects.begin(); it != m_namedObjects.end(); it++) {
            name = it->first;
            xfer->xferAsciiString(&name);
            ObjectID id;

            if (it->second != nullptr) {
                id = it->second->Get_ID();
            } else {
                id = INVALID_OBJECT_ID;
            }

            xfer->xferObjectID(&id);
        }
    } else {
        m_namedObjects.clear();
        std::pair<Utf8String, Object *> pair;

        for (unsigned short k = 0; k < named_objects_count; k++) {
            xfer->xferAsciiString(&name);
            ObjectID id;
            xfer->xferObjectID(&id);
            Object *obj = g_theGameLogic->Find_Object_By_ID(id);
            captainslog_relassert(obj != nullptr || id == INVALID_OBJECT_ID,
                6,
                "ScriptEngine::xfer - Unable to find object by ID for m_namedObjects");
            pair.first = name;
            pair.second = obj;
            m_namedObjects.push_back(pair);
        }
    }

    xfer->xferBool(&m_firstUpdate);
    xfer->xferUser(&m_fade, sizeof(m_fade));
    xfer->xferReal(&m_minFade);
    xfer->xferReal(&m_maxFade);
    xfer->xferReal(&m_curFadeValue);
    xfer->xferInt(&m_curFadeFrame);
    xfer->xferInt(&m_fadeFramesIncrease);
    xfer->xferInt(&m_fadeFramesHold);
    xfer->xferInt(&m_fadeFramesDecrease);
    Xfer_List_AsciiString(xfer, &m_completedVideo);
    Xfer_List_AsciiString_Uint(xfer, &m_completedSpeech);
    Xfer_List_AsciiString_Uint(xfer, &m_completedAudio);
    Xfer_List_AsciiString(xfer, &m_uiInteraction);
    unsigned short player_count = MAX_PLAYER_COUNT;
    xfer->xferUnsignedShort(&player_count);
    captainslog_relassert(player_count == MAX_PLAYER_COUNT,
        6,
        "ScriptEngine::xfer - MAX_PLAYER_COUNT has changed, m_triggeredSpecialPowers size is now different and we must "
        "version this");

    for (unsigned short j = 0; j < player_count; j++) {
        Xfer_List_AsciiString_ObjectID(xfer, &m_triggeredSpecialPowers[j]);
    }

    player_count = MAX_PLAYER_COUNT;
    xfer->xferUnsignedShort(&player_count);
    captainslog_relassert(player_count == MAX_PLAYER_COUNT,
        6,
        "ScriptEngine::xfer - MAX_PLAYER_COUNT has changed, m_midwaySpecialPowers size is now different and we must "
        "version "
        "this");

    for (unsigned short j = 0; j < player_count; j++) {
        Xfer_List_AsciiString_ObjectID(xfer, &m_midwaySpecialPowers[j]);
    }

    player_count = MAX_PLAYER_COUNT;
    xfer->xferUnsignedShort(&player_count);
    captainslog_relassert(player_count == MAX_PLAYER_COUNT,
        6,
        "ScriptEngine::xfer - MAX_PLAYER_COUNT has changed, m_finishedSpecialPowers size is now different and we must "
        "version this");

    for (unsigned short j = 0; j < player_count; j++) {
        Xfer_List_AsciiString_ObjectID(xfer, &m_finishedSpecialPowers[j]);
    }

    player_count = MAX_PLAYER_COUNT;
    xfer->xferUnsignedShort(&player_count);
    captainslog_relassert(player_count == MAX_PLAYER_COUNT,
        6,
        "ScriptEngine::xfer - MAX_PLAYER_COUNT has changed, m_completedUpgrades size is now different and we must "
        "version "
        "this");

    for (unsigned short j = 0; j < player_count; j++) {
        Xfer_List_AsciiString_ObjectID(xfer, &m_completedUpgrades[j]);
    }

    player_count = MAX_PLAYER_COUNT;
    xfer->xferUnsignedShort(&player_count);
    captainslog_relassert(player_count == MAX_PLAYER_COUNT,
        6,
        "ScriptEngine::xfer - MAX_PLAYER_COUNT has changed, m_acquiredSciences size is now different and we must "
        "version "
        "this");

    for (unsigned short j = 0; j < player_count; j++) {
        xfer->xferScienceVec(&m_acquiredSciences[j]);
    }

    Xfer_List_AsciiString_Coord3D(xfer, &m_toppleDirections);
    xfer->xferReal(&m_breezeInfo.direction);
    xfer->xferReal(&m_breezeInfo.sway_direction.x);
    xfer->xferReal(&m_breezeInfo.sway_direction.y);
    xfer->xferReal(&m_breezeInfo.intensity);
    xfer->xferReal(&m_breezeInfo.lean);
    xfer->xferReal(&m_breezeInfo.randomness);
    xfer->xferShort(&m_breezeInfo.period);
    xfer->xferShort(&m_breezeInfo.version);
    xfer->xferUser(&m_gameDifficulty, sizeof(m_gameDifficulty));
    xfer->xferBool(&m_freezeByScript);

    if (version >= 2) {
        unsigned short named_reveals_count = static_cast<unsigned short>(m_namedReveals.size());
        xfer->xferUnsignedShort(&named_reveals_count);

        if (xfer->Get_Mode() == XFER_SAVE) {
            for (auto it = m_namedReveals.begin(); it != m_namedReveals.end(); it++) {
                xfer->xferAsciiString(&it->reveal_name);
                xfer->xferAsciiString(&it->waypoint_name);
                xfer->xferReal(&it->radius);
                xfer->xferAsciiString(&it->player);
            }
        } else {
            captainslog_relassert(
                m_namedReveals.empty(), 6, "ScriptEngine::xfer - m_namedReveals should be empty but is not!");
            NamedReveal reveal;

            for (unsigned short n = 0; n < named_reveals_count; n++) {
                xfer->xferAsciiString(&reveal.reveal_name);
                xfer->xferAsciiString(&reveal.waypoint_name);
                xfer->xferReal(&reveal.radius);
                xfer->xferAsciiString(&reveal.player);
                m_namedReveals.push_back(reveal);
            }
        }

        unsigned short all_object_type_list_count = static_cast<unsigned short>(m_allObjectTypeLists.size());
        xfer->xferUnsignedShort(&all_object_type_list_count);

        if (xfer->Get_Mode() == XFER_SAVE) {
            for (auto it = m_allObjectTypeLists.begin(); it != m_allObjectTypeLists.end(); it++) {
                xfer->xferSnapshot(*it);
            }
        } else {
            captainslog_relassert(
                m_allObjectTypeLists.empty(), 6, "ScriptEngine::xfer - m_allObjectTypeLists should be empty but is not!");

            for (unsigned short j = 0; j < all_object_type_list_count; j++) {
                ObjectTypes *types = new ObjectTypes();
                xfer->xferSnapshot(types);
                m_allObjectTypeLists.push_back(types);
            }
        }
    }

    if (version < 3) {
        m_useObjectDifficultyBonuses = true;
    } else {
        xfer->xferBool(&m_useObjectDifficultyBonuses);
    }

    if (version >= 4) {
        xfer->xferAsciiString(&m_currentTrackName);
    }

    if (version < 5) {
        m_chooseVictimAlwaysUsesNormal = false;
    } else {
        xfer->xferBool(&m_chooseVictimAlwaysUsesNormal);
    }

    if (xfer->Get_Mode() == XFER_LOAD && m_fade == FADE_NONE) {
        m_fade = FADE_MULTIPLY;
        m_curFadeFrame = 0;
        m_minFade = 1.0f;
        m_maxFade = 0.0f;
        m_fadeFramesIncrease = 0;
        m_fadeFramesHold = 0;
        m_fadeFramesDecrease = 33;
        m_curFadeValue = 0.0f;
    }
}

void ScriptEngine::Load_Post_Process()
{
    g_theScriptActions->Do_Enable_Or_Disable_Object_Difficulty_Bonuses(m_useObjectDifficultyBonuses);

    if (m_currentTrackName.Is_Not_Empty()) {
        AudioEventRTS audio(m_currentTrackName);
        audio.Set_Player_Index(g_thePlayerList->Get_Local_Player()->Get_Player_Index());
        g_theAudio->Add_Audio_Event(&audio);
    }
}

void ScriptEngine::Debug_Victory()
{
    ScriptAction *action = new ScriptAction(ScriptAction::VICTORY);
    g_theScriptActions->Execute_Action(action);
}

Script *ScriptEngine::Find_Script_By_Name(const Utf8String &script_name)
{
    return Find_Script(script_name);
}

void ScriptEngine::Append_Message(const Utf8String &str, bool is_true_message, bool should_pause)
{
#ifdef PLATFORM_WINDOWS
    Utf8String message;
    message.Format("%d ", g_theGameLogic->Get_Frame());

    if (is_true_message) {
        message.Concat("Run script - ");
    } else {
        message.Concat("Run script false -");
    }

    message += str;

    if (s_debugDll != nullptr) {
        void (*AppendMessage)(const char *);

        if (should_pause) {
            AppendMessage = reinterpret_cast<void (*)(const char *)>(GetProcAddress(s_debugDll, "AppendMessageAndPause"));
        } else {
            AppendMessage = reinterpret_cast<void (*)(const char *)>(GetProcAddress(s_debugDll, "AppendMessage"));
        }

        if (AppendMessage != nullptr) {
            AppendMessage(message.Str());
        }
    }
#endif
}

void ScriptEngine::Adjust_Variable(const Utf8String &str, int value, bool should_pause)
{
#ifdef PLATFORM_WINDOWS
    if (s_debugDll != nullptr) {
        void (*AdjustVariable)(const char *, const char *);

        if (should_pause) {
            AdjustVariable =
                reinterpret_cast<void (*)(const char *, const char *)>(GetProcAddress(s_debugDll, "AdjustVariableAndPause"));
        } else {
            AdjustVariable =
                reinterpret_cast<void (*)(const char *, const char *)>(GetProcAddress(s_debugDll, "AdjustVariable"));
        }

        if (AdjustVariable != nullptr) {
            char buf[12];
            sprintf(buf, "%d", value);
            AdjustVariable(str.Str(), buf);
        }
    }
#endif
}

void ScriptEngine::Update_Frame_Number()
{
#ifdef PLATFORM_WINDOWS
    if (!g_theScriptEngine->Is_Time_Fast() && s_debugDll != nullptr) {
        void (*SetFrameNumber)(int) = reinterpret_cast<void (*)(int)>(GetProcAddress(s_debugDll, "SetFrameNumber"));

        if (SetFrameNumber != nullptr) {
            SetFrameNumber(g_theGameLogic->Get_Frame());
        }
    }
#endif
}

void ScriptEngine::Append_All_Particle_Systems()
{
#ifdef PLATFORM_WINDOWS
    if (s_particleDll != nullptr) {
        void (*RemoveAllParticleSystems)(void) =
            reinterpret_cast<void (*)(void)>(GetProcAddress(s_particleDll, "RemoveAllParticleSystems"));

        if (RemoveAllParticleSystems != nullptr) {
            RemoveAllParticleSystems();
            void (*AppendParticleSystem)(const char *) =
                reinterpret_cast<void (*)(const char *)>(GetProcAddress(s_particleDll, "AppendParticleSystem"));

            if (AppendParticleSystem != nullptr) {
                auto start = g_theParticleSystemManager->Get_Starting_Template();
                auto end = g_theParticleSystemManager->Get_Ending_Template();

                while (start != end) {
                    AppendParticleSystem((*start).first.Str());
                    start++;
                }
            }
        }
    }
#endif
}

void ScriptEngine::Append_All_Thing_Templates()
{
#ifdef PLATFORM_WINDOWS
    if (s_particleDll != nullptr) {
        void (*RemoveAllThingTemplates)(void) =
            reinterpret_cast<void (*)(void)>(GetProcAddress(s_particleDll, "RemoveAllThingTemplates"));

        if (RemoveAllThingTemplates != nullptr) {
            RemoveAllThingTemplates();
            void (*AppendThingTemplate)(const char *) =
                reinterpret_cast<void (*)(const char *)>(GetProcAddress(s_particleDll, "AppendThingTemplate"));

            if (AppendThingTemplate != nullptr) {
                for (ThingTemplate *thing = g_theThingFactory->First_Template(); thing != nullptr;
                     thing = thing->Friend_Get_Next_Template()) {
                    AppendThingTemplate(thing->Get_Name().Str());
                }
            }
        }
    }
#endif
}

void ScriptEngine::Add_Updated_Particle_System(Utf8String particle_system_name)
{
#ifdef PLATFORM_WINDOWS
    void (*AppendParticleSystem)(const char *);
    void (*UpdateSystemUseParameters)(void *);
    ParticleSystemTemplate *tmplate;

    if (s_particleDll == nullptr
        || (AppendParticleSystem =
                   reinterpret_cast<void (*)(const char *)>(GetProcAddress(s_particleDll, "AppendParticleSystem")))
            == nullptr
        || (UpdateSystemUseParameters =
                   reinterpret_cast<void (*)(void *)>(GetProcAddress(s_particleDll, "UpdateSystemUseParameters")))
            == nullptr
        || (tmplate = g_theParticleSystemManager->New_Template(particle_system_name)) == 0) {
        return;
    } else {
        AppendParticleSystem(tmplate->Get_Name().Str());
        UpdateSystemUseParameters(tmplate);
    }
#endif
}

Utf8String ScriptEngine::Get_Particle_System_Name()
{
#ifdef PLATFORM_WINDOWS
    if (s_particleDll == nullptr) {
        return Utf8String::s_emptyString;
    } else {
        void (*GetSelectedParticleSystemName)(char *) =
            reinterpret_cast<void (*)(char *)>(GetProcAddress(s_particleDll, "GetSelectedParticleSystemName"));

        if (GetSelectedParticleSystemName != nullptr) {
            static char part_sys_name[1024];
            GetSelectedParticleSystemName(part_sys_name);
            return part_sys_name;
        } else {
            return Utf8String::s_emptyString;
        }
    }
#else
    return Utf8String::s_emptyString;
#endif
}

void ScriptEngine::Update_AsciiString_Parms_To_System(ParticleSystemTemplate *particle_template)
{
#ifdef PLATFORM_WINDOWS
    if (s_particleDll != nullptr && particle_template != nullptr) {
        void (*GetSelectedParticleAsciiStringParm)(int32_t, char *, ParticleSystemTemplate **) =
            reinterpret_cast<void (*)(int32_t, char *, ParticleSystemTemplate **)>(
                GetProcAddress(s_particleDll, "GetSelectedParticleAsciiStringParm"));

        char name[128];
        ParticleSystemTemplate *templ;

        if (GetSelectedParticleAsciiStringParm != nullptr) {
            GetSelectedParticleAsciiStringParm(0, name, &templ);

            if (templ == particle_template) {
                particle_template->m_particleTypeName.Set(name);
            }

            GetSelectedParticleAsciiStringParm(1, name, &templ);

            if (templ == particle_template) {
                particle_template->m_slaveSystemName.Set(name);
            }

            GetSelectedParticleAsciiStringParm(2, name, &templ);

            if (templ == particle_template) {
                particle_template->m_attachedSystemName.Set(name);
            }
        }
    }
#endif
}

void ScriptEngine::Update_AsciiString_Parms_From_System(ParticleSystemTemplate *particle_template)
{
#ifdef PLATFORM_WINDOWS
    if (s_particleDll != nullptr && particle_template != nullptr) {
        void (*UpdateParticleAsciiStringParm)(int32_t, const char *, void **) =
            reinterpret_cast<void (*)(int32_t, const char *, void **)>(
                GetProcAddress(s_particleDll, "UpdateParticleAsciiStringParm"));

        if (UpdateParticleAsciiStringParm != nullptr) {
            UpdateParticleAsciiStringParm(0, particle_template->m_particleTypeName.Str(), nullptr);
            UpdateParticleAsciiStringParm(1, particle_template->m_slaveSystemName.Str(), nullptr);
            UpdateParticleAsciiStringParm(2, particle_template->m_attachedSystemName.Str(), nullptr);
        }
    }
#endif
}

void ScriptEngine::Write_Out_INI()
{
    char fname[128];
    File *backup_file = nullptr;
    int count = 0;

    do {
        if (backup_file != nullptr) {
            backup_file->Close();
            backup_file = nullptr;
        }

        sprintf(fname, "%s%d.%s", "Data\\INI\\ParticleSystem", count, "BAK");
        backup_file = g_theFileSystem->Open_File(fname, File::READ | File::BINARY);
        count++;
    } while (backup_file != nullptr);

    backup_file = g_theFileSystem->Open_File(fname, File::WRITE | File::BINARY);

    if (backup_file != nullptr) {
        File *old_file = g_theFileSystem->Open_File("Data\\INI\\ParticleSystem.ini", File::READ | File::BINARY);

        if (old_file != nullptr) {
            for (;;) {
                if (old_file->Position() == old_file->Size()) {
                    break;
                }

                char c;
                old_file->Read(&c, sizeof(c));
                backup_file->Write(&c, sizeof(c));
            }

            old_file->Close();
            old_file = nullptr;
            backup_file->Close();
            backup_file = nullptr;
        }

        File *new_file = g_theFileSystem->Open_File("Data\\INI\\ParticleSystem.ini", File::WRITE | File::BINARY);

        if (new_file != nullptr) {
            auto start = g_theParticleSystemManager->Get_Starting_Template();
            auto end = g_theParticleSystemManager->Get_Ending_Template();

            while (start != end) {
                Write_Single_Particle_System(new_file, (*start).second);
                start++;
            }

            new_file->Close();
        } else {
            captainslog_dbgassert(false, "Unable to open ParticleSystem.ini. Is it write protected?");
        }
    }
}

const std::string HEADER("ParticleSystem");
const std::string SEP_SPACE("");
const std::string SEP_HEAD("  ");
const std::string SEP_EOL("\n");
const std::string SEP_TAB("\t");
const std::string STR_TRUE("Yes");
const std::string STR_FALSE("No");
const std::string EQ_WITH_SPACES(" = ");
const std::string STR_R("R:");
const std::string STR_G("G:");
const std::string STR_B("B:");
const std::string STR_X("X:");
const std::string STR_Y("Y:");
const std::string STR_Z("Z:");
const std::string STR_END("End");
const std::string F_PRIORITY("Priority");
const std::string F_ISONESHOT("IsOneShot");
const std::string F_SHADER("Shader");
const std::string F_TYPE("Type");
const std::string F_PARTICLENAME("ParticleName");
const std::string F_ANGLEX("AngleX");
const std::string F_ANGLEY("AngleY");
const std::string F_ANGLEZ("AngleZ");
const std::string F_ANGLERATEX("AngularRateX");
const std::string F_ANGLERATEY("AngularRateY");
const std::string F_ANGLERATEZ("AngularRateZ");
const std::string F_ANGLEDAMP("AngularDamping");
const std::string F_VELOCITYDAMP("VelocityDamping");
const std::string F_GRAVITY("Gravity");
const std::string F_SLAVESYSTEM("SlaveSystem");
const std::string F_SLAVEPOS("SlavePosOffset");
const std::string F_ATTACHED("PerParticleAttachedSystem");
const std::string F_LIFETIME("Lifetime");
const std::string F_SYSLIFETIME("SystemLifetime");
const std::string F_SIZE("Size");
const std::string F_STARTSIZERATE("StartSizeRate");
const std::string F_SIZERATE("SizeRate");
const std::string F_SIZERATEDAMP("SizeRateDamping");
const std::string F_ALPHA1("Alpha1");
const std::string F_ALPHA2("Alpha2");
const std::string F_ALPHA3("Alpha3");
const std::string F_ALPHA4("Alpha4");
const std::string F_ALPHA5("Alpha5");
const std::string F_ALPHA6("Alpha6");
const std::string F_ALPHA7("Alpha7");
const std::string F_ALPHA8("Alpha8");
const std::string F_COLOR1("Color1");
const std::string F_COLOR2("Color2");
const std::string F_COLOR3("Color3");
const std::string F_COLOR4("Color4");
const std::string F_COLOR5("Color5");
const std::string F_COLOR6("Color6");
const std::string F_COLOR7("Color7");
const std::string F_COLOR8("Color8");
const std::string F_COLORSCALE("ColorScale");
const std::string F_BURSTDELAY("BurstDelay");
const std::string F_BURSTCOUNT("BurstCount");
const std::string F_INITIALDELAY("InitialDelay");
const std::string F_DRIFTVELOCITY("DriftVelocity");
const std::string F_VELOCITYTYPE("VelocityType");
const std::string F_VELORTHOX("VelOrthoX");
const std::string F_VELORTHOY("VelOrthoY");
const std::string F_VELORTHOZ("VelOrthoZ");
const std::string F_VELSPHERE("VelSpherical");
const std::string F_HEMISPHERE("VelHemispherical");
const std::string F_VELCYLRAD("VelCylindricalRadial");
const std::string F_VELCYLNOR("VelCylindricalNormal");
const std::string F_VELOUTWARD("VelOutward");
const std::string F_VELOUTOTHER("VelOutwardOther");
const std::string F_VOLUMETYPE("VolumeType");
const std::string F_VOLLINESTART("VolLineStart");
const std::string F_VOLLINEEND("VolLineEnd");
const std::string F_VOLBOXHALF("VolBoxHalfSize");
const std::string F_VOLSPHERERAD("VolSphereRadius");
const std::string F_VOLCYLRAD("VolCylinderRadius");
const std::string F_VOLCYLLEN("VolCylinderLength");
const std::string F_ISHOLLOW("IsHollow");
const std::string F_ISXYPLANAR("IsGroundAligned");
const std::string F_ISEMITABOVEGROUNDONLY("IsEmitAboveGroundOnly");
const std::string F_ISPARTICLEUPTOWARDSEMITTER("IsParticleUpTowardsEmitter");
const std::string F_WINDMOTION("WindMotion");
const std::string F_WINDANGLECHANGEMIN("WindAngleChangeMin");
const std::string F_WINDANGLECHANGEMAX("WindAngleChangeMax");
const std::string F_WINDPINGPONGSTARTANGLEMIN("WindPingPongStartAngleMin");
const std::string F_WINDPINGPONGSTARTANGLEMAX("WindPingPongStartAngleMax");
const std::string F_WINDPINGPONGENDANGLEMIN("WindPingPongEndAngleMin");
const std::string F_WINDPINGPONGENDANGLEMAX("WindPingPongEndAngleMax");

constexpr const char *ParticlePriorityNames[PARTICLE_PRIORITY_COUNT + 1] = {
    "NONE",
    "WEAPON_EXPLOSION",
    "SCORCHMARK",
    "DUST_TRAIL",
    "BUILDUP",
    "DEBRIS_TRAIL",
    "UNIT_DAMAGE_FX",
    "DEATH_EXPLOSION",
    "SEMI_CONSTANT",
    "CONSTANT",
    "WEAPON_TRAIL",
    "AREA_EFFECT",
    "CRITICAL",
    "ALWAYS_RENDER",
    nullptr,
};

constexpr const char *ParticleShaderTypeNames[] = {
    "NONE",
    "ADDITIVE",
    "ALPHA",
    "ALPHA_TEST",
    "MULTIPLY",
    nullptr,
};

constexpr const char *ParticleTypeNames[] = {
    "NONE",
    "PARTICLE",
    "DRAWABLE",
    "STREAK",
    "VOLUME_PARTICLE",
    "SMUDGE",
    nullptr,
};

constexpr const char *EmissionVelocityTypeNames[] = {
    "NONE",
    "ORTHO",
    "SPHERICAL",
    "HEMISPHERICAL",
    "CYLINDRICAL",
    "OUTWARD",
    nullptr,
};

constexpr const char *EmissionVolumeTypeNames[] = {
    "NONE",
    "POINT",
    "LINE",
    "BOX",
    "SPHERE",
    "CYLINDER",
    nullptr,
};

constexpr const char *WindMotionTypeNames[] = {
    "NONE",
    "Unused",
    "PingPong",
    "Circular",
    nullptr,
};

void ScriptEngine::Write_Single_Particle_System(File *out, ParticleSystemTemplate *templ)
{
    if (templ != nullptr) {
        if (out != nullptr) {
            if (!templ->Get_Name().Is_Empty()) {
                std::string str;
                char buf1[128];
                char buf2[128];
                char buf3[128];
                char buf4[128];

                str += HEADER;
                str += SEP_SPACE;
                str += templ->Get_Name().Str();
                str += SEP_EOL;
                str += SEP_HEAD;
                str += F_PRIORITY;
                str += EQ_WITH_SPACES;
                str += ParticlePriorityNames[templ->m_priority];
                str += SEP_EOL;

                str += SEP_HEAD;
                str += F_ISONESHOT;
                str += EQ_WITH_SPACES;

                if (templ->m_isOneShot) {
                    str += STR_TRUE;
                } else {
                    str += STR_FALSE;
                }

                str += SEP_EOL;

                str += SEP_HEAD;
                str += F_SHADER;
                str += EQ_WITH_SPACES;
                str += ParticleShaderTypeNames[templ->m_shaderType];
                str += SEP_EOL;

                str += SEP_HEAD;
                str += F_TYPE;
                str += EQ_WITH_SPACES;
                str += ParticleTypeNames[templ->m_particleType];
                str += SEP_EOL;

                str += SEP_HEAD;
                str += F_PARTICLENAME;
                str += EQ_WITH_SPACES;
                str += templ->m_particleTypeName.Str();
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_angleZ.Get_Min());
                sprintf(buf2, "%.2f", templ->m_angleZ.Get_Max());
                str += SEP_HEAD;
                str += F_ANGLEZ;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_angularRateZ.Get_Min());
                sprintf(buf2, "%.2f", templ->m_angularRateZ.Get_Max());
                str += SEP_HEAD;
                str += F_ANGLERATEZ;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_angularDamping.Get_Min());
                sprintf(buf2, "%.2f", templ->m_angularDamping.Get_Max());
                str += SEP_HEAD;
                str += F_ANGLEDAMP;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_velDamping.Get_Min());
                sprintf(buf2, "%.2f", templ->m_velDamping.Get_Max());
                str += SEP_HEAD;
                str += F_VELOCITYDAMP;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_gravity);
                str += SEP_HEAD;
                str += F_GRAVITY;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_EOL;

                if (!templ->m_slaveSystemName.Is_Empty()) {
                    str += SEP_HEAD;
                    str += F_SLAVESYSTEM;
                    str += EQ_WITH_SPACES;
                    str += templ->m_slaveSystemName.Str();
                    str += SEP_EOL;

                    sprintf(buf1, "%s%.2f", STR_X.c_str(), templ->m_slavePosOffset.x);
                    sprintf(buf2, "%s%.2f", STR_Y.c_str(), templ->m_slavePosOffset.y);
                    sprintf(buf3, "%s%.2f", STR_Z.c_str(), templ->m_slavePosOffset.z);
                    str += SEP_HEAD;
                    str += F_SLAVEPOS;
                    str += EQ_WITH_SPACES;
                    str += buf1;
                    str += SEP_SPACE;
                    str += buf2;
                    str += SEP_SPACE;
                    str += buf3;
                    str += SEP_EOL;
                }

                if (!templ->m_attachedSystemName.Is_Empty()) {
                    str += SEP_HEAD;
                    str += F_ATTACHED;
                    str += EQ_WITH_SPACES;
                    str += templ->m_attachedSystemName.Str();
                    str += SEP_EOL;
                }

                sprintf(buf1, "%.2f", templ->m_lifetime.Get_Min());
                sprintf(buf2, "%.2f", templ->m_lifetime.Get_Max());
                str += SEP_HEAD;
                str += F_LIFETIME;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_EOL;

                sprintf(buf1, "%d", templ->m_systemLifetime);
                str += SEP_HEAD;
                str += F_SYSLIFETIME;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_startSize.Get_Min());
                sprintf(buf2, "%.2f", templ->m_startSize.Get_Max());
                str += SEP_HEAD;
                str += F_SIZE;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_startSizeRate.Get_Min());
                sprintf(buf2, "%.2f", templ->m_startSizeRate.Get_Max());
                str += SEP_HEAD;
                str += F_STARTSIZERATE;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_sizeRate.Get_Min());
                sprintf(buf2, "%.2f", templ->m_sizeRate.Get_Max());
                str += SEP_HEAD;
                str += F_SIZERATE;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_sizeRateDamping.Get_Min());
                sprintf(buf2, "%.2f", templ->m_sizeRateDamping.Get_Max());
                str += SEP_HEAD;
                str += F_SIZERATEDAMP;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_alphaKey[0].var.Get_Min());
                sprintf(buf2, "%.2f", templ->m_alphaKey[0].var.Get_Max());
                sprintf(buf3, "%d", templ->m_alphaKey[0].frame);
                str += SEP_HEAD;
                str += F_ALPHA1;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_SPACE;
                str += buf3;
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_alphaKey[1].var.Get_Min());
                sprintf(buf2, "%.2f", templ->m_alphaKey[1].var.Get_Max());
                sprintf(buf3, "%d", templ->m_alphaKey[1].frame);
                str += SEP_HEAD;
                str += F_ALPHA2;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_SPACE;
                str += buf3;
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_alphaKey[2].var.Get_Min());
                sprintf(buf2, "%.2f", templ->m_alphaKey[2].var.Get_Max());
                sprintf(buf3, "%d", templ->m_alphaKey[2].frame);
                str += SEP_HEAD;
                str += F_ALPHA3;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_SPACE;
                str += buf3;
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_alphaKey[3].var.Get_Min());
                sprintf(buf2, "%.2f", templ->m_alphaKey[3].var.Get_Max());
                sprintf(buf3, "%d", templ->m_alphaKey[3].frame);
                str += SEP_HEAD;
                str += F_ALPHA4;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_SPACE;
                str += buf3;
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_alphaKey[4].var.Get_Min());
                sprintf(buf2, "%.2f", templ->m_alphaKey[4].var.Get_Max());
                sprintf(buf3, "%d", templ->m_alphaKey[4].frame);
                str += SEP_HEAD;
                str += F_ALPHA5;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_SPACE;
                str += buf3;
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_alphaKey[5].var.Get_Min());
                sprintf(buf2, "%.2f", templ->m_alphaKey[5].var.Get_Max());
                sprintf(buf3, "%d", templ->m_alphaKey[5].frame);
                str += SEP_HEAD;
                str += F_ALPHA6;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_SPACE;
                str += buf3;
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_alphaKey[6].var.Get_Min());
                sprintf(buf2, "%.2f", templ->m_alphaKey[6].var.Get_Max());
                sprintf(buf3, "%d", templ->m_alphaKey[6].frame);
                str += SEP_HEAD;
                str += F_ALPHA7;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_SPACE;
                str += buf3;
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_alphaKey[7].var.Get_Min());
                sprintf(buf2, "%.2f", templ->m_alphaKey[7].var.Get_Max());
                sprintf(buf3, "%d", templ->m_alphaKey[7].frame);
                str += SEP_HEAD;
                str += F_ALPHA8;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_SPACE;
                str += buf3;
                str += SEP_EOL;

                sprintf(buf1,
                    "%s%d",
                    STR_R.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[0].color.red * 255.0f + 0.5f));
                sprintf(buf2,
                    "%s%d",
                    STR_G.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[0].color.green * 255.0f + 0.5f));
                sprintf(buf3,
                    "%s%d",
                    STR_B.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[0].color.blue * 255.0f + 0.5f));
                sprintf(buf4, "%d", templ->m_colorKey[0].frame);
                str += SEP_HEAD;
                str += F_COLOR1;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_SPACE;
                str += buf3;
                str += SEP_SPACE;
                str += buf4;
                str += SEP_EOL;

                sprintf(buf1,
                    "%s%d",
                    STR_R.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[1].color.red * 255.0f + 0.5f));
                sprintf(buf2,
                    "%s%d",
                    STR_G.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[1].color.green * 255.0f + 0.5f));
                sprintf(buf3,
                    "%s%d",
                    STR_B.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[1].color.blue * 255.0f + 0.5f));
                sprintf(buf4, "%d", templ->m_colorKey[1].frame);
                str += SEP_HEAD;
                str += F_COLOR2;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_SPACE;
                str += buf3;
                str += SEP_SPACE;
                str += buf4;
                str += SEP_EOL;

                sprintf(buf1,
                    "%s%d",
                    STR_R.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[2].color.red * 255.0f + 0.5f));
                sprintf(buf2,
                    "%s%d",
                    STR_G.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[2].color.green * 255.0f + 0.5f));
                sprintf(buf3,
                    "%s%d",
                    STR_B.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[2].color.blue * 255.0f + 0.5f));
                sprintf(buf4, "%d", templ->m_colorKey[2].frame);
                str += SEP_HEAD;
                str += F_COLOR3;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_SPACE;
                str += buf3;
                str += SEP_SPACE;
                str += buf4;
                str += SEP_EOL;

                sprintf(buf1,
                    "%s%d",
                    STR_R.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[3].color.red * 255.0f + 0.5f));
                sprintf(buf2,
                    "%s%d",
                    STR_G.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[3].color.green * 255.0f + 0.5f));
                sprintf(buf3,
                    "%s%d",
                    STR_B.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[3].color.blue * 255.0f + 0.5f));
                sprintf(buf4, "%d", templ->m_colorKey[3].frame);
                str += SEP_HEAD;
                str += F_COLOR4;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_SPACE;
                str += buf3;
                str += SEP_SPACE;
                str += buf4;
                str += SEP_EOL;

                sprintf(buf1,
                    "%s%d",
                    STR_R.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[4].color.red * 255.0f + 0.5f));
                sprintf(buf2,
                    "%s%d",
                    STR_G.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[4].color.green * 255.0f + 0.5f));
                sprintf(buf3,
                    "%s%d",
                    STR_B.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[4].color.blue * 255.0f + 0.5f));
                sprintf(buf4, "%d", templ->m_colorKey[4].frame);
                str += SEP_HEAD;
                str += F_COLOR5;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_SPACE;
                str += buf3;
                str += SEP_SPACE;
                str += buf4;
                str += SEP_EOL;

                sprintf(buf1,
                    "%s%d",
                    STR_R.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[5].color.red * 255.0f + 0.5f));
                sprintf(buf2,
                    "%s%d",
                    STR_G.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[5].color.green * 255.0f + 0.5f));
                sprintf(buf3,
                    "%s%d",
                    STR_B.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[5].color.blue * 255.0f + 0.5f));
                sprintf(buf4, "%d", templ->m_colorKey[5].frame);
                str += SEP_HEAD;
                str += F_COLOR6;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_SPACE;
                str += buf3;
                str += SEP_SPACE;
                str += buf4;
                str += SEP_EOL;

                sprintf(buf1,
                    "%s%d",
                    STR_R.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[6].color.red * 255.0f + 0.5f));
                sprintf(buf2,
                    "%s%d",
                    STR_G.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[6].color.green * 255.0f + 0.5f));
                sprintf(buf3,
                    "%s%d",
                    STR_B.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[6].color.blue * 255.0f + 0.5f));
                sprintf(buf4, "%d", templ->m_colorKey[6].frame);
                str += SEP_HEAD;
                str += F_COLOR7;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_SPACE;
                str += buf3;
                str += SEP_SPACE;
                str += buf4;
                str += SEP_EOL;

                sprintf(buf1,
                    "%s%d",
                    STR_R.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[7].color.red * 255.0f + 0.5f));
                sprintf(buf2,
                    "%s%d",
                    STR_G.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[7].color.green * 255.0f + 0.5f));
                sprintf(buf3,
                    "%s%d",
                    STR_B.c_str(),
                    GameMath::Fast_To_Int_Truncate(templ->m_colorKey[7].color.blue * 255.0f + 0.5f));
                sprintf(buf4, "%d", templ->m_colorKey[7].frame);
                str += SEP_HEAD;
                str += F_COLOR8;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_SPACE;
                str += buf3;
                str += SEP_SPACE;
                str += buf4;
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_colorScale.Get_Min());
                sprintf(buf2, "%.2f", templ->m_colorScale.Get_Max());
                str += SEP_HEAD;
                str += F_COLORSCALE;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_burstDelay.Get_Min());
                sprintf(buf2, "%.2f", templ->m_burstDelay.Get_Max());
                str += SEP_HEAD;
                str += F_BURSTDELAY;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_burstCount.Get_Min());
                sprintf(buf2, "%.2f", templ->m_burstCount.Get_Max());
                str += SEP_HEAD;
                str += F_BURSTCOUNT;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_EOL;

                sprintf(buf1, "%.2f", templ->m_initialDelay.Get_Min());
                sprintf(buf2, "%.2f", templ->m_initialDelay.Get_Max());
                str += SEP_HEAD;
                str += F_INITIALDELAY;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_EOL;

                sprintf(buf1, "%s%.2f", STR_X.c_str(), templ->m_driftVelocity.x);
                sprintf(buf2, "%s%.2f", STR_Y.c_str(), templ->m_driftVelocity.y);
                sprintf(buf3, "%s%.2f", STR_Z.c_str(), templ->m_driftVelocity.z);
                str += SEP_HEAD;
                str += F_DRIFTVELOCITY;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_SPACE;
                str += buf2;
                str += SEP_SPACE;
                str += buf3;
                str += SEP_EOL;

                str += SEP_HEAD;
                str += F_VELOCITYTYPE;
                str += EQ_WITH_SPACES;
                str += EmissionVelocityTypeNames[templ->m_emissionVelocityType];
                str += SEP_EOL;

                switch (templ->m_emissionVelocityType) {
                    case ParticleSystemInfo::EMISSION_VELOCITY_ORTHO:
                        sprintf(buf1, "%.2f", templ->m_emissionVelocity.ortho.x.Get_Min());
                        sprintf(buf2, "%.2f", templ->m_emissionVelocity.ortho.x.Get_Max());
                        str += SEP_HEAD;
                        str += F_VELORTHOX;
                        str += EQ_WITH_SPACES;
                        str += buf1;
                        str += SEP_SPACE;
                        str += buf2;
                        str += SEP_EOL;

                        sprintf(buf1, "%.2f", templ->m_emissionVelocity.ortho.y.Get_Min());
                        sprintf(buf2, "%.2f", templ->m_emissionVelocity.ortho.y.Get_Max());
                        str += SEP_HEAD;
                        str += F_VELORTHOY;
                        str += EQ_WITH_SPACES;
                        str += buf1;
                        str += SEP_SPACE;
                        str += buf2;
                        str += SEP_EOL;

                        sprintf(buf1, "%.2f", templ->m_emissionVelocity.ortho.z.Get_Min());
                        sprintf(buf2, "%.2f", templ->m_emissionVelocity.ortho.z.Get_Max());
                        str += SEP_HEAD;
                        str += F_VELORTHOZ;
                        str += EQ_WITH_SPACES;
                        str += buf1;
                        str += SEP_SPACE;
                        str += buf2;
                        str += SEP_EOL;
                        break;
                    case ParticleSystemInfo::EMISSION_VELOCITY_SPHERICAL:
                        sprintf(buf1, "%.2f", templ->m_emissionVelocity.spherical.Get_Min());
                        sprintf(buf2, "%.2f", templ->m_emissionVelocity.spherical.Get_Max());
                        str += SEP_HEAD;
                        str += F_VELSPHERE;
                        str += EQ_WITH_SPACES;
                        str += buf1;
                        str += SEP_SPACE;
                        str += buf2;
                        str += SEP_EOL;
                        break;
                    case ParticleSystemInfo::EMISSION_VELOCITY_HEMISPHERICAL:
                        sprintf(buf1, "%.2f", templ->m_emissionVelocity.hemispherical.Get_Min());
                        sprintf(buf2, "%.2f", templ->m_emissionVelocity.hemispherical.Get_Max());
                        str += SEP_HEAD;
                        str += F_HEMISPHERE;
                        str += EQ_WITH_SPACES;
                        str += buf1;
                        str += SEP_SPACE;
                        str += buf2;
                        str += SEP_EOL;
                        break;
                    case ParticleSystemInfo::EMISSION_VELOCITY_CYLINDRICAL:
                        sprintf(buf1, "%.2f", templ->m_emissionVelocity.cylindrical.radial.Get_Min());
                        sprintf(buf2, "%.2f", templ->m_emissionVelocity.cylindrical.radial.Get_Max());
                        str += SEP_HEAD;
                        str += F_VELCYLRAD;
                        str += EQ_WITH_SPACES;
                        str += buf1;
                        str += SEP_SPACE;
                        str += buf2;
                        str += SEP_EOL;

                        sprintf(buf1, "%.2f", templ->m_emissionVelocity.cylindrical.normal.Get_Min());
                        sprintf(buf2, "%.2f", templ->m_emissionVelocity.cylindrical.normal.Get_Max());
                        str += SEP_HEAD;
                        str += F_VELCYLNOR;
                        str += EQ_WITH_SPACES;
                        str += buf1;
                        str += SEP_SPACE;
                        str += buf2;
                        str += SEP_EOL;
                        break;
                    case ParticleSystemInfo::EMISSION_VELOCITY_OUTWARD:
                        sprintf(buf1, "%.2f", templ->m_emissionVelocity.outward.outward.Get_Min());
                        sprintf(buf2, "%.2f", templ->m_emissionVelocity.outward.outward.Get_Max());
                        str += SEP_HEAD;
                        str += F_VELOUTWARD;
                        str += EQ_WITH_SPACES;
                        str += buf1;
                        str += SEP_SPACE;
                        str += buf2;
                        str += SEP_EOL;

                        sprintf(buf1, "%.2f", templ->m_emissionVelocity.outward.other.Get_Min());
                        sprintf(buf2, "%.2f", templ->m_emissionVelocity.outward.other.Get_Max());
                        str += SEP_HEAD;
                        str += F_VELOUTOTHER;
                        str += EQ_WITH_SPACES;
                        str += buf1;
                        str += SEP_SPACE;
                        str += buf2;
                        str += SEP_EOL;
                        break;
                }

                str += SEP_HEAD;
                str += F_VOLUMETYPE;
                str += EQ_WITH_SPACES;
                str += EmissionVolumeTypeNames[templ->m_emissionVolumeType];
                str += SEP_EOL;

                if (templ->m_emissionVolumeType != ParticleSystemInfo::EMISSION_VOLUME_POINT) {
                    switch (templ->m_emissionVolumeType) {
                        case ParticleSystemInfo::EMISSION_VOLUME_LINE:
                            sprintf(buf1, "%s%.2f", STR_X.c_str(), templ->m_emissionVolume.line.start.x);
                            sprintf(buf2, "%s%.2f", STR_Y.c_str(), templ->m_emissionVolume.line.start.y);
                            sprintf(buf3, "%s%.2f", STR_Z.c_str(), templ->m_emissionVolume.line.start.z);
                            str += SEP_HEAD;
                            str += F_VOLLINESTART;
                            str += EQ_WITH_SPACES;
                            str += buf1;
                            str += SEP_SPACE;
                            str += buf2;
                            str += SEP_SPACE;
                            str += buf3;
                            str += SEP_EOL;

                            sprintf(buf1, "%s%.2f", STR_X.c_str(), templ->m_emissionVolume.line.end.x);
                            sprintf(buf2, "%s%.2f", STR_Y.c_str(), templ->m_emissionVolume.line.end.y);
                            sprintf(buf3, "%s%.2f", STR_Z.c_str(), templ->m_emissionVolume.line.end.z);
                            str += SEP_HEAD;
                            str += F_VOLLINEEND;
                            str += EQ_WITH_SPACES;
                            str += buf1;
                            str += SEP_SPACE;
                            str += buf2;
                            str += SEP_SPACE;
                            str += buf3;
                            str += SEP_EOL;
                            break;
                        case ParticleSystemInfo::EMISSION_VOLUME_BOX:
                            sprintf(buf1, "%s%.2f", STR_X.c_str(), templ->m_emissionVolume.box.x);
                            sprintf(buf2, "%s%.2f", STR_Y.c_str(), templ->m_emissionVolume.box.y);
                            sprintf(buf3, "%s%.2f", STR_Z.c_str(), templ->m_emissionVolume.box.z);
                            str += SEP_HEAD;
                            str += F_VOLBOXHALF;
                            str += EQ_WITH_SPACES;
                            str += buf1;
                            str += SEP_SPACE;
                            str += buf2;
                            str += SEP_SPACE;
                            str += buf3;
                            str += SEP_EOL;
                            break;
                        case ParticleSystemInfo::EMISSION_VOLUME_SPHERE:
                            sprintf(buf1, "%.2f", templ->m_emissionVolume.sphere);
                            str += SEP_HEAD;
                            str += F_VOLSPHERERAD;
                            str += EQ_WITH_SPACES;
                            str += buf1;
                            str += SEP_EOL;
                            break;
                        case ParticleSystemInfo::EMISSION_VOLUME_CYLINDER:
                            sprintf(buf1, "%.2f", templ->m_emissionVolume.cylinder.radius);
                            str += SEP_HEAD;
                            str += F_VOLCYLRAD;
                            str += EQ_WITH_SPACES;
                            str += buf1;
                            str += SEP_EOL;
                            sprintf(buf1, "%.2f", templ->m_emissionVolume.cylinder.length);
                            str += SEP_HEAD;
                            str += F_VOLCYLLEN;
                            str += EQ_WITH_SPACES;
                            str += buf1;
                            str += SEP_EOL;
                            break;
                    }
                }

                str += SEP_HEAD;
                str += F_ISHOLLOW;
                str += EQ_WITH_SPACES;

                if (templ->m_isEmissionVolumeHollow) {
                    str += STR_TRUE;
                } else {
                    str += STR_FALSE;
                }

                str += SEP_EOL;

                str += SEP_HEAD;
                str += F_ISXYPLANAR;
                str += EQ_WITH_SPACES;

                if (templ->m_isGroundAligned) {
                    str += STR_TRUE;
                } else {
                    str += STR_FALSE;
                }

                str += SEP_EOL;

                str += SEP_HEAD;
                str += F_ISEMITABOVEGROUNDONLY;
                str += EQ_WITH_SPACES;

                if (templ->m_isEmitAboveGroundOnly) {
                    str += STR_TRUE;
                } else {
                    str += STR_FALSE;
                }

                str += SEP_EOL;

                str += SEP_HEAD;
                str += F_ISPARTICLEUPTOWARDSEMITTER;
                str += EQ_WITH_SPACES;

                if (templ->m_isParticleUpTowardsEmitter) {
                    str += STR_TRUE;
                } else {
                    str += STR_FALSE;
                }

                str += SEP_EOL;

                str += SEP_HEAD;
                str += F_WINDMOTION;
                str += EQ_WITH_SPACES;
                str += WindMotionTypeNames[templ->m_windMotion];
                str += SEP_EOL;

                sprintf(buf1, "%f", templ->m_windAngleChangeMin);
                str += SEP_HEAD;
                str += F_WINDANGLECHANGEMIN;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_EOL;

                sprintf(buf1, "%f", templ->m_windAngleChangeMax);
                str += SEP_HEAD;
                str += F_WINDANGLECHANGEMAX;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_EOL;

                sprintf(buf1, "%f", templ->m_windMotionStartAngleMin);
                str += SEP_HEAD;
                str += F_WINDPINGPONGSTARTANGLEMIN;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_EOL;

                sprintf(buf1, "%f", templ->m_windMotionStartAngleMax);
                str += SEP_HEAD;
                str += F_WINDPINGPONGSTARTANGLEMAX;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_EOL;

                sprintf(buf1, "%f", templ->m_windMotionEndAngleMin);
                str += SEP_HEAD;
                str += F_WINDPINGPONGENDANGLEMIN;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_EOL;

                sprintf(buf1, "%f", templ->m_windMotionEndAngleMax);
                str += SEP_HEAD;
                str += F_WINDPINGPONGENDANGLEMAX;
                str += EQ_WITH_SPACES;
                str += buf1;
                str += SEP_EOL;

                str += STR_END;
                str += SEP_EOL;
                str += SEP_EOL;

                out->Write(str.c_str(), str.size());
            }
        }
    }
}

int ScriptEngine::Get_Editor_Behavior()
{
#ifdef PLATFORM_WINDOWS
    if (s_particleDll == nullptr) {
        return 0;
    }

    int (*NextParticleEditorBehavior)(void) =
        reinterpret_cast<int (*)(void)>(GetProcAddress(s_particleDll, "NextParticleEditorBehavior"));

    if (NextParticleEditorBehavior != nullptr) {
        return NextParticleEditorBehavior();
    } else {
        return 0;
    }
#else
    return 0;
#endif
}

void ScriptEngine::Update_And_Set_Current_System()
{
    Utf8String name = Get_Particle_System_Name();
    Add_Updated_Particle_System(name);
    ParticleSystemTemplate *tmplate = g_theParticleSystemManager->Find_Template(name);

    if (tmplate != nullptr) {
        Update_AsciiString_Parms_To_System(tmplate);
        Update_AsciiString_Parms_From_System(tmplate);
        Update_Panel_Parameters(tmplate);

        if (s_particlesForever) {
            s_particleSystem->Stop();
            s_particleSystem->Destroy();
            s_particlesForever = false;
        }

        s_particleSystem = g_theParticleSystemManager->Create_Particle_System(tmplate, true);

        if (s_particleSystem != nullptr) {
            if (s_particleSystem->Is_Forever()) {
                s_particlesForever = true;
            }

            ParticleSystemTemplate *parent_template =
                g_theParticleSystemManager->Find_Parent_Template(tmplate->Get_Name(), 0);

            if (parent_template != nullptr) {
                ParticleSystem *master_system = g_theParticleSystemManager->Create_Particle_System(parent_template, true);

                if (master_system != nullptr) {
                    ParticleSystem::Merge_Related_Systems(master_system, s_particleSystem, true);
                    master_system->Stop();
                    master_system->Destroy();
                }
            }

            Coord3D pos;
            pos.x = 500.0f;
            pos.y = 500.0f;
            pos.z = g_theTerrainLogic->Get_Ground_Height(pos.x, pos.y, nullptr) + 50.0f;
            s_particleSystem->Set_Position(pos);
        }
    }
}

void ScriptEngine::Update_Panel_Parameters(ParticleSystemTemplate *particle_template)
{
#ifdef PLATFORM_WINDOWS
    if (s_particleDll != nullptr) {
        void (*UpdateCurrentParticleSystem)(ParticleSystemTemplate *) = reinterpret_cast<void (*)(ParticleSystemTemplate *)>(
            GetProcAddress(s_particleDll, "UpdateCurrentParticleSystem"));

        if (UpdateCurrentParticleSystem != nullptr) {
            UpdateCurrentParticleSystem(particle_template);
        }
    }
#endif
}

void ScriptEngine::Reload_Particle_System_From_INI(Utf8String particle_system_name)
{
#ifdef PLATFORM_WINDOWS
    static char inibuffer[1032];
    inibuffer[0] = '\0';
    File *psys_file;
    File *temp_file = nullptr;
    if (s_particleDll != nullptr && !particle_system_name.Is_Empty()
        && (psys_file = g_theFileSystem->Open_File("Data\\INI\\ParticleSystem.ini", File::READ | File::BINARY)) != nullptr) {
        for (;;) {
            while (!psys_file->Eof()) {
                if (INI::Is_Declaration_Of_Type("ParticleSystem", particle_system_name, inibuffer)) {
                    break;
                }

                psys_file->Next_Line(inibuffer, 1028);
            }

            if (psys_file->Eof()) {
                throw 0;
            }

            temp_file = g_theFileSystem->Open_File("temporary.ini", File::WRITE | File::BINARY);

            if (temp_file == nullptr) {
                throw 0;
            }

            while (!psys_file->Eof()) {
                if (INI::Is_End_Of_Block(inibuffer)) {
                    break;
                }

                temp_file->Write(inibuffer, strlen(inibuffer));
                psys_file->Next_Line(inibuffer, 1028);
            }

            if (psys_file->Eof()) {
                throw 0;
            }

            temp_file->Write(inibuffer, strlen(inibuffer));
            temp_file->Close();
            temp_file = nullptr;

            if (s_particlesForever) {
                s_particleSystem->Stop();
                s_particleSystem->Destroy();
                s_particlesForever = false;
            }

            INI ini;
            ini.Load("temporary.ini", INI_LOAD_OVERWRITE, nullptr);
            ParticleSystemTemplate *tmplate = g_theParticleSystemManager->Find_Template(particle_system_name);
            Update_AsciiString_Parms_From_System(tmplate);
            Update_Panel_Parameters(tmplate);
        }
    }
#endif
}

int ScriptEngine::Get_New_Current_Particle_Cap()
{
#ifdef PLATFORM_WINDOWS
    if (s_particleDll == nullptr) {
        return -1;
    }

    int (*GetNewParticleCap)(void) = reinterpret_cast<int (*)(void)>(GetProcAddress(s_particleDll, "GetNewParticleCap"));

    if (GetNewParticleCap != nullptr) {
        return GetNewParticleCap();
    } else {
        return -1;
    }
#else
    return -1;
#endif
}

void ScriptEngine::Update_Current_Particle_Cap()
{
#ifdef PLATFORM_WINDOWS
    if (s_particleDll != nullptr) {
        void (*UpdateCurrentParticleCap)(int) =
            reinterpret_cast<void (*)(int)>(GetProcAddress(s_particleDll, "UpdateCurrentParticleCap"));

        if (UpdateCurrentParticleCap != nullptr) {
            UpdateCurrentParticleCap(g_theWriteableGlobalData->m_maxParticleCount);
        }
    }
#endif
}

void ScriptEngine::Update_Current_Particle_Count()
{
#ifdef PLATFORM_WINDOWS
    if (s_particleDll != nullptr) {
        void (*UpdateCurrentNumParticles)(int) =
            reinterpret_cast<void (*)(int)>(GetProcAddress(s_particleDll, "UpdateCurrentNumParticles"));

        if (UpdateCurrentNumParticles != nullptr) {
            UpdateCurrentNumParticles(g_theParticleSystemManager->Get_Particle_Count());
        }
    }
#endif
}

void ScriptEngine::Reload_Textures()
{
    W3DAssetManager::Get_Instance()->Release_All_Textures();
}
