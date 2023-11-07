/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Control Bar
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
#include "audioeventrts.h"
#include "behaviormodule.h"
#include "gamewindow.h"
#include "ingameui.h"

class Player;
class PlayerTemplate;
class WindowVideoManager;
class AnimateWindowManager;
class ControlBarSchemeManager;

enum ControlBarContext
{
    CB_CONTEXT_NONE,
    CB_CONTEXT_COMMAND,
    CB_CONTEXT_STRUCTURE_INVENTORY,
    CB_CONTEXT_BEACON,
    CB_CONTEXT_UNDER_CONSTRUCTION,
    CB_CONTEXT_MULTI_SELECT,
    CB_CONTEXT_UNK,
    CB_CONTEXT_OBSERVER,
    CB_CONTEXT_OCL_TIMER,
    NUM_CB_CONTEXTS,
};

enum ControlBarStages
{
    CONTROL_BAR_STAGE_DEFAULT,
    CONTROL_BAR_STAGE_SQUISHED,
    CONTROL_BAR_STAGE_LOW,
    CONTROL_BAR_STAGE_HIDDEN,
    CONTROL_BAR_STAGE_MAX,
};

enum CBCommandStatus
{
    CBC_COMMAND_NOT_USED,
    CBC_COMMAND_USED,
};

enum CommandAvailability
{
    COMMAND_AVAILABILITY_DISABLED,
    COMMAND_AVAILABILITY_ENABLED,
    COMMAND_AVAILABILITY_ENABLED_AND_ACTIVE,
    COMMAND_AVAILABILITY_HIDDEN,
    COMMAND_AVAILABILITY_NOT_READY,
    COMMAND_AVAILABILITY_DISABLED_PERMANENTLY,
};

class CommandSet : public Overridable
{
    IMPLEMENT_POOL(CommandSet)

public:
    enum
    {
        MAX_COMMAND_BUTTONS = 18,
    };

    CommandSet(Utf8String const &name);
    virtual ~CommandSet() override;
    const CommandButton *Get_Command_Button(int button_index) const;
    void Friend_Add_To_List(CommandSet **next);

    Utf8String Get_Name() const { return m_name; }
    CommandSet *Get_Next_Command_Set() const { return m_nextCommandSet; }

    static FieldParse *Get_Field_Parse() { return s_commandSetFieldParseTable; }
    static void Parse_Command_Button(INI *ini, void *formal, void *store, const void *user_data);

private:
    Utf8String m_name;
    const CommandButton *m_command[MAX_COMMAND_BUTTONS];
    CommandSet *m_nextCommandSet;

    static FieldParse s_commandSetFieldParseTable[];
};

enum GUICommand
{
    GUI_COMMAND_NONE,
    GUI_COMMAND_DOZER_CONSTRUCT,
    GUI_COMMAND_DOZER_CONSTRUCT_CANCEL,
    GUI_COMMAND_UNIT_BUILD,
    GUI_COMMAND_CANCEL_UNIT_BUILD,
    GUI_COMMAND_PLAYER_UPGRADE,
    GUI_COMMAND_OBJECT_UPGRADE,
    GUI_COMMAND_CANCEL_UPGRADE,
    GUI_COMMAND_ATTACK_MOVE,
    GUI_COMMAND_GUARD,
    GUI_COMMAND_GUARD_WITHOUT_PURSUIT,
    GUI_COMMAND_GUARD_FLYING_UNITS_ONLY,
    GUI_COMMAND_STOP,
    GUI_COMMAND_WAYPOINTS,
    GUI_COMMAND_EXIT_CONTAINER,
    GUI_COMMAND_EVACUATE,
    GUI_COMMAND_EXECUTE_RAILED_TRANSPORT,
    GUI_COMMAND_BEACON_DELETE,
    GUI_COMMAND_SET_RALLY_POINT,
    GUI_COMMAND_SELL,
    GUI_COMMAND_FIRE_WEAPON,
    GUI_COMMAND_SPECIAL_POWER,
    GUI_COMMAND_PURCHASE_SCIENCE,
    GUI_COMMAND_HACK_INTERNET,
    GUI_COMMAND_TOGGLE_OVERCHARGE,
    GUI_COMMAND_COMBATDROP,
    GUI_COMMAND_SWITCH_WEAPON,
    GUI_COMMAND_HIJACK_VEHICLE,
    GUI_COMMAND_CONVERT_TO_CARBOMB,
    GUI_COMMAND_SABOTAGE_BUILDING,
    GUI_COMMAND_PLACE_BEACON,
    GUI_COMMAND_SPECIAL_POWER_FROM_SHORTCUT,
    GUI_COMMAND_SPECIAL_POWER_CONSTRUCT,
    GUI_COMMAND_SPECIAL_POWER_CONSTRUCT_FROM_SHORTCUT,
    GUI_COMMAND_SELECT_ALL_UNITS_OF_TYPE,
};

