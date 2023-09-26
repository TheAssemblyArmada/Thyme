/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D Game Window Manager
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
#include "gamewindowmanager.h"

class W3DGameWindowManager : public GameWindowManager
{
public:
    W3DGameWindowManager();
    virtual ~W3DGameWindowManager() override;

    virtual void Init() override;

    virtual GameWindow *Allocate_New_Window() override;

    virtual WindowDrawFunc Get_Push_Button_Image_Draw_Func() override;
    virtual WindowDrawFunc Get_Push_Button_Draw_Func() override;

    virtual WindowDrawFunc Get_Check_Box_Image_Draw_Func() override;
    virtual WindowDrawFunc Get_Check_Box_Draw_Func() override;

    virtual WindowDrawFunc Get_Radio_Button_Image_Draw_Func() override;
    virtual WindowDrawFunc Get_Radio_Button_Draw_Func() override;

    virtual WindowDrawFunc Get_Tab_Control_Image_Draw_Func() override;
    virtual WindowDrawFunc Get_Tab_Control_Draw_Func() override;

    virtual WindowDrawFunc Get_List_Box_Image_Draw_Func() override;
    virtual WindowDrawFunc Get_List_Box_Draw_Func() override;

    virtual WindowDrawFunc Get_Combo_Box_Image_Draw_Func() override;
    virtual WindowDrawFunc Get_Combo_Box_Draw_Func() override;

    virtual WindowDrawFunc Get_Horizontal_Slider_Image_Draw_Func() override;
    virtual WindowDrawFunc Get_Horizontal_Slider_Draw_Func() override;

    virtual WindowDrawFunc Get_Vertical_Slider_Image_Draw_Func() override;
    virtual WindowDrawFunc Get_Vertical_Slider_Draw_Func() override;

    virtual WindowDrawFunc Get_Progress_Bar_Image_Draw_Func() override;
    virtual WindowDrawFunc Get_Progress_Bar_Draw_Func() override;

    virtual WindowDrawFunc Get_Static_Text_Image_Draw_Func() override;
    virtual WindowDrawFunc Get_Static_Text_Draw_Func() override;

    virtual WindowDrawFunc Get_Text_Entry_Image_Draw_Func() override;
    virtual WindowDrawFunc Get_Text_Entry_Draw_Func() override;

    virtual WindowDrawFunc Get_Default_Draw() override;
};
