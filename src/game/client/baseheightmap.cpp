/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Base Height Map Render Object
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "baseheightmap.h"
#ifndef GAME_DLL
BaseHeightMapRenderObjClass *g_theTerrainRenderObject;
#endif

int BaseHeightMapRenderObjClass::Get_Static_Diffuse(int x, int y)
{
#ifdef GAME_DLL
    return Call_Function<int, int>(PICK_ADDRESS(0x00754E70, 0x00613479), x, y);
#else
    return 0;
#endif
}

float BaseHeightMapRenderObjClass::Get_Max_Cell_Height(float x, float y)
{
#ifdef GAME_DLL
    return Call_Function<float, float>(PICK_ADDRESS(0x00752D00, 0x005FF000), x, y);
#else
    return 0;
#endif
}
