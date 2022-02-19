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
#pragma once

#include "asciistring.h"
#include "coord.h"
#include "gametype.h"
#include "globaldata.h"
#include "mempoolobj.h"
#include "science.h"
#include "scriptaction.h"
#include "scriptcondition.h"
#include "scripttemplate.h"
#include "snapshot.h"
#include "subsysteminterface.h"
#include <list>
#include <map>
#include <stdint.h>
#include <vector>

class Object;
class ObjectTypes;
class Player;
class PolygonTrigger;
class Script;
class ScriptAction;
class SequentialScript;
class Team;
class ThingTemplate;

struct BreezeInfo
{
    float direction;
    Coord2D sway_direction;
    float intensity;
    float lean;
    float randomness;
    int16_t period;
    int16_t version;
};

struct TCounter
{
    int32_t value;
    Utf8String name;
    bool is_countdown_timer;
};

struct TFlag
{
    bool value;
    Utf8String name;
};

struct NamedReveal
{
    Utf8String reveal_name;
    Utf8String waypoint_name;
    float radius;
    Utf8String player;
};

class AttackPriorityInfo : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(AttackPriorityInfo)

public:
    AttackPriorityInfo() : m_name(), m_defaultPriority(1), m_priorityMap(nullptr) {}
    virtual ~AttackPriorityInfo() override;

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    void Reset();

private:
    Utf8String m_name;
    int32_t m_defaultPriority;
    std::map<const ThingTemplate *, int32_t> *m_priorityMap;
};

class ScriptEngine : public SubsystemInterface, public SnapShot
{
    enum
    {
        MAX_COUNTERS = 256,
        MAX_FLAGS = 256,
        MAX_ATTACK_PRIORITIES = 256,
        MAX_PLAYER_COUNT = 16,
        MAX_DEBUG_SCRIPTS = 10,
    };

public:
    enum TFade
    {
        FADE_NONE,
        FADE_SUBTRACT,
        FADE_ADD,
        FADE_SATURATE,
        FADE_MULTIPLY,
    };

    ScriptEngine();
    virtual ~ScriptEngine() override;

    // SubsystemInterface virtuals.
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    // Snapshot virtuals.
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    // ScriptEngine virtuals.
    virtual void New_Map();
    virtual ActionTemplate *Get_Action_Template(int32_t index);
    virtual ConditionTemplate *Get_Condition_Template(int32_t index);
    virtual void Start_End_Game_Timer();
    virtual void Start_Quick_End_Game_Timer();
    virtual void Start_Close_Window_Timer();
    virtual void Run_Script(const Utf8String &script_name, Team *team);
    virtual void Run_Object_Script(const Utf8String &script_name, Object *obj);
    virtual Team *Get_Team_Named(const Utf8String &team_name);
    virtual Player *Get_Skirmish_Enemy_Player();
    virtual Player *Get_Current_Player();
    virtual Player *Get_Player_From_AsciiString(const Utf8String &player_name);
    virtual ObjectTypes *Get_Object_Types(const Utf8String &list_name);
    virtual void Do_ObjectType_List_Maintenance(const Utf8String &list_name, const Utf8String &type_name, bool add_type);
    virtual PolygonTrigger *Get_Qualified_Trigger_Area_By_Name(Utf8String trigger_area);
    virtual void Evaluate_Conditions(Script *script, Team *team, Player *player);
    virtual void Friend_Execute_Action(ScriptAction *action, Team *team);
    virtual Object *Get_Unit_Named(const Utf8String &unit_name);
    virtual bool Did_Unit_Exist(const Utf8String &unit_name);
    virtual void Add_Object_To_Cache(Object *obj);
    virtual void Remove_Object_From_Cache(Object *obj);
    virtual void Transfer_Object_Name(const Utf8String &obj_name, Object *obj);
    virtual void Notify_Of_Object_Destruction(Object *obj);
    virtual void Notify_Of_Completed_Video(const Utf8String &video_name);
    virtual void Notify_Of_Triggered_Special_Power(int32_t player_idx, const Utf8String &power_name, ObjectID source);
    virtual void Notify_Of_Midway_Special_Power(int32_t player_idx, const Utf8String &power_name, ObjectID source);
    virtual void Notify_Of_Completed_Special_Power(int32_t player_idx, const Utf8String &power_name, ObjectID source);
    virtual void Notify_Of_Completed_Upgrade(int32_t player_idx, const Utf8String &upgrade_name, ObjectID source);
    virtual void Notify_Of_Acquired_Science(int32_t player_idx, ScienceType science);
    virtual void Signal_UI_Interact(const Utf8String &hook_name);
    virtual bool Is_Video_Complete(const Utf8String &video_name, bool remove_from_list);
    virtual bool Is_Speech_Complete(const Utf8String &speech_name, bool remove_from_list);
    virtual bool Is_Audio_Complete(const Utf8String &audio_name, bool remove_from_list);
    virtual bool Is_Special_Power_Triggered(
        int32_t player_idx, const Utf8String &power_name, bool remove_from_list, ObjectID source);
    virtual bool Is_Special_Power_Midway(
        int32_t player_idx, const Utf8String &power_name, bool remove_from_list, ObjectID source);
    virtual bool Is_Special_Power_Complete(
        int32_t player_idx, const Utf8String &power_name, bool remove_from_list, ObjectID source);
    virtual bool Is_Upgrade_Complete(int32_t player_idx, const Utf8String &upgrade_name, bool remove_from_list, ObjectID source);
    virtual bool Is_Science_Acquired(int32_t player_idx, ScienceType science, bool remove_from_list);
    virtual void Adjust_Topple_Direction(Object *obj, Coord2D *direction);
    virtual void Adjust_Topple_Direction(Object *obj, Coord3D *direction);
    virtual Script *Find_Script_By_Name(const Utf8String &script_name);