enum CommandButtonMappedBorderType
{
    COMMAND_BUTTON_BORDER_NONE,
    COMMAND_BUTTON_BORDER_BUILD,
    COMMAND_BUTTON_BORDER_UPGRADE,
    COMMAND_BUTTON_BORDER_ACTION,
    COMMAND_BUTTON_BORDER_SYSTEM,
    COMMAND_BUTTON_BORDER_COUNT,
};

enum CommandButtonOptions
{
    COMMAND_OPTION_NEED_TARGET_ENEMY_OBJECT = (1 << 0),
    COMMAND_OPTION_NEED_TARGET_NEUTRAL_OBJECT = (1 << 1),
    COMMAND_OPTION_NEED_TARGET_ALLY_OBJECT = (1 << 2),
    COMMAND_OPTION_NEED_TARGET_OBJECT = COMMAND_OPTION_NEED_TARGET_ALLY_OBJECT | COMMAND_OPTION_NEED_TARGET_NEUTRAL_OBJECT
        | COMMAND_OPTION_NEED_TARGET_ENEMY_OBJECT,
    COMMAND_OPTION_RESERVED = (1 << 3),
    COMMAND_OPTION_ALLOW_SHRUBBERY_TARGET = (1 << 4),
    COMMAND_OPTION_NEED_TARGET_POS = (1 << 5),
    COMMAND_OPTION_NEED_UPGRADE = (1 << 6),
    COMMAND_OPTION_NEED_SPECIAL_POWER_SCIENCE = (1 << 7),
    COMMAND_OPTION_OK_FOR_MULTI_SELECT = (1 << 8),
    COMMAND_OPTION_CONTEXTMODE_COMMAND = (1 << 9),
    COMMAND_OPTION_CHECK_LIKE = (1 << 10),
    COMMAND_OPTION_ALLOW_MINE_TARGET = (1 << 11),
    COMMAND_OPTION_ATTACK_OBJECTS_POSITION = (1 << 12),
    COMMAND_OPTION_OPTION_ONE = (1 << 13),
    COMMAND_OPTION_OPTION_TWO = (1 << 14),
    COMMAND_OPTION_OPTION_THREE = (1 << 15),
    COMMAND_OPTION_NOT_QUEUEABLE = (1 << 16),
    COMMAND_OPTION_SINGLE_USE_COMMAND = (1 << 17),
    COMMAND_OPTION_DO_NOT_USE = (1 << 18),
    COMMAND_OPTION_SCRIPT_ONLY = (1 << 19),
    COMMAND_OPTION_IGNORES_UNDERPOWERED = (1 << 20),
    COMMAND_OPTION_USES_MINE_CLEARING_WEAPONSET = (1 << 21),
    COMMAND_OPTION_CAN_USE_WAYPOINTS = (1 << 22),
    COMMAND_OPTION_MUST_BE_STOPPED = (1 << 23),
};

class CommandButton : public Overridable
{
    IMPLEMENT_POOL(CommandButton)

public:
    CommandButton();
    virtual ~CommandButton() override;

