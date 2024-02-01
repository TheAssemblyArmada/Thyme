/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief In Game UI
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
#include "bitflags.h"
#include "buildassistant.h"
#include "color.h"
#include "coord.h"
#include "drawable.h"
#include "gamewindow.h"
#include "mouse.h"
#include "object.h"
#include "radiusdecal.h"
#include "snapshot.h"
#include "subsysteminterface.h"
#include <list>
#include <map>

class CommandButton;
class Drawable;
class VideoBuffer;
class VideoStream;
class GameWindow;
class WindowLayout;
class Shadow;
class GameMessage;

enum RadiusCursorType
{
    RADIUS_CURSOR_NONE,
    RADIUS_CURSOR_ATTACK_DAMAGE_AREA,
    RADIUS_CURSOR_ATTACK_SCATTER_AREA,
    RADIUS_CURSOR_ATTACK_CONTINUE_AREA,
    RADIUS_CURSOR_GUARD_AREA,
    RADIUS_CURSOR_EMERGENCY_REPAIR,
    RADIUS_CURSOR_FRIENDLY_SPECIALPOWER,
    RADIUS_CURSOR_OFFENSIVE_SPECIALPOWER,
    RADIUS_CURSOR_SUPERWEAPON_SCATTER_AREA,
    RADIUS_CURSOR_PARTICLECANNON,
    RADIUS_CURSOR_A10STRIKE,
    RADIUS_CURSOR_CARPETBOMB,
    RADIUS_CURSOR_DAISYCUTTER,
    RADIUS_CURSOR_PARADROP,
    RADIUS_CURSOR_SPYSATELLITE,
    RADIUS_CURSOR_SPECTREGUNSHIP,
    RADIUS_CURSOR_HELIX_NAPALM_BOMB,
    RADIUS_CURSOR_NUCLEARMISSILE,
    RADIUS_CURSOR_EMPPULSE,
    RADIUS_CURSOR_ARTILLERYBARRAGE,
    RADIUS_CURSOR_NAPALMSTRIKE,
    RADIUS_CURSOR_CLUSTERMINES,
    RADIUS_CURSOR_SCUDSTORM,
    RADIUS_CURSOR_ANTHRAXBOMB,
    RADIUS_CURSOR_AMBUSH,
    RADIUS_CURSOR_RADAR,
    RADIUS_CURSOR_SPYDRONE,
    RADIUS_CURSOR_FRENZY,
    RADIUS_CURSOR_CLEARMINES,
    RADIUS_CURSOR_AMBULANCE,
};

enum
{
    MAX_BUILD_PROGRESS = 64,
};

struct BuildProgress
{
    ThingTemplate *m_thingTemplate;
    float m_percentComplete;
    GameWindow *m_control;
};

enum WorldAnimationOptions
{
    WORLD_ANIMATION_UNK2 = 0,
    WORLD_ANIMATION_UNK = 1,
};

class SuperweaponInfo : public MemoryPoolObject
{
    IMPLEMENT_POOL(SuperweaponInfo);

public:
    SuperweaponInfo(ObjectID object_id,
        unsigned int countdown_seconds,
        bool hidden,
        bool is_missing_science,
        bool is_ready,
        bool has_played_sound,
        const Utf8String &font_name,
        int font_size,
        bool font_bold,
        int color,
        const SpecialPowerTemplate *power_template);
    ~SuperweaponInfo();
    void Set_Font(const Utf8String &name, int size, bool bold);
    void Set_Text(Utf16String &name, Utf16String &time);
    void Draw_Name(int x, int y, int color, int border_color);
    void Draw_Time(int x, int y, int color, int border_color);
    float Get_Height() const;

    const SpecialPowerTemplate *Get_Special_Power_Template() const { return m_powerTemplate; }

private:
    DisplayString *m_name;
    DisplayString *m_time;
    int m_color;
    const SpecialPowerTemplate *m_powerTemplate;
    Utf8String m_unk;
    ObjectID m_objectID;
    unsigned int m_countdownSeconds;
    bool m_hidden;
    bool m_isMissingScience;
    bool m_isReady;
    bool m_hasPlayedSound;
    bool m_refreshCountdown;
    friend class InGameUI;
};

