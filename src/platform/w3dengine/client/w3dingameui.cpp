/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D In Game UI
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dingameui.h"
#include "w3dview.h"

View *W3DInGameUI::Create_View()
{
    return new W3DView();
}
