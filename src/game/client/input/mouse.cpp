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
#include "mouse.h"
#include "colorspace.h"
#include "display.h"
#include "displaystringmanager.h"
#include "gametext.h"
#include "globaldata.h"
#include "ini.h"
#include "keyboard.h"
#include "messagestream.h"
#include "rtsutils.h"
#include "scriptengine.h"
#include <algorithm>
#include <cstddef>

using rts::Get_Time;

#ifndef GAME_DLL
Mouse *g_theMouse = nullptr;
#endif

// If you add additional cursors, you will need to add an entry to the MouseCursor enum in mouse.h to match it.
const char *Mouse::s_cursorNames[] = {
    "None",
    "Normal",
    "Arrow",
    "Scroll",
    "Target",
    "Move",
    "AttackMove",
    "AttackObj",
    "ForceAttackObj",
    "ForceAttackGround",
    "Build",
    "InvalidBuild",
    "GenericInvalid",
    "Select",
    "EnterFriendly",
    "EnterAggressive",
    "SetRallyPoint",
    "GetRepaired",
    "GetHealed",
    "DoRepair",
    "ResumeConstruction",
    "CaptureBuilding",
    "SnipeVehicle",
    "LaserGuidedMissiles",
    "TankHunterTNTAttack",
    "StabAttack",
    "PlaceRemoteCharge",
    "PlaceTimedCharge",
    "Defector",
    "Dock",
    "FireFlame",
    "FireBomb",
    "PlaceBeacon",
    "DisguiseAsVehicle",
    "Waypoint",
    "OutRange",
    "StabAttackInvalid",
    "PlaceChargeInvalid",
    "Hack",
    "ParticleUplinkCannon",
    nullptr,
};

CursorInfo::CursorInfo() :
    cursor_text_color{ 0, 0, 0, 0 },
    cursor_text_drop_color{ 0, 0, 0, 0 },
    w3d_scale(1.0f),
    loop(true),
    hot_spot{ 16, 16 },
    frames(1),
    fps(20.0f),
    directions(1)
{
    cursor_name.Clear();
    cursor_text.Clear();
    texture_name.Clear();
    image_name.Clear();
    w3d_model_name.Clear();
    w3d_anim_name.Clear();
}

/**
 * 0x004029D0
 */
Mouse::Mouse() :
    m_tooltipFontName("Times New Roman"),
    m_tooltipFontSize(12),
    m_tooltipFontIsBold(false),
    // m_tooltipFont("Times New Roman"),
    m_tooltipAnimateBackground(true),
    m_tooltipFillTime(50),
    m_tooltipDelayTime(50),
    m_tooltipWidth(15.0f),
    m_tooltipScale(0.0f),
    m_tooltipColorText{ 220, 220, 220, 255 },
    m_tooltipColorHighlight{ 255, 255, 0, 255 },
    m_tooltipColorShadow{ 0, 0, 0, 255 },
    m_tooltipColorBackground{ 20, 20, 0, 127 },
    m_tooltipColorBorder{ 0, 0, 0, 255 },
    m_currentRedrawMode(g_theWriteableGlobalData == nullptr || g_theWriteableGlobalData->m_winCursors ? RM_WINDOWS : RM_W3D),
    m_useTooltipAltTextColor(false),
    m_useTooltipAltBackColor(false),
    m_adjustTooltipAltColor(false),
    m_orthoCamera(false),
    m_orthoZoom(1.0f),
    m_dragTolerance(0),
    m_dragTolerance3D(0),
    m_dragToleranceMS(0),
    m_numButtons(0),
    m_numAxes(0),
    m_forceFeedback(false),
    m_tooltipString(),
    m_tooltipDisplayString(nullptr),
    m_displayTooltip(false),
    m_isTooltipEmpty(true),
    m_minX(0),
    m_maxX(0),
    m_minY(0),
    m_maxY(0),
    m_inputFrame(0),
    m_deadInputFrame(0),
    m_inputMovesAbsolute(false),
    m_visible(false),
    m_currentCursor(CURSOR_ARROW),
    m_cursorTextDisplayString(nullptr),
    m_cursorTextColor{ 255, 255, 255, 255 },
    m_cursorTextDropColor{ 255, 255, 255, 255 },
    m_tooltipDelay(-1),
    m_highlightPos(0),
    m_highlightUpdateStart(0),
    m_stillTime(0),
    m_tooltipColorTextCopy{ 255, 255, 255, 255 },
    m_tooltipColorBackgroundCopy{ 0, 0, 0, 255 },
    m_eventCount(0)
{
    memset(m_mouseEvents, 0, sizeof(m_mouseEvents));
    memset(&m_currMouse, 0, sizeof(m_currMouse));
    memset(&m_prevMouse, 0, sizeof(m_prevMouse));
}

