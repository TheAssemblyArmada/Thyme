/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Control Bar
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

void W3D_Cameo_Movie_Draw(GameWindow *window, WinInstanceData *instance);
void W3D_Left_HUD_Draw(GameWindow *window, WinInstanceData *instance);
void W3D_Right_HUD_Draw(GameWindow *window, WinInstanceData *instance);
void W3D_Power_Draw(GameWindow *window, WinInstanceData *instance);
void W3D_Command_Bar_Grid_Draw(GameWindow *window, WinInstanceData *instance);
void W3D_Command_Bar_Gen_Exp_Draw(GameWindow *window, WinInstanceData *instance);
void W3D_Command_Bar_Top_Draw(GameWindow *window, WinInstanceData *instance);
void W3D_Command_Bar_Background_Draw(GameWindow *window, WinInstanceData *instance);
void W3D_Command_Bar_Foreground_Draw(GameWindow *window, WinInstanceData *instance);
void W3D_Draw_Map_Preview(GameWindow *window, WinInstanceData *instance);
void W3D_Command_Bar_Help_Popup_Draw(GameWindow *window, WinInstanceData *instance);
void W3D_No_Draw(GameWindow *window, WinInstanceData *instance);
