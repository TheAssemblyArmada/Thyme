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

#include "always.h"
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
class ParticleSystem;
class ParticleSystemTemplate;
class Player;
class PolygonTrigger;
class Script;
class ScriptGroup;
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
    int value;
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

#ifdef GAME_DLL
    AttackPriorityInfo *Hook_Ctor() { return new (this) AttackPriorityInfo(); }
    void Hook_Dtor() { AttackPriorityInfo::~AttackPriorityInfo(); }
#endif

    virtual ~AttackPriorityInfo() override;

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    void Reset();
    void Set_Priority(const ThingTemplate *tmplate, int priority);
    int Get_Priority(const ThingTemplate *tmplate);

    Utf8String Get_Name() const { return m_name; }
    void Set_Name(const Utf8String &name) { m_name = name; }
    void Set_Default_Priority(int priority) { m_defaultPriority = priority; }

private:
    Utf8String m_name;
    int m_defaultPriority;
    std::map<const ThingTemplate *, int> *m_priorityMap;
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

#ifdef GAME_DLL
    ScriptEngine *Hook_Ctor() { return new (this) ScriptEngine(); }
    void Hook_Dtor() { ScriptEngine::~ScriptEngine(); }
#endif

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
    virtual ActionTemplate *Get_Action_Template(int index);
    virtual ConditionTemplate *Get_Condition_Template(int index);
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
    virtual bool Evaluate_Conditions(Script *script, Team *team, Player *player);
    virtual void Friend_Execute_Action(ScriptAction *action, Team *team);
    virtual Object *Get_Unit_Named(const Utf8String &unit_name);
    virtual bool Did_Unit_Exist(const Utf8String &unit_name);
    virtual void Add_Object_To_Cache(Object *obj);
    virtual void Remove_Object_From_Cache(Object *obj);
    virtual void Transfer_Object_Name(const Utf8String &obj_name, Object *obj);
    virtual void Notify_Of_Object_Destruction(Object *obj);
    virtual void Notify_Of_Completed_Video(const Utf8String &video_name);
    virtual void Notify_Of_Triggered_Special_Power(int player_idx, const Utf8String &power_name, ObjectID source);
    virtual void Notify_Of_Midway_Special_Power(int player_idx, const Utf8String &power_name, ObjectID source);
    virtual void Notify_Of_Completed_Special_Power(int player_idx, const Utf8String &power_name, ObjectID source);
    virtual void Notify_Of_Completed_Upgrade(int player_idx, const Utf8String &upgrade_name, ObjectID source);
    virtual void Notify_Of_Acquired_Science(int player_idx, ScienceType science);
    virtual void Signal_UI_Interact(const Utf8String &hook_name);
    virtual bool Is_Video_Complete(const Utf8String &video_name, bool remove_from_list);
    virtual bool Is_Speech_Complete(const Utf8String &speech_name, bool remove_from_list);
    virtual bool Is_Audio_Complete(const Utf8String &audio_name, bool remove_from_list);
    virtual bool Is_Special_Power_Triggered(
        int player_idx, const Utf8String &power_name, bool remove_from_list, ObjectID source);
    virtual bool Is_Special_Power_Midway(
        int player_idx, const Utf8String &power_name, bool remove_from_list, ObjectID source);
    virtual bool Is_Special_Power_Complete(
        int player_idx, const Utf8String &power_name, bool remove_from_list, ObjectID source);
    virtual bool Is_Upgrade_Complete(int player_idx, const Utf8String &upgrade_name, bool remove_from_list, ObjectID source);
    virtual bool Is_Science_Acquired(int player_idx, ScienceType science, bool remove_from_list);
    virtual void Adjust_Topple_Direction(Object *obj, Coord2D *direction);
    virtual void Adjust_Topple_Direction(Object *obj, Coord3D *direction);
    virtual Script *Find_Script_By_Name(const Utf8String &script_name);

    void Notify_Of_Object_Creation_Or_Destruction();

    void Set_Global_Difficulty(GameDifficulty diff);
    Utf8String Get_Stats(float *slowest_scripts, float *time_last_frame, float *time);
    void Update_Fades();
    void Clear_Flag(const Utf8String &flag);
    void Clear_Team_Flags();
    int Allocate_Counter(const Utf8String &counter);
    const TCounter *Get_Counter(const Utf8String &counter);
    void Create_Named_Map_Reveal(
        const Utf8String &reveal, const Utf8String &waypoint, float radius, const Utf8String &player);
    void Do_Named_Map_Reveal(const Utf8String &reveal);
    void Undo_Named_Map_Reveal(const Utf8String &reveal);
    void Remove_Named_Map_Reveal(const Utf8String &reveal);
    int Allocate_Flag(const Utf8String &flag);
    ScriptGroup *Find_Group(const Utf8String &group);
    Script *Find_Script(const Utf8String &script);
    bool Evaluate_Counter(Condition *condition);
    void Set_Counter(ScriptAction *action);
    void Set_Fade(ScriptAction *action);
    void Set_Sway(ScriptAction *action);
    void Add_Counter(ScriptAction *action);
    void Sub_Counter(ScriptAction *action);
    bool Evaluate_Flag(Condition *condition);
    void Set_Flag(ScriptAction *action);
    AttackPriorityInfo *Find_Attack_Info(const Utf8String &name, bool add_if_not_found);
    const AttackPriorityInfo *Get_Default_Attack_Info();
    void Set_Priority_Thing(ScriptAction *action);
    void Set_Priority_Kind(ScriptAction *action);
    void Set_Priority_Default(ScriptAction *action);
    int Get_Object_Count(int player, const Utf8String &name) const;
    void Set_Object_Count(int player, const Utf8String &name, int count);
    bool Evaluate_Timer(Condition *condition);
    void Set_Timer(ScriptAction *action, bool millisecond_timer, bool random);
    void Pause_Timer(ScriptAction *action);
    void Restart_Timer(ScriptAction *action);
    void Adjust_Timer(ScriptAction *action, bool millisecond_timer, bool add);
    void Enable_Script(ScriptAction *action);
    void Disable_Script(ScriptAction *action);
    void Call_Subroutine(ScriptAction *action);
    void Check_Conditions_For_Team_Names(Script *script);
    void Execute_Scripts(Script *script);
    void Execute_Script(Script *script);
    bool Evaluate_Condition(Condition *condition);
    void Set_Topple_Direction(const Utf8String &name, const Coord3D *direction);
    void Execute_Actions(ScriptAction *action);
    void Create_Named_Cache();
    void Append_Sequential_Script(const SequentialScript *script);
    void Remove_Sequential_Script(SequentialScript *script);
    void Remove_All_Sequential_Scripts(Object *obj);
    void Remove_All_Sequential_Scripts(Team *team);
    void Notify_Of_Team_Destruction(Team *team);
    void Set_Sequential_Timer(Object *obj, int timer);
    void Set_Sequential_Timer(Team *team, int timer);
    void Evaluate_And_Progress_All_Sequential_Scripts();
    bool Has_Unit_Completed_Sequential_Script(Object *obj, const Utf8String &name);
    bool Has_Team_Completed_Sequential_Script(Team *team, const Utf8String &name);
    bool Get_Enable_Vtune() const;
    void Set_Enable_Vtune(bool set);
    void Particle_Editor_Update();
    void Force_Unfreeze_Time();
    void Adjust_Debug_Variable_Data(const Utf8String &variable, int value, bool pause);
    void Debug_Victory();

    void Set_Use_Object_Difficulty_Bonus(bool bonus) { m_useObjectDifficultyBonuses = bonus; }
    bool Get_Use_Object_Difficulty_Bonus() const { return m_useObjectDifficultyBonuses; }
    bool Get_Choose_Victim_Always_Uses_Normal() const { return m_chooseVictimAlwaysUsesNormal; }
    int Get_Object_Creation_Destruction_Frame() const { return m_objectCreationDestructionFrame; }

    TFade Get_Fade() { return m_fade; }
    float Get_Fade_Value() { return m_curFadeValue; }
    BreezeInfo *Get_Breeze_Info() { return &m_breezeInfo; }
    bool Is_Time_Frozen_Script() { return m_freezeByScript; }
    bool Is_Time_Frozen_Debug();
    bool Is_Time_Fast();
    void Append_Debug_Message(const Utf8String &message, bool should_pause);
    GameDifficulty Get_Difficulty() { return m_gameDifficulty; }
    const AttackPriorityInfo *Get_Attack_Info(Utf8String const &name);
    bool Has_Shown_MP_Local_Defeat_Window() const { return m_hasShownMPLocalDefeatWindow; }
    void Mark_MP_Local_Defeat_Window_Shown() { m_hasShownMPLocalDefeatWindow = true; }
    bool Is_End_Game_Timer_Running() const { return m_endGameTimer >= 0; }

    void Disable_Breeze() { m_breezeInfo.intensity = 0.0f; }
    void Do_Freeze_Time() { m_freezeByScript = true; }
    void Do_Unfreeze_Time() { m_freezeByScript = false; }

    void Set_Current_Track_Name(Utf8String track) { m_currentTrackName = track; }
    void Set_Choose_Victim_Always_Uses_Normal(bool set) { m_chooseVictimAlwaysUsesNormal = set; }

    static void Parse_Script_Action(INI *ini);
    static void Parse_Script_Condition(INI *ini);