/**
 * 0x00402E50
 */
Mouse::~Mouse()
{
    if (m_tooltipDisplayString != nullptr) {
        g_theDisplayStringManager->Free_Display_String(m_tooltipDisplayString);
    }

    m_tooltipDisplayString = nullptr;

    if (m_cursorTextDisplayString != nullptr) {
        g_theDisplayStringManager->Free_Display_String(m_cursorTextDisplayString);
    }

    m_cursorTextDisplayString = nullptr;
}

/**
 * @brief Initialises the subsystem.
 *
 * 0x00402FC0
 */
void Mouse::Init()
{
    m_currentRedrawMode =
        g_theWriteableGlobalData == nullptr || g_theWriteableGlobalData->m_winCursors ? RM_WINDOWS : m_currentRedrawMode;
    m_numButtons = 2;
    m_numAxes = 2;
    m_forceFeedback = false;
    Notify_Resolution_Change();
    m_tooltipString.Clear();
    m_stillTime = Get_Time(); // possible bugfix ported from BFME
    m_displayTooltip = false;
    memset(m_mouseEvents, 0, sizeof(m_mouseEvents));
    memset(&m_currMouse, 0, sizeof(m_currMouse));
    memset(&m_prevMouse, 0, sizeof(m_prevMouse));
    m_minX = 0;
    m_maxX = 799; // Width of default 800x600 res - 1;
    m_minY = 0;
    m_maxY = 599; // Height of default 800x600 res - 1;
    m_inputFrame = 0;
    m_deadInputFrame = 0;
    m_inputMovesAbsolute = false;
    m_eventCount = 0;
    m_currentCursor = CURSOR_ARROW;
    m_cursorTextDisplayString = g_theDisplayStringManager->New_Display_String();
}

/**
 * @brief Resets the subsystem.
 *
 * 0x00403160
 */
void Mouse::Reset()
{
    if (m_cursorTextDisplayString != nullptr) {
        m_cursorTextDisplayString->Reset();
    }
}

/**
 * @brief Updates the subsystem.
 *
 * 0x00403170
 */
void Mouse::Update()
{
    ++m_inputFrame;
    Update_Mouse_Data();
}

/**
 * @brief Parses mouse configuration data from Data/INI/Mouse.ini.
 *
 * 0x00402F40
 */
void Mouse::Parse_INI()
{
    INI ini;
    ini.Load("Data/INI/Mouse.ini", INI_LOAD_OVERWRITE, nullptr);
}

/**
 * @brief Adds input messages to the message stream based on recorded mouse events.
 *
 * 0x004031F0
 */