class FloatingTextData : public MemoryPoolObject
{
    IMPLEMENT_POOL(FloatingTextData);

public:
    FloatingTextData();
    ~FloatingTextData();

private:
    int m_color;
    Utf16String m_text;
    DisplayString *m_dString;
    Coord3D m_pos3D;
    int m_frameTimeOut;
    int m_frameCount;
    friend class InGameUI;
};

class WorldAnimationData
{
public:
    WorldAnimationData();

private:
    Anim2D *m_anim;
    Coord3D m_pos;
    int m_frame;
    WorldAnimationOptions m_options;
    float m_zRise;
    friend class InGameUI;
};

class PopupMessageData : public MemoryPoolObject
{
    IMPLEMENT_POOL(PopupMessageData);

private:
    Utf16String m_message;
    int m_xPos;
    int m_yPos;
    int m_width;
    int m_color;
    bool m_pause;
    bool m_pauseMusic;
    WindowLayout *m_windowLayout;
    friend class InGameUI;
};

class NamedTimerInfo : public MemoryPoolObject
{
    IMPLEMENT_POOL(NamedTimerInfo);

private:
    Utf8String m_timerName;
    Utf16String m_timerText;
    DisplayString *m_displayString;
    unsigned int m_timestamp;
    int m_color;
    bool m_countdown;
    friend class InGameUI;
};

class InGameUI : public SubsystemInterface, public SnapShot
{
public:
    struct MilitarySubtitleData
    {
        Utf16String subtitle;
        unsigned int index;
        ICoord2D position;
        DisplayString *display_strings[4];
        unsigned int current_display_string;
        unsigned int lifetime;
        bool block_drawn;
        unsigned int block_begin_frame;
        ICoord2D block_pos;
        unsigned int increment_on_frame;
        int color;
    };

    enum MouseMode
    {
        MOUSEMODE_DEFAULT,
        MOUSEMODE_BUILD_PLACE,
        MOUSEMODE_GUI_COMMAND,
        MOUSEMODE_MAX,
    };

    enum HintType
    {
        MOVE_HINT,
        ATTACK_HINT,
        NUM_HINT_TYPES,
    };

    enum SelectionRules
    {
        SELECTION_ANY,
        SELECTION_ALL,
    };

    enum ActionType
    {
        ACTIONTYPE_NONE,
        ACTIONTYPE_ATTACK_OBJECT,
        ACTIONTYPE_GET_REPAIRED_AT,
        ACTIONTYPE_DOCK_AT,
        ACTIONTYPE_GET_HEALED_AT,
        ACTIONTYPE_REPAIR_OBJECT,
        ACTIONTYPE_RESUME_CONSTRUCTION,
        ACTIONTYPE_ENTER_OBJECT,
        ACTIONTYPE_HIJACK_VEHICLE,
        ACTIONTYPE_CONVERT_OBJECT_TO_CARBOMB,
        ACTIONTYPE_CAPTURE_BUILDING_VIA_HACKING,
        ACTIONTYPE_DISABLE_VEHICLE_VIA_HACKING,
        ACTIONTYPE_STEAL_CASH_VIA_HACKING,
        ACTIONTYPE_DISABLE_BUILDING_VIA_HACKING,
        ACTIONTYPE_MAKE_OBJECT_DEFECTOR,
        ACTIONTYPE_SET_RALLY_POINT,
        ACTIONTYPE_COMBAT_DROP,
        ACTIONTYPE_SABOTAGE_BUILDING,
        NUM_ACTIONTYPES,
    };

    enum
    {
        MAX_MOVE_HINTS = 256,
    };

    enum
    {
        MAX_UI_MESSAGES = 6,
    };

