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
#include "gadgetcombobox.h"
#include "gadgetlistbox.h"
#include "gadgetstatictext.h"
#include "gadgettextentry.h"
#include "gamelogic.h"
#include "gamewindowmanager.h"
#include "ingameui.h"
#include "selectionxlat.h"
#include "view.h"
#include "windowlayout.h"

GameWindow::GameWindow() :
    m_status(0),
    m_size{ 0, 0 },
    m_region{ 0, 0, 0, 0 },
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
    Win_Set_Draw_Func(g_theWindowManager->Get_Default_Draw());
    Win_Set_Input_Func(g_theWindowManager->Get_Default_Input());
    Win_Set_System_Func(g_theWindowManager->Get_Default_System());
    Win_Set_Tooltip_Func(nullptr);
}

GameWindow::~GameWindow()
{
    delete m_inputData;
    delete m_editData;
}

void GameWindow::Normalize_Window_Region()
{
    if (m_region.lo.x > m_region.hi.x) {
        int x = m_region.lo.x;
        m_region.lo.x = m_region.hi.x;
        m_region.hi.x = x;
    }

    if (m_region.lo.y > m_region.hi.y) {
        int y = m_region.lo.y;
        m_region.lo.y = m_region.hi.y;
        m_region.hi.y = y;
    }
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

bool GameWindow::Win_Get_Enabled()
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

    m_status |= status;

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
    m_instData.m_font = font;

    if ((Win_Get_Style() & GWS_SCROLL_LISTBOX) != 0) {
        Gadget_List_Box_Set_Font(this, font);
    } else if ((Win_Get_Style() & GWS_COMBO_BOX) != 0) {
        Gadget_Combo_Box_Set_Font(this, font);
    } else if ((Win_Get_Style() & GWS_ENTRY_FIELD) != 0) {
        Gadget_Text_Entry_Set_Font(this, font);
    } else if ((Win_Get_Style() & GWS_STATIC_TEXT) != 0) {
        Gadget_Static_Text_Set_Font(this, font);
    } else {
        DisplayString *string = m_instData.Get_Text_DisplayString();

        if (string != nullptr) {
            string->Set_Font(font);
        }

        DisplayString *tooltip_string = m_instData.Get_Tooltip_DisplayString();

        if (tooltip_string != nullptr) {
            tooltip_string->Set_Font(font);
        }
    }
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

int GameWindow::Win_Get_IME_Composite_Border_Color()
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

        for (GameWindow *parent_window = window->Win_Get_Parent(); parent_window != nullptr;
             parent_window = parent_window->m_parent) {
            r_x += parent_window->m_region.lo.x;
            r_y += parent_window->m_region.lo.y;
        }

        if (x >= r_x && x <= window->m_size.x + r_x && y >= r_y && y <= window->m_size.y + r_y) {

            bool is_enabled = ignore_enable_check || (window->m_status & WIN_STATUS_ENABLED) != 0;

            if ((window->m_status & WIN_STATUS_HIDDEN) == 0) {

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
            && (!ignore_hidden || (window->m_status & WIN_STATUS_HIDDEN) == 0)) {

            return window->Win_Point_In_Child(x, y, ignore_enable_check, false);
        }
    }

    return this;
}

int GameWindow::Win_Set_Enabled_Image(int index, const Image *image)
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

int GameWindow::Win_Set_Disabled_Image(int index, const Image *image)
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

int GameWindow::Win_Set_Hilite_Image(int index, const Image *image)
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

int GameWindow::Win_Set_Size(int width, int height)
{
    m_size.x = width;
    m_size.y = height;
    m_region.hi.x = width + m_region.lo.x;
    m_region.hi.y = height + m_region.lo.y;
    g_theWindowManager->Win_Send_System_Msg(this, GGM_RESIZED, width, height);
    return 0;
}

int GameWindow::Win_Hide(bool hide)
{
    if (hide) {
        m_status |= WIN_STATUS_HIDDEN;
        g_theWindowManager->Window_Hiding(this);
    } else {
        m_status &= ~WIN_STATUS_HIDDEN;
    }

    return 0;
}