    GUICommand Get_Command() const { return m_command; }
    CommandButton *Get_Next() const { return m_nextCommandButton; }
    const SpecialPowerTemplate *Get_Special_Power() const { return m_specialPower; }
    unsigned int Get_Options() const { return m_options; }
    WeaponSlotType Get_Weapon_Slot() const { return m_weaponSlot; }
    int Get_Max_Shots_To_Fire() const { return m_maxShotsToFire; }
    const Utf8String &Get_Name() const { return m_name; }
    const UpgradeTemplate *Get_Upgrade_Template() const { return m_upgradeTemplate; }
    Override<ThingTemplate> Get_Template() const { return m_thingTemplate; }
    const std::vector<ScienceType> *Get_Sciences() const { return &m_sciences; }
    const Image *Get_Button_Image() const { return m_buttonImage; }
    const Utf8String &Get_Text_Label() const { return m_textLabel; }
    const Utf8String &Get_Descript_Label() const { return m_descriptLabel; }
    const Utf8String &Get_Conflicting_Label() const { return m_conflictingLabel; }
    int Get_Cameo_Flash_Time() const { return m_cameoFlashTime; }
    CommandButtonMappedBorderType Get_Border() const { return m_commandButtonBorder; }
    const Utf8String &Get_Cursor_Name() const { return m_cursorName; }
    const Utf8String &Get_Invalid_Cursor_Name() const { return m_invalidCursorName; }
    RadiusCursorType Get_Radius_Cursor_Type() const { return m_radiusCursorType; }
    const Utf8String &Get_Purchased_Label() const { return m_purchasedLabel; }
    const AudioEventRTS *Get_Unit_Specific_Sound() const { return &m_unitSpecificSound; }

    void Set_Cameo_Flash_Time(int time) { m_cameoFlashTime = time; }
    void Set_Name(const Utf8String &name) { m_name = name; }
    void Set_Button_Image(const Image *image) { m_buttonImage = image; }

    bool Is_Valid_Object_Target(const Player *player, const Object *obj) const;
    bool Is_Valid_Object_Target(const Object *obj1, const Object *obj2) const;
    bool Is_Valid_Relationship_Target(Relationship r) const;
    bool Is_Valid_To_Use_On(Object *obj, const Object *obj2, const Coord3D *pos, CommandSourceType type) const;
    bool Is_Ready(const Object *obj) const;
    bool Is_Valid_Object_Target(const Drawable *drawable, const Drawable *drawable2) const;
    bool Is_Context_Command() const;
    void Copy_Images_From(const CommandButton *button, bool set_dirty) const;
    void Copy_Button_Text_From(const CommandButton *button, bool conflicting_label, bool set_dirty) const;
    void Cache_Button_Image();
    void Friend_Add_To_List(CommandButton **next);

    static FieldParse *Get_Field_Parse() { return s_commandButtonFieldParseTable; }
    static void Parse_Command(INI *ini, void *formal, void *store, const void *user_data);

private:
    Utf8String m_name;
    GUICommand m_command;
    CommandButton *m_nextCommandButton;
    unsigned int m_options;
    ThingTemplate *m_thingTemplate;
    UpgradeTemplate *m_upgradeTemplate;
    SpecialPowerTemplate *m_specialPower;
    RadiusCursorType m_radiusCursorType;
    Utf8String m_cursorName;
    Utf8String m_invalidCursorName;
    mutable Utf8String m_textLabel;
    mutable Utf8String m_descriptLabel;
    Utf8String m_purchasedLabel;
    Utf8String m_conflictingLabel;
    WeaponSlotType m_weaponSlot;
    int m_maxShotsToFire;
    std::vector<ScienceType> m_sciences;
    CommandButtonMappedBorderType m_commandButtonBorder;
    Utf8String m_buttonImageString;
    int m_unk;
    AudioEventRTS m_unitSpecificSound;
    mutable const Image *m_buttonImage;
    int m_cameoFlashTime;
    static FieldParse s_commandButtonFieldParseTable[];
};

class ControlBar : public SubsystemInterface
{
public:
    struct QueueEntry
    {
        GameWindow *control;
        ProductionEntry::ProductionType type;
        union
        {
            ProductionID production_id;
            const UpgradeTemplate *upgrade_template;
        };
    };

    struct ContainEntry
    {
        GameWindow *button;
        ObjectID id;
    };

    enum
    {
        SPECIAL_POWER_SHORTCUT_BUTTON_COUNT = 11,
        CONTEXT_PARENT_COUNT = 9,
        UNIT_UPGRADE_WINDOW_COUNT = 5,
        RANK_1_BUTTON_COUNT = 4,
        RANK_3_BUTTON_COUNT = 15,
        RANK_8_BUTTON_COUNT = 4,
        QUEUE_ENTRY_COUNT = 9,
    };