void Mouse::Create_Stream_Messages()
{
    if (g_theMessageStream != nullptr) {
        unsigned call_time = Get_Time();
        GameMessage *start_msg = g_theMessageStream->Append_Message(GameMessage::MSG_RAW_MOUSE_POSITION);
        start_msg->Append_Pixel_Arg(m_currMouse.pos);
        start_msg->Append_Int_Arg(g_theKeyboard->Get_Modifiers());
        unsigned delay = 0;

        if (!g_theWriteableGlobalData->m_scriptDebug) {
            delay = m_tooltipDelay < 0 ? m_tooltipDelayTime : m_tooltipDelay;
        }

        // See if the amont of time the mouse has been still is enough to setup displaying tool tip.
        if (call_time - m_stillTime < delay) {
            m_displayTooltip = false;
        } else {
            if (!m_displayTooltip) {
                m_highlightPos = 0;
                m_highlightUpdateStart = Get_Time();
            }

            m_displayTooltip = true;
        }

        for (int i = 0; i < m_eventCount; ++i) {
            Process_Mouse_Event(i);

            // If the mouse has moved, set m_stillTime to be the time we got at function start to time elapsed time since
            // last move.
            if (m_currMouse.delta_pos.x != 0 || m_currMouse.delta_pos.y != 0) {
                m_stillTime = call_time;
            }

            GameMessage *event_msg = nullptr;

            switch (m_currMouse.left_event) {
                case 5:
                    event_msg = g_theMessageStream->Append_Message(GameMessage::MSG_RAW_MOUSE_LEFT_BUTTON_DOWN);
                    event_msg->Append_Pixel_Arg(m_currMouse.pos);
                    event_msg->Append_Int_Arg(g_theKeyboard->Get_Modifiers());
                    event_msg->Append_Int_Arg(m_currMouse.wheel_pos);
                    break;
                case 6:
                    event_msg = g_theMessageStream->Append_Message(GameMessage::MSG_RAW_MOUSE_LEFT_BUTTON_UP);
                    event_msg->Append_Pixel_Arg(m_currMouse.pos);
                    event_msg->Append_Int_Arg(g_theKeyboard->Get_Modifiers());
                    event_msg->Append_Int_Arg(m_currMouse.wheel_pos);
                    break;
                case 7:
                    event_msg = g_theMessageStream->Append_Message(GameMessage::MSG_RAW_MOUSE_LEFT_DOUBLE_CLICK);
                    event_msg->Append_Pixel_Arg(m_currMouse.pos);
                    event_msg->Append_Int_Arg(g_theKeyboard->Get_Modifiers());
                    event_msg->Append_Int_Arg(m_currMouse.wheel_pos);
                    break;
                case 8:
                    event_msg = g_theMessageStream->Append_Message(GameMessage::MSG_RAW_MOUSE_LEFT_DRAG);
                    event_msg->Append_Pixel_Arg(m_currMouse.pos);
                    event_msg->Append_Pixel_Arg(m_currMouse.delta_pos);
                    event_msg->Append_Int_Arg(g_theKeyboard->Get_Modifiers());
                    break;
                default:
                    break;
            }

            switch (m_currMouse.middle_event) {
                case 9:
                    event_msg = g_theMessageStream->Append_Message(GameMessage::MSG_RAW_MOUSE_MIDDLE_BUTTON_DOWN);
                    event_msg->Append_Pixel_Arg(m_currMouse.pos);
                    event_msg->Append_Int_Arg(g_theKeyboard->Get_Modifiers());
                    event_msg->Append_Int_Arg(m_currMouse.wheel_pos);
                    break;
                case 10:
                    event_msg = g_theMessageStream->Append_Message(GameMessage::MSG_RAW_MOUSE_MIDDLE_BUTTON_UP);
                    event_msg->Append_Pixel_Arg(m_currMouse.pos);
                    event_msg->Append_Int_Arg(g_theKeyboard->Get_Modifiers());
                    event_msg->Append_Int_Arg(m_currMouse.wheel_pos);
                    break;
                case 11:
                    event_msg = g_theMessageStream->Append_Message(GameMessage::MSG_RAW_MOUSE_MIDDLE_DOUBLE_CLICK);
                    event_msg->Append_Pixel_Arg(m_currMouse.pos);
                    event_msg->Append_Int_Arg(g_theKeyboard->Get_Modifiers());
                    event_msg->Append_Int_Arg(m_currMouse.wheel_pos);
                    break;
                case 12:
                    event_msg = g_theMessageStream->Append_Message(GameMessage::MSG_RAW_MOUSE_MIDDLE_DRAG);
                    event_msg->Append_Pixel_Arg(m_currMouse.pos);
                    event_msg->Append_Pixel_Arg(m_currMouse.delta_pos);
                    event_msg->Append_Int_Arg(g_theKeyboard->Get_Modifiers());
                    break;
                default:
                    break;
            }

            switch (m_currMouse.right_event) {
                case 13:
                    event_msg = g_theMessageStream->Append_Message(GameMessage::MSG_RAW_MOUSE_RIGHT_BUTTON_DOWN);
                    event_msg->Append_Pixel_Arg(m_currMouse.pos);
                    event_msg->Append_Int_Arg(g_theKeyboard->Get_Modifiers());
                    event_msg->Append_Int_Arg(m_currMouse.wheel_pos);
                    break;
                case 14:
                    event_msg = g_theMessageStream->Append_Message(GameMessage::MSG_RAW_MOUSE_RIGHT_BUTTON_UP);
                    event_msg->Append_Pixel_Arg(m_currMouse.pos);
                    event_msg->Append_Int_Arg(g_theKeyboard->Get_Modifiers());
                    event_msg->Append_Int_Arg(m_currMouse.wheel_pos);
                    break;
                case 15:
                    event_msg = g_theMessageStream->Append_Message(GameMessage::MSG_RAW_MOUSE_RIGHT_DOUBLE_CLICK);
                    event_msg->Append_Pixel_Arg(m_currMouse.pos);
                    event_msg->Append_Int_Arg(g_theKeyboard->Get_Modifiers());
                    event_msg->Append_Int_Arg(m_currMouse.wheel_pos);
                    break;
                case 16:
                    event_msg = g_theMessageStream->Append_Message(GameMessage::MSG_RAW_MOUSE_RIGHT_DRAG);
                    event_msg->Append_Pixel_Arg(m_currMouse.pos);
                    event_msg->Append_Pixel_Arg(m_currMouse.delta_pos);
                    event_msg->Append_Int_Arg(g_theKeyboard->Get_Modifiers());
                    break;
                default:
                    break;
            }

            if (m_currMouse.wheel_delta != 0) {
                GameMessage *wheel_msg = g_theMessageStream->Append_Message(GameMessage::MSG_RAW_MOUSE_WHEEL);
                wheel_msg->Append_Pixel_Arg(m_currMouse.pos);
                wheel_msg->Append_Int_Arg(m_currMouse.wheel_delta / 120);
                wheel_msg->Append_Int_Arg(g_theKeyboard->Get_Modifiers());
            }
        }
    }
}

