/**
 * @file
 *
 * @author Jonathan Wilson
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
#include "w3dview.h"
#include "shadermanager.h"
void W3DView::Set_Fade_Parameters(int32_t frames, int32_t direction)
{
    ScreenBWFilter::Set_Fade_Parameters(frames, direction);
    ScreenCrossFadeFilter::Set_Fade_Parameters(frames, direction);
}

void W3DView::Set_View_Filter_Pos(const Coord3D *pos)
{
    ScreenMotionBlurFilter::Set_Zoom_To_Pos(pos);
}

bool W3DView::Update_Camera_Movements()
{
#ifdef GAME_DLL
    return Call_Method<bool, W3DView>(PICK_ADDRESS(0x0076D680, 0x006A5728), this);
#else
    return false;
#endif
}