    InGameUI();
    virtual ~InGameUI() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;
    virtual void Draw() override = 0;
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;
    virtual void Popup_Message(
        const Utf8String &message, int width_percent, int height_percent, int width, bool pause, bool pause_music);
    virtual void Popup_Message(const Utf8String &message,
        int width_percent,
        int height_percent,
        int width,
        int color,
        bool pause,
        bool pause_music);
    virtual void Message_Color(const RGBColor *color, Utf16String message, ...);
    virtual void Message(Utf16String message, ...);
    virtual void Message(Utf8String message, ...);
    virtual void Toggle_Messages() { m_messagesOn = !m_messagesOn; }
    virtual bool Is_Messages_On() { return m_messagesOn; }
    virtual void Military_Subtitle(const Utf8String &subtitle, int duration);
    virtual void Remove_Military_Subtitle();
    virtual void Display_Cant_Build_Message(LegalBuildCode code);
    virtual void Begin_Area_Select_Hint(const GameMessage *msg);
    virtual void End_Area_Select_Hint(const GameMessage *msg);
    virtual void Create_Move_Hint(const GameMessage *msg);
    virtual void Create_Attack_Hint(const GameMessage *msg);
    virtual void Create_Force_Attack_Hint(const GameMessage *msg);
    virtual void Create_Mouseover_Hint(const GameMessage *msg);
    virtual void Create_Command_Hint(const GameMessage *msg);
    virtual void Create_Garrison_Hint(const GameMessage *msg);
    virtual void Add_Superweapon(
        int player_index, const Utf8String &power_name, ObjectID id, const SpecialPowerTemplate *power_template);
    virtual bool Remove_Superweapon(
        int player_index, const Utf8String &power_name, ObjectID id, const SpecialPowerTemplate *power_template);
    virtual void Object_Changed_Team(const Object *obj, int old_player_index, int new_player_index);
    virtual void Set_Superweapon_Display_Enabled_By_Script(bool enable);
    virtual bool Get_Superweapon_Display_Enabled_By_Script();
    virtual void Hide_Object_Superweapon_Display_By_Script(const Object *obj);
    virtual void Show_Object_Superweapon_Display_By_Script(const Object *obj);
    virtual void Set_Scrolling(bool is_scrolling);
    virtual bool Is_Scrolling();
    virtual void Set_Selecting(bool is_selecting);
    virtual bool Is_Selecting();
    virtual void Set_Scroll_Amount(Coord2D amt);
    virtual Coord2D Get_Scroll_Amount();
    virtual void Set_GUI_Command(const CommandButton *command);
    virtual const CommandButton *Get_GUI_Command();
    virtual void Place_Build_Available(const ThingTemplate *build, Drawable *build_drawable);
    virtual const ThingTemplate *Get_Pending_Place_Type();
    virtual ObjectID Get_Pending_Place_Source_Object_ID();
#ifndef GAME_DEBUG_STRUCTS
    virtual bool Get_Prevent_Left_Click_Deselection_In_Alternate_Mouse_Mode_For_One_Click();
    virtual void Set_Prevent_Left_Click_Deselection_In_Alternate_Mouse_Mode_For_One_Click(bool prevent);
#endif
    virtual void Set_Placement_Start(const ICoord2D *start);
    virtual void Set_Placement_End(const ICoord2D *end);
    virtual bool Is_Placement_Anchored();
    virtual void Get_Placement_Points(ICoord2D *start, ICoord2D *end);
    virtual float Get_Placement_Angle();
    virtual void Select_Drawable(Drawable *drawable);
    virtual void Deselect_Drawable(Drawable *drawable);
    virtual void Deselect_All_Drawables(bool post_msg);
    virtual int Get_Select_Count() { return m_selectCount; }
    virtual int Get_Max_Select_Count() { return m_maxSelectCount; }
    virtual unsigned int Get_Frame_Selection_Changed() { return m_frameSelectionChanged; }
    virtual const std::list<Drawable *> *Get_All_Selected_Drawables();
    virtual const std::list<Drawable *> *Get_All_Selected_Local_Drawables();
    virtual Drawable *Get_First_Selected_Drawable();
    virtual DrawableID Get_Solo_Nexus_Selected_Drawable_ID() { return m_soloNexusSelectedDrawableID; }
    virtual bool Is_Drawable_Selected(DrawableID id_to_check);
    virtual bool Is_Any_Selected_KindOf(KindOfType kindof);
    virtual bool Is_All_Selected_KindOf(KindOfType kindof);
    virtual void Set_Radius_Cursor(
        RadiusCursorType cursor, const SpecialPowerTemplate *power_template, WeaponSlotType weapon_slot);
    virtual void Set_Radius_Cursor_None() { Set_Radius_Cursor(RADIUS_CURSOR_NONE, nullptr, WEAPONSLOT_PRIMARY); }
    virtual void Set_Input_Enabled(bool enable);
    virtual bool Get_Input_Enabled() { return m_inputEnabled; }
    virtual void Disregard_Drawable(Drawable *draw);
    virtual void Pre_Draw();
    virtual void Post_Draw();
    virtual void Play_Movie(const Utf8String &movie_name);
    virtual void Stop_Movie();
    virtual VideoBuffer *Video_Buffer();
    virtual void Play_Cameo_Movie(const Utf8String &movie_name);
    virtual void Stop_Cameo_Movie();
    virtual VideoBuffer *Cameo_Video_Buffer();
    virtual DrawableID Get_Moused_Over_Drawable_ID();
    virtual void Set_Quit_Menu_Visible(bool visible) { m_isQuitMenuVisible = visible; }
    virtual bool Is_Quit_Menu_Visible() { return m_isQuitMenuVisible; }
    virtual const FieldParse *Get_Field_Parse() { return s_fieldParseTable; }
    virtual int Select_Units_Matching_Current_Selection();
    virtual int Select_Matching_Across_Screen();
    virtual int Select_Matching_Across_Map();
    virtual int Select_Matching_Across_Region(IRegion2D *region);
    virtual int Select_All_Units_By_Type(BitFlags<KINDOF_COUNT> must_be_set, BitFlags<KINDOF_COUNT> must_be_clear);
    virtual int Select_All_Units_By_Type_Across_Screen(
        BitFlags<KINDOF_COUNT> must_be_set, BitFlags<KINDOF_COUNT> must_be_clear);
    virtual int Select_All_Units_By_Type_Across_Map(
        BitFlags<KINDOF_COUNT> must_be_set, BitFlags<KINDOF_COUNT> must_be_clear);
    virtual int Select_All_Units_By_Type_Across_Region(
        IRegion2D *region, BitFlags<KINDOF_COUNT> must_be_set, BitFlags<KINDOF_COUNT> must_be_clear);
    virtual void Build_Region(const ICoord2D *anchor, const ICoord2D *dest, IRegion2D *region);
    virtual bool Get_Displayed_Max_Warning() { return m_displayedMaxWarning; }
    virtual void Set_Displayed_Max_Warning(bool selected) { m_displayedMaxWarning = selected; }
    virtual void Add_Floating_Text(const Utf16String &text, const Coord3D *pos, int color);
    virtual void Add_Idle_Worker(Object *obj);
    virtual void Remove_Idle_Worker(Object *obj, int slot);
    virtual void Select_Next_Idle_Worker();
    virtual void Recreate_Control_Bar();
    virtual void Disable_Tooltips_Until(unsigned int frame);
    virtual void Clear_Tooltips_Disabled();
    virtual bool Are_Tooltips_Disabled();
    virtual int Get_Idle_Worker_Count();
    virtual Object *Find_Idle_Worker(Object *obj);
    virtual void Show_Idle_Worker_Layout();
    virtual void Hide_Idle_Worker_Layout();
    virtual void Update_Idle_Worker();
    virtual void Reset_Idle_Worker();
#ifdef GAME_DEBUG_STRUCTS
    virtual void Debug_Add_Floating_Text(const Utf8String &text, const Coord3D *pos, int color);
#endif
    virtual View *Create_View() = 0;