    ControlBar();
    virtual ~ControlBar();
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    bool Get_Build_Tooltip_Layout_Visible() const { return m_buildTooltipLayoutVisible; }
    const CommandButton *Get_Command_Buttons() const { return m_commandButtons; }
    ControlBarSchemeManager *Get_Control_Bar_Scheme_Manager() const { return m_controlBarSchemeManager; }
    const Image *Get_Gen_Arrow_Image() const { return m_genArrowImage; }
    Player *Get_Observer_Player() const { return m_observerPlayer; }
    int Get_Command_Bar_Border_Color() const { return m_CommandBarBorderColor; }
    bool Is_Observer() const { return m_isObserver; }
    void Set_Cameo_Flash(bool flash) { m_cameoFlash = flash; }
    void Set_Gen_Arrow_Image(const Image *image) { m_genArrowImage = image; }
    void Set_Observer_Player(Player *player) { m_observerPlayer = player; }
    void Set_Command_Bar_Border_Color(int color) { m_CommandBarBorderColor = color; }

    const CommandSet *Find_Command_Set(const Utf8String &name);
    const CommandButton *Find_Command_Button(const Utf8String &name);
    CommandButton *Find_Non_Const_Command_Button(const Utf8String &name);
    CommandSet *Find_Non_Const_Command_Set(const Utf8String &name);

    void Get_Background_Marker_Pos(int *x, int *y);
    CommandAvailability Get_Command_Availability(
        const CommandButton *button, Object *obj, GameWindow *window, GameWindow *window2, bool b);
    void Get_Foreground_Marker_Pos(int *x, int *y);
    const Image *Get_Star_Image();

    void Hide_Build_Tooltip_Layout();
    void Hide_Communicator(bool hide);
    void Hide_Purchase_Science();
    void Hide_Special_Power_Shortcut();

    void Init_Observer_Controls();
    void Init_Special_Power_Shortcut_Bar(Player *player);

    void On_Drawable_Deselected(Drawable *draw);
    void On_Drawable_Selected(Drawable *draw);
    void On_Player_Rank_Changed(const Player *player);
    void On_Player_Science_Purchase_Points_Changed(const Player *player);

    void Populate_Beacon(Object *beacon);
    void Populate_Build_Queue(Object *producer);
    void Populate_Build_Tooltip_Layout(const CommandButton *button, GameWindow *window);
    void Populate_Command(Object *obj);
    void Populate_Multi_Select();
    void Populate_OCL_Timer(Object *obj);
    void Populate_Observer_Info_Window();
    void Populate_Observer_List();
    void Populate_Purchase_Science(Player *player);
    void Populate_Special_Power_Shortcut(Player *player);
    void Populate_Structure_Inventory(Object *building);
    void Populate_Under_Construction(Object *object_under_construction);

    CBCommandStatus Process_Command_Transition_UI(GameWindow *control, GadgetGameMessage gadget_message);
    CBCommandStatus Process_Command_UI(GameWindow *control, GadgetGameMessage gadget_message);
    CBCommandStatus Process_Context_Sensitive_Button_Click(GameWindow *button, GadgetGameMessage gadget_message);
    CBCommandStatus Process_Context_Sensitive_Button_Transition(GameWindow *button, GadgetGameMessage gadget_message);

    void Reset_Build_Queue_Data();
    void Reset_Common_Command_Data();
    void Reset_Contain_Data();

    void Set_Command_Bar_Border(GameWindow *button, CommandButtonMappedBorderType type);
    void Set_Control_Bar_Scheme_By_Name(const Utf8String &name);
    void Set_Control_Bar_Scheme_By_Player(Player *p);
    void Set_Control_Bar_Scheme_By_Player_Template(PlayerTemplate *tmplate);
    void Set_Control_Command(const Utf8String &button_window_name, GameWindow *parent, const CommandButton *command_button);
    void Set_Control_Command(GameWindow *button, const CommandButton *command_button);
    void Set_Default_Control_Bar_Config();
    void Set_Hidden_Control_Bar();
    void Set_Low_Control_Bar_Config();
    void Set_Portrait_By_Image(const Image *image);
    void Set_Portrait_By_Object(Object *obj);
    void Set_Squished_Control_Bar_Config();
    void Set_Up_Down_Images();

    void Show_Build_Tooltip_Layout(GameWindow *window);
    void Show_Purchase_Science();
    void Show_Rally_Point(const Coord3D *loc);
    void Show_Special_Power_Shortcut();

