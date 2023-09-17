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
#include "w3dgamewindow.h"

GameWindow *W3DGameWindowManager::Allocate_New_Window()
{
    return new W3DGameWindow();
}
