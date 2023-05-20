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

#include "always.h"
#include "audiomanager.h"
#include "mempoolobj.h"
#include "scriptparam.h"

class Object;

class ScriptAction : public MemoryPoolObject
{
    IMPLEMENT_POOL(ScriptAction);

    enum
    {
        MAX_ACTION_PARAMETERS = 12,
    };

public:
    enum ScriptActionType
    {
        DEBUG_MESSAGE_BOX,
        SET_FLAG,
        SET_COUNTER,
        VICTORY,
        DEFEAT,
        NO_OP,
        SET_TIMER,
        PLAY_SOUND_EFFECT,
        ENABLE_SCRIPT,
        DISABLE_SCRIPT,
        CALL_SUBROUTINE,
        PLAY_SOUND_EFFECT_AT,
        DAMAGE_MEMBERS_OF_TEAM,
        MOVE_TEAM_TO,
        MOVE_CAMERA_TO,
        INCREMENT_COUNTER,
        DECREMENT_COUNTER,
        MOVE_CAMERA_ALONG_WAYPOINT_PATH,
        ROTATE_CAMERA,
        RESET_CAMERA,
        SET_MILLISECOND_TIMER,
        CAMERA_MOD_FREEZE_TIME,
        SET_VISUAL_SPEED_MULTIPLIER,
        CREATE_OBJECT,
        SUSPEND_BACKGROUND_SOUNDS,
        RESUME_BACKGROUND_SOUNDS,
        CAMERA_MOD_SET_FINAL_ZOOM,
        CAMERA_MOD_SET_FINAL_PITCH,
        CAMERA_MOD_FREEZE_ANGLE,
        CAMERA_MOD_SET_FINAL_SPEED_MULTIPLIER,
        CAMERA_MOD_SET_ROLLING_AVERAGE,
        CAMERA_MOD_FINAL_LOOK_TOWARD,
        CAMERA_MOD_LOOK_TOWARD,
        TEAM_ATTACK_TEAM,
        CREATE_REINFORCEMENT_TEAM,
        MOVE_CAMERA_TO_SELECTION,
        TEAM_FOLLOW_WAYPOINTS,
        TEAM_SET_STATE,
        MOVE_NAMED_UNIT_TO,
        NAMED_ATTACK_NAMED,
        CREATE_NAMED_ON_TEAM_AT_WAYPOINT,
        CREATE_UNNAMED_ON_TEAM_AT_WAYPOINT,
        NAMED_APPLY_ATTACK_PRIORITY_SET,
        TEAM_APPLY_ATTACK_PRIORITY_SET,
        SET_BASE_CONSTRUCTION_SPEED,
        NAMED_SET_ATTITUDE,
        TEAM_SET_ATTITUDE,
        NAMED_ATTACK_AREA,
        NAMED_ATTACK_TEAM,
        TEAM_ATTACK_AREA,
        TEAM_ATTACK_NAMED,
        TEAM_LOAD_TRANSPORTS,
        NAMED_ENTER_NAMED,
        TEAM_ENTER_NAMED,
        NAMED_EXIT_ALL,
        TEAM_EXIT_ALL,
        NAMED_FOLLOW_WAYPOINTS,
        NAMED_GUARD,
        TEAM_GUARD,
        NAMED_HUNT,
        TEAM_HUNT,
        PLAYER_SELL_EVERYTHING,
        PLAYER_DISABLE_BASE_CONSTRUCTION,
        PLAYER_DISABLE_FACTORIES,
        PLAYER_DISABLE_UNIT_CONSTRUCTION,
        PLAYER_ENABLE_BASE_CONSTRUCTION,
        PLAYER_ENABLE_FACTORIES,
        PLAYER_ENABLE_UNIT_CONSTRUCTION,
        CAMERA_MOVE_HOME,
        BUILD_TEAM,
        NAMED_DAMAGE,
        NAMED_DELETE,
        TEAM_DELETE,
        NAMED_KILL,
        TEAM_KILL,
        PLAYER_KILL,
        DISPLAY_TEXT,
        CAMEO_FLASH,
        NAMED_FLASH,
        TEAM_FLASH,
        MOVIE_PLAY_FULLSCREEN,
        MOVIE_PLAY_RADAR,
        SOUND_PLAY_NAMED,
        SPEECH_PLAY,
        PLAYER_TRANSFER_OWNERSHIP_PLAYER,
        NAMED_TRANSFER_OWNERSHIP_PLAYER,
        PLAYER_RELATES_PLAYER,
        RADAR_CREATE_EVENT,
        RADAR_DISABLE,
        RADAR_ENABLE,
        MAP_REVEAL_AT_WAYPOINT,
        TEAM_AVAILABLE_FOR_RECRUITMENT,
        TEAM_COLLECT_NEARBY_FOR_TEAM,
        TEAM_MERGE_INTO_TEAM,
        DISABLE_INPUT,
        ENABLE_INPUT,
        PLAYER_HUNT,
        SOUND_AMBIENT_PAUSE,
        SOUND_AMBIENT_RESUME,
        MUSIC_SET_TRACK,
        SET_TREE_SWAY,
        DEBUG_STRING,
        MAP_REVEAL_ALL,
        TEAM_GARRISON_SPECIFIC_BUILDING,
        EXIT_SPECIFIC_BUILDING,
        TEAM_GARRISON_NEAREST_BUILDING,
        TEAM_EXIT_ALL_BUILDINGS,
        NAMED_GARRISON_SPECIFIC_BUILDING,
        NAMED_GARRISON_NEAREST_BUILDING,
        NAMED_EXIT_BUILDING,
        PLAYER_GARRISON_ALL_BUILDINGS,
        PLAYER_EXIT_ALL_BUILDINGS,
        TEAM_WANDER,
        TEAM_PANIC,
        SETUP_CAMERA,
        CAMERA_LETTERBOX_BEGIN,
        CAMERA_LETTERBOX_END,
        ZOOM_CAMERA,
        PITCH_CAMERA,
        CAMERA_FOLLOW_NAMED,
        OVERSIZE_TERRAIN,
        CAMERA_FADE_ADD,
        CAMERA_FADE_SUBTRACT,
        CAMERA_FADE_SATURATE,
        CAMERA_FADE_MULTIPLY,
        CAMERA_BW_MODE_BEGIN,
        CAMERA_BW_MODE_END,
        DRAW_SKYBOX_BEGIN,
        DRAW_SKYBOX_END,
        SET_ATTACK_PRIORITY_THING,
        SET_ATTACK_PRIORITY_KIND_OF,
        SET_DEFAULT_ATTACK_PRIORITY,
        CAMERA_STOP_FOLLOW,
        CAMERA_MOTION_BLUR,
        CAMERA_MOTION_BLUR_JUMP,
        CAMERA_MOTION_BLUR_FOLLOW,
        CAMERA_MOTION_BLUR_END_FOLLOW,
        FREEZE_TIME,
        UNFREEZE_TIME,
        SHOW_MILITARY_CAPTION,
        CAMERA_SET_AUDIBLE_DISTANCE,
        SET_STOPPING_DISTANCE,
        NAMED_SET_STOPPING_DISTANCE,
        SET_FPS_LIMIT,
        MUSIC_SET_VOLUME,
        MAP_SHROUD_AT_WAYPOINT,
        MAP_SHROUD_ALL,
        SET_RANDOM_TIMER,
        SET_RANDOM_MSEC_TIMER,
        STOP_TIMER,
        RESTART_TIMER,
        ADD_TO_MSEC_TIMER,
        SUB_FROM_MSEC_TIMER,
        TEAM_TRANSFER_TO_PLAYER,
        PLAYER_SET_MONEY,
        PLAYER_GIVE_MONEY,
        DISABLE_SPECIAL_POWER_DISPLAY,
        ENABLE_SPECIAL_POWER_DISPLAY,
        NAMED_HIDE_SPECIAL_POWER_DISPLAY,
        NAMED_SHOW_SPECIAL_POWER_DISPLAY,
        DISPLAY_COUNTDOWN_TIMER,
        HIDE_COUNTDOWN_TIMER,
        ENABLE_COUNTDOWN_TIMER_DISPLAY,
        DISABLE_COUNTDOWN_TIMER_DISPLAY,
        NAMED_STOP_SPECIAL_POWER_COUNTDOWN,
        NAMED_START_SPECIAL_POWER_COUNTDOWN,
        NAMED_SET_SPECIAL_POWER_COUNTDOWN,
        NAMED_ADD_SPECIAL_POWER_COUNTDOWN,
        NAMED_FIRE_SPECIAL_POWER_AT_WAYPOINT,
        NAMED_FIRE_SPECIAL_POWER_AT_NAMED,
        REFRESH_RADAR,
        CAMERA_TETHER_NAMED,
        CAMERA_STOP_TETHER_NAMED,
        CAMERA_SET_DEFAULT,
        NAMED_STOP,
        TEAM_STOP,
        TEAM_STOP_AND_DISBAND,
        RECRUIT_TEAM,
        TEAM_SET_OVERRIDE_RELATION_TO_TEAM,
        TEAM_REMOVE_OVERRIDE_RELATION_TO_TEAM,
        TEAM_REMOVE_ALL_OVERRIDE_RELATIONS,
        CAMERA_LOOK_TOWARD_OBJECT,
        NAMED_FIRE_WEAPON_FOLLOWING_WAYPOINT_PATH,
        TEAM_SET_OVERRIDE_RELATION_TO_PLAYER,
        TEAM_REMOVE_OVERRIDE_RELATION_TO_PLAYER,
        PLAYER_SET_OVERRIDE_RELATION_TO_TEAM,
        PLAYER_REMOVE_OVERRIDE_RELATION_TO_TEAM,
        UNIT_EXECUTE_SEQUENTIAL_SCRIPT,
        UNIT_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING,
        UNIT_STOP_SEQUENTIAL_SCRIPT,
        TEAM_EXECUTE_SEQUENTIAL_SCRIPT,
        TEAM_EXECUTE_SEQUENTIAL_SCRIPT_LOOPING,
        TEAM_STOP_SEQUENTIAL_SCRIPT,
        UNIT_GUARD_FOR_FRAMECOUNT,
        UNIT_IDLE_FOR_FRAMECOUNT,
        TEAM_GUARD_FOR_FRAMECOUNT,
        TEAM_IDLE_FOR_FRAMECOUNT,
        WATER_CHANGE_HEIGHT,
        NAMED_USE_COMMANDBUTTON_ABILITY_ON_NAMED,
        NAMED_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT,
        WATER_CHANGE_HEIGHT_OVER_TIME,
        MAP_SWITCH_BORDER,
        TEAM_GUARD_POSITION,
        TEAM_GUARD_OBJECT,
        TEAM_GUARD_AREA,
        OBJECT_FORCE_SELECT,
        CAMERA_LOOK_TOWARD_WAYPOINT,
        UNIT_DESTROY_ALL_CONTAINED,
        RADAR_FORCE_ENABLE,
        RADAR_REVERT_TO_NORMAL,
        SCREEN_SHAKE,
        TECHTREE_MODIFY_BUILDABILITY_OBJECT,
        WAREHOUSE_SET_VALUE,
        OBJECT_CREATE_RADAR_EVENT,
        TEAM_CREATE_RADAR_EVENT,
        DISPLAY_CINEMATIC_TEXT,
        DEBUG_CRASH_BOX,
        SOUND_DISABLE_TYPE,
        SOUND_ENABLE_TYPE,
        SOUND_ENABLE_ALL,
        AUDIO_OVERRIDE_VOLUME_TYPE,
        AUDIO_RESTORE_VOLUME_TYPE,
        AUDIO_RESTORE_VOLUME_ALL_TYPE,
        INGAME_POPUP_MESSAGE,
        SET_CAVE_INDEX,
        NAMED_SET_HELD,
        NAMED_SET_TOPPLE_DIRECTION,
        UNIT_MOVE_TOWARDS_NEAREST_OBJECT_TYPE,
        TEAM_MOVE_TOWARDS_NEAREST_OBJECT_TYPE,
        MAP_REVEAL_ALL_PERM,
        MAP_REVEAL_ALL_UNDO_PERM,
        NAMED_SET_REPULSOR,
        TEAM_SET_REPULSOR,
        TEAM_WANDER_IN_PLACE,
        TEAM_INCREASE_PRIORITY,
        TEAM_DECREASE_PRIORITY,
        DISPLAY_COUNTER,
        HIDE_COUNTER,
        TEAM_USE_COMMANDBUTTON_ABILITY_ON_NAMED,
        TEAM_USE_COMMANDBUTTON_ABILITY_AT_WAYPOINT,
        NAMED_USE_COMMANDBUTTON_ABILITY,
        TEAM_USE_COMMANDBUTTON_ABILITY,
        NAMED_FLASH_WHITE,
        TEAM_FLASH_WHITE,
        SKIRMISH_BUILD_BUILDING,
        SKIRMISH_FOLLOW_APPROACH_PATH,
        IDLE_ALL_UNITS,
        RESUME_SUPPLY_TRUCKING,
        NAMED_CUSTOM_COLOR,
        SKIRMISH_MOVE_TO_APPROACH_PATH,
        SKIRMISH_BUILD_BASE_DEFENSE_FRONT,
        SKIRMISH_FIRE_SPECIAL_POWER_AT_MOST_COST,
        NAMED_RECEIVE_UPGRADE,
        PLAYER_REPAIR_NAMED_STRUCTURE,
        SKIRMISH_BUILD_BASE_DEFENSE_FLANK,
        SKIRMISH_BUILD_STRUCTURE_FRONT,
        SKIRMISH_BUILD_STRUCTURE_FLANK,
        SKIRMISH_ATTACK_NEAREST_GROUP_WITH_VALUE,
        SKIRMISH_PERFORM_COMMANDBUTTON_ON_MOST_VALUABLE_OBJECT,
        SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_ALL,
        SKIRMISH_WAIT_FOR_COMMANDBUTTON_AVAILABLE_PARTIAL,
        TEAM_SPIN_FOR_FRAMECOUNT,
        TEAM_ALL_USE_COMMANDBUTTON_ON_NAMED,
        TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_UNIT,
        TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_GARRISONED_BUILDING,
        TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_KINDOF,
        TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING,
        TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_ENEMY_BUILDING_CLASS,
        TEAM_ALL_USE_COMMANDBUTTON_ON_NEAREST_OBJECTTYPE,
        TEAM_PARTIAL_USE_COMMANDBUTTON,
        TEAM_CAPTURE_NEAREST_UNOWNED_FACTION_UNIT,
        PLAYER_CREATE_TEAM_FROM_CAPTURED_UNITS,
        PLAYER_ADD_SKILLPOINTS,
        PLAYER_ADD_RANKLEVEL,
        PLAYER_SET_RANKLEVEL,
        PLAYER_SET_RANKLEVELLIMIT,
        PLAYER_GRANT_SCIENCE,
        PLAYER_PURCHASE_SCIENCE,
        TEAM_HUNT_WITH_COMMAND_BUTTON,
        TEAM_WAIT_FOR_NOT_CONTAINED_ALL,
        TEAM_WAIT_FOR_NOT_CONTAINED_PARTIAL,
        TEAM_FOLLOW_WAYPOINTS_EXACT,
        NAMED_FOLLOW_WAYPOINTS_EXACT,
        TEAM_SET_EMOTICON,
        NAMED_SET_EMOTICON,
        AI_PLAYER_BUILD_SUPPLY_CENTER,
        AI_PLAYER_BUILD_UPGRADE,
        OBJECTLIST_ADDOBJECTTYPE,
        OBJECTLIST_REMOVEOBJECTTYPE,
        MAP_REVEAL_PERMANENTLY_AT_WAYPOINT,
        MAP_UNDO_REVEAL_PERMANENTLY_AT_WAYPOINT,
        NAMED_SET_STEALTH_ENABLED,
        TEAM_SET_STEALTH_ENABLED,
        EVA_SET_ENABLED_DISABLED,
        OPTIONS_SET_OCCLUSION_MODE,
        LOCALDEFEAT,
        OPTIONS_SET_DRAWICON_UI_MODE,
        OPTIONS_SET_PARTICLE_CAP_MODE,
        PLAYER_SCIENCE_AVAILABILITY,
        UNIT_AFFECT_OBJECT_PANEL_FLAGS,
        TEAM_AFFECT_OBJECT_PANEL_FLAGS,
        PLAYER_SELECT_SKILLSET,
        SCRIPTING_OVERRIDE_HULK_LIFETIME,
        NAMED_FACE_NAMED,
        NAMED_FACE_WAYPOINT,
        TEAM_FACE_NAMED,
        TEAM_FACE_WAYPOINT,
        COMMANDBAR_REMOVE_BUTTON_OBJECTTYPE,
        COMMANDBAR_ADD_BUTTON_OBJECTTYPE_SLOT,
        UNIT_SPAWN_NAMED_LOCATION_ORIENTATION,
        PLAYER_AFFECT_RECEIVING_EXPERIENCE,
        PLAYER_EXCLUDE_FROM_SCORE_SCREEN,
        TEAM_GUARD_SUPPLY_CENTER,
        ENABLE_SCORING,
        DISABLE_SCORING,
        SOUND_SET_VOLUME,
        SPEECH_SET_VOLUME,
        DISABLE_BORDER_SHROUD,
        ENABLE_BORDER_SHROUD,
        OBJECT_ALLOW_BONUSES,
        SOUND_REMOVE_ALL_DISABLED,
        SOUND_REMOVE_TYPE,
        TEAM_GUARD_IN_TUNNEL_NETWORK,
        QUICKVICTORY,
        SET_INFANTRY_LIGHTING_OVERRIDE,
        RESET_INFANTRY_LIGHTING_OVERRIDE,
        TEAM_DELETE_LIVING,
        RESIZE_VIEW_GUARDBAND,
        DELETE_ALL_UNMANNED,
        CHOOSE_VICTIM_ALWAYS_USES_NORMAL,
        CAMERA_ENABLE_SLAVE_MODE,
        CAMERA_DISABLE_SLAVE_MODE,
        CAMERA_ADD_SHAKER_AT,
        SET_TRAIN_HELD,
        NAMED_SET_EVAC_LEFT_OR_RIGHT,
        ENABLE_OBJECT_SOUND,
        DISABLE_OBJECT_SOUND,
        NAMED_USE_COMMANDBUTTON_ABILITY_USING_WAYPOINT_PATH,
        NAMED_SET_UNMANNED_STATUS,
        TEAM_SET_UNMANNED_STATUS,
        NAMED_SET_BOOBYTRAPPED,
        TEAM_SET_BOOBYTRAPPED,
        SHOW_WEATHER,
        AI_PLAYER_BUILD_TYPE_NEAREST_TEAM,