private:
    std::vector<SequentialScript *>::iterator Cleanup_Sequential_Script(
        std::vector<SequentialScript *>::iterator it, bool delete_sequence);
    void Remove_Object_Types(ObjectTypes *obj);

    void Add_Action_Template_Info(Template *tmplate);
    void Add_Condition_Template_Info(Template *tmplate);

    static void Append_Message(const Utf8String &str, bool is_true_message, bool should_pause);
    static void Adjust_Variable(const Utf8String &str, int value, bool should_pause);
    static void Update_Frame_Number();
    static void Append_All_Particle_Systems();
    static void Append_All_Thing_Templates();
    static void Add_Updated_Particle_System(Utf8String particle_system_name);
    static Utf8String Get_Particle_System_Name();
    static void Update_AsciiString_Parms_To_System(ParticleSystemTemplate *particle_template);
    static void Update_AsciiString_Parms_From_System(ParticleSystemTemplate *particle_template);
    static void Write_Out_INI();
    static void Write_Single_Particle_System(File *out, ParticleSystemTemplate *templ);
    static int Get_Editor_Behavior();
    static void Update_And_Set_Current_System();
    static void Update_Panel_Parameters(ParticleSystemTemplate *particle_template);
    static void Reload_Particle_System_From_INI(Utf8String particle_system_name);
    static int Get_New_Current_Particle_Cap();
    static void Update_Current_Particle_Cap();
    static void Update_Current_Particle_Count();
    static void Reload_Textures();