/**
 * @brief Sets the mouse position.
 *
 * 0x00403B40
 */
void Mouse::Set_Position(int x, int y)
{
    m_currMouse.pos.x = x;
    m_currMouse.pos.y = y;
}

/**
 * @brief Sets the limits on what mouse coordinates are valid based on current resolution.
 *
 * 0x00403B40
 */
void Mouse::Set_Mouse_Limits()
{
    m_minX = 0;
    m_minY = 0;

    if (g_theDisplay) {
        m_maxX = g_theDisplay->Get_Width();
        m_maxY = g_theDisplay->Get_Height();
    }
}

/**
 * @brief Causes mouse to update its configuration for the current resolution settings.
 *
 * 0x00403090
 */
void Mouse::Notify_Resolution_Change()
{
    if (m_tooltipDisplayString != nullptr) {
        g_theDisplayStringManager->Free_Display_String(m_tooltipDisplayString);
    }

    m_tooltipDisplayString = g_theDisplayStringManager->New_Display_String();

    Utf8String font_name;
    int font_size;
    bool font_bold;

    if (g_theGlobalLanguage != nullptr && g_theGlobalLanguage->Tooltip().Name().Is_Not_Empty()) {
        font_bold = g_theGlobalLanguage->Tooltip().Bold();
        font_size = g_theGlobalLanguage->Adjust_Font_Size(g_theGlobalLanguage->Tooltip().Point_Size());
        font_name = g_theGlobalLanguage->Tooltip().Name();
    } else {
        font_bold = m_tooltipFontIsBold;
        font_size = g_theGlobalLanguage->Adjust_Font_Size(m_tooltipFontSize);
        font_name = m_tooltipFontName;
    }

    m_tooltipDisplayString->Set_Font(g_theFontLibrary->Get_Font(font_name, font_size, font_bold));
    m_tooltipDisplayString->Set_Word_Wrap(TOOLTIP_WRAP);
}

void Mouse::Check_For_Drag()
{
    if (m_currMouse.left_state != 0) {
        if (m_prevMouse.left_event == 5 || m_prevMouse.left_event == 8) {
            m_currMouse.left_event = 8;
        }
    }

    if (m_currMouse.right_state != 0) {
        if (m_prevMouse.right_event == 13 || m_prevMouse.right_event == 16) {
            m_currMouse.right_event = 16;
        }
    }

    if (m_currMouse.middle_state != 0) {
        if (m_prevMouse.middle_event == 9 || m_prevMouse.middle_event == 12) {
            m_currMouse.middle_event = 12;
        }
    }
}

/**
 * @brief Updates mouse's internal data state.
 */
void Mouse::Update_Mouse_Data()
{
    static bool _busy = false;

    int events = 0;

    if (!_busy) {
        _busy = true;
        uint8_t tmp = 0;

        do {
            do {
                tmp = Get_Mouse_Event(&m_mouseEvents[events], 1);
            } while (tmp == UINT8_MAX);

            ++events;
        } while (tmp != 0 && events < MAX_EVENTS);

        _busy = false;
    }

    if (events > 0) {
        m_eventCount = events - 1;
    } else {
        m_eventCount = 0;
    }

    if (events != 0) {
        m_deadInputFrame = m_inputFrame;
    }
}

/**
 * @brief Processes a given mouse event in the event buffer.
 *
 * 0x004024E0
 */
