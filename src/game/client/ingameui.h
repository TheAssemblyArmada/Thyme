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
#include "color.h"
#include "coord.h"
#include "drawable.h"
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
class VideoStreamInterface;
class GameWindow;
class PopupMessageData;
class WindowLayout;
class SuperweaponInfo;
class NamedTimerInfo;
class FloatingTextData;
class WorldAnimationData;
class MilitarySubtitleData;
class Shadow;
class GameMessage;

enum LegalBuildCode
{
    LBC_OK,
    LBC_RESTRICTED_TERRAIN,
    LBC_NOT_FLAT_ENOUGH,
    LBC_OBJECTS_IN_THE_WAY,
    LBC_NO_CLEAR_PATH,
    LBC_SHROUD,
    LBC_TOO_CLOSE_TO_SUPPLIES,
};

enum RadiusCursorType
{
    RADIUS_CURSOR_UNK,
};

struct BuildProgress
{
    ThingTemplate *m_thingTemplate;
    float m_percentComplete;
    GameWindow *m_control;
};

class InGameUI : public SubsystemInterface, public SnapShot
{
public:
    virtual ~InGameUI() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;
    virtual void Draw() override = 0;
    virtual void Popup_Message(Utf8String const &message, int32_t i1, int32_t i2, int32_t i3, bool b1, bool b2);
    virtual void Popup_Message(Utf8String const &message, int32_t i1, int32_t i2, int32_t i3, int32_t i4, bool b1, bool b2);
    virtual void Message_Color(RGBColor const *color, wchar_t const *message);
    virtual void Message(Utf16String message, ...);
    virtual void Message(Utf8String message, ...);
    virtual void Toggle_Messages();
    virtual bool Is_Messages_On();
    virtual void Military_Subtitle(Utf8String const &subtitle, int32_t duration);
    virtual void Remove_Military_Subtitle();
    virtual void Display_Cant_Build_Message(LegalBuildCode code);
    virtual void Begin_Area_Select_Hint(GameMessage const *msg);
    virtual void End_Area_Select_Hint(GameMessage const *msg);
    virtual void Create_Move_Hint(GameMessage const *msg);
    virtual void Create_Attack_Hint(GameMessage const *msg);
    virtual void Create_Force_Attack_Hint(GameMessage const *msg);
    virtual void create_Mouseover_Hint(GameMessage const *msg);
    virtual void Create_Command_Hint(GameMessage const *msg);
    virtual void Create_Garrison_Hint(GameMessage const *msg);
    virtual void Add_Superweapon(
        int32_t player_index, Utf8String const &power_name, ObjectID id, SpecialPowerTemplate const *power_template);
    virtual bool Remove_Superweapon(
        int32_t player_index, Utf8String const &power_name, ObjectID id, SpecialPowerTemplate const *power_template);
    virtual void Object_Changed_Team(Object const *obj, int32_t old_player_index, int32_t new_player_index);
    virtual void Set_Superweapon_Display_Enabled_By_Script(bool enable);
    virtual bool Get_Superweapon_Display_Enabled_By_Script();
    virtual void Hide_Object_Superweapon_Display_By_Script(Object const *obj);
    virtual void Show_Object_Superweapon_Display_By_Script(Object const *obj);
    virtual void Set_Scrolling(bool is_scrolling);
    virtual bool Is_Scrolling();
    virtual void Set_Selecting(bool is_selecting);
    virtual bool Is_Selecting();
    virtual void Set_Scroll_Amount(Coord2D amt);
    virtual Coord2D Get_Scroll_Amount();
    virtual void Set_GUI_Command(CommandButton const *command);
    virtual CommandButton const *Get_GUI_Command();
    virtual void Place_Build_Available(ThingTemplate const *build, Drawable *build_drawable);
    virtual ThingTemplate const *Get_Pending_Place_Type();
    virtual ObjectID const Get_Pending_Place_Source_Object_ID();
#ifndef GAME_DEBUG_STRUCTS
    virtual bool Get_Prevent_Left_Click_Deselection_In_Alternate_Mouse_Mode_For_One_Click();
    virtual void Set_Prevent_Left_Click_Deselection_In_Alternate_Mouse_Mode_For_One_Click(bool b);
#endif
    virtual void Set_Placement_Start(ICoord2D const *start);
    virtual void Set_Placement_End(ICoord2D const *end);
    virtual bool Is_Placement_Anchored();
    virtual void Get_Placement_Points(ICoord2D *start, ICoord2D *end);
    virtual float Get_Placement_Angle();
    virtual void Select_Drawable(Drawable *drawable);
    virtual void Deselect_Drawable(Drawable *drawable);
    virtual void Deselect_All_Drawables(bool post_msg);
    virtual int32_t Get_Select_Count();
    virtual int32_t Get_Max_Select_Count();
    virtual uint32_t Get_Frame_Selection_Changed();
    virtual std::list<Drawable *> const *Get_All_Selected_Drawables();
    virtual std::list<Drawable *> const *Get_All_Selected_Local_Drawables();
    virtual Drawable *Get_First_Selected_Drawable();
    virtual DrawableID Get_Solo_Nexus_Selected_Drawable_ID();
    virtual bool Is_Drawable_Selected(DrawableID id_to_check);
    virtual bool Is_Any_Selected_KindOf(KindOfType kindof);
    virtual bool Is_All_Selected_KindOf(KindOfType kindof);
    virtual void Set_Radius_Cursor(
        RadiusCursorType cursor, SpecialPowerTemplate const *power_template, WeaponSlotType weapon_slot);
    virtual void Set_Radius_Cursor_None();
    virtual void Set_Input_Enabled(bool enable);
    virtual bool Get_Input_Enabled();
    virtual void Disregard_Drawable(Drawable *draw);
    virtual void Pre_Draw();
    virtual void Post_Draw();
    virtual void Play_Movie(Utf8String const &movie_name);
    virtual void Stop_Movie();
    virtual VideoBuffer *Video_Buffer();
    virtual void Play_Cameo_Movie(Utf8String const &movie_name);
    virtual void Stop_Cameo_Movie();
    virtual VideoBuffer *Cameo_Video_Buffer();
    virtual DrawableID Get_Moused_Over_Drawable_ID();
    virtual void Set_Quit_Menu_Visible(bool visible);
    virtual bool Is_Quit_Menu_Visible();
    virtual FieldParse const *Get_Field_Parse();
    virtual int32_t Select_Units_Matching_Current_Selection();
    virtual int32_t Select_Matching_Across_Screen();
    virtual int32_t Select_Matching_Across_Map();
    virtual int32_t Select_Matching_Across_Region(IRegion2D *);
    virtual int32_t Select_All_Units_By_Type(BitFlags<KINDOF_COUNT> flags1, BitFlags<KINDOF_COUNT> flags2);
    virtual int32_t Select_All_Units_By_Type_Across_Screen(BitFlags<KINDOF_COUNT> flags1, BitFlags<KINDOF_COUNT> flags2);
    virtual int32_t Select_All_Units_By_Type_Across_Map(BitFlags<KINDOF_COUNT> flags1, BitFlags<KINDOF_COUNT> flags2);
    virtual int32_t Select_All_Units_By_Type_Across_Region(
        IRegion2D *region, BitFlags<KINDOF_COUNT> flags1, BitFlags<KINDOF_COUNT> flags2);
    virtual void Build_Region(ICoord2D const *anchor, ICoord2D const *dest, IRegion2D *region);
    virtual bool Get_Displayed_Max_Warning();
    virtual void Set_Displayed_Max_Warning(bool selected);
    virtual void Add_Floating_Text(Utf16String const &text, Coord3D const *pos, int32_t color);
    virtual void Add_Idle_Worker(Object *obj);
    virtual void Remove_Idle_Worker(Object *obj, int32_t slot);
    virtual void Select_Next_Idle_Worker();
    virtual void Recreate_Control_Bar();
    virtual void Disable_Tooltips_Until(uint32_t until);
    virtual void Clear_Tooltips_Disabled();
    virtual bool Are_Tooltips_Disabled();
    virtual int32_t Get_Idle_Worker_Count();
    virtual Object *Find_Idle_Worker(Object *obj);
    virtual void Show_Idle_Worker_Layout();
    virtual void Hide_Idle_Worker_Layout();
    virtual void Update_Idle_Worker();
    virtual void Reset_Idle_Worker();
#ifdef GAME_DEBUG_STRUCTS
    virtual void Debug_Add_Floating_Text(Utf8String const &text, Coord3D const *pos, int32_t color);
#endif
    virtual View *Create_View() = 0;