    void Set_Global_Difficulty(GameDifficulty diff);
    void Set_Use_Object_Difficulty_Bonus(bool bonus) { m_useObjectDifficultyBonuses = bonus; }

    TFade Get_Fade() { return m_fade; }
    float Get_Fade_Value() { return m_curFadeValue; }
    BreezeInfo *Get_Breeze_Info() { return &m_breezeInfo; }
    bool Is_Time_Frozen_Script() { return m_freezeByScript; }
    bool Is_Time_Frozen_Debug();
    bool Is_Time_Fast();

private:
    void Init_Action_Templates();
    void Init_Condition_Templates();
    std::vector<SequentialScript *>::iterator Cleanup_Sequential_Scripts(
        std::vector<SequentialScript *>::iterator it, bool clean_danglers);
    void Remove_Object_Types(ObjectTypes *obj);

    static void Update_Current_Particle_Cap();

private:
    std::vector<SequentialScript *> m_sequentialScripts;
    ActionTemplate m_actionTemplates[ScriptAction::ACTION_COUNT];
    ConditionTemplate m_conditionTemplates[Condition::CONDITION_COUNT];
    TCounter m_counters[MAX_COUNTERS];
    int32_t m_numCounters;
    TFlag m_flags[MAX_FLAGS];
    int32_t m_numFlags;
    AttackPriorityInfo m_attackPriorityInfo[MAX_ATTACK_PRIORITIES];
    int32_t m_numAttackInfo;
    int32_t m_endGameTimer;
    int32_t m_closeWindowTimer;
    Team *m_callingTeam;
    Object *m_callingObject;
    Team *m_conditionTeam;
    Object *m_conditionObject;
    std::vector<std::pair<Utf8String, Object *>> m_namedObjects;
    bool m_firstUpdate;
    Player *m_currentPlayer;
    int32_t m_unkInt1;
    Utf8String m_currentTrackName;
    TFade m_fade;
    float m_minFade;
    float m_maxFade;
    float m_curFadeValue;
    int32_t m_curFadeFrame;
    int32_t m_fadeFramesIncrease;
    int32_t m_fadeFramesHold;
    int32_t m_fadeFramesDecrease;
    int32_t m_objectCreationDestructionFrame;
    std::map<Utf8String, unsigned> m_playerObjectCounts[16];
    std::list<Utf8String> m_completedVideo;
    std::list<std::pair<Utf8String, unsigned>> m_completedSpeech;
    std::list<std::pair<Utf8String, unsigned>> m_completedAudio;
    std::list<Utf8String> m_uiInteraction;
    std::list<std::pair<Utf8String, ObjectID>> m_triggeredSpecialPowers[MAX_PLAYER_COUNT];
    std::list<std::pair<Utf8String, ObjectID>> m_midwaySpecialPowers[MAX_PLAYER_COUNT];
    std::list<std::pair<Utf8String, ObjectID>> m_finishedSpecialPowers[MAX_PLAYER_COUNT];
    std::list<std::pair<Utf8String, ObjectID>> m_completedUpgrades[MAX_PLAYER_COUNT];
    std::vector<ScienceType> m_acquiredSciences[MAX_PLAYER_COUNT];
    std::list<std::pair<Utf8String, Coord3D>> m_toppleDirections;
    std::vector<NamedReveal> m_namedReveals;
    BreezeInfo m_breezeInfo;
    GameDifficulty m_gameDifficulty;
    bool m_freezeByScript;
    std::vector<ObjectTypes *> m_allObjectTypeLists;
    bool m_useObjectDifficultyBonuses;
    bool m_unkBool1;
    bool m_hasShowMPLocalDefeatWindow;
#ifdef GAME_DEBUG_STRUCTS
    double m_numFrames;
    double m_totalUpdateTime;
    double m_maxUpdateTime;
    double m_frameUpdateTime;
#endif

#ifdef GAME_DLL
    static bool &s_canAppContinue;
    static int32_t &s_currentFrame;
    static int32_t &s_lastFrame;
    static void *&s_debugDll;
    static void *&s_particleDll;
#else
    static bool s_canAppContinue;
    static int32_t s_currentFrame;
    static int32_t s_lastFrame;
    static void *s_debugDll;
    static void *s_particleDll;
#endif
};

#ifdef GAME_DLL
extern ScriptEngine *&g_theScriptEngine;
#else
extern ScriptEngine *g_theScriptEngine;
#endif