void Mouse::Process_Mouse_Event(int event_num)
{
    m_currMouse.left_event = 0;
    m_currMouse.right_event = 0;
    m_currMouse.middle_event = 0;
    m_currMouse.wheel_delta = 0;
    m_currMouse.wheel_pos = m_mouseEvents[event_num].wheel_pos;

    if (event_num == 0) {
        Check_For_Drag();
    }

    Move_Mouse(m_mouseEvents[event_num].pos.x, m_mouseEvents[event_num].pos.y, m_inputMovesAbsolute);
    m_currMouse.wheel_delta += m_mouseEvents[event_num].wheel_delta;

    // Process the states of each mouse button (assumes 3 buttons).
    if (m_mouseEvents[event_num].left_frame != 0) {
        if (m_currMouse.left_state != m_mouseEvents[event_num].left_state) {
            switch (m_mouseEvents[event_num].left_state) {
                case 1:
                    m_currMouse.left_event = 5;
                    m_currMouse.left_state = 1;
                    m_currMouse.left_frame = m_inputFrame;
                    break;
                case 2:
                    m_currMouse.left_event = 7;
                    m_currMouse.left_state = 2;
                    m_currMouse.left_frame = m_inputFrame;
                    break;
                default:
                    m_currMouse.left_event = 6;
                    m_currMouse.left_state = 0;
                    m_currMouse.left_frame = m_inputFrame;
                    break;
            }
        }
    } else if (m_currMouse.left_state != 0) {
        if (m_prevMouse.left_event == 5 || m_prevMouse.left_event == 8) {
            m_currMouse.left_event = 8;
        }
    }

    if (m_mouseEvents[event_num].right_frame != 0) {
        if (m_currMouse.right_state != m_mouseEvents[event_num].right_state) {
            switch (m_mouseEvents[event_num].right_state) {
                case 1:
                    m_currMouse.right_event = 13;
                    m_currMouse.right_state = 1;
                    m_currMouse.right_frame = m_inputFrame;
                    break;
                case 2:
                    m_currMouse.right_event = 15;
                    m_currMouse.right_state = 2;
                    m_currMouse.right_frame = m_inputFrame;
                    break;
                default:
                    m_currMouse.right_event = 14;
                    m_currMouse.right_state = 0;
                    m_currMouse.right_frame = m_inputFrame;
                    break;
            }
        }
    } else if (m_currMouse.right_state != 0) {
        if (m_prevMouse.right_event == 13 || m_prevMouse.right_event == 16) {
            m_currMouse.right_event = 16;
        }
    }

    if (m_mouseEvents[event_num].middle_frame != 0) {
        if (m_currMouse.middle_state != m_mouseEvents[event_num].middle_state) {
            switch (m_mouseEvents[event_num].middle_state) {
                case 1:
                    m_currMouse.middle_event = 9;
                    m_currMouse.middle_state = 1;
                    m_currMouse.middle_frame = m_inputFrame;
                    break;
                case 2:
                    m_currMouse.middle_event = 11;
                    m_currMouse.middle_state = 2;
                    m_currMouse.middle_frame = m_inputFrame;
                    break;
                default:
                    m_currMouse.middle_event = 10;
                    m_currMouse.middle_state = 0;
                    m_currMouse.middle_frame = m_inputFrame;
                    break;
            }
        }
    } else if (m_currMouse.middle_state != 0) {
        if (m_prevMouse.middle_event == 9 || m_prevMouse.middle_event == 12) {
            m_currMouse.middle_event = 12;
        }
    }

    m_currMouse.delta_pos.x = m_currMouse.pos.x - m_prevMouse.pos.x;
    m_currMouse.delta_pos.y = m_currMouse.pos.y - m_prevMouse.pos.y;
    memcpy(&m_prevMouse, &m_currMouse, sizeof(m_prevMouse));
}

/**
 * @brief Moves the mouse position.
 *
 * 0x00402450
 */
void Mouse::Move_Mouse(int x, int y, int absolute)
{
    if (absolute) {
        m_currMouse.pos.x = x;
        m_currMouse.pos.y = y;
    } else {
        m_currMouse.pos.x += x;
        m_currMouse.pos.y += y;
    }

    m_currMouse.pos.x = std::clamp(m_currMouse.pos.x, m_minX, m_maxX);
    m_currMouse.pos.y = std::clamp(m_currMouse.pos.y, m_minY, m_maxY);
}

/**
 * @brief Gets the cursor enum value from its name. Used for parsing configuration files.
 */
MouseCursor Mouse::Get_Cursor_Index(const Utf8String &name)
{
    if (name.Is_Empty()) {
        return CURSOR_INVALID;
    }

    for (MouseCursor i = CURSOR_NONE; i < CURSOR_COUNT; ++i) {
        if (name == s_cursorNames[i]) {
            return i;
        }
    }

    captainslog_debug("Invalid cursor name '%s", name.Str());
    return CURSOR_INVALID;
}

// 0x00403FC0 (Originally called Set_Cursor but renamed to prevent confusion)
void Mouse::Set_Mouse_Text(const MouseCursor cursor)
{
    if (m_currentCursor != cursor) {

        if (m_cursorTextDisplayString != nullptr) {
            if (cursor < CURSOR_NONE || cursor >= CURSOR_COUNT) {
                captainslog_debug("Bad cursor ID %d", cursor);
            } else {
                Utf16String string = Utf16String::s_emptyString;
                const RGBAColorInt *color = nullptr;
                const RGBAColorInt *drop_color = nullptr;
                const CursorInfo *info = &m_cursorInfo[cursor];

                if (!info->cursor_text.Is_Empty()) {
                    color = &info->cursor_text_color;
                    drop_color = &info->cursor_text_drop_color;
                    string = g_theGameText->Fetch(info->cursor_text, nullptr);
                }

                Set_Mouse_Text(string, color, drop_color);
            }
        }
    }
}

