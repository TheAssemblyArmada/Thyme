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

#include "gamewindow.h"

#include "audioeventrts.h"
#include "audiomanager.h"
#include "displaystring.h"
#include "gamelogic.h"

#if 0
GameWindow::GameWindow() :
    m_status(0),
    m_size{ 0, 0 },
    m_region{0, 0, 0, 0},
    m_cursorX(0),
    m_cursorY(0),
    m_userData(nullptr),
    m_inputData(nullptr),
    m_input(nullptr),
    m_system(nullptr),
    m_draw(nullptr),
    m_tooltip(nullptr),
    m_next(nullptr),
    m_prev(nullptr),
    m_parent(nullptr),
    m_child(nullptr),
    m_nextLayout(nullptr),
    m_prevLayout(nullptr),
    m_layout(nullptr),
    m_editData(nullptr)
{
    // TODO, needs WindowManager
    // Win_Set_Draw_Func;
    // Win_Set_Input_Func;
    // Win_Set_System_Func;
    // Win_Set_Tooltip_Func;
}
#endif

GameWindow::~GameWindow()
{
    delete m_inputData;
    delete m_editData;
}

void GameWindow::Normalize_Window_Region()
{
    std::swap(m_region.lo.x, m_region.hi.x);
    std::swap(m_region.lo.y, m_region.hi.y);
}

int GameWindow::Win_Next_Tab()
{
    return 0;
}

int GameWindow::Win_Prev_Tab()
{
    return 0;
}

int GameWindow::Win_Set_Position(int x, int y)
{
    m_region.lo.x = x;
    m_region.lo.y = y;
    m_region.hi.x = m_size.x + x;
    m_region.hi.y = m_size.y + y;

    Normalize_Window_Region();

    return 0;
}

int GameWindow::Win_Get_Position(int *x, int *y)
{
    if (x == nullptr || y == nullptr) {
        // TODO figure out what -3 means
        return -3;
    }

    *x = m_region.lo.x;
    *y = m_region.lo.y;

    return 0;
}

int GameWindow::Win_Set_Cursor_Position(int x, int y)
{
    m_cursorX = x;
    m_cursorY = y;

    return 0;
}

int GameWindow::Win_Get_Cursor_Position(int *x, int *y)
{
    if (x != nullptr) {
        *x = m_cursorX;
    }
    if (y != nullptr) {
        *y = m_cursorY;
    }

    return 0;
}

int GameWindow::Win_Get_Screen_Position(int *x, int *y)
{
    // BUGFIX make sure its not nullptr
    if (x == nullptr || y == nullptr) {
        // TODO figure out what -3 means
        return -3;
    }

    GameWindow *parent = m_parent;

    *x = m_region.lo.x;
    *y = m_region.lo.y;

    while (parent != nullptr) {
        *x += parent->m_region.lo.x;
        *y += parent->m_region.lo.y;
        parent = parent->m_parent;
    }

    return 0;
}

int GameWindow::Win_Get_Region(IRegion2D *region)
{
    if (region != nullptr) {
        *region = m_region;
    }

    return 0;
}

bool GameWindow::Win_Point_In_Window(int x, int y)
{
    int win_x;
    int win_y;
    int width;
    int height;

    Win_Get_Screen_Position(&win_x, &win_y);
    Win_Get_Size(&width, &height);

    return x >= win_x && x <= width + win_x && y >= win_y && y <= height + win_y;
}

int GameWindow::Win_Get_Size(int *width, int *height)
{
    if (!width || !height) {
        return -3;
    }

    *width = m_size.x;
    *height = m_size.y;

    return 0;
}

int GameWindow::Win_Enable(bool enable)
{
    if (enable) {
        m_status |= WIN_STATUS_ENABLED;
    } else {
        m_status &= ~WIN_STATUS_ENABLED;
    }

    if (m_child != nullptr) {
        for (GameWindow *i = m_child; i != nullptr; i = i->m_next) {
            i->Win_Enable(enable);
        }
    }
    return 0;
}