        ACTION_COUNT,
    };

protected:
    virtual ~ScriptAction() override;

public:
    ScriptAction();
    ScriptAction(ScriptActionType type);

    ScriptAction *Duplicate();
    ScriptAction *Duplicate_And_Qualify(const Utf8String &str1, const Utf8String &str2, const Utf8String &str3);
    Utf8String Get_UI_Text();
    int Get_UI_Strings(Utf8String *const strings);

    ScriptActionType Get_Action_Type() const { return m_actionType; }
    int Get_Num_Parameters() const { return m_numParams; }
    ScriptAction *Get_Next() const { return m_nextAction; }

    Parameter *Get_Parameter(int ndx) const
    {
        if (ndx < 0 || ndx >= m_numParams) {
            return nullptr;
        } else {
            return m_params[ndx];
        }
    }

    void Set_Warnings(bool set) { m_hasWarnings = set; }
    void Set_Next_Action(ScriptAction *action) { m_nextAction = action; }

    static bool Parse_Action_Data_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data);
    static bool Parse_Action_False_Data_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data);
    static void Write_Action_Data_Chunk(DataChunkOutput &output, ScriptAction *action);
    static void Write_Action_False_Data_Chunk(DataChunkOutput &output, ScriptAction *action);

private:
    void Set_Action_Type(ScriptActionType type);
    static ScriptAction *Parse_Action(DataChunkInput &input, DataChunkInfo *info, void *data);