// 0x00403A80
void Mouse::Set_Mouse_Text(const Utf16String text, const RGBAColorInt *color, const RGBAColorInt *drop_color)
{
    if (m_cursorTextDisplayString != nullptr) {
        m_cursorTextDisplayString->Set_Text(text);

        if (color != nullptr) {
            m_cursorTextColor = *color;
        }

        if (drop_color != nullptr) {
            m_cursorTextDropColor = *drop_color;
        }
    }
}

/**
 * @brief Parses "Mouse" configuration sections from .ini files.
 * @see ini.cpp
 * Was originally INI::parseMouseDefinition
 *
 * 0x004041F0
 */
void Mouse::Parse_Mouse_Definition(INI *ini)
{
    static const FieldParse _static_mouse_parsers[] = {
        { "TooltipFontName", &INI::Parse_AsciiString, nullptr, offsetof(Mouse, m_tooltipFontName) },
        { "TooltipFontSize", &INI::Parse_Int, nullptr, offsetof(Mouse, m_tooltipFontSize) },
        { "TooltipFontIsBold", &INI::Parse_Bool, nullptr, offsetof(Mouse, m_tooltipFontIsBold) },
        { "TooltipAnimateBackground", &INI::Parse_Bool, nullptr, offsetof(Mouse, m_tooltipAnimateBackground) },
        { "TooltipFillTime", &INI::Parse_Int, nullptr, offsetof(Mouse, m_tooltipFillTime) },
        { "TooltipDelayTime", &INI::Parse_Int, nullptr, offsetof(Mouse, m_tooltipDelayTime) },
        { "TooltipTextColor", &INI::Parse_RGBA_Color_Int, nullptr, offsetof(Mouse, m_tooltipColorText) },
        { "TooltipHighlightColor", &INI::Parse_RGBA_Color_Int, nullptr, offsetof(Mouse, m_tooltipColorHighlight) },
        { "TooltipShadowColor", &INI::Parse_RGBA_Color_Int, nullptr, offsetof(Mouse, m_tooltipColorShadow) },
        { "TooltipBackgroundColor", &INI::Parse_RGBA_Color_Int, nullptr, offsetof(Mouse, m_tooltipColorBackground) },
        { "TooltipBorderColor", &INI::Parse_RGBA_Color_Int, nullptr, offsetof(Mouse, m_tooltipColorBorder) },
        { "TooltipWidth", &INI::Parse_Percent_To_Real, nullptr, offsetof(Mouse, m_tooltipWidth) },
        { "CursorMode", &INI::Parse_Int, nullptr, offsetof(Mouse, m_currentRedrawMode) },
        { "UseTooltipAltTextColor", &INI::Parse_Bool, nullptr, offsetof(Mouse, m_useTooltipAltTextColor) },
        { "UseTooltipAltBackColor", &INI::Parse_Bool, nullptr, offsetof(Mouse, m_useTooltipAltBackColor) },
        { "AdjustTooltipAltColor", &INI::Parse_Bool, nullptr, offsetof(Mouse, m_adjustTooltipAltColor) },
        { "OrthoCamera", &INI::Parse_Bool, nullptr, offsetof(Mouse, m_orthoCamera) },
        { "OrthoZoom", &INI::Parse_Real, nullptr, offsetof(Mouse, m_orthoZoom) },
        { "DragTolerance", &INI::Parse_Unsigned_Int, nullptr, offsetof(Mouse, m_dragTolerance) },
        { "DragTolerance3D", &INI::Parse_Unsigned_Int, nullptr, offsetof(Mouse, m_dragTolerance3D) },
        { "DragToleranceMS", &INI::Parse_Unsigned_Int, nullptr, offsetof(Mouse, m_dragToleranceMS) },
        { nullptr, nullptr, nullptr, 0 },
    };

    if (g_theMouse != nullptr) {
        ini->Init_From_INI(g_theMouse, _static_mouse_parsers);
    }
}

/**
 * @brief Parses "MouseCursor" configuration sections from .ini files.
 * @see ini.cpp
 * Was originally INI::parseMouseCursorDefinition
 *
 * 0x00404060
 */