int GameWindow::Win_Set_Parent(GameWindow *parent)
{
    if (m_parent != nullptr) {
        g_theWindowManager->Unlink_Child_Window(this);
    } else {
        g_theWindowManager->Unlink_Window(this);
    }

    if (parent != nullptr) {
        g_theWindowManager->Add_Window_To_Parent(this, parent);
    } else {
        g_theWindowManager->Link_Window(this);
        m_parent = nullptr;
    }

    return 0;
}

int GameWindow::Win_Set_Draw_Func(WindowDrawFunc draw)
{
    if (draw != nullptr) {
        m_draw = draw;
    } else {
        m_draw = g_theWindowManager->Get_Default_Draw();
    }

    return 0;
}

int GameWindow::Win_Set_Input_Func(WindowCallbackFunc input)
{
    if (input != nullptr) {
        m_input = input;
    } else {
        m_input = g_theWindowManager->Get_Default_Input();
    }

    return 0;
}

int GameWindow::Win_Set_System_Func(WindowCallbackFunc system)
{
    if (system != nullptr) {
        m_system = system;
    } else {
        m_system = g_theWindowManager->Get_Default_System();
    }

    return 0;
}

int GameWindow::Win_Set_Callbacks(WindowCallbackFunc input, WindowDrawFunc draw, WindowTooltipFunc tooltip)
{
    Win_Set_Input_Func(input);
    Win_Set_Draw_Func(draw);
    Win_Set_Tooltip_Func(tooltip);
    return 0;
}

int GameWindow::Win_Bring_To_Top()
{
    GameWindow *parent = Win_Get_Parent();

    if (parent != nullptr) {
        g_theWindowManager->Unlink_Child_Window(this);
        g_theWindowManager->Add_Window_To_Parent(this, parent);
    } else {
        for (GameWindow *window = g_theWindowManager->Win_Get_Window_List(); window != nullptr; window = window->m_next) {
            if (window == nullptr) {
                return -3;
            }
        }

        g_theWindowManager->Unlink_Window(this);
        g_theWindowManager->Link_Window(this);
    }

    if (m_layout != nullptr) {
        WindowLayout *layout = m_layout;
        layout->Remove_Window(this);
        layout->Add_Window(this);
    }

    return 0;
}

GameWindow *GameWindow::Find_First_Leaf()
{
    GameWindow *window = this;

    while (window->m_parent != nullptr) {
        window = window->m_parent;
    }

    while (window->m_child != nullptr) {
        window = window->m_child;
    }

    return window;
}

GameWindow *GameWindow::Find_Last_Leaf()
{
    GameWindow *window = this;

    while (window->m_parent != nullptr) {
        window = window->m_parent;
    }

    while (window->m_child != nullptr) {
        for (window = window->m_child; window->m_next != nullptr; window = window->m_next) {
        }
    }

    return window;
}

GameWindow *GameWindow::Find_Prev_Leaf()
{
    GameWindow *window = this;

    if (m_prev != nullptr) {
        GameWindow *prev = m_prev;

        while (prev->m_child != nullptr && (prev->m_status & WIN_STATUS_TAB_STOP) == 0) {
            for (prev = prev->m_child; prev->m_next != nullptr; prev = prev->m_next) {
            }
        }

        return m_prev;
    } else {
        do {
            if (window->m_parent == nullptr) {
                if (window != nullptr) {
                    return window->Find_Last_Leaf();
                } else {
                    return nullptr;
                }
            }

            window = window->m_parent;
        } while (window->m_parent == nullptr || window->m_prev == nullptr);

        GameWindow *prev = m_prev;

        while (prev->m_child != nullptr && (prev->m_status & WIN_STATUS_TAB_STOP) == 0) {
            for (prev = prev->m_child; prev->m_next != nullptr; prev = prev->m_next) {
            }
        }

        return prev;
    }
}