private:
    std::vector<SequentialScript *> m_sequentialScripts;
    ActionTemplate m_actionTemplates[ScriptAction::ACTION_COUNT];
    ConditionTemplate m_conditionTemplates[Condition::CONDITION_COUNT];
    TCounter m_counters[MAX_COUNTERS];
    int m_numCounters;
    TFlag m_flags[MAX_FLAGS];
    int m_numFlags;
    AttackPriorityInfo m_attackPriorityInfo[MAX_ATTACK_PRIORITIES];
    int m_numAttackInfo;
    int m_endGameTimer;
    int m_closeWindowTimer;
    Team *m_callingTeam;
    Object *m_callingObject;
    Team *m_conditionTeam;
    Object *m_conditionObject;
    std::vector<std::pair<Utf8String, Object *>> m_namedObjects;
    bool m_firstUpdate;
    Player *m_currentPlayer;
    int m_unkInt1;
    Utf8String m_currentTrackName;
    TFade m_fade;
    float m_minFade;
    float m_maxFade;
    float m_curFadeValue;
    int m_curFadeFrame;
    int m_fadeFramesIncrease;
    int m_fadeFramesHold;
    int m_fadeFramesDecrease;
    int m_objectCreationDestructionFrame;
    std::map<Utf8String, int> m_playerObjectCounts[MAX_PLAYER_COUNT];
    std::list<Utf8String> m_completedVideo;
    std::list<std::pair<Utf8String, unsigned int>> m_completedSpeech;
    std::list<std::pair<Utf8String, unsigned int>> m_completedAudio;
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
    bool m_chooseVictimAlwaysUsesNormal;
    bool m_hasShownMPLocalDefeatWindow;
#ifdef GAME_DEBUG_STRUCTS
    double m_numFrames;
    double m_totalUpdateTime;
    double m_maxUpdateTime;
    double m_frameUpdateTime;
#endif

    static bool s_canAppContinue;
    static int s_currentFrame;
    static int s_lastFrame;
    static ParticleSystem *s_particleSystem;
    static char s_bufferToCheck[1032];
    static bool s_particlesForever;
    static bool s_appIsFast;
#ifdef PLATFORM_WINDOWS
    static HMODULE s_debugDll;
    static HMODULE s_particleDll;
#endif
};

#ifdef GAME_DLL
extern ScriptEngine *&g_theScriptEngine;
#else
extern ScriptEngine *g_theScriptEngine;
#endif
