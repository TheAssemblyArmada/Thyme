/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Script engine object to coordinate script execution.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "scriptengine.h"
#include "globaldata.h"

#ifndef GAME_DLL
ScriptEngine *g_theScriptEngine;
bool ScriptEngine::s_canAppContinue;
int ScriptEngine::s_currentFrame;
int ScriptEngine::s_lastFrame;
void *ScriptEngine::s_debugDll;
void *ScriptEngine::s_particleDll;
#endif

AttackPriorityInfo::~AttackPriorityInfo() {}

void AttackPriorityInfo::Xfer_Snapshot(Xfer *xfer) {}

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
    m_fadeFramesIncrease(0),
    m_fadeFramesHold(0),
    m_fadeFramesDecrease(0),
    m_objectCreationDestructionFrame(0),
    m_gameDifficulty(DIFFICULTY_NORMAL),
    m_freezeByScript(false),
    m_useObjectDifficultyBonuses(true),
    m_unkBool1(false),
    m_hasShowMPLocalDefeatWindow(false)
{
    s_canAppContinue = true;
    s_currentFrame = 0;
    s_lastFrame = 0;
    Set_Global_Difficulty(DIFFICULTY_NORMAL);
}

ScriptEngine::~ScriptEngine()
{
    // Unload the debug dll if we are destroying the script engine.
    if (s_debugDll != nullptr) {
#ifdef PLATFORM_WINDOWS
        void (*DestroyDebugDialog)() = (void (*)(void))GetProcAddress((HMODULE)s_debugDll, "DestroyDebugDialog");
#else
        // TODO
#endif
        if (DestroyDebugDialog != nullptr) {
            DestroyDebugDialog();
        }
#ifdef PLATFORM_WINDOWS
        FreeLibrary((HMODULE)s_debugDll);
#else
        // TODO
#endif
        s_debugDll = nullptr;
    }

    // Unload the particle editor dll if we are destroying the script engine.
    if (s_particleDll != nullptr) {
#ifdef PLATFORM_WINDOWS
        void (*DestroyParticleSystemDialog)() =
            (void (*)(void))GetProcAddress((HMODULE)s_particleDll, "DestroyParticleSystemDialog");
#else
        // TODO
#endif
        if (DestroyParticleSystemDialog != nullptr) {
            DestroyParticleSystemDialog();
        }
#ifdef PLATFORM_WINDOWS
        FreeLibrary((HMODULE)s_particleDll);
#else
        // TODO
#endif
        s_particleDll = nullptr;
    }
}

void ScriptEngine::Init() {}

void ScriptEngine::Reset() {}

void ScriptEngine::Update() {}

void ScriptEngine::Xfer_Snapshot(Xfer *xfer) {}

void ScriptEngine::Load_Post_Process() {}

void ScriptEngine::New_Map() {}

ActionTemplate *ScriptEngine::Get_Action_Template(int index)
{
    return nullptr;
}

ConditionTemplate *ScriptEngine::Get_Condition_Template(int index)
{
    return nullptr;
}

void ScriptEngine::Start_End_Game_Timer() {}

void ScriptEngine::Start_Quick_End_Game_Timer() {}

void ScriptEngine::Start_Close_Window_Timer() {}

void ScriptEngine::Run_Script(const Utf8String &script_name, Team *team) {}

void ScriptEngine::Run_Object_Script(const Utf8String &script_name, Object *obj) {}

Team *ScriptEngine::Get_Team_Named(const Utf8String &team_name)
{
    return nullptr;
}

Player *ScriptEngine::Get_Skirmish_Enemy_Player()
{
    return nullptr;
}

Player *ScriptEngine::Get_Current_Player()
{
    return nullptr;
}

Player *ScriptEngine::Get_Player_From_AsciiString(const Utf8String &player_name)
{
    return nullptr;
}

ObjectTypes *ScriptEngine::Get_Object_Types(const Utf8String &list_name)
{
    return nullptr;
}

void ScriptEngine::Do_ObjectType_List_Maintenance(const Utf8String &list_name, const Utf8String &type_name, bool add_type) {}

PolygonTrigger *ScriptEngine::Get_Qualified_Trigger_Area_By_Name(Utf8String trigger_area)
{
    return nullptr;
}

void ScriptEngine::Evaluate_Conditions(Script *script, Team *team, Player *player) {}

void ScriptEngine::Friend_Execute_Action(ScriptAction *action, Team *team) {}

Object *ScriptEngine::Get_Unit_Named(const Utf8String &unit_name)
{
    return nullptr;
}

bool ScriptEngine::Did_Unit_Exist(const Utf8String &unit_name)
{
    return false;
}

void ScriptEngine::Add_Object_To_Cache(Object *obj) {}

void ScriptEngine::Remove_Object_From_Cache(Object *obj) {}

void ScriptEngine::Transfer_Object_Name(const Utf8String &obj_name, Object *obj) {}

void ScriptEngine::Notify_Of_Object_Destruction(Object *obj) {}

void ScriptEngine::Notify_Of_Completed_Video(const Utf8String &video_name) {}

void ScriptEngine::Notify_Of_Triggered_Special_Power(int player_idx, const Utf8String &power_name, ObjectID source) {}

void ScriptEngine::Notify_Of_Midway_Special_Power(int player_idx, const Utf8String &power_name, ObjectID source) {}

void ScriptEngine::Notify_Of_Completed_Special_Power(int player_idx, const Utf8String &power_name, ObjectID source) {}

void ScriptEngine::Notify_Of_Completed_Upgrade(int player_idx, const Utf8String &upgrade_name, ObjectID source) {}

void ScriptEngine::Notify_Of_Acquired_Science(int player_idx, ScienceType science) {}

void ScriptEngine::Signal_UI_Interact(const Utf8String &hook_name) {}

bool ScriptEngine::Is_Video_Complete(const Utf8String &video_name, bool remove_from_list)
{
    return false;
}

bool ScriptEngine::Is_Speech_Complete(const Utf8String &speech_name, bool remove_from_list)
{
    return false;
}

bool ScriptEngine::Is_Audio_Complete(const Utf8String &audio_name, bool remove_from_list)
{
    return false;
}

bool ScriptEngine::Is_Special_Power_Triggered(
    int player_idx, const Utf8String &power_name, bool remove_from_list, ObjectID source)
{
    return false;
}

bool ScriptEngine::Is_Special_Power_Midway(
    int player_idx, const Utf8String &power_name, bool remove_from_list, ObjectID source)
{
    return false;
}

bool ScriptEngine::Is_Special_Power_Complete(
    int player_idx, const Utf8String &power_name, bool remove_from_list, ObjectID source)
{
    return false;
}

bool ScriptEngine::Is_Upgrade_Complete(
    int player_idx, const Utf8String &upgrade_name, bool remove_from_list, ObjectID source)
{
    return false;
}

bool ScriptEngine::Is_Science_Acquired(int player_idx, ScienceType science, bool remove_from_list)
{
    return false;
}

void ScriptEngine::Adjust_Topple_Direction(Object *obj, Coord2D *direction) {}

void ScriptEngine::Adjust_Topple_Direction(Object *obj, Coord3D *direction) {}

Script *ScriptEngine::Find_Script_By_Name(const Utf8String &script_name)
{
    return nullptr;
}

void ScriptEngine::Set_Global_Difficulty(GameDifficulty diff)
{
    captainslog_debug("ScriptEngine::setGlobalDifficulty(%d)\n", (int)diff);
    m_gameDifficulty = diff;
}
