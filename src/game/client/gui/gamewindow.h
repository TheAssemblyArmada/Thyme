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

class GameFont;

// TODO confirm
enum WindowMsgHandledType
{
    MSG_IGNORED,
    MSG_HANDLED,
};

typedef WindowMsgHandledType(__cdecl *WindowCallbackFunc)(
    GameWindow *window, unsigned int message, unsigned int data_1, unsigned int data_2);

typedef void(__cdecl *WindowDrawFunc)(GameWindow *window, WinInstanceData *instance);
typedef void(__cdecl *WindowTooltipFunc)(GameWindow *window, WinInstanceData *instance, unsigned int mouse);

struct GameWindowEditData
{
    Utf8String system_callback_string;
    Utf8String input_callback_string;
    Utf8String tooltip_callback_string;
    Utf8String draw_callback_string;
};

class GameWindow : public MemoryPoolObject
{
    IMPLEMENT_ABSTRACT_POOL(GameWindow);

public:
    // TODO others
    enum Status
    {
        WIN_STATUS_NONE,

        WIN_STATUS_ENABLED = 1 << 3,
        WIN_STATUS_HIDDEN = 1 << 4,
    };

    GameWindow();
    virtual ~GameWindow();

    virtual void Win_Draw_Border() = 0;
    virtual int Win_Set_Text(Utf16String new_text);
    virtual void Win_Set_Font(GameFont *font);

    void Normalize_Window_Region();

    int Win_Set_Position(int x, int y);
    int Win_Get_Position(int *x, int *y);
    int Win_Get_Screen_Position(int *x, int *y);

    int Win_Get_Size(int *width, int *height);

    int Win_Enable(bool enable);

    bool Win_Is_Hidden();

    int Win_Clear_Status(int status);
    int Win_Get_Status();

    unsigned Win_Get_Style();

    int Win_Get_Text_Length();

    GameFont *Win_Get_Font();

    int Win_Get_Enabled_Text_Color();
    int Win_Get_Enabled_Text_Border_Color();

    int Win_Get_Hilite_Text_Color();
    int Win_Get_Hilite_Text_Border_Color();

    int Win_Set_Instance_Data(WinInstanceData *instance);
    WinInstanceData *Win_Get_Instance_Data();

    void *Win_Get_User_Data();
    void Win_Set_User_Data(void *user_data);

    int Win_Set_Window_Id(int id);
    int Win_Get_Window_Id();

    GameWindow *Win_Get_Parent();
    GameWindow *Win_Get_Child();

    int Win_Set_Owner(GameWindow *owner);
    GameWindow *Win_Get_Owner();

    void Win_Set_Next_In_Layout(GameWindow *layout);
    void Win_Set_Prev_In_Layout(GameWindow *layout);
    void Win_Set_Layout(WindowLayout *layout);

    WindowLayout *Win_Get_Layout();
    GameWindow *Win_Get_Next_In_Layout();
    GameWindow *Win_Get_Prev_In_Layout();

    int Win_Set_Tooltip_Func(WindowTooltipFunc tooltip);

    GameWindow *Win_Point_In_Child(int x, int y, bool ignore_enable_check, bool audio_event);
    GameWindow *Win_Point_In_Any_Child(int x, int y, bool ignore_hidden, bool ignore_enable_check);

    int Win_Set_Enabled_Image(int index, Image *image);
    int Win_Set_Enabled_Color(int index, int color);

    GameWindowEditData *Win_Get_Edit_Data();

protected:
    int m_status;
    ICoord2D m_size;
    IRegion2D m_region;
    int m_cursorX;
    int m_cursorY;
    void *m_userData;
    WinInstanceData m_instData;
#ifdef GAME_DEBUG_STRUCTS
    void *m_inputData;
#endif
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