bool GameWindow::Win_Is_Enabled()
{
    return (m_status & WIN_STATUS_ENABLED) != 0;
}

bool GameWindow::Win_Is_Hidden()
{
    return (m_status & WIN_STATUS_HIDDEN) != 0;
}

int GameWindow::Win_Set_Status(int status)
{
    int old = m_status;

    m_status |= ~status;

    return old;
}

int GameWindow::Win_Clear_Status(int status)
{
    int old = m_status;

    m_status &= ~status;

    return old;
}

int GameWindow::Win_Get_Status()
{
    return m_status;
}

unsigned int GameWindow::Win_Get_Style()
{
    return m_instData.m_style;
}

void GameWindow::Win_Set_Hilite_State(bool state)
{
    // TODO resolve enum
    if (state) {
        m_instData.m_state |= 2;
    } else {
        m_instData.m_state &= ~2;
    }
}

void GameWindow::Win_Set_Draw_Offset(int x, int y)
{
    m_instData.m_imageOffset.x = x;
    m_instData.m_imageOffset.y = y;
}

void GameWindow::Win_Get_Draw_Offset(int *x, int *y)
{
    if (x != nullptr && y != nullptr) {
        *x = m_instData.m_imageOffset.x;
        *y = m_instData.m_imageOffset.y;
    }
}

int GameWindow::Win_Set_Text(Utf16String new_text)
{
    m_instData.Set_Text(new_text);

    return 0;
}

Utf16String GameWindow::Win_Get_Text()
{
    return m_instData.Get_Text();
}

int GameWindow::Win_Get_Text_Length()
{
    return m_instData.Get_Text_Length();
}

GameFont *GameWindow::Win_Get_Font()
{
    return m_instData.Get_Font();
}

void GameWindow::Win_Set_Font(GameFont *font)
{
    // TODO needs gadgets
#ifdef GAME_DLL
    Call_Method<void, GameWindow, GameFont *>(PICK_ADDRESS(0x004F8FA0, 0x0086E2F5), this, font);
#endif
}

int GameWindow::Win_Get_Enabled_Text_Color()
{
    return m_instData.m_enabledText.color;
}

int GameWindow::Win_Get_Enabled_Text_Border_Color()
{
    return m_instData.m_enabledText.borderColor;
}

int GameWindow::Win_Get_IME_Composite_Text_Color()
{
    return m_instData.m_imeCompositeText.color;
}

int GameWindow::Win_Get_IME_Composite_Text_Border_Color()
{
    return m_instData.m_imeCompositeText.borderColor;
}

int GameWindow::Win_Get_Disabled_Text_Color()
{
    return m_instData.m_disabledText.color;
}

int GameWindow::Win_Get_Disabled_Text_Border_Color()
{
    return m_instData.m_disabledText.borderColor;
}

int GameWindow::Win_Get_Hilite_Text_Color()
{
    return m_instData.m_hiliteText.color;
}

int GameWindow::Win_Get_Hilite_Text_Border_Color()
{
    return m_instData.m_hiliteText.borderColor;
}

int GameWindow::Win_Set_Instance_Data(WinInstanceData *instance)
{
    DisplayString *text = m_instData.m_text;
    DisplayString *tooltip = m_instData.m_tooltip;

    m_instData = *instance;

    m_instData.m_text = text;
    m_instData.m_tooltip = tooltip;
    m_instData.m_videoBuffer = nullptr;

    if (instance->Get_Text_Length() != 0) {
        m_instData.Set_Text(instance->Get_Text());
    }

    if (instance->Get_Tooltip_Text_Length() != 0) {
        m_instData.Set_Tooltip_Text(instance->Get_Tooltip_Text());
    }

    return 0;
}

WinInstanceData *GameWindow::Win_Get_Instance_Data()
{
    return &m_instData;
}

void *GameWindow::Win_Get_User_Data()
{
    return m_userData;
}

void GameWindow::Win_Set_User_Data(void *user_data)
{
    m_userData = user_data;
}

void GameWindow::Win_Set_Tooltip(Utf16String tooltip)
{
    m_instData.Set_Tooltip_Text(tooltip);
}

