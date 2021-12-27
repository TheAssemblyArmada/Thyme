/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D game client
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dgameclient.h"
#include "w3ddisplay.h"
#include "w3dmouse.h"
#include "w3dstatuscircle.h"

Mouse *W3DGameClient::Create_Mouse()
{
    Win32Mouse *mouse = new W3DMouse();
    g_theWin32Mouse = mouse;
    return mouse;
}

Display *W3DGameClient::Create_GameDisplay()
{
    return new W3DDisplay;
}

void W3DGameClient::Set_Team_Color(int red, int green, int blue)
{
    W3DStatusCircle::g_needUpdate = true;
    W3DStatusCircle::g_diffuse = blue + ((green + (red << 8)) << 8);
}