    void Add_World_Animation(
        Anim2DTemplate *anim, const Coord3D *pos, WorldAnimationOptions options, float time, float z_rise);
    void Set_Mouse_Cursor(MouseCursor cursor);
    SuperweaponInfo *Find_SW_Info(
        int player_index, const Utf8String &power_name, ObjectID id, const SpecialPowerTemplate *power_template);
    void Add_Named_Timer(const Utf8String &timer_name, const Utf16String &text, bool countdown);
    void Remove_Named_Timer(const Utf8String &timer_name);
    void Show_Named_Timer_Display(bool show);
    void Handle_Radius_Cursor();
    void Trigger_Double_Click_Attack_Move_Guard_Hint();
    void Evaluate_Solo_Nexus(Drawable *drawable);
    void Handle_Build_Placements();
    void Register_Window_Layout(WindowLayout *layout);
    void Unregister_Window_Layout(WindowLayout *layout);
    void Free_Message_Resources();
    void Add_Message_Text(const Utf16String &formatted_message, const RGBColor *rgb_color);
    void Remove_Message_At_Index(int index);
    void Destroy_Placement_Icons();
    void Expire_Hint(HintType type, unsigned int hint_index);
    void Create_Control_Bar();
    void Create_Replay_Control();
    bool Are_Selected_Objects_Controllable() const;
    void Reset_Camera();
    bool Can_Selected_Objects_Non_Attack_Interact_With_Object(const Object *object_to_interact_with, SelectionRules rule);
    CanAttackResult Get_Can_Selected_Objects_Attack(
        ActionType action, const Object *object_to_interact_with, SelectionRules rule, bool force_to_attack) const;
    bool Can_Selected_Objects_Do_Action(
        ActionType action, const Object *object_to_interact_with, SelectionRules rule, bool additional_checking) const;
    bool Can_Selected_Objects_Do_Special_Power(CommandButton *command,
        const Object *object_to_interact_with,
        Coord3D *position,
        SelectionRules rule,
        unsigned int options,
        Object *custom_object) const;
    bool Can_Selected_Objects_Override_Special_Power_Destination(
        const Coord3D *pos, SelectionRules rule, SpecialPowerType type) const;
    bool Can_Selected_Objects_Effectively_Use_Weapon(
        CommandButton *command, const Object *object_to_interact_with, Coord3D *position, SelectionRules rule) const;
    void Update_Floating_Text();
    void Draw_Floating_Text();
    void Clear_Floating_Text();
    void Clear_Popup_Message_Data();
    void Clear_World_Animations();
    void Update_And_Draw_World_Animations();