    bool Get_Drawable_Caption_Bold() const { return m_drawableCaptionBold; }
    int32_t Get_Drawable_Caption_Size() const { return m_drawableCaptionPointSize; }
    Utf8String Get_Drawable_Caption_Font() const { return m_drawableCaptionFont; }
    int32_t Get_Drawable_Caption_Color() const { return m_drawableCaptionColor; }

protected:
    struct MoveHintStruct
    {
        Coord3D pos;
        uint32_t source_id;
        uint32_t frame;
    };

    struct UIMessage
    {
        Utf16String full_text;
        DisplayString *display_string;
        uint32_t timestamp;
        int32_t color;
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
    MoveHintStruct m_moveHint[256];
    int32_t m_nextMoveHint;
    CommandButton *m_pendingGUICommand;
    BuildProgress m_buildProgress[64];
    CommandButton *m_pendingPlaceType;
    ObjectID m_pendingPlaceSourceObjectID;
#ifndef GAME_DEBUG_STRUCTS
    bool m_preventLeftClickDeselectionInAlternateMouseModeForOneClick; // not 100% identified yet
#endif
    Drawable **m_placeIcon;
    bool m_placeAnchorInProgress;
    ICoord2D m_placeAnchorStart;
    ICoord2D m_placeAnchorEnd;
    int32_t m_selectCount;
    int32_t m_maxSelectCount;
    int32_t m_frameSelectionChanged; // not 100% identified yet
    int32_t m_doubleClickCounter; // not 100% identified yet
    Coord3D m_radiusDecalPos; // not 100% identified yet
    VideoBuffer *m_videoBuffer;
    VideoStreamInterface *m_videoStream;
    VideoBuffer *m_cameoVideoBuffer;
    VideoStreamInterface *m_cameoVideoStream;
    UIMessage m_uiMessages[6];
    std::map<Utf8String, std::list<SuperweaponInfo *>> m_superweapons[16];
    Coord2D m_superweaponPosition;
    float m_superweaponFlashDuration;
    Utf8String m_superweaponNormalFont;
    int32_t m_superweaponNormalPointSize;
    bool m_superweaponNormalBold;
    Utf8String m_superweaponReadyFont;
    int32_t m_superweaponReadyPointSize;
    bool m_superweaponReadyBold;
    int32_t m_superweaponLastFlashFrame;
    int32_t m_superweaponFlashColor;
    bool m_superweaponHidden;
    std::map<Utf8String, NamedTimerInfo *> m_namedTimers;
    Coord2D m_namedTimerPosition;
    float m_namedTimerFlashDuration;
    int32_t m_namedTimerLastFlashFrame; // not 100% identified yet
    int32_t m_namedTimerFlashColor;
    bool m_namedTimerUsedFlashColor; // not 100% identified yet
    bool m_showNamedTimers; // not 100% identified yet
    Utf8String m_namedTimerNormalFont;
    int32_t m_namedTimerNormalPointSize;
    bool m_namedTimerNormalBold;
    int32_t m_namedTimerNormalColor;
    Utf8String m_namedTimerReadyFont;
    int32_t m_namedTimerReadyPointSize;
    bool m_namedTimerReadyBold;
    int32_t m_namedTimerReadyColor;
    Utf8String m_drawableCaptionFont;
    int32_t m_drawableCaptionPointSize;
    bool m_drawableCaptionBold;
    int32_t m_drawableCaptionColor;
    uint32_t m_tooltipsDisabled; // not 100% identified yet
    MilitarySubtitleData *m_militarySubtitle;
    bool m_isScrolling;
    bool m_isSelecting;
    int32_t m_mouseMode;
    MouseCursor m_mouseCursor; // not 100% identified yet
    ObjectID m_mousedOverObjectID;
    Coord2D m_scrollAmt;
    bool m_isQuitMenuVisible;
    bool m_messagesOn;
    int32_t m_messageColor1;
    int32_t m_messageColor2;
    ICoord2D m_messagePosition;
    Utf8String m_messageFont;
    int32_t m_messagePointSize;
    bool m_messageBold;
    int32_t m_messageDelayMS;
    RGBAColorInt m_militaryCaptionColor;
    ICoord2D m_militaryCaptionPosition;
    Utf8String m_militaryCaptionTitleFont;
    int32_t m_militaryCaptionTitlePointSize;
    bool m_militaryCaptionTitleBold;
    Utf8String m_militaryCaptionFont;
    int32_t m_militaryCaptionPointSize;
    bool m_militaryCaptionBold;
    bool m_militaryCaptionRandomizeTyping;
    int32_t m_militaryCaptionSpeed;
    RadiusDecalTemplate m_radiusDecalTemplate[30];
    RadiusDecal m_radiusDecal;
    RadiusCursorType m_radiusDecalType; // not 100% identified yet
    std::list<FloatingTextData *> m_floatingTextList;
    int32_t m_floatingTextTimeOut;
    float m_floatingTextMoveUpSpeed;
    float m_floatingTextMoveVanishRate;
    PopupMessageData *m_popupMessageData;
    int32_t m_popupMessageColor;
    bool m_unk1; // not 100% identified yet
    bool m_unk2; // not 100% identified yet
    bool m_unk3; // not 100% identified yet
    bool m_unk4; // not 100% identified yet
    bool m_unk5; // not 100% identified yet
    bool m_cameraRotateLeft; // not 100% identified yet
    bool m_cameraRotateRight; // not 100% identified yet
    bool m_cameraZoomIn; // not 100% identified yet
    bool m_unk6; // not 100% identified yet
    bool m_cameraZoomOut; // not 100% identified yet
    bool m_drawRMBScrollAnchor;
    bool m_moveRMBScrollAnchor;
    bool m_unk7; // not 100% identified yet
    std::list<WorldAnimationData *> m_worldAnimations;
    std::list<Object *> m_idleWorkerLists[16];
    GameWindow *m_idleWorkerWin;
    int32_t m_idleWorkerCount; // not 100% identified yet
    DrawableID m_soloNexusSelectedDrawableID;
};

#ifdef GAME_DLL
extern InGameUI *&g_theInGameUI;
#else
extern InGameUI *g_theInGameUI;
#endif
