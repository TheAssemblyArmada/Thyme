/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Generals Exp Points
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

WindowMsgHandledType Generals_Exp_Points_Input(
    GameWindow *window, unsigned int message, unsigned int data_1, unsigned int data_2);
WindowMsgHandledType Generals_Exp_Points_System(
    GameWindow *window, unsigned int message, unsigned int data_1, unsigned int data_2);
