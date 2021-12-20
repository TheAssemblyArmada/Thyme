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
    return Call_Method<int, BaseHeightMapRenderObjClass, int, int>(PICK_ADDRESS(0x00754E70, 0x00613479), this, x, y);
#else
    return 0;
#endif
}

float BaseHeightMapRenderObjClass::Get_Max_Cell_Height(float x, float y)
{
#ifdef GAME_DLL
    return Call_Method<float, BaseHeightMapRenderObjClass, float, float>(PICK_ADDRESS(0x00752D00, 0x005FF000), this, x, y);
#else
    return 0;
#endif
}

void BaseHeightMapRenderObjClass::Set_Time_Of_Day(TimeOfDayType time)
{
#ifdef GAME_DLL
    Call_Method<void, BaseHeightMapRenderObjClass, TimeOfDayType>(0x007553B0, this, time);
#endif
}

void BaseHeightMapRenderObjClass::Do_The_Light(VertexFormatXYZDUV2 *vb,
    Vector3 *light,
    Vector3 *normal,
    RefMultiListIterator<RenderObjClass> *lights,
    unsigned char alpha)
{
#ifdef GAME_DLL
    Call_Method<void,
        BaseHeightMapRenderObjClass,
        VertexFormatXYZDUV2 *,
        Vector3 *,
        Vector3 *,
        RefMultiListIterator<RenderObjClass> *,
        unsigned char>(0x007512A0, this, vb, light, normal, lights, alpha);
#endif
}

float BaseHeightMapRenderObjClass::Get_Height_Map_Height(float x, float y, Coord3D *pos)
{
#ifdef GAME_DLL
    return Call_Method<float, BaseHeightMapRenderObjClass, float, float, Coord3D *>(0x00752580, this, x, y, pos);
#else
    return 0.0f;
#endif
}

void Do_Trees(RenderInfoClass &rinfo)
{
#ifdef GAME_DLL
    Call_Function<void, RenderInfoClass &>(0x00750450, rinfo);
#endif
}

void BaseHeightMapRenderObjClass::Notify_Shroud_Changed()
{
#ifdef GAME_DLL
    Call_Method<void, BaseHeightMapRenderObjClass>(0x007552D0, this);
#endif
}