int GameWindow::Win_Set_Window_Id(int id)
{
    m_instData.m_id = id;

    return 0;
}

int GameWindow::Win_Get_Window_Id()
{
    return m_instData.m_id;
}

GameWindow *GameWindow::Win_Get_Parent()
{
    return m_parent;
}

bool GameWindow::Win_Is_Child(GameWindow *window)
{
    while (window != nullptr) {
        if (this == window->m_parent) {
            return true;
        }
        window = window->m_parent;
    }
    return false;
}

GameWindow *GameWindow::Win_Get_Child()
{
    return m_child;
}

int GameWindow::Win_Set_Owner(GameWindow *owner)
{
    if (owner != nullptr) {
        m_instData.m_owner = owner;
    } else {
        m_instData.m_owner = this;
    }

    return 0;
}

GameWindow *GameWindow::Win_Get_Owner()
{
    return m_instData.Get_Owner();
}

void GameWindow::Win_Set_Next(GameWindow *window)
{
    m_next = window;
}

GameWindow *GameWindow::Win_Get_Next()
{
    return m_next;
}

void GameWindow::Win_Set_Prev(GameWindow *window)
{
    m_prev = window;
}

GameWindow *GameWindow::Win_Get_Prev()
{
    return m_prev;
}

void GameWindow::Win_Set_Next_In_Layout(GameWindow *layout)
{
    m_nextLayout = layout;
}

void GameWindow::Win_Set_Prev_In_Layout(GameWindow *layout)
{
    m_prevLayout = layout;
}

void GameWindow::Win_Set_Layout(WindowLayout *layout)
{
    m_layout = layout;
}

WindowLayout *GameWindow::Win_Get_Layout()
{
    return m_layout;
}

GameWindow *GameWindow::Win_Get_Next_In_Layout()
{
    return m_nextLayout;
}

GameWindow *GameWindow::Win_Get_Prev_In_Layout()
{
    return m_prevLayout;
}

int GameWindow::Win_Set_Tooltip_Func(WindowTooltipFunc tooltip)
{
    m_tooltip = tooltip;

    return 0;
}

int GameWindow::Win_Draw_Window()
{
    if (!Win_Is_Hidden()) {
        if (m_draw != nullptr) {
            m_draw(this, &m_instData);
        }
    }

    return 0;
}

GameWindow *GameWindow::Win_Point_In_Child(int x, int y, bool ignore_enable_check, bool audio_event)
{
    for (GameWindow *window = m_child; window != nullptr; window = window->m_next) {
        int r_x = window->m_region.lo.x;
        int r_y = window->m_region.lo.y;

        for (GameWindow *j = window->Win_Get_Parent(); j != nullptr; j = j->m_parent) {
            r_x += j->m_region.lo.x;
            r_y += j->m_region.lo.y;
        }

        if (x >= r_x && x <= window->m_size.x + r_x && y >= r_y && y <= window->m_size.y + r_y) {

            bool is_enabled = ignore_enable_check || Win_Is_Enabled() ? true : false;
            bool is_hidden = Win_Is_Hidden();

            if (!is_hidden) {

                if (is_enabled) {
                    return window->Win_Point_In_Child(x, y, ignore_enable_check, audio_event);
                }

                if (audio_event) {
                    AudioEventRTS event("GUIClickDisabled");

                    if (g_theAudio != nullptr) {
                        g_theAudio->Add_Audio_Event(&event);
                    }
                }
            }
        }
    }

    return this;
}

GameWindow *GameWindow::Win_Point_In_Any_Child(int x, int y, bool ignore_hidden, bool ignore_enable_check)
{
    for (GameWindow *window = m_child; window != nullptr; window = window->m_next) {
        int r_x = window->m_region.lo.x;
        int r_y = window->m_region.lo.y;

        for (GameWindow *parent_window = window->m_parent; parent_window != nullptr;
             parent_window = parent_window->m_parent) {
            r_x += parent_window->m_region.lo.x;
            r_y += parent_window->m_region.lo.y;
        }

        if (x >= r_x && x <= window->m_size.x + r_x && y >= r_y && y <= window->m_size.y + r_y
            && (!ignore_hidden || !Win_Is_Hidden())) {

            return window->Win_Point_In_Child(x, y, ignore_enable_check, false);
        }
    }

    return this;
}