    void Update_Build_Queue_Disabled_Images(const Image *image);
    void Update_Build_Up_Clock_Color(int color);
    void Update_Command_Bar_Border_Colors(int build, int action, int upgrade, int system);
    void Update_Command_Marker_Image(const Image *image);
    void Update_Construction_Text_Display(Object *obj);
    void Update_Context_Beacon();
    void Update_Context_Command();
    void Update_Context_Multi_Select();
    void Update_Context_OCL_Timer();
    void Update_Context_Purchase_Science();
    void Update_Context_Structure_Inventory();
    void Update_Context_Under_Construction();
    void Update_OCL_Timer_Text_Display(unsigned int remaining_frames, float progress);
    void Update_Purchase_Science();
    void Update_Radar_Attack_Glow();
    void Update_Right_HUD_Image(const Image *image);
    void Update_Slot_Exit_Image(const Image *image);
    void Update_Special_Power_Shortcut();
    void Update_Up_Down_Images(const Image *toggle_button_up_in_image,
        const Image *toggle_button_up_on_image,
        const Image *toggle_button_up_pushed_image,
        const Image *toggle_button_down_in_image,
        const Image *toggle_button_down_on_image,
        const Image *toggle_button_down_pushed_image,
        const Image *general_button_enable_image,
        const Image *general_button_hilited_image);

    void Add_Common_Commands(Drawable *draw, bool first_drawable);
    void Animate_Special_Power_Shortcut(bool forward);
    void Delete_Build_Tooltip_Layout();
    void Do_Transport_Inventory_UI(Object *transport, const CommandSet *command_set);
    void Draw_Special_Power_Shortcut_Multiplier_Text();
    void Draw_Transition_Handler();
    void Evaluate_Context_UI();
    bool Has_Any_Shortcut_Selection();
    void Mark_UI_Dirty();
    CommandButton *New_Command_Button(const Utf8String &name);
    CommandButton *New_Command_Button_Override(CommandButton *button);
    CommandSet *New_Command_Set(const Utf8String &name);
    CommandSet *New_Command_Set_Override(CommandSet *set);
    void Post_Process_Commands();
    void Preload_Assets(TimeOfDayType time_of_day);
    void Repopulate_Build_Tooltip_Layout();
    void Switch_Control_Bar_Stage(ControlBarStages stage);
    void Switch_To_Context(ControlBarContext context, Drawable *draw);
    void Toggle_Control_Bar_Stage();
    void Toggle_Purchase_Science();
    void Trigger_Radar_Attack_Glow();