void Mouse::Parse_Cursor_Definition(INI *ini)
{
    static const FieldParse _cursor_parsers[] = {
        { "CursorText", &INI::Parse_AsciiString, nullptr, offsetof(CursorInfo, cursor_text) },
        { "CursorTextColor", &INI::Parse_RGBA_Color_Int, nullptr, offsetof(CursorInfo, cursor_text_color) },
        { "CursorTextDropColor", &INI::Parse_RGBA_Color_Int, nullptr, offsetof(CursorInfo, cursor_text_drop_color) },
        { "W3DModel", &INI::Parse_AsciiString, nullptr, offsetof(CursorInfo, w3d_model_name) },
        { "W3DAnim", &INI::Parse_AsciiString, nullptr, offsetof(CursorInfo, w3d_anim_name) },
        { "W3DScale", &INI::Parse_Real, nullptr, offsetof(CursorInfo, w3d_scale) },
        { "Loop", &INI::Parse_Bool, nullptr, offsetof(CursorInfo, loop) },
        { "Image", &INI::Parse_AsciiString, nullptr, offsetof(CursorInfo, image_name) },
        { "Texture", &INI::Parse_AsciiString, nullptr, offsetof(CursorInfo, texture_name) },
        { "HotSpot", &INI::Parse_ICoord2D, nullptr, offsetof(CursorInfo, hot_spot) },
        { "Frames", &INI::Parse_Int, nullptr, offsetof(CursorInfo, frames) },
        { "FPS", &INI::Parse_Real, nullptr, offsetof(CursorInfo, fps) },
        { "Directions", &INI::Parse_Int, nullptr, offsetof(CursorInfo, directions) },
        { nullptr, nullptr, nullptr, 0 },
    };

    Utf8String tok = ini->Get_Next_Token();

    if (g_theMouse != nullptr && tok.Is_Not_Empty()) {
        ini->Init_From_INI(&g_theMouse->m_cursorInfo[g_theMouse->Get_Cursor_Index(tok)], _cursor_parsers);
    }
}

bool Mouse::Is_Click(ICoord2D *click_1_coord, ICoord2D *click_2_coord, unsigned click_1_time, unsigned click_2_time) const
{
    return static_cast<unsigned>(abs(click_1_coord->x - click_2_coord->x)) <= m_dragTolerance
        && static_cast<unsigned>(abs(click_1_coord->y - click_2_coord->y)) <= m_dragTolerance
        && click_2_time - click_1_time <= m_dragToleranceMS;
}

void Mouse::Reset_Tooltip_Delay()
{
    m_stillTime = Get_Time();
    m_displayTooltip = false;
}

void Mouse::Draw_Cursor_Text() const
{
    if (m_cursorTextDisplayString) {
        uint32_t tcolor =
            Make_Color(m_cursorTextColor.red, m_cursorTextColor.green, m_cursorTextColor.blue, m_cursorTextColor.alpha);
        uint32_t dcolor = Make_Color(
            m_cursorTextDropColor.red, m_cursorTextDropColor.green, m_cursorTextDropColor.blue, m_cursorTextDropColor.alpha);

        int width;
        int height;
        m_cursorTextDisplayString->Get_Size(&width, &height);
        m_cursorTextDisplayString->Draw(m_currMouse.pos.x - width / 2, m_currMouse.pos.y - height / 2, tcolor, dcolor);
    }
}

void Mouse::Set_Cursor_Tooltip(Utf16String tooltip, int delay, const RGBColor *color, float scale)
{
    m_isTooltipEmpty = tooltip.Is_Empty();
    m_tooltipDelay = delay;
    bool scale_changed = false;

    if (!tooltip.Is_Empty()) {
        if (scale != m_tooltipScale) {
            scale_changed = true;
            unsigned int length = (g_theDisplay->Get_Width() * m_tooltipWidth * scale);

            if (length < 10) {
                length = 120;
            } else if (length > g_theDisplay->Get_Width()) {
                length = g_theDisplay->Get_Width();
            }

            m_tooltipDisplayString->Set_Word_Wrap(length);
            m_tooltipScale = scale;
        }
    }

    if (scale_changed || (!m_isTooltipEmpty && tooltip.Compare(m_tooltipDisplayString->Get_Text()))) {
        m_tooltipDisplayString->Set_Text(tooltip);
    }

    if (color != nullptr) {
        if (m_useTooltipAltTextColor) {
            if (m_adjustTooltipAltColor) {
                m_tooltipColorTextCopy.red = GameMath::Fast_To_Int_Truncate((color->red + 1.0f) * 255.0f / 2.0f);
                m_tooltipColorTextCopy.green = GameMath::Fast_To_Int_Truncate((color->green + 1.0f) * 255.0f / 2.0f);
                m_tooltipColorTextCopy.blue = GameMath::Fast_To_Int_Truncate((color->blue + 1.0f) * 255.0f / 2.0f);
            } else {
                m_tooltipColorTextCopy.red = GameMath::Fast_To_Int_Truncate(color->red * 255.0f);
                m_tooltipColorTextCopy.green = GameMath::Fast_To_Int_Truncate(color->green * 255.0f);
                m_tooltipColorTextCopy.blue = GameMath::Fast_To_Int_Truncate(color->blue * 255.0f);
            }

            m_tooltipColorTextCopy.alpha = m_tooltipColorText.alpha;
        }

        if (m_useTooltipAltBackColor) {
            if (m_adjustTooltipAltColor) {
                m_tooltipColorBackgroundCopy.red = GameMath::Fast_To_Int_Truncate(color->red * 255.0f * 0.5f);
                m_tooltipColorBackgroundCopy.green = GameMath::Fast_To_Int_Truncate(color->green * 255.0f * 0.5f);
                m_tooltipColorBackgroundCopy.blue = GameMath::Fast_To_Int_Truncate(color->blue * 255.0f * 0.5f);
            } else {
                m_tooltipColorBackgroundCopy.red = GameMath::Fast_To_Int_Truncate(color->red * 255.0f);
                m_tooltipColorBackgroundCopy.green = GameMath::Fast_To_Int_Truncate(color->green * 255.0f);
                m_tooltipColorBackgroundCopy.blue = GameMath::Fast_To_Int_Truncate(color->blue * 255.0f);
            }

            m_tooltipColorBackgroundCopy.alpha = m_tooltipColorBackground.alpha;
        }
    } else {
        m_tooltipColorTextCopy = m_tooltipColorText;
        m_tooltipColorBackgroundCopy = m_tooltipColorBackground;
    }
}