private:
    ScriptActionType m_actionType;
    int m_numParams;
    Parameter *m_params[MAX_ACTION_PARAMETERS];
    ScriptAction *m_nextAction;
    bool m_hasWarnings;
};

class ScriptActionsInterface : public SubsystemInterface
{
public:
    ScriptActionsInterface();
    virtual ~ScriptActionsInterface() override;
    virtual void Execute_Action(ScriptAction *action) = 0;
    virtual void Close_Windows(bool close) = 0;
    virtual void Do_Enable_Or_Disable_Object_Difficulty_Bonuses(bool enable) = 0;
};

class ScriptActions : public ScriptActionsInterface
{
public:
    ScriptActions();
    virtual ~ScriptActions() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;
    virtual void Execute_Action(ScriptAction *action) override;
    virtual void Close_Windows(bool close) override;
    virtual void Do_Enable_Or_Disable_Object_Difficulty_Bonuses(bool enable) override;

    void Do_Quick_Victory();
    void Do_Set_Infantry_Lighting_Override(float override);
    void Do_Victory();
    void Do_Defeat();
    void Do_Local_Defeat();
    void Change_Object_Panel_Flag_For_Single_Object(Object *obj, const Utf8String &flag, bool set);
    void Do_Debug_Message(const Utf8String &msg, bool pause);
    void Do_Play_Sound_Effect(const Utf8String &sound);
    void Do_Play_Sound_Effect_At(const Utf8String &sound, Utf8String waypoint);
    void Do_Enable_Object_Sound(const Utf8String &name, bool enable);
    void Do_Damage_Team_Members(const Utf8String &team, float amount);
    void Do_Move_To_Waypoint(const Utf8String &team, const Utf8String &waypoint);
    void Do_Named_Move_To_Waypoint(const Utf8String &unit, const Utf8String &waypoint);
    void Do_Camera_Follow_Named(Utf8String unit, bool snap_to_unit);
    void Do_Stop_Camera_Follow_Unit();
    void Do_Set_Team_State(const Utf8String &team, const Utf8String &state);
    void Do_Create_Reinforcements(const Utf8String &team, const Utf8String &waypoint);
    void Do_Move_Camera_To(Utf8String waypoint, float sec, float stutter_sec, float f1, float f2);
    void Do_Zoom_Camera(float zoom, float sec, float f1, float f2);
    void Do_Pitch_Camera(float pitch, float sec, float f1, float f2);
    void Do_Oversize_The_Terrain(int amount);
    void Do_Setup_Camera(const Utf8String &waypoint, float zoom, float pitch, const Utf8String &look_at_waypoint);
    void Do_Mod_Camera_Look_Toward(const Utf8String &waypoint);
    void Do_Mod_Camera_Final_Look_Toward(const Utf8String &waypoint);
    void Do_Mod_Camera_Move_To_Selection();
    void Do_Reset_Camera(Utf8String waypoint, float sec, float f1, float f2);
    void Do_Rotate_Camera(float rotations, float sec, float f1, float f2);
    void Do_Rotate_Camera_Toward_Object(const Utf8String &unit, float sec, float hold_sec, float f1, float f2);
    void Do_Rotate_Camera_Toward_Waypoint(const Utf8String &waypoint, float sec, float f1, float f2, bool b);
    void Do_Move_Camera_Along_Waypoint_Path(const Utf8String &waypoint, float sec, float stutter_sec, float f1, float f2);
    void Do_Create_Object(const Utf8String &name, const Utf8String &team, const Utf8String &str, Coord3D *pos, float angle);
    void Do_Attack(const Utf8String &attacker, const Utf8String &victim);
    void Do_Named_Attack(const Utf8String &attacker, const Utf8String &victim);
    void Do_Build_Building(const Utf8String &building);
    void Do_Build_Supply_Center(const Utf8String &player, const Utf8String &name, int supplies);
    void Do_Build_Object_Nearest_Team(const Utf8String &player, const Utf8String &name, const Utf8String &team);
    void Do_Build_Upgrade(const Utf8String &player, const Utf8String &name);
    void Do_Build_Base_Defense(bool build);
    void Do_Build_Base_Structure(const Utf8String &name, bool build);
    void Create_Unit_On_Team_At(
        const Utf8String &unit, const Utf8String &obj_type, const Utf8String &team, const Utf8String &waypoint);
    void Update_Named_Attack_Priority_Set(const Utf8String &name, const Utf8String &priority_set);
    void Update_Team_Attack_Priority_Set(const Utf8String &name, const Utf8String &priorty_set);
    void Update_Base_Construction_Speed(const Utf8String &player, int speed);
    void Update_Named_Set_Attitude(const Utf8String &name, int attitude);
    void Update_Team_Set_Attitude(const Utf8String &team, int attitude);
    void Do_Named_Set_Repulsor(const Utf8String &name, bool set);
    void Do_Team_Set_Repulsor(const Utf8String &team, bool set);
    void Do_Named_Attack_Area(const Utf8String &name, const Utf8String &area);
    void Do_Named_Attack_Team(const Utf8String &name, const Utf8String &team);
    void Do_Team_Attack_Area(const Utf8String &team, const Utf8String &area);
    void Do_Team_Attack_Named(const Utf8String &team, const Utf8String &named);
    void Do_Load_All_Transports(const Utf8String &team);
    void Do_Named_Enter_Named(const Utf8String &src_name, const Utf8String &dest_name);
    void Do_Team_Enter_Named(const Utf8String &team, const Utf8String &dest_name);
    void Do_Named_Exit_All(const Utf8String &name);
    void Do_Team_Exit_All(const Utf8String &team);
    void Do_Named_Set_Garrison_Evac_Disposition(const Utf8String &name, unsigned int disposition);
    void Do_Named_Follow_Waypoints(const Utf8String &name, const Utf8String &waypoint);
    void Do_Named_Follow_Waypoints_Exact(const Utf8String &name, const Utf8String &waypoint);
    void Do_Team_Follow_Skirmish_Approach_Path(const Utf8String &team, const Utf8String &path, bool as_team);
    void Do_Team_Move_To_Skirmish_Approach_Path(const Utf8String &team, const Utf8String &path);
    void Do_Team_Follow_Waypoints(const Utf8String &team, const Utf8String &waypoint, bool as_team);
    void Do_Team_Follow_Waypoints_Exact(const Utf8String &team, const Utf8String &waypoint, bool as_team);
    void Do_Named_Guard(const Utf8String &name);
    void Do_Team_Guard(const Utf8String &team);
    void Do_Team_Guard_Position(const Utf8String &team, const Utf8String &wapoint);
    void Do_Team_Guard_Object(const Utf8String &team, const Utf8String &name);
    void Do_Team_Guard_Area(const Utf8String &team, const Utf8String &area);
    void Do_Named_Hunt(const Utf8String &name);
    void Do_Team_Hunt(const Utf8String &team);
    void Do_Team_Hunt_With_Command_Button(const Utf8String &team, const Utf8String &button);
    void Do_Player_Hunt(const Utf8String &player);
    void Do_Player_Sell_Everything(const Utf8String &player);
    void Do_Player_Disable_Base_Construction(const Utf8String &player);
    void Do_Player_Disable_Factories(const Utf8String &player, const Utf8String &name);
    void Do_Player_Disable_Unit_Construction(const Utf8String &player);
    void Do_Player_Enable_Base_Construction(const Utf8String &player);
    void Do_Player_Enable_Factories(const Utf8String &player, const Utf8String &object);
    void Do_Player_Repair_Structure(const Utf8String &player, const Utf8String &structure);
    void Do_Player_Enable_Unit_Construction(const Utf8String &player);
    void Do_Camera_Move_Home();
    void Do_Build_Team(const Utf8String &team);
    void Do_Recruit_Team(const Utf8String &team, float radius);
    void Do_Named_Damage(const Utf8String &name, int damage);
    void Do_Named_Delete(const Utf8String &name);
    void Do_Team_Delete(const Utf8String &team, bool ignore_dead);
    void Do_Team_Wander(const Utf8String &team, const Utf8String &waypoint);
    void Do_Team_Increase_Priority(const Utf8String &team);
    void Do_Team_Decrease_Priority(const Utf8String &team);
    void Do_Team_Wander_In_Place(const Utf8String &team);
    void Do_Team_Panic(const Utf8String &team, const Utf8String &waypoint);
    void Do_Named_Kill(const Utf8String &name);
    void Do_Team_Kill(const Utf8String &team);
    void Do_Player_Kill(const Utf8String &player);
    void Do_Display_Text(const Utf8String &text);
    void Do_In_Game_Popup_Message(const Utf8String &message, int i1, int i2, int i3, bool b);
    void Do_Display_Cinematic_Text(const Utf8String &text, const Utf8String &font, int frames);
    void Do_Cameo_Flash(const Utf8String &cameo, int time);
    void Do_Named_Custom_Color(const Utf8String &name, int color);
    void Do_Named_Flash(const Utf8String &name, int time, const RGBColor *color);
    void Do_Team_Flash(const Utf8String &team, int time, const RGBColor *color);
    void Do_Movie_Play_FullScreen(const Utf8String &movie);
    void Do_Movie_Play_Radar(const Utf8String &movie);
    void Do_Sound_Play_From_Named(const Utf8String &sound, const Utf8String &unit);
    void Do_Speech_Play(const Utf8String &speech);
    void Do_Player_Transfer_Assets_To_Player(const Utf8String &source, const Utf8String &destination);
    void Do_Named_Transfer_Assets_To_Player(const Utf8String &unit, const Utf8String &player);
    void Exclude_Player_From_Score_Screen(const Utf8String &player);
    void Enable_Scoring(bool enable);
    void Update_Player_Relation_Toward_Player(const Utf8String &source, int relation, const Utf8String &destination);
    void Do_Radar_Create_Event(Coord3D *pos, int event);
    void Do_Object_Radar_Create_Event(const Utf8String &name, int event);
    void Do_Team_Radar_Create_Event(const Utf8String &team, int event);
    void Do_Radar_Disable();
    void Do_Radar_Enable();
    void Do_Camera_Motion_Blur_Jump(const Utf8String &waypoint, bool saturate);
    void Do_Camera_Motion_Blur(bool zoom_in, bool saturate);
    void Do_Reveal_Map_At_Waypoint(const Utf8String &waypoint, float radius, const Utf8String &str);
    void Do_Shroud_Map_At_Waypoint(const Utf8String &waypoint, float radius, const Utf8String &str);
    void Do_Reveal_Map_Entire(const Utf8String &player);
    void Do_Reveal_Map_EntirePermanently(bool reveal, const Utf8String &player);
    void Do_Shroud_Map_Entire(const Utf8String &player);
    void Do_Team_Available_For_Recruitment(const Utf8String &team, bool availability);
    void Do_Collect_Nearby_For_Team(const Utf8String &team);
    void Do_Merge_Team_Into_Team(const Utf8String &source, const Utf8String &desination);
    void Do_Disable_Input();
    void Do_Enable_Input();
    void Do_Set_Border_Shroud(bool shroud);
    void Do_Idle_All_Player_Units(const Utf8String &player);
    void Do_Resume_Supply_Trucking_For_Idle_Units(const Utf8String &player);
    void Do_Ambient_Sounds_Pause(bool pausing);
    void Do_Music_Track_Change(const Utf8String &music, bool fadeout, bool fadein);
    void Do_Team_Garrison_Specific_Building(const Utf8String &team, const Utf8String &building);
    void Do_Exit_Specific_Building(const Utf8String &building);
    void Do_Team_Garrison_Nearest_Building(const Utf8String &team);
    void Do_Team_Exit_All_Buildings(const Utf8String &team);
    void Do_Unit_Garrison_Specific_Building(const Utf8String &unit, const Utf8String &building);
    void Do_Unit_Garrison_Nearest_Building(const Utf8String &unit);
    void Do_Named_Enable_Stealth(const Utf8String &name, bool enable);
    void Do_Team_Enable_Stealth(const Utf8String &team, bool enable);
    void Do_Named_Set_Unmanned(const Utf8String &name);
    void Do_Team_Set_Unmanned(const Utf8String &team);
    void Do_Named_Set_Boobytrapped(const Utf8String &trap, const Utf8String &name);
    void Do_Team_Set_Boobytrapped(const Utf8String &trap, const Utf8String &team);
    void Do_Unit_Exit_Building(const Utf8String &unit);
    void Do_Player_Garrison_All_Buildings(const Utf8String &player);
    void Do_Player_Exit_All_Buildings(const Utf8String &player);
    void Do_Letter_Box_Mode(bool start_letter_box);
    void Do_Black_White_Mode(bool start_bw_mode, int frames);
    void Do_Sky_Box(bool start_draw);
    void Do_Weather(bool snow_visible);
    void Do_Freeze_Time();
    void Do_Unfreeze_Time();
    void Do_Military_Caption(const Utf8String &briefing, int duration);
    void Do_Camera_Set_Audible_Distance(float distance);
    void Do_Set_Stopping_Distance(const Utf8String &team, float distance);
    void Do_Named_Set_Held(const Utf8String &name, bool held);
    void Do_Named_Set_StoppingDistance(const Utf8String &name, float distance);
    void Do_Disable_Special_Power_Display();
    void Do_Enable_Special_Power_Display();
    void Do_Named_Hide_Special_Power_Display(const Utf8String &name);
    void Do_Named_Show_Special_Power_Display(const Utf8String &name);
    void Do_Audio_Set_Volume(AudioAffect effect, float new_volume);
    void Do_Transfer_Team_To_Player(const Utf8String &team, const Utf8String &player);
    void Do_Set_Money(const Utf8String &player, int money);
    void Do_Give_Money(const Utf8String &player, int money);
    void Do_Display_Counter(const Utf8String &counter, const Utf8String &text);
    void Do_Hide_Counter(const Utf8String &counter);
    void Do_Display_Countdown_Timer(const Utf8String &timer, const Utf8String &text);
    void Do_Hide_Countdown_Timer(const Utf8String &timer);
    void Do_Disable_Countdown_Timer_Display();
    void Do_Enable_Countdown_Timer_Display();
    void Do_Named_Stop_Special_Power_Countdown(const Utf8String &name, const Utf8String &power, bool stop);
    void Do_Named_Set_Special_Power_Countdown(const Utf8String &name, const Utf8String &power, int seconds);
    void Do_Named_Add_Special_Power_Countdown(const Utf8String &name, const Utf8String &power, int seconds);
    void Do_Named_Fire_Special_Power_At_Waypoint(
        const Utf8String &name, const Utf8String &power, const Utf8String &waypoint);
    void Do_Skirmish_Fire_Special_Power_At_Most_Cost(const Utf8String &player, const Utf8String &power);
    void Do_Named_Fire_Special_Power_At_Named(const Utf8String &name, const Utf8String &waypoint, const Utf8String &target);
    void Do_Named_Use_Command_Button_Ability(const Utf8String &name, const Utf8String &ability);
    void Do_Named_Use_Command_Button_Ability_On_Named(
        const Utf8String &name, const Utf8String &ability, const Utf8String &target);
    void Do_Named_Use_Command_Button_Ability_At_Waypoint(
        const Utf8String &name, const Utf8String &ability, const Utf8String &waypoint);
    void Do_Named_Use_Command_Button_Ability_Using_Waypoint_Path(
        const Utf8String &name, const Utf8String &ability, const Utf8String &waypoint);
    void Do_Team_Use_Command_Button_Ability(const Utf8String &team, const Utf8String &ability);
    void Do_Team_Use_Command_Button_Ability_On_Named(
        const Utf8String &team, const Utf8String &ability, const Utf8String &name);
    void Do_Team_Use_Command_Button_Ability_At_Waypoint(
        const Utf8String &team, const Utf8String &ability, const Utf8String &waypoint);
    void Do_Radar_Refresh();
    void Do_Camera_Tether_Named(const Utf8String &unit, bool snap_to_unit, float play);
    void Do_Camera_Stop_Tether_Named();
    void Do_Camera_Set_Default(float pitch, float angle, float max_height);
    void Do_Named_Stop(const Utf8String &name);
    void Do_Team_Stop(const Utf8String &team, bool disband);
    void Do_Team_Set_Override_Relation_To_Team(const Utf8String &team, const Utf8String &other, int relation);
    void Do_Team_Remove_Override_Relation_To_Team(const Utf8String &team, const Utf8String &other);
    void Do_Player_Set_Override_Relation_To_Team(const Utf8String &player, const Utf8String &other, int relation);
    void Do_Player_Remove_Override_Relation_To_Team(const Utf8String &player, const Utf8String &team);
    void Do_Team_Set_Override_Relation_To_Player(const Utf8String &team, const Utf8String &player, int relation);
    void Do_Team_Remove_Override_Relation_To_Player(const Utf8String &team, const Utf8String &player);
    void Do_Team_Remove_All_Override_Relations(const Utf8String &team);
    void Do_Unit_Start_Sequential_Script(const Utf8String &unit, const Utf8String &script, int loop);
    void Do_Unit_Stop_Sequential_Script(const Utf8String &unit);
    void Do_Named_Fire_Weapon_Following_Waypoint_Path(const Utf8String &name, const Utf8String &waypoint);
    void Do_Team_Start_Sequential_Script(const Utf8String &team, const Utf8String &script, int loop);
    void Do_Team_Stop_Sequential_Script(const Utf8String &team);
    void Do_Unit_Guard_For_Framecount(const Utf8String &name, int count);
    void Do_Unit_Idle_For_Framecount(const Utf8String &name, int count);
    void Do_Team_Guard_For_Framecount(const Utf8String &team, int count);
    void Do_Team_Idle_For_Framecount(const Utf8String &team, int count);
    void Do_Water_Change_Height(const Utf8String &water, float height);
    void Do_Water_Change_Height_Over_Time(const Utf8String &water, float height, float time, float damage);
    void Do_Border_Switch(int border);
    void Do_Force_Object_Selection(
        const Utf8String &team, const Utf8String &object, bool center_in_view, const Utf8String &audio);
    void Do_Destroy_All_Contained(const Utf8String &unit, int damage_type);
    void Do_Radar_Force_Enable();
    void Do_Radar_Revert_Normal();
    void Do_Screen_Shake(unsigned int shake);
    void Do_Modify_Buildable_Status(const Utf8String &name, int status);
    void Do_Set_Cave_Index(const Utf8String &name, int index);
    void Do_Set_Warehouse_Value(const Utf8String &warehouse, int value);
    void Do_Sound_Enable_Type(const Utf8String &sound, bool enable);
    void Do_Sound_Remove_All_Disabled();
    void Do_Sound_Remove_Type(const Utf8String &sound);
    void Do_Sound_Override_Volume(const Utf8String &sound, float volume);
    void Do_Set_Topple_Direction(const Utf8String &name, const Coord3D *direction);
    void Do_Move_Unit_Towards_Nearest(const Utf8String &name, const Utf8String &nearest, Utf8String trigger_area);
    void Do_Move_Team_Towards_Nearest(const Utf8String &team, const Utf8String &nearest, Utf8String trigger_area);
    void Do_Unit_Receive_Upgrade(const Utf8String &unit, const Utf8String &upgrade);
    void Do_Skirmish_Attack_Nearest_Group_With_Value(const Utf8String &team, int i1, int i2);
    void Do_Skirmish_Command_Button_On_Most_Valuable(const Utf8String &team, const Utf8String &button, float range, bool b);
    void Do_Team_Spin_For_Framecount(const Utf8String &team, int count);
    void Do_Team_Use_Command_Button_On_Named(const Utf8String &team, const Utf8String &button, const Utf8String &named);
    void Do_Team_Use_Command_Button_On_Nearest_Enemy(const Utf8String &team, const Utf8String &button);
    void Do_Team_Use_Command_Button_On_Nearest_Garrisoned_Building(const Utf8String &team, const Utf8String &button);
    void Do_Team_Use_Command_Button_On_Nearest_Kindof(const Utf8String &team, const Utf8String &button, int kind_of);
    void Do_Team_Use_Command_Button_On_Nearest_Building(const Utf8String &team, const Utf8String &button);
    void Do_Team_Use_Command_Button_On_Nearest_Building_Class(
        const Utf8String &team, const Utf8String &button, int building);
    void Do_Team_Use_Command_Button_On_Nearest_Object_Type(
        const Utf8String &team, const Utf8String &button, const Utf8String &name);
    void Do_Team_Partial_Use_Command_Button(float, const Utf8String &team, const Utf8String &button);
    void Do_Team_Capture_Nearest_Unowned_Faction_Unit(const Utf8String &team);
    void Do_Create_Team_From_Captured_Units(const Utf8String &name, const Utf8String &team);
    void Do_Player_Add_Skill_Points(const Utf8String &player, int points);
    void Do_Player_Add_Rank_Levels(const Utf8String &player, int levels);
    void Do_Player_Set_Rank_Level(const Utf8String &player, int level);
    void Do_Map_Set_Rank_Level_Limit(int level);
    void Do_Player_Grant_Science(const Utf8String &player, const Utf8String &science);
    void Do_Player_Purchase_Science(const Utf8String &player, const Utf8String &science);
    void Do_Player_Set_Science_Availability(
        const Utf8String &player, const Utf8String &science, const Utf8String &availability);
    void Do_Team_Emoticon(const Utf8String &team, const Utf8String &icon, float time);
    void Do_Named_Emoticon(const Utf8String &name, const Utf8String &icon, float time);
    void Do_Object_Type_List_Maintenance(const Utf8String &list, const Utf8String &type, bool add_type);
    void Do_Reveal_Map_At_Waypoint_Permanent(
        const Utf8String &waypoint, float radius, const Utf8String &player, const Utf8String &reveal);
    void Do_Undo_Reveal_Map_At_Waypoint_Permanent(const Utf8String &reveal);
    void Do_Eva_Enabled_Disabled(bool enabled);
    void Do_Set_Occlusion_Mode(bool mode);
    void Do_Set_Draw_Icon_UI_Mode(bool mode);
    void Do_Set_Dynamic_LOD_Mode(bool mode);
    void Do_Override_Hulk_Lifetime(float lifetime);
    void Do_C3_Camera_Enable_Slave_Mode(const Utf8String &str1, const Utf8String &str2);
    void Do_C3_Camera_Disable_Slave_Mode();
    void Do_C3_Camera_Shake(const Utf8String &waypoint, float f1, float f2, float f3);
    void Do_Named_Face_Named(const Utf8String &name, const Utf8String &face);
    void Do_Named_Face_Waypoint(const Utf8String &name, const Utf8String &waypoint);
    void Do_Team_Face_Named(const Utf8String &team, const Utf8String &face);
    void Do_Team_Face_Waypoint(const Utf8String &team, const Utf8String &waypoint);
    void Do_Affect_Object_Panel_Flags_Unit(const Utf8String &name, const Utf8String &flag, bool affect);
    void Do_Affect_Object_Panel_Flags_Team(const Utf8String &team, const Utf8String &flag, bool affect);
    void Do_Affect_Player_Skillset(const Utf8String &player, int skill_set);
    void Do_Guard_Supply_Center(const Utf8String &team, int center);
    void Do_Team_Guard_In_Tunnel_Network(const Utf8String &team);
    void Do_Remove_Command_Bar_Button(const Utf8String &button, const Utf8String &name);
    void Do_Add_Command_Bar_Button(const Utf8String &button, const Utf8String &name, int i);
    void Do_Affect_Skill_Points_Modifier(const Utf8String &player, float modifier);
    void Do_Resize_View_Guardband(float x, float y);
    void Delete_All_Unmanned();
    void Do_Choose_Victim_Always_Uses_Normal(bool always_uses_normal);
    void Do_Named_Set_Train_Held(const Utf8String &name, bool held);
};
