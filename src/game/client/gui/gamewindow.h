/**
 * @file
 *
 * @author tomsons26
 *
 * @brief
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
#include "coord.h"
#include "gamelogic.h"
#include "mempoolobj.h"
#include "unicodestring.h"
#include "wininstancedata.h"

class GameWindowManager;
class GameFont;

// TODO confirm
enum WindowMsgHandledType
{
    MSG_IGNORED,
    MSG_HANDLED,
};

// TODO confirm
enum WinInputReturnCode
{
    WIN_INPUT_NOT_USED,
    WIN_INPUT_USED,
};

enum WinStatus
{
    WIN_STATUS_NONE,

    WIN_STATUS_ACTIVE = 1 << 0,
    WIN_STATUS_TOGGLE = 1 << 1,
    WIN_STATUS_DRAGABLE = 1 << 2,
    WIN_STATUS_ENABLED = 1 << 3,
    WIN_STATUS_HIDDEN = 1 << 4,
    WIN_STATUS_ABOVE = 1 << 5,
    WIN_STATUS_BELOW = 1 << 6,
    WIN_STATUS_IMAGE = 1 << 7,
    WIN_STATUS_TAB_STOP = 1 << 8,
    WIN_STATUS_NO_INPUT = 1 << 9,
    WIN_STATUS_NO_FOCUS = 1 << 10,
    WIN_STATUS_DESTROYED = 1 << 11,
    WIN_STATUS_BORDER = 1 << 12,
    WIN_STATUS_SMOOTH_TEXT = 1 << 13,
    WIN_STATUS_ONE_LINE = 1 << 14,
    WIN_STATUS_NO_FLUSH = 1 << 15,
    WIN_STATUS_SEE_THRU = 1 << 16,
    WIN_STATUS_RIGHT_CLICK = 1 << 17,
    WIN_STATUS_WRAP_CENTERED = 1 << 18,
    WIN_STATUS_CHECK_LIKE = 1 << 19,
    WIN_STATUS_HOTKEY_TEXT = 1 << 20,
    WIN_STATUS_USE_OVERLAY_STATES = 1 << 21,
    WIN_STATUS_NOT_READY = 1 << 22,
    WIN_STATUS_FLASHING = 1 << 23,
    WIN_STATUS_ALWAYS_COLOR = 1 << 24,
    WIN_STATUS_ON_MOUSE_DOWN = 1 << 25,
    WIN_STATUS_UNK = 1 << 26,
};

enum GameWindowStyle
{
    GWS_PUSH_BUTTON = 1 << 0,
    GWS_RADIO_BUTTON = 1 << 1,

    GWS_CHECK_BOX = 1 << 2,

    GWS_VERT_SLIDER = 1 << 3,
    GWS_HORZ_SLIDER = 1 << 4,

    GWS_SCROLL_LISTBOX = 1 << 5,

    GWS_ENTRY_FIELD = 1 << 6,
    GWS_STATIC_TEXT = 1 << 7,

    GWS_PROGRESS_BAR = 1 << 8,

    GWS_USER_WINDOW = 1 << 9,

    GWS_MOUSE_TRACK = 1 << 10,

    GWS_ANIMATED = 1 << 11,

    GWS_TAB_STOP = 1 << 12,
    GWS_TAB_CONTROL = 1 << 13,
    GWS_TAB_PANE = 1 << 14,

    GWS_COMBO_BOX = 1 << 15,
    GWS_ALL_SLIDER = GWS_VERT_SLIDER | GWS_HORZ_SLIDER,
    GWS_GADGET_WINDOW = GWS_COMBO_BOX | GWS_TAB_CONTROL | GWS_PROGRESS_BAR | GWS_STATIC_TEXT | GWS_ENTRY_FIELD
        | GWS_SCROLL_LISTBOX | GWS_HORZ_SLIDER | GWS_VERT_SLIDER | GWS_CHECK_BOX | GWS_RADIO_BUTTON | GWS_PUSH_BUTTON,
};

enum GameWindowMessage
{
    GWM_NONE,

    GWM_CREATE, // Freshly created window
    GWM_DESTROY, // Window is being destroyed
    GWM_ACTIVATE,
    GWM_ENABLE,

    // Left mouse
    GWM_LEFT_DOWN,
    GWM_LEFT_UP,
    GWM_LEFT_CLICK,
    GWM_LEFT_DRAG,

    // Middle mouse
    GWM_MIDDLE_DOWN,
    GWM_MIDDLE_UP,
    GWM_MIDDLE_CLICK,
    GWM_MIDDLE_DRAG,

    // Right mouse
    GWM_RIGHT_DOWN,
    GWM_RIGHT_UP,
    GWM_RIGHT_CLICK,
    GWM_RIGHT_DRAG,

    GWM_MOUSE_ENTERING,
    GWM_MOUSE_LEAVING,

    GWM_WHEEL_UP,
    GWM_WHEEL_DOWN,

    GWM_CHAR, // Keyboard input

    GWM_SCRIPT_CREATE, // Freshly created window from a WND script

    GWM_INPUT_FOCUS, // Window gained input focus

    GWM_MOUSE_POS, // Mouse position

    GWM_IME_CHAR, // IME input

    GWM_IME_STRING,
};

enum GadgetGameMessage
{
    GGM_LEFT_DRAG = 0x4000,
    GGM_SET_LABEL,
    GGM_GET_LABEL,
    GGM_FOCUS_CHANGE,
    GGM_RESIZED,
    GGM_CLOSE,
    GBM_MOUSE_ENTERING,
    GBM_MOUSE_LEAVING,
    GBM_SELECTED,
    GBM_SELECTED_RIGHT,
    GBM_SET_SELECTION,
    GSM_SLIDER_TRACK,
    GSM_SET_SLIDER,
    GSM_SET_MIN_MAX,
    GSM_SLIDER_DONE,
    GLM_ADD_ENTRY,
    GLM_DEL_ENTRY,
    GLM_DEL_ALL,
    GLM_SELECTED,
    GLM_DOUBLE_CLICKED,
    GLM_RIGHT_CLICKED,
    GLM_SET_SELECTION,
    GLM_GET_SELECTION,
    GLM_TOGGLE_MULTI_SELECTION,
    GLM_GET_TEXT,
    GLM_SET_UP_BUTTON,
    GLM_SET_DOWN_BUTTON,
    GLM_SET_SLIDER,
    GLM_SCROLL_BUFFER,
    GLM_UPDATE_DISPLAY,
    GLM_GET_ITEM_DATA,
    GLM_SET_ITEM_DATA,
    GCM_ADD_ENTRY,
    GCM_DEL_ENTRY,
    GCM_DEL_ALL,
    GCM_SELECTED,
    GCM_GET_TEXT,
    GCM_SET_TEXT,
    GCM_EDIT_DONE,
    GCM_GET_ITEM_DATA,
    GCM_SET_ITEM_DATA,
    GCM_GET_SELECTION,
    GCM_SET_SELECTION,
    GCM_UPDATE_TEXT,
    GEM_GET_TEXT,
    GEM_SET_TEXT,
    GEM_EDIT_DONE,
    GEM_UPDATE_TEXT,
    GPM_SET_PROGRESS,
};

typedef WindowMsgHandledType(__cdecl *WindowCallbackFunc)(
    GameWindow *window, unsigned int message, unsigned int data_1, unsigned int data_2);

typedef void(__cdecl *WindowDrawFunc)(GameWindow *window, WinInstanceData *instance);
typedef void(__cdecl *WindowTooltipFunc)(GameWindow *window, WinInstanceData *instance, unsigned int mouse);

typedef void(__cdecl *WindowMsgBoxCallbackFunc)();

struct GameWindowEditData
{
    Utf8String system_callback_string;
    Utf8String input_callback_string;
    Utf8String tooltip_callback_string;
    Utf8String draw_callback_string;
};

class GameWindow : public MemoryPoolObject
{
    friend GameWindowManager;
    friend void W3DGameWinDefaultDraw(GameWindow *window, WinInstanceData *instance);

    IMPLEMENT_ABSTRACT_POOL(GameWindow);

public:
    GameWindow();
    virtual ~GameWindow();

    virtual void Win_Draw_Border() = 0;

    virtual int Win_Set_Text(Utf16String new_text);
    virtual void Win_Set_Font(GameFont *font);

    void Normalize_Window_Region();

    int Win_Next_Tab();
    int Win_Prev_Tab();

    int Win_Set_Position(int x, int y);
    int Win_Get_Position(int *x, int *y);

    int Win_Set_Cursor_Position(int x, int y);
    int Win_Get_Cursor_Position(int *x, int *y);

    int Win_Get_Screen_Position(int *x, int *y);

    int Win_Get_Region(IRegion2D *region);

    bool Win_Point_In_Window(int x, int y);

    int Win_Set_Size(int width, int height);
    int Win_Get_Size(int *width, int *height);

    int Win_Enable(bool enable);
    bool Win_Get_Enabled();

    int Win_Hide(bool hide);
    bool Win_Is_Hidden();

    int Win_Set_Status(int status);
    int Win_Clear_Status(int status);
    int Win_Get_Status();

    unsigned Win_Get_Style();

    void Win_Set_Hilite_State(bool state);

    void Win_Set_Draw_Offset(int x, int y);
    void Win_Get_Draw_Offset(int *x, int *y);

    Utf16String Win_Get_Text();
    int Win_Get_Text_Length();

    GameFont *Win_Get_Font();

    int Win_Get_Enabled_Text_Color();
    int Win_Get_Enabled_Text_Border_Color();

    int Win_Get_IME_Composite_Text_Color();
    int Win_Get_IME_Composite_Border_Color();

    int Win_Get_Disabled_Text_Color();
    int Win_Get_Disabled_Text_Border_Color();

    int Win_Get_Hilite_Text_Color();
    int Win_Get_Hilite_Text_Border_Color();

    int Win_Set_Instance_Data(WinInstanceData *instance);
    WinInstanceData *Win_Get_Instance_Data();

    void *Win_Get_User_Data();
    void Win_Set_User_Data(void *user_data);

    void Win_Set_Tooltip(Utf16String tooltip);

    int Win_Set_Window_Id(int id);
    int Win_Get_Window_Id();

    int Win_Set_Parent(GameWindow *parent);
    GameWindow *Win_Get_Parent();

    bool Win_Is_Child(GameWindow *window);
    GameWindow *Win_Get_Child();

    int Win_Set_Owner(GameWindow *owner);
    GameWindow *Win_Get_Owner();

    void Win_Set_Next(GameWindow *window);
    GameWindow *Win_Get_Next();

    void Win_Set_Prev(GameWindow *window);
    GameWindow *Win_Get_Prev();

    void Win_Set_Next_In_Layout(GameWindow *layout);
    void Win_Set_Prev_In_Layout(GameWindow *layout);
    void Win_Set_Layout(WindowLayout *layout);

    WindowLayout *Win_Get_Layout();
    GameWindow *Win_Get_Next_In_Layout();
    GameWindow *Win_Get_Prev_In_Layout();

    int Win_Set_Draw_Func(WindowDrawFunc draw);
    int Win_Set_Input_Func(WindowCallbackFunc input);
    int Win_Set_System_Func(WindowCallbackFunc system);
    int Win_Set_Tooltip_Func(WindowTooltipFunc tooltip);
    int Win_Set_Callbacks(WindowCallbackFunc input, WindowDrawFunc draw, WindowTooltipFunc tooltip);

    int Win_Draw_Window();

    GameWindow *Win_Point_In_Child(int x, int y, bool ignore_enable_check, bool audio_event);
    GameWindow *Win_Point_In_Any_Child(int x, int y, bool ignore_hidden, bool ignore_enable_check);

    int Win_Set_Enabled_Image(int index, const Image *image);
    int Win_Set_Enabled_Color(int index, int color);
    int Win_Set_Enabled_Border_Color(int index, int color);

    const Image *Win_Get_Enabled_Image(int index) { return m_instData.m_enabledDrawData[index].image; }
    int Win_Get_Enabled_Color(int index) { return m_instData.m_enabledDrawData[index].color; }
    int Win_Get_Enabled_Border_Color(int index) { return m_instData.m_enabledDrawData[index].borderColor; }

    int Win_Set_Disabled_Image(int index, const Image *image);
    int Win_Set_Disabled_Color(int index, int color);
    int Win_Set_Disabled_Border_Color(int index, int color);

    const Image *Win_Get_Disabled_Image(int index) { return m_instData.m_disabledDrawData[index].image; }
    int Win_Get_Disabled_Color(int index) { return m_instData.m_disabledDrawData[index].color; }
    int Win_Get_Disabled_Border_Color(int index) { return m_instData.m_disabledDrawData[index].borderColor; }

    int Win_Set_Hilite_Image(int index, const Image *image);
    int Win_Set_Hilite_Color(int index, int color);
    int Win_Set_Hilite_Border_Color(int index, int color);

    int Win_Get_Hilite_Color(int index) { return m_instData.m_hiliteDrawData[index].color; }
    int Win_Get_Hilite_Border_Color(int index) { return m_instData.m_hiliteDrawData[index].borderColor; }

    WindowCallbackFunc Win_Get_Input_Func();
    WindowCallbackFunc Win_Get_System_Func();
    WindowTooltipFunc Win_Get_Tooltip_Func();
    WindowDrawFunc Win_Get_Draw_Func();

    int Win_Bring_To_Top();

    void Win_Set_Edit_Data(GameWindowEditData *edit_data);
    GameWindowEditData *Win_Get_Edit_Data();

    GameWindow *Find_First_Leaf();
    GameWindow *Find_Last_Leaf();
    GameWindow *Find_Prev_Leaf();
    GameWindow *Find_Next_Leaf();

    void Win_Set_Disabled_Text_Colors(int color, int border_color);
    void Win_Set_Enabled_Text_Colors(int color, int border_color);
    void Win_Set_Hilite_Text_Colors(int color, int border_color);
    void Win_Set_IME_Composite_Text_Colors(int color, int border_color);

    int Win_Activate();

    int Get_Tooltip_Delay() { return m_instData.m_tooltipDelay; }
    void Set_tooltip_Delay(int delay) { m_instData.m_tooltipDelay = delay; }

    void Win_Set_ID(int id) { m_instData.m_id = id; }

protected:
    int m_status;
    ICoord2D m_size;
    IRegion2D m_region;
    int m_cursorX;
    int m_cursorY;
    void *m_userData;
    WinInstanceData m_instData;
    void *m_inputData;
    WindowCallbackFunc m_input;
    WindowCallbackFunc m_system;
    WindowDrawFunc m_draw;
    WindowTooltipFunc m_tooltip;
    GameWindow *m_next;
    GameWindow *m_prev;
    GameWindow *m_parent;
    GameWindow *m_child;
    GameWindow *m_nextLayout;
    GameWindow *m_prevLayout;
    WindowLayout *m_layout;
    GameWindowEditData *m_editData;
};

class ModalWindow : public MemoryPoolObject
{
    IMPLEMENT_POOL(ModalWindow);

public:
    ModalWindow() :
        // BUGFIX init all members
        m_window(nullptr),
        m_next(nullptr)
    {
    }
    virtual ~ModalWindow() override {}

public:
    GameWindow *m_window;
    ModalWindow *m_next;
};

WindowMsgHandledType Game_Win_Default_Input(GameWindow *window, unsigned int msg, unsigned int data1, unsigned int data2);
WindowMsgHandledType Game_Win_Block_Input(GameWindow *window, unsigned int msg, unsigned int data1, unsigned int data2);
WindowMsgHandledType Game_Win_Default_System(GameWindow *window, unsigned int msg, unsigned int data1, unsigned int data2);
void Game_Win_Default_Tooltip(GameWindow *window, WinInstanceData *inst_data, unsigned int mouse);
void Game_Win_Default_Draw(GameWindow *window, WinInstanceData *inst_data);