int GameWindow::Win_Set_Enabled_Image(int index, Image *image)
{
    if (index < 0 || index >= WinInstanceData::MAX_DRAW_DATA) {
        captainslog_debug("Set enabled image, index out of range '%d'", index);
        return -3;
    }

    m_instData.m_enabledDrawData[index].image = image;
    return 0;
}

int GameWindow::Win_Set_Enabled_Color(int index, int color)
{
    if (index < 0 || index >= WinInstanceData::MAX_DRAW_DATA) {
        captainslog_debug("Set enabled color, index out of range '%d'", index);
        return -3;
    }

    m_instData.m_enabledDrawData[index].color = color;
    return 0;
}

int GameWindow::Win_Set_Enabled_Border_Color(int index, int color)
{
    if (index < 0 || index >= WinInstanceData::MAX_DRAW_DATA) {
        captainslog_debug("Set enabled border color, index out of range '%d'", index);
        return -3;
    }

    m_instData.m_enabledDrawData[index].borderColor = color;
    return 0;
}

int GameWindow::Win_Set_Disabled_Image(int index, Image *image)
{
    if (index < 0 || index >= WinInstanceData::MAX_DRAW_DATA) {
        captainslog_debug("Set disabled image, index out of range '%d'", index);
        return -3;
    }

    m_instData.m_disabledDrawData[index].image = image;
    return 0;
}

int GameWindow::Win_Set_Disabled_Color(int index, int color)
{
    if (index < 0 || index >= WinInstanceData::MAX_DRAW_DATA) {
        captainslog_debug("Set disabled color, index out of range '%d'", index);
        return -3;
    }

    m_instData.m_disabledDrawData[index].color = color;
    return 0;
}

int GameWindow::Win_Set_Disabled_Border_Color(int index, int color)
{
    if (index < 0 || index >= WinInstanceData::MAX_DRAW_DATA) {
        captainslog_debug("Set disabled border color, index out of range '%d'", index);
        return -3;
    }

    m_instData.m_disabledDrawData[index].borderColor = color;
    return 0;
}

int GameWindow::Win_Set_Hilite_Image(int index, Image *image)
{
    if (index < 0 || index >= WinInstanceData::MAX_DRAW_DATA) {
        captainslog_debug("Set hilite image, index out of range '%d'", index);
        return -3;
    }

    m_instData.m_hiliteDrawData[index].image = image;
    return 0;
}

int GameWindow::Win_Set_Hilite_Color(int index, int color)
{
    if (index < 0 || index >= WinInstanceData::MAX_DRAW_DATA) {
        captainslog_debug("Set hilite color, index out of range '%d'", index);
        return -3;
    }

    m_instData.m_hiliteDrawData[index].color = color;
    return 0;
}

int GameWindow::Win_Set_Hilite_Border_Color(int index, int color)
{
    if (index < 0 || index >= WinInstanceData::MAX_DRAW_DATA) {
        captainslog_debug("Set hilite border color, index out of range '%d'", index);
        return -3;
    }

    m_instData.m_hiliteDrawData[index].borderColor = color;
    return 0;
}

WindowCallbackFunc GameWindow::Win_Get_Input_Func()
{
    return m_input;
}

WindowCallbackFunc GameWindow::Win_Get_System_Func()
{
    return m_system;
}

WindowTooltipFunc GameWindow::Win_Get_Tooltip_Func()
{
    return m_tooltip;
}

WindowDrawFunc GameWindow::Win_Get_Draw_Func()
{
    return m_draw;
}

void GameWindow::Win_Set_Edit_Data(GameWindowEditData *edit_data)
{
    m_editData = edit_data;
}

GameWindowEditData *GameWindow::Win_Get_Edit_Data()
{
    return m_editData;
}