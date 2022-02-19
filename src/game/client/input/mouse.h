/**
 * @file
 *
 * @author OmniBlade
 * @author tomsons26
 *
 * @brief Base class for mouse input handlers.
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
#include "color.h"
#include "coord.h"
#include "displaystring.h"
#include "globallanguage.h"
#include "subsysteminterface.h"

// If you add additional cursors, you will need to add a string to the Mouse::s_cursorNames array in mouse.cpp to match it.
enum MouseCursor
{
    CURSOR_INVALID = -1,
    CURSOR_NONE,
    CURSOR_NORMAL,
    CURSOR_ARROW,
    CURSOR_SCROLL,
    CURSOR_TARGET,
    CURSOR_MOVE,
    CURSOR_ATTACK_MOVE,
    CURSOR_ATTACK_OBJ,
    CURSOR_FORCE_ATTACK_OBJ,
    CURSOR_FORCE_ATTACK_GROUND,
    CURSOR_BUILD,
    CURSOR_INVALID_BUILD,
    CURSOR_GENERIC_INVALID,
    CURSOR_SELECT,
    CURSOR_ENTER_FRIENDLY,
    CURSOR_ENTER_AGRESSIVE,
    CURSOR_SET_RALLY_POINT,
    CURSOR_GET_REPAIRED,
    CURSOR_GET_HEALED,
    CURSOR_DO_REPAIR,
    CURSOR_RESUME_CONSTRUCTION,
    CURSOR_CAPTURE_BUILDING,
    CURSOR_SNIPE_VEHICLE,
    CURSOR_LASER_GUIDED_MISSLES,
    CURSOR_TANK_HUNTER_TNT_ATTACK,
    CURSOR_STAB_ATTACK,
    CURSOR_PLACE_REMOTE_CHARGE,
    CURSOR_PLACE_TIMED_CHARGE,
    CURSOR_DEFECTOR,
    CURSOR_DOCK,
    CURSOR_FIRE_FLAME,
    CURSOR_FIRE_BOMB,
    CURSOR_PLACE_BEACON,
    CURSOR_DISGUISE_AS_VEHICLE,
    CURSOR_WAYPOINT,
    CURSOR_OUT_RANGE,
    CURSOR_STAB_ATTACK_INVALID,
    CURSOR_PLACE_CHARGE_INVALID,
    CURSOR_HACK,
    CURSOR_PARTICLE_UPLINK_CANNON,
    CURSOR_COUNT,
};

DEFINE_ENUMERATION_OPERATORS(MouseCursor);

struct MouseIO
{
    enum MouseState
    {
        MOUSE_STATE_UP,
        MOUSE_STATE_DOWN,
        MOUSE_STATE_DBLCLICK,
    };
    ICoord2D pos;
    int32_t wheel_pos;
    int32_t wheel_delta;
    ICoord2D delta_pos;
    int32_t left_state;
    int32_t left_event;
    int32_t left_frame;
    int32_t right_state;
    int32_t right_event;
    int32_t right_frame;
    int32_t middle_state;
    int32_t middle_event;
    int32_t middle_frame;
};

class CursorInfo
{
public:
    CursorInfo();

public:
    Utf8String unk_string;
    Utf8String cursor_text;
    RGBAColorInt cursor_text_color;
    RGBAColorInt cursor_text_drop_color;
    Utf8String texture_name;
    Utf8String image_name;
    Utf8String w3d_model_name;
    Utf8String w3d_anim_name;
    float w3d_scale;
    bool loop;
    ICoord2D hot_spot;
    int32_t frames;
    float fps;
    int32_t directions;
};

class INI;

class Mouse : public SubsystemInterface
{
    ALLOW_HOOKING

protected:
    enum RedrawMode
    {
        RM_WINDOWS,
        RM_W3D,
        RM_POLYGON,
        RM_DX8,
        RM_MAX,
    };

    enum
    {
        TOOLTIP_WRAP = 120,
        MAX_EVENTS = 256,
    };

public:
    Mouse();
    virtual ~Mouse();

    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;
    virtual void Draw() override {}

    virtual void Parse_INI();
    virtual void Init_Cursor_Resources() = 0;
    virtual void Create_Stream_Messages();
    virtual void Set_Position(int32_t x, int32_t y);
    virtual void Set_Cursor(MouseCursor cursor) = 0;
    virtual void Capture() = 0;
    virtual void Release_Capture() = 0;
    virtual void Set_Mouse_Limits();
    virtual void Set_Redraw_Mode(RedrawMode mode) { m_currentRedrawMode = mode; }
    virtual RedrawMode Get_Redraw_Mode() { return m_currentRedrawMode; }
    virtual void Set_Visibility(bool visibility) { m_visible = visibility; }
    virtual uint8_t Get_Mouse_Event(MouseIO *io, int8_t unk) = 0;

    void Notify_Resolution_Change();

    MouseCursor Get_Mouse_Cursor() const { return m_currentCursor; }
    MouseIO *Get_Mouse_Status() { return &m_currMouse; }

    static void Parse_Mouse_Definition(INI *ini);
    static void Parse_Cursor_Definition(INI *ini);

    bool Is_Click(ICoord2D *click_1_coord, ICoord2D *click_2_coord, unsigned click_1_time, unsigned click_2_time) const;
    void Check_For_Drag();
    void Reset_Tooltip_Delay();
    void Draw_Cursor_Text() const;

protected:
    void Update_Mouse_Data();
    void Process_Mouse_Event(int32_t event_num);
    void Move_Mouse(int32_t x, int32_t y, int32_t absolute); // TODO Should be bool absolute, fix after verifying correctness.
    MouseCursor Get_Cursor_Index(const Utf8String &name);
    void Set_Mouse_Text(const MouseCursor cursor);
    void Set_Mouse_Text(const Utf16String text, const RGBAColorInt *color, const RGBAColorInt *drop_color);

protected:
    CursorInfo m_cursorInfo[CURSOR_COUNT];
    Utf8String m_tooltipFontName;
    int32_t m_tooltipFontSize;
    bool m_tooltipFontIsBold;
    bool m_tooltipAnimateBackground;
    int32_t m_tooltipFillTime;
    int32_t m_tooltipDelayTime;
    float m_tooltipWidth;
    float unkFloat;
    RGBAColorInt m_tooltipColorText;
    RGBAColorInt m_tooltipColorHighlight;
    RGBAColorInt m_tooltipColorShadow;
    RGBAColorInt m_tooltipColorBackground;
    RGBAColorInt m_tooltipColorBorder;
    RedrawMode m_currentRedrawMode;
    bool m_useTooltipAltTextColor;
    bool m_useTooltipAltBackColor;
    bool m_adjustTooltipAltColor;
    bool m_orthoCamera;
    float m_orthoZoom;
    unsigned m_dragTolerance;
    unsigned m_dragTolerance3D;
    unsigned m_dragToleranceMS;
    int8_t m_numButtons;
    int8_t m_numAxes;
    bool m_forceFeedback;
    Utf16String m_tooltipString;
    DisplayString *m_tooltipDisplayString;
    bool m_displayTooltip;
    bool m_isTooltipEmpty;
    MouseIO m_mouseEvents[MAX_EVENTS];
    MouseIO m_currMouse;
    MouseIO m_prevMouse;
    int32_t m_minX;
    int32_t m_maxX;
    int32_t m_minY;
    int32_t m_maxY;
    int32_t m_inputFrame;
    int32_t m_deadInputFrame;
    bool m_inputMovesAbsolute;
    bool m_visible;
    MouseCursor m_currentCursor;
    DisplayString *m_cursorTextDisplayString;
    RGBAColorInt m_cursorTextColor;
    RGBAColorInt m_cursorTextDropColor;
    int32_t m_tooltipDelay;
    int32_t m_highlightPos;
    int32_t m_highlightUpdateStart;
    unsigned m_stillTime;
    RGBAColorInt m_tooltipColorTextCopy;
    RGBAColorInt m_tooltipColorBackgroundCopy;
    int32_t m_eventCount;

private:
    static const char *s_cursorNames[];
};

#ifdef GAME_DLL
extern Mouse *&g_theMouse;
#else
extern Mouse *g_theMouse;
#endif