/**
 * @file
 *
 * @author OmniBlade
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

#ifndef MOUSE_H
#define MOUSE_H

#include "always.h"
#include "color.h"
#include "coord.h"
#include "displaystring.h"
#include "globallanguage.h"
#include "subsysteminterface.h"

struct MouseIO
{
    ICoord2D pos;
    int wheel_pos;
    int wheel_delta;
    ICoord2D delta_pos;
    int left_state;
    int left_event;
    int left_frame;
    int right_state;
    int right_event;
    int right_frame;
    int middle_state;
    int middle_event;
    int middle_frame;
};

struct CursorInfo
{
    AsciiString unkAsciiString;
    AsciiString cursorText;
    RGBAColorInt cursorTextColor;
    RGBAColorInt cursorTextDropColor;
    AsciiString textureName;
    AsciiString imageName;
    AsciiString W3DModelName;
    AsciiString W3DAnimName;
    float W3DScale;
    bool loop;
    Coord2D hotSpot;
    int frames;
    float fps;
    int directions;
};

class Mouse : public SubsystemInterface
{
    enum MouseCursor
    {
        NONE,
        GENERIC,
        UNKNOWN,
    };

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
		INFO_COUNT = 40,
		MAX_EVENTS = 256,
	};
public:
	Mouse();
	virtual ~Mouse();

	virtual void Init() override;
	virtual void Reset() override;
	virtual void Update() override;
	virtual void Parse_INI();
	virtual void Init_Cursor_Resources() = 0;
	virtual void Create_Stream_Messages();
	virtual void Set_Position(int x, int y);
	virtual void Set_Cursor(MouseCursor cursor) = 0;
	virtual void Capture() = 0;
	virtual void Release_Capture() = 0;
	virtual void Set_Mouse_Limits();
	virtual void Set_Redraw_Mode(RedrawMode mode) { m_currentRedrawMode = mode; }
	virtual RedrawMode Get_Redraw_Mode() { return m_currentRedrawMode; }
    virtual void Set_Visibility(bool visibility) { m_visible = visibility; }
    virtual int8_t Get_Mouse_Event(MouseIO *io, int8_t unk) = 0;

	void Notify_Resolution_Change();

protected:
	void Update_Mouse_Data();
	void Process_Mouse_Event(int event_num);
    void Move_Mouse(int x, int y, int absolute); // TODO Should be bool absolute, fix after verifying correctness.

private:
    CursorInfo m_cursorInfo[INFO_COUNT];
    AsciiString m_tooltipFontName;
    int m_tooltipFontSize;
    bool m_tooltipFontIsBold;
    //FontDesc m_tooltipFont;
    bool m_tooltipAnimateBackground;
    int m_tooltipFillTime;
    int m_tooltipDelayTime;
    float m_toolTipwidth;
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
    int m_dragTolerance;
    int m_dragTolerance3D;
    int m_dragToleranceMS;
    int8_t m_numButtons;
    int8_t m_numAxes;
    bool m_forceFeedback;
    UnicodeString m_tooltipString;
    DisplayString *m_tooltipDisplayString;
    bool m_displayTooltip;
    bool m_isTooltipEmpty;
    MouseIO m_mouseEvents[MAX_EVENTS];
    MouseIO m_currMouse;
    MouseIO m_prevMouse;
    int m_minX;
    int m_maxX;
    int m_minY;
    int m_maxY;
    int m_inputFrame;
    int m_deadInputFrame;
    bool m_inputMovesAbsolute;
    bool m_visible;
    MouseCursor m_currentCursor;
    DisplayString *m_cursorTextDisplayString;
    RGBAColorInt m_cursorTextColor;
    RGBAColorInt m_cursorTextDropColor;
    int m_tooltipDelay;
    int unkInt1;
    int unkInt2;
    unsigned m_stillTime;
    RGBAColorInt m_tooltipColorTextCopy;
    RGBAColorInt m_tooltipColorBackgroundCopy;
    int unkInt3;
};

#ifndef THYME_STANDALONE
#include "hooker.h"

extern Mouse *&g_theMouse;
#else
extern Mouse *g_theMouse;
#endif

#endif // MOUSE_H