    static void Parse_In_Game_UI_Definition(INI *ini);

    bool Get_Drawable_Caption_Bold() const { return m_drawableCaptionBold; }
    int Get_Drawable_Caption_Size() const { return m_drawableCaptionPointSize; }
    Utf8String Get_Drawable_Caption_Font() const { return m_drawableCaptionFont; }
    int Get_Drawable_Caption_Color() const { return m_drawableCaptionColor; }
    bool Get_Draw_RMB_Scroll_Anchor() const { return m_drawRMBScrollAnchor; }
    bool Get_Move_RMB_Scroll_Anchor() const { return m_moveRMBScrollAnchor; }
    const PopupMessageData *Get_Popup_Message_Data() const { return m_popupMessageData; }

    int Get_Message_Color(bool secondary) const
    {
        if (secondary) {
            return m_messageColor2;
        } else {
            return m_messageColor1;
        }
    }

    void Set_No_Radar_Edge_Sound(bool b) { m_noRadarEdgeSound = b; }
    void Set_Prefer_Selection(bool b) { m_preferSelection = b; }
    void Set_Waypoint_Mode(bool b) { m_waypointMode = b; }
    void Set_Force_To_Move_Mode(bool b) { m_forceToMoveMode = b; }
    void Set_Force_To_Attack_Mode(bool b) { m_forceToAttackMode = b; }
    void Toggle_Attack_Move_To_Mode() { m_attackMoveToMode = !m_attackMoveToMode; }
    void Clear_Attack_Move_To_Mode() { m_attackMoveToMode = false; }
    void Set_Camera_Rotating_Left(bool b) { m_cameraRotateLeft = b; }
    void Set_Camera_Rotating_Right(bool b) { m_cameraRotateRight = b; }
    void Set_Camera_Zooming_In(bool b) { m_cameraZoomIn = b; }
    void Set_Camera_Zooming_Out(bool b) { m_cameraZoomOut = b; }
    void Set_Camera_Drawing_Trackable(bool b) { m_cameraDrawingTrackable = b; }
    void Increment_Select_Count() { m_selectCount++; }
    void Decrement_Select_Count() { m_selectCount--; }
    void Set_Draw_RMB_Scroll_Anchor(bool set) { m_drawRMBScrollAnchor = set; }
    void Set_Move_RMB_Scroll_Anchor(bool set) { m_moveRMBScrollAnchor = set; }