    static void Parse_Command_Button_Definition(INI *ini);
    static void Parse_Command_Set_Definition(INI *ini);
    static void Parse_Control_Bar_Scheme_Definition(INI *ini);
    static void Populate_Inv_Data_Callback(Object *obj, void *user_data);
    static const Image *Calculate_Veterancy_Overlay_For_Object(const Object *object);
    static const Image *Calculate_Veterancy_Overlay_For_Thing(const ThingTemplate *thing);
    static void Populate_Button_Proc(Object *obj, void *user_data);

#ifdef GAME_DLL
    ControlBar *Hook_Ctor() { return new (this) ControlBar; }
#endif

private:
    WindowVideoManager *m_videoManager;
    AnimateWindowManager *m_controlBarAnimateWindowManager;
    AnimateWindowManager *m_specialPowerShortcutAnimateWindowManager;
    AnimateWindowManager *m_unkAnimateWindowManager;
    int m_parentXPosition;
    int m_parentYPosition;
    int m_controlBarConfig;
    bool m_UIDirty;
    CommandButton *m_commandButtons;
    CommandSet *m_commandSets;
    ControlBarSchemeManager *m_controlBarSchemeManager;
    GameWindow *m_contextParent[CONTEXT_PARENT_COUNT];
    Drawable *m_currentSelectedDrawable;
    ControlBarContext m_currContext;
    DrawableID m_rallyPointDrawableID;
    float m_displayedConstructPercent;
    unsigned int m_oclTimerFrame;
    unsigned int m_displayedQueueCount;
    unsigned int m_lastRecordedInventoryCount;
    GameWindow *m_rightHUDWindow;
    GameWindow *m_rightHUDCameoWindow;
    GameWindow *m_unitUpgradeWindows[UNIT_UPGRADE_WINDOW_COUNT];
    GameWindow *m_unitSelectedWindow;
    GameWindow *m_popupCommunicator;
    WindowLayout *m_generalsPointsLayout;
    GameWindow *m_rank1Buttons[RANK_1_BUTTON_COUNT];
    GameWindow *m_rank3Buttons[RANK_3_BUTTON_COUNT];
    GameWindow *m_rank8Buttons[RANK_8_BUTTON_COUNT];
    GameWindow *m_specialPowerShortcutButtons[SPECIAL_POWER_SHORTCUT_BUTTON_COUNT];
    GameWindow *m_specialPowerShortcutButtonParents[SPECIAL_POWER_SHORTCUT_BUTTON_COUNT];
    int m_specialPowerShortcutButtonUnk[SPECIAL_POWER_SHORTCUT_BUTTON_COUNT];
    int m_specialPowerShortcutButtonCount;
    WindowLayout *m_specialPowerShortcutBarLayout;
    GameWindow *m_specialPowerShortcutBarParent;
    GameWindow *m_commandWindows[CommandSet::MAX_COMMAND_BUTTONS];
    const CommandButton *m_commonCommands[CommandSet::MAX_COMMAND_BUTTONS];
    QueueEntry m_queueData[QUEUE_ENTRY_COUNT];
    bool m_cameoFlash;
    bool m_unk1;
    int m_unk2;
    int m_unk3;
    int m_unk4;
    int m_unk5;
    int m_unk6;
    int m_unk7;
    int m_unk8;
    int m_unk9;
    GameWindow *m_unkWindow;
    int m_unk10;
    int m_buildUpClockColor;
    bool m_isObserver;
    Player *m_observerPlayer;
    WindowLayout *m_controlBarPopupDescriptionLayout;
    bool m_buildTooltipLayoutVisible;
    int m_commandButtonBorderBuildColor;
    int m_commandButtonBorderActionColor;
    int m_commandButtonBorderUpgradeColor;
    int m_commandButtonBorderSystemColor;
    int m_CommandBarBorderColor;
    const Image *m_barButtonGenStarOnIcon;
    const Image *m_barButtonGenStarOffIcon;
    const Image *m_toggleButtonUpInImage;
    const Image *m_toggleButtonUpOnImage;
    const Image *m_toggleButtonUpPushedImage;
    const Image *m_toggleButtonDownInImage;
    const Image *m_toggleButtonDownOnImage;
    const Image *m_toggleButtonDownPushedImage;
    int m_unk11;
    const Image *m_genArrowImage;
    const Image *m_generalButtonEnableImage;
    const Image *m_generalButtonHilitedImage;
    bool m_starHilight;
    int m_sciencePurchasePoints;
    int m_foregroundMarkerPosX;
    int m_foregroundMarkerPosY;
    int m_backgroundMarkerPosX;
    int m_backgroundMarkerPosY;
    bool m_triggerRadarAttackGlow;
    int m_radarAttackGlowCounter;
    GameWindow *m_radarAttackGlowWindow;
#ifdef GAME_DEBUG_STRUCTS
    unsigned int m_uiDirtyFrame;
    int m_uiDirtyCounter;
#endif

    static ContainEntry s_containData[CommandSet::MAX_COMMAND_BUTTONS];
    static const Image *s_rankEliteIcon;
    static const Image *s_rankHeroicIcon;
    static const Image *s_rankVeteranIcon;
    friend void Hide_Control_Bar(bool immediate);
    friend void Show_Control_Bar(bool immediate);
    friend void Toggle_Control_Bar(bool immediate);
};

void Control_Bar_Popup_Description_Update_Func(WindowLayout *layout, void *user_data);
WindowMsgHandledType Beacon_Window_Input(
    GameWindow *text_entry, unsigned int message, unsigned int data_1, unsigned int data_2);
void Hide_Control_Bar(bool immediate);
void Show_Control_Bar(bool immediate);
void Toggle_Control_Bar(bool immediate);
WindowMsgHandledType Control_Bar_Observer_System(
    GameWindow *window, unsigned int message, unsigned int data_1, unsigned int data_2);
WindowMsgHandledType Left_HUD_Input(GameWindow *window, unsigned int message, unsigned int data_1, unsigned int data_2);
WindowMsgHandledType Control_Bar_Input(GameWindow *window, unsigned int message, unsigned int data_1, unsigned int data_2);
WindowMsgHandledType Control_Bar_System(GameWindow *window, unsigned int message, unsigned int data_1, unsigned int data_2);

#ifdef GAME_DLL
extern ControlBar *&g_theControlBar;
#else
extern ControlBar *g_theControlBar;
#endif