void Mouse::Draw_Tooltip()
{
    if (g_theScriptEngine->Get_Fade() == ScriptEngine::FADE_NONE && m_displayTooltip && g_theDisplay != nullptr
        && m_tooltipDisplayString != nullptr && m_tooltipDisplayString->Get_Text_Length() > 0 && !m_isTooltipEmpty) {
        int string_w;
        int string_h;
        m_tooltipDisplayString->Get_Size(&string_w, &string_h);
        int mouse_x = m_currMouse.pos.x + 20;
        int mouse_y = m_currMouse.pos.y;

        if (mouse_x + string_w + 4 > m_maxX) {
            mouse_x -= string_w + 20;
        }

        if (mouse_y + string_h + 4 > m_maxY) {
            mouse_y -= string_h;
        }

        int width;

        if (m_tooltipAnimateBackground) {
            width = GameMath::Min(string_w, m_highlightPos);
        } else {
            width = string_w;
        }

        g_theDisplay->Draw_Fill_Rect(mouse_x,
            mouse_y,
            width + 2,
            string_h + 2,
            Make_Color(m_tooltipColorBackgroundCopy.red,
                m_tooltipColorBackgroundCopy.green,
                m_tooltipColorBackgroundCopy.blue,
                m_tooltipColorBackgroundCopy.alpha));

        g_theDisplay->Draw_Open_Rect(mouse_x,
            mouse_y,
            width + 2,
            string_h + 2,
            1.0f,
            Make_Color(m_tooltipColorBorder.red,
                m_tooltipColorBorder.green,
                m_tooltipColorBorder.blue,
                m_tooltipColorBorder.alpha));

        IRegion2D clip_region;
        clip_region.lo.x = mouse_x + 2;
        clip_region.lo.y = mouse_y + 1;
        clip_region.hi.x = mouse_x + m_highlightPos + 2;
        clip_region.hi.y = mouse_y + string_h + 1;
        m_tooltipDisplayString->Set_Clip_Region(&clip_region);

        m_tooltipDisplayString->Draw(mouse_x + 2,
            mouse_y + 1,
            Make_Color(m_tooltipColorTextCopy.red,
                m_tooltipColorTextCopy.green,
                m_tooltipColorTextCopy.blue,
                m_tooltipColorTextCopy.alpha),
            Make_Color(m_tooltipColorShadow.red,
                m_tooltipColorShadow.green,
                m_tooltipColorShadow.blue,
                m_tooltipColorShadow.alpha));

        clip_region.lo.x = mouse_x + m_highlightPos - 13;
        clip_region.lo.y = mouse_y + 1;
        clip_region.hi.x = mouse_x + m_highlightPos + 2;
        clip_region.hi.y = mouse_y + string_h + 1;
        m_tooltipDisplayString->Set_Clip_Region(&clip_region);

        m_tooltipDisplayString->Draw(mouse_x + 2,
            mouse_y + 1,
            Make_Color(m_tooltipColorHighlight.red,
                m_tooltipColorHighlight.green,
                m_tooltipColorHighlight.blue,
                m_tooltipColorHighlight.alpha),
            Make_Color(m_tooltipColorShadow.red,
                m_tooltipColorShadow.green,
                m_tooltipColorShadow.blue,
                m_tooltipColorShadow.alpha));

        if (m_highlightPos < string_w + 15) {
            m_highlightPos = (rts::Get_Time() - m_highlightUpdateStart) * string_w / m_tooltipFillTime;
        }
    }
}