GameWindow *GameWindow::Find_Next_Leaf()
{
    GameWindow *window = this;

    if (m_next != nullptr) {
        if ((m_next->m_status & WIN_STATUS_TAB_STOP) != 0) {
            return m_next;
        } else {
            for (GameWindow *next = m_next; next != nullptr; next = next->m_child) {
                if (next->m_child == nullptr || (next->m_status & WIN_STATUS_TAB_STOP) != 0) {
                    return next;
                }
            }

            return nullptr;
        }
    } else {
        while (window->m_parent != nullptr) {
            window = window->m_parent;

            if (window->m_parent != nullptr && window->m_next != nullptr) {
                for (window = window->m_next; window != nullptr; window = window->m_child) {
                    if (window->m_child == nullptr || (window->m_status & WIN_STATUS_TAB_STOP) != 0) {
                        return window;
                    }
                }
            }
        }

        if (window != nullptr) {
            return window->Find_First_Leaf();
        } else {
            return nullptr;
        }
    }
}

void GameWindow::Win_Set_Disabled_Text_Colors(int color, int border_color)
{
    m_instData.m_disabledText.color = color;
    m_instData.m_disabledText.borderColor = border_color;

    if ((m_instData.Get_Style() & GWS_COMBO_BOX) != 0) {
        Gadget_Combo_Box_Set_Disabled_Text_Colors(this, color, border_color);
    }
}

void GameWindow::Win_Set_Enabled_Text_Colors(int color, int border_color)
{
    m_instData.m_enabledText.color = color;
    m_instData.m_enabledText.borderColor = border_color;

    if ((m_instData.Get_Style() & GWS_COMBO_BOX) != 0) {
        Gadget_Combo_Box_Set_Enabled_Text_Colors(this, color, border_color);
    }
}

void GameWindow::Win_Set_Hilite_Text_Colors(int color, int border_color)
{
    m_instData.m_hiliteText.color = color;
    m_instData.m_hiliteText.borderColor = border_color;

    if ((m_instData.Get_Style() & GWS_COMBO_BOX) != 0) {
        Gadget_Combo_Box_Set_Hilite_Text_Colors(this, color, border_color);
    }
}

void GameWindow::Win_Set_IME_Composite_Text_Colors(int color, int border_color)
{
    m_instData.m_imeCompositeText.color = color;
    m_instData.m_imeCompositeText.borderColor = border_color;

    if ((m_instData.Get_Style() & GWS_COMBO_BOX) != 0) {
        Gadget_Combo_Box_Set_IME_Composite_Text_Colors(this, color, border_color);
    }
}

int GameWindow::Win_Activate()
{
    int ret = Win_Bring_To_Top();

    if (ret != 0) {
        return ret;
    }

    m_status |= WIN_STATUS_ACTIVE;
    Win_Hide(false);
    return 0;
}

WindowMsgHandledType Game_Win_Default_Input(GameWindow *window, unsigned int msg, unsigned int data1, unsigned int data2)
{
    return MSG_IGNORED;
}

WindowMsgHandledType Game_Win_Block_Input(GameWindow *window, unsigned int msg, unsigned int data1, unsigned int data2)
{
    if (msg == GWM_CHAR || msg == GWM_MOUSE_POS) {
        return MSG_IGNORED;
    }

    if (msg == GWM_LEFT_UP) {
        g_theSelectionTranslator->Set_Left_Mouse_Button(false);
        g_theSelectionTranslator->Set_Drag_Selecting(false);
        g_theTacticalView->Set_Mouse_Lock(false);
        g_theInGameUI->Set_Selecting(false);
        g_theInGameUI->End_Area_Select_Hint(nullptr);
    }

    return MSG_HANDLED;
}

WindowMsgHandledType Game_Win_Default_System(GameWindow *window, unsigned int msg, unsigned int data1, unsigned int data2)
{
    return MSG_IGNORED;
}

void Game_Win_Default_Tooltip(GameWindow *window, WinInstanceData *inst_data, unsigned int mouse) {}

void Game_Win_Default_Draw(GameWindow *window, WinInstanceData *inst_data) {}
