/**
 * @file
 *
 * @author tomsons26
 *
 * @brief WND UI system manager.
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
#include "gamewindow.h"
#include "subsysteminterface.h"
#include "wininstancedata.h"
#include <list>

class WindowLayoutInfo;
class GlobalLanguage;
class GameFont;
class Image;
class GameWindow;
class ModalWindow;
class WindowLayout;

struct _RadioButtonData;
struct _TabControlData;
struct _ListboxData;
struct _SliderData;
struct _TextData;
struct _EntryData;
struct _ComboBoxData;

enum
{
    WIN_ERR_OK = 0,
    WIN_ERR_GENERAL_FAILURE = -1,
    WIN_ERR_INVALID_WINDOW = -2,
    WIN_ERR_INVALID_PARAMETER = -3,
    WIN_ERR_MOUSE_CAPTURED = -4,
    WIN_ERR_KEYBOARD_CAPTURED = -5,
    WIN_ERR_OUT_OF_WINDOWS = -6,
};

enum
{
    MSG_BOX_YES = 1,
    MSG_BOX_NO = 2,
    MSG_BOX_CANCEL = 4,
    MSG_BOX_OK = 8,
};

struct _MessageBoxData
{
    WindowMsgBoxCallbackFunc yes_callback;
    WindowMsgBoxCallbackFunc no_callback;
    WindowMsgBoxCallbackFunc ok_callback;
    WindowMsgBoxCallbackFunc cancel_callback;
};

class GameWindowManager : public SubsystemInterface
{
    friend GlobalLanguage;

public:
    GameWindowManager();
    virtual ~GameWindowManager() override;

    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;

    virtual GameWindow *Allocate_New_Window() = 0;

    virtual WindowDrawFunc Get_Push_Button_Image_Draw_Func() = 0;
    virtual WindowDrawFunc Get_Push_Button_Draw_Func() = 0;

    virtual WindowDrawFunc Get_Check_Box_Image_Draw_Func() = 0;
    virtual WindowDrawFunc Get_Check_Box_Draw_Func() = 0;

    virtual WindowDrawFunc Get_Radio_Button_Image_Draw_Func() = 0;
    virtual WindowDrawFunc Get_Radio_Button_Draw_Func() = 0;

    virtual WindowDrawFunc Get_Tab_Control_Image_Draw_Func() = 0;
    virtual WindowDrawFunc Get_Tab_Control_Draw_Func() = 0;

    virtual WindowDrawFunc Get_List_Box_Image_Draw_Func() = 0;
    virtual WindowDrawFunc Get_List_Box_Draw_Func() = 0;

    virtual WindowDrawFunc Get_Combo_Box_Image_Draw_Func() = 0;
    virtual WindowDrawFunc Get_Combo_Box_Draw_Func() = 0;

    virtual WindowDrawFunc Get_Horizontal_Slider_Image_Draw_Func() = 0;
    virtual WindowDrawFunc Get_Horizontal_Slider_Draw_Func() = 0;

    virtual WindowDrawFunc Get_Vertical_Slider_Image_Draw_Func() = 0;
    virtual WindowDrawFunc Get_Vertical_Slider_Draw_Func() = 0;

    virtual WindowDrawFunc Get_Progress_Bar_Image_Draw_Func() = 0;
    virtual WindowDrawFunc Get_Progress_Bar_Draw_Func() = 0;

    virtual WindowDrawFunc Get_Static_Text_Image_Draw_Func() = 0;
    virtual WindowDrawFunc Get_Static_Text_Draw_Func() = 0;

    virtual WindowDrawFunc Get_Text_Entry_Image_Draw_Func() = 0;
    virtual WindowDrawFunc Get_Text_Entry_Draw_Func() = 0;

    virtual WindowDrawFunc Get_Default_Draw();
    virtual WindowCallbackFunc Get_Default_System();
    virtual WindowCallbackFunc Get_Default_Input();
    virtual WindowTooltipFunc Get_Default_Tooltip();

    virtual GameWindow *Go_Go_Message_Box(int x,
        int y,
        int width,
        int height,
        unsigned short flags,
        Utf16String title,
        Utf16String body,
        WindowMsgBoxCallbackFunc yes_callback,
        WindowMsgBoxCallbackFunc no_callback,
        WindowMsgBoxCallbackFunc ok_callback,
        WindowMsgBoxCallbackFunc cancel_callback);

    virtual GameWindow *Go_Go_Message_Box(int x,
        int y,
        int width,
        int height,
        unsigned short flags,
        Utf16String title,
        Utf16String body,
        WindowMsgBoxCallbackFunc yes_callback,
        WindowMsgBoxCallbackFunc no_callback,
        WindowMsgBoxCallbackFunc ok_callback,
        WindowMsgBoxCallbackFunc cancel_callback,
        bool use_quit_box);

    virtual GameWindow *Go_Go_Gadget_Push_Button(GameWindow *parent,
        unsigned int status,
        int x,
        int y,
        int width,
        int height,
        WinInstanceData *inst_data,
        GameFont *font,
        bool assign_visuals);

    virtual GameWindow *Go_Go_Gadget_Checkbox(GameWindow *parent,
        unsigned int status,
        int x,
        int y,
        int width,
        int height,
        WinInstanceData *inst_data,
        GameFont *font,
        bool assign_visuals);

    virtual GameWindow *Go_Go_Gadget_Radio_Button(GameWindow *parent,
        unsigned int status,
        int x,
        int y,
        int width,
        int height,
        WinInstanceData *inst_data,
        _RadioButtonData *data,
        GameFont *font,
        bool assign_visuals);

    virtual GameWindow *Go_Go_Gadget_Tab_Control(GameWindow *parent,
        unsigned int status,
        int x,
        int y,
        int width,
        int height,
        WinInstanceData *inst_data,
        _TabControlData *data,
        GameFont *font,
        bool assign_visuals);

    virtual GameWindow *Go_Go_Gadget_List_Box(GameWindow *parent,
        unsigned int status,
        int x,
        int y,
        int width,
        int height,
        WinInstanceData *inst_data,
        _ListboxData *data,
        GameFont *font,
        bool assign_visuals);

    virtual GameWindow *Go_Go_Gadget_Slider(GameWindow *parent,
        unsigned int status,
        int x,
        int y,
        int width,
        int height,
        WinInstanceData *inst_data,
        _SliderData *data,
        GameFont *font,
        bool assign_visuals);

    virtual GameWindow *Go_Go_Gadget_Progress_Bar(GameWindow *parent,
        unsigned int status,
        int x,
        int y,
        int width,
        int height,
        WinInstanceData *inst_data,
        GameFont *font,
        bool assign_visuals);

    virtual GameWindow *Go_Go_Gadget_Static_Text(GameWindow *parent,
        unsigned int status,
        int x,
        int y,
        int width,
        int height,
        WinInstanceData *inst_data,
        _TextData *data,
        GameFont *font,
        bool assign_visuals);

    virtual GameWindow *Go_Go_Gadget_Text_Entry(GameWindow *parent,
        unsigned int status,
        int x,
        int y,
        int width,
        int height,
        WinInstanceData *inst_data,
        _EntryData *data,
        GameFont *font,
        bool assign_visuals);

    virtual GameWindow *Go_Go_Gadget_Combo_Box(GameWindow *parent,
        unsigned int status,
        int x,
        int y,
        int width,
        int height,
        WinInstanceData *inst_data,
        _ComboBoxData *data,
        GameFont *font,
        bool assign_visuals);

    virtual void Assign_Default_Gadget_Look(GameWindow *gadget, GameFont *font, bool assign_visuals);

    virtual GameWindow *Win_Create_From_Script(Utf8String filename, WindowLayoutInfo *info);

    virtual WindowLayout *Win_Create_Layout(Utf8String filename);

    virtual void Free_Static_Strings();

    virtual GameWindow *Win_Create(GameWindow *parent,
        unsigned int status,
        int x,
        int y,
        int width,
        int height,
        WindowCallbackFunc system,
        WinInstanceData *inst_data);

    virtual int Win_Destroy(GameWindow *window);
    virtual int Win_Destroy_All();

    virtual GameWindow *Win_Get_Window_List();

    virtual void Hide_Windows_In_Range(GameWindow *base_window, int first, int last, bool hide_flag);
    virtual void Enable_Windows_In_Range(GameWindow *base_window, int first, int last, bool enable_flag);

    virtual void Window_Hiding(GameWindow *window);

    virtual void Win_Repaint();

    virtual void Win_Next_Tab(GameWindow *window);
    virtual void Win_Prev_Tab(GameWindow *window);

    virtual void Register_Tab_List(std::list<GameWindow *> list);
    virtual void Clear_Tab_List();

    virtual WinInputReturnCode Win_Process_Mouse_Event(GameWindowMessage msg, ICoord2D *mouse_pos, void *data);

    virtual WinInputReturnCode Win_Process_Key(unsigned char key, unsigned char state);

    virtual GameWindow *Win_Get_Focus();
    virtual int Win_Set_Focus(GameWindow *window);

    virtual void Win_Set_Grab_Window(GameWindow *window);
    virtual GameWindow *Win_Get_Grab_Window();

    virtual void Win_Set_Lone_Window(GameWindow *window);

    virtual bool Is_Enabled(GameWindow *window);
    virtual bool Is_Hidden(GameWindow *window);

    virtual void Add_Window_To_Parent(GameWindow *window, GameWindow *parent);
    virtual void Add_Window_To_Parent_At_End(GameWindow *window, GameWindow *parent);

    virtual WindowMsgHandledType Win_Send_System_Msg(GameWindow *window, unsigned msg, unsigned data1, unsigned data2);
    virtual WindowMsgHandledType Win_Send_Input_Msg(GameWindow *window, unsigned msg, unsigned data1, unsigned data2);

    virtual GameWindow *Win_Get_Window_From_Id(GameWindow *window, int id);

    virtual int Win_Capture(GameWindow *window);
    virtual int Win_Release(GameWindow *window);
    virtual GameWindow *Get_Win_Capture();

    virtual int Win_Set_Modal(GameWindow *window);
    virtual int Win_Unset_Modal(GameWindow *window);

    virtual void Win_Draw_Image(const Image *image, int start_x, int start_y, int end_x, int end_y, int color);

    virtual void Win_Fill_Rect(int color, float width, int start_x, int start_y, int end_x, int end_y);

    virtual void Win_Open_Rect(int color, float width, int start_x, int start_y, int end_x, int end_y);

    virtual void Win_Draw_Line(int color, float width, int start_x, int start_y, int end_x, int end_y);

    virtual int Win_Make_Color(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);

    virtual const Image *Win_Find_Image(const char *name);

    virtual int Win_Font_Height(GameFont *font);

    virtual int Win_Is_Digit(int c);
    virtual int Win_Is_Ascii(int c);
    virtual int Win_Is_Al_Num(int c);

    virtual void Win_Format_Text(GameFont *font, Utf16String text, int color, int x, int y, int width, int height);
    virtual void Win_Get_Text_Size(GameFont *font, Utf16String text, int *width, int *height, int max_width);

    virtual Utf16String Win_Text_Label_To_Text(Utf8String label);

    virtual GameFont *Win_Find_Font(Utf8String font_name, int point_size, bool bold);
    virtual GameWindow *Get_Window_Under_Cursor(int x, int y, bool ignore_enabled);

    void Link_Window(GameWindow *window);
    void Insert_Window_Ahead_Of(GameWindow *window, GameWindow *ahead_of);
    void Unlink_Window(GameWindow *window);
    void Unlink_Child_Window(GameWindow *window);

    int Draw_Window(GameWindow *window);

    bool Init_Test_GUI();

protected:
    void Dump_Window(GameWindow *window);

    void Process_Destroy_List();

private:
    GameWindow *m_windowList;
    GameWindow *m_windowTail;
    GameWindow *m_destroyList;

    GameWindow *m_currMouseRgn;
    GameWindow *m_mouseCaptor;

    GameWindow *m_keyboardFocus;

    ModalWindow *m_modalHead;

    GameWindow *m_grabWindow;
    GameWindow *m_loneWindow;

    std::list<GameWindow *> m_tabList;

    const Image *m_cursorBitmap; // needs confirming
    unsigned int m_captureFlags; // needs confirming
};

#ifdef GAME_DLL
extern GameWindowManager *&g_theWindowManager;
#else
extern GameWindowManager *g_theWindowManager;
#endif

WindowMsgHandledType Pass_Selected_Buttons_To_Parent_System(
    GameWindow *window, unsigned int msg, unsigned int data_1, unsigned int data_2);
WindowMsgHandledType Pass_Messages_To_Parent_System(
    GameWindow *window, unsigned int msg, unsigned int data_1, unsigned int data_2);
