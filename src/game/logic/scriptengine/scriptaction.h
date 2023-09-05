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
    ~ScriptAction() override;

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
