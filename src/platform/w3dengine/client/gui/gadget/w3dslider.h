/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Slider
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

void W3D_Gadget_Horizontal_Slider_Draw(GameWindow *slider, WinInstanceData *data);
void W3D_Gadget_Horizontal_Slider_Image_Draw(GameWindow *slider, WinInstanceData *data);

void W3D_Gadget_Vertical_Slider_Draw(GameWindow *slider, WinInstanceData *data);
void W3D_Gadget_Vertical_Slider_Image_Draw(GameWindow *slider, WinInstanceData *data);