    bool Is_No_Radar_Edge_Sound() const { return m_noRadarEdgeSound; }
    bool Is_Prefer_Selection() const { return m_preferSelection; }
    bool Is_In_Waypoint_Mode() const { return m_waypointMode; }
    bool Is_In_Force_To_Attack_Mode() const { return m_forceToAttackMode; }
    bool Is_In_Force_To_Move_Mode() const { return m_forceToMoveMode; }
    bool Is_In_Attack_Move_To_Mode() const { return m_attackMoveToMode; }
    bool Is_Camera_Rotating_Left() const { return m_cameraRotateLeft; }
    bool Is_Camera_Rotating_Right() const { return m_cameraRotateRight; }
    bool Is_Camera_Zooming_In() const { return m_cameraZoomIn; }
    bool Is_Camera_Zooming_Out() const { return m_cameraZoomOut; }
    bool Is_Camera_Tracking_Drawable() const { return m_cameraDrawingTrackable; }

protected:
    struct MoveHintStruct
    {
        Coord3D pos;
        unsigned int source_id;
        unsigned int frame;
    };

    struct UIMessage
    {
        Utf16String full_text;
        DisplayString *display_string;
        unsigned int timestamp;
        int color;
    };

    bool m_superweaponDisplayEnabledByScript;
    bool m_inputEnabled;
    std::list<WindowLayout *> m_windowLayoutList; // not 100% identified yet
    Utf8String m_currentlyPlayingMovie;
    std::list<Drawable *> m_selectedDrawables; // not 100% identified yet
    std::list<Drawable *> m_selectedLocalDrawables; // not 100% identified yet
    bool m_isDragSelecting;
    IRegion2D m_dragSelectRegion;
    bool m_displayedMaxWarning;
    MoveHintStruct m_moveHint[MAX_MOVE_HINTS];
    int m_nextMoveHint;
    const CommandButton *m_pendingGUICommand;
    BuildProgress m_buildProgress[MAX_BUILD_PROGRESS];
    const ThingTemplate *m_pendingPlaceType;
    ObjectID m_pendingPlaceSourceObjectID;
#ifndef GAME_DEBUG_STRUCTS
    bool m_preventLeftClickDeselectionInAlternateMouseModeForOneClick; // not 100% identified yet
#endif
    Drawable **m_placeIcon;
    bool m_placeAnchorInProgress;
    ICoord2D m_placeAnchorStart;
    ICoord2D m_placeAnchorEnd;
    int m_selectCount;
    int m_maxSelectCount;
    int m_frameSelectionChanged; // not 100% identified yet
    int m_doubleClickCounter; // not 100% identified yet
    Coord3D m_radiusDecalPos; // not 100% identified yet
    VideoBuffer *m_videoBuffer;
    VideoStream *m_videoStream;
    VideoBuffer *m_cameoVideoBuffer;
    VideoStream *m_cameoVideoStream;
    UIMessage m_uiMessages[MAX_UI_MESSAGES];
    std::map<Utf8String, std::list<SuperweaponInfo *>> m_superweapons[MAX_PLAYER_COUNT];
    Coord2D m_superweaponPosition;
    float m_superweaponFlashDuration;
    Utf8String m_superweaponNormalFont;
    int m_superweaponNormalPointSize;
    bool m_superweaponNormalBold;
    Utf8String m_superweaponReadyFont;
    int m_superweaponReadyPointSize;
    bool m_superweaponReadyBold;
    int m_superweaponLastFlashFrame;
    int m_superweaponFlashColor;
    bool m_superweaponHidden;
    std::map<Utf8String, NamedTimerInfo *> m_namedTimers;
    Coord2D m_namedTimerPosition;
    float m_namedTimerFlashDuration;
    int m_namedTimerLastFlashFrame; // not 100% identified yet
    int m_namedTimerFlashColor;
    bool m_namedTimerUsedFlashColor; // not 100% identified yet
    bool m_showNamedTimers; // not 100% identified yet
    Utf8String m_namedTimerNormalFont;
    int m_namedTimerNormalPointSize;
    bool m_namedTimerNormalBold;
    int m_namedTimerNormalColor;
    Utf8String m_namedTimerReadyFont;
    int m_namedTimerReadyPointSize;
    bool m_namedTimerReadyBold;
    int m_namedTimerReadyColor;
    Utf8String m_drawableCaptionFont;
    int m_drawableCaptionPointSize;
    bool m_drawableCaptionBold;
    int m_drawableCaptionColor;
    unsigned int m_tooltipsDisabled; // not 100% identified yet
    MilitarySubtitleData *m_militarySubtitle;
    bool m_isScrolling;
    bool m_isSelecting;
    MouseMode m_mouseMode;
    MouseCursor m_mouseCursor; // not 100% identified yet
    DrawableID m_mousedOverDrawableID;
    Coord2D m_scrollAmt;
    bool m_isQuitMenuVisible;
    bool m_messagesOn;
    int m_messageColor1;
    int m_messageColor2;
    ICoord2D m_messagePosition;
    Utf8String m_messageFont;
    int m_messagePointSize;
    bool m_messageBold;
    int m_messageDelayMS;
    RGBAColorInt m_militaryCaptionColor;
    ICoord2D m_militaryCaptionPosition;
    Utf8String m_militaryCaptionTitleFont;
    int m_militaryCaptionTitlePointSize;
    bool m_militaryCaptionTitleBold;
    Utf8String m_militaryCaptionFont;
    int m_militaryCaptionPointSize;
    bool m_militaryCaptionBold;
    bool m_militaryCaptionRandomizeTyping;
    int m_militaryCaptionSpeed;
    RadiusDecalTemplate m_radiusDecalTemplate[30];
    RadiusDecal m_radiusDecal;
    RadiusCursorType m_radiusDecalType; // not 100% identified yet
    std::list<FloatingTextData *> m_floatingTextList;
    int m_floatingTextTimeOut;
    float m_floatingTextMoveUpSpeed;
    float m_floatingTextMoveVanishRate;
    PopupMessageData *m_popupMessageData;
    int m_popupMessageColor;
    bool m_waypointMode;
    bool m_forceToAttackMode; // not 100% identified yet
    bool m_forceToMoveMode; // not 100% identified yet
    bool m_attackMoveToMode; // not 100% identified yet
    bool m_preferSelection; // not 100% identified yet
    bool m_cameraRotateLeft; // not 100% identified yet
    bool m_cameraRotateRight; // not 100% identified yet
    bool m_cameraZoomIn; // not 100% identified yet
    bool m_cameraDrawingTrackable; // not 100% identified yet
    bool m_cameraZoomOut; // not 100% identified yet
    bool m_drawRMBScrollAnchor;
    bool m_moveRMBScrollAnchor;
    bool m_noRadarEdgeSound; // not 100% identified yet
    std::list<WorldAnimationData *> m_worldAnimations;
    std::list<Object *> m_idleWorkerLists[MAX_PLAYER_COUNT];
    GameWindow *m_idleWorkerWin;
    int m_idleWorkerCount; // not 100% identified yet
    DrawableID m_soloNexusSelectedDrawableID;

    static const FieldParse s_fieldParseTable[];
};

void Hide_Replay_Controls();
void Show_Replay_Controls();
void Toggle_Replay_Controls();

WindowMsgHandledType Idle_Worker_System(GameWindow *window, unsigned int message, unsigned int data_1, unsigned int data_2);

#ifdef GAME_DLL
extern InGameUI *&g_theInGameUI;
extern GameWindow *&g_replayWindow;
#else
extern InGameUI *g_theInGameUI;
extern GameWindow *g_replayWindow;
#endif
