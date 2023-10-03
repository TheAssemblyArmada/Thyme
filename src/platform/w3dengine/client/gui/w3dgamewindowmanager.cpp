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
#include "w3dgamewindowmanager.h"
#include "display.h"
#include "w3dcheckbox.h"
#include "w3dcombobox.h"
#include "w3dgamewindow.h"
#include "w3dlistbox.h"
#include "w3dprogressbar.h"
#include "w3dpushbutton.h"
#include "w3dradiobutton.h"
#include "w3dslider.h"
#include "w3dstatictext.h"
#include "w3dtabcontrol.h"
#include "w3dtextentry.h"

W3DGameWindowManager::W3DGameWindowManager() {}

W3DGameWindowManager::~W3DGameWindowManager() {}

void W3DGameWindowManager::Init()
{
    GameWindowManager::Init();
}

GameWindow *W3DGameWindowManager::Allocate_New_Window()
{
    return new W3DGameWindow();
}

WindowDrawFunc W3DGameWindowManager::Get_Push_Button_Image_Draw_Func()
{
    return W3D_Gadget_Push_Button_Image_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_Push_Button_Draw_Func()
{
    return W3D_Gadget_Push_Button_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_Check_Box_Image_Draw_Func()
{
    return W3D_Gadget_Check_Box_Image_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_Check_Box_Draw_Func()
{
    return W3D_Gadget_Check_Box_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_Radio_Button_Image_Draw_Func()
{
    return W3D_Gadget_Radio_Button_Image_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_Radio_Button_Draw_Func()
{
    return W3D_Gadget_Radio_Button_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_Tab_Control_Image_Draw_Func()
{
    return W3D_Gadget_Tab_Control_Image_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_Tab_Control_Draw_Func()
{
    return W3D_Gadget_Tab_Control_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_List_Box_Image_Draw_Func()
{
    return W3D_Gadget_List_Box_Image_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_List_Box_Draw_Func()
{
    return W3D_Gadget_List_Box_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_Combo_Box_Image_Draw_Func()
{
    return W3D_Gadget_Combo_Box_Image_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_Combo_Box_Draw_Func()
{
    return W3D_Gadget_Combo_Box_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_Horizontal_Slider_Image_Draw_Func()
{
    return W3D_Gadget_Horizontal_Slider_Image_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_Horizontal_Slider_Draw_Func()
{
    return W3D_Gadget_Horizontal_Slider_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_Vertical_Slider_Image_Draw_Func()
{
    return W3D_Gadget_Vertical_Slider_Image_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_Vertical_Slider_Draw_Func()
{
    return W3D_Gadget_Vertical_Slider_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_Progress_Bar_Image_Draw_Func()
{
    return W3D_Gadget_Progress_Bar_Image_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_Progress_Bar_Draw_Func()
{
    return W3D_Gadget_Progress_Bar_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_Static_Text_Image_Draw_Func()
{
    return W3D_Gadget_Static_Text_Image_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_Static_Text_Draw_Func()
{
    return W3D_Gadget_Static_Text_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_Text_Entry_Image_Draw_Func()
{
    return W3D_Gadget_Text_Entry_Image_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_Text_Entry_Draw_Func()
{
    return W3D_Gadget_Text_Entry_Draw;
}

WindowDrawFunc W3DGameWindowManager::Get_Default_Draw()
{
    return W3D_Game_Win_Default_Draw;
}
