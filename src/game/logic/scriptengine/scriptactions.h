/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Script engine action processor.
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
#include "audiomanager.h"
#include "coord.h"
#include "subsysteminterface.h"

#ifdef GAME_DLL
#include "hooker.h"
#endif

class Object;
class RGBColor;
class ScriptAction;
class Team;

class ScriptActionsInterface : public SubsystemInterface
{
public:
    ScriptActionsInterface() {}
    virtual ~ScriptActionsInterface() override {}
    virtual void Execute_Action(ScriptAction *action) = 0;
    virtual void Close_Windows(bool close) = 0;
    virtual void Do_Enable_Or_Disable_Object_Difficulty_Bonuses(bool enable) = 0;
};

class ScriptActions : public ScriptActionsInterface
{
public:
#ifdef GAME_DLL
    ScriptActions *Hook_Ctor() { return new (this) ScriptActions(); }
    void Hook_Dtor() { ScriptActions::~ScriptActions(); }
#endif

    ScriptActions();
    virtual ~ScriptActions() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override {}
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

private:
    bool m_windowsClosed;
    Utf8String m_name;
};

#ifdef GAME_DLL
extern ScriptActionsInterface *&g_theScriptActions;
#else
extern ScriptActionsInterface *g_theScriptActions;
#endif
