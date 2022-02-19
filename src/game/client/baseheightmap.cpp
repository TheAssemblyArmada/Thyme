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

int32_t BaseHeightMapRenderObjClass::Get_Static_Diffuse(int32_t x, int32_t y)
{
#ifdef GAME_DLL
    return Call_Method<int32_t, BaseHeightMapRenderObjClass, int32_t, int32_t>(PICK_ADDRESS(0x00754E70, 0x006017D2), this, x, y);
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
    Call_Method<void, BaseHeightMapRenderObjClass, TimeOfDayType>(PICK_ADDRESS(0x007553B0, 0x00601F0B), this, time);
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
        unsigned char>(PICK_ADDRESS(0x007512A0, 0x005FD3A8), this, vb, light, normal, lights, alpha);
#endif
}

float BaseHeightMapRenderObjClass::Get_Height_Map_Height(float x, float y, Coord3D *pos)
{
#ifdef GAME_DLL
    return Call_Method<float, BaseHeightMapRenderObjClass, float, float, Coord3D *>(
        PICK_ADDRESS(0x00752580, 0x005FE5B6), this, x, y, pos);
#else
    return 0.0f;
#endif
}

void Do_Trees(RenderInfoClass &rinfo)
{
#ifdef GAME_DLL
    Call_Function<void, RenderInfoClass &>(PICK_ADDRESS(0x00750450, 0x005FC2EE), rinfo);
#endif
}

void BaseHeightMapRenderObjClass::Notify_Shroud_Changed()
{
#ifdef GAME_DLL
    Call_Method<void, BaseHeightMapRenderObjClass>(PICK_ADDRESS(0x007552D0, 0x00601DDA), this);
#endif
}

void BaseHeightMapRenderObjClass::Add_Tree(
    DrawableID drawable, Coord3D location, float scale, float angle, float random, W3DTreeDrawModuleData const *module)
{
#ifdef GAME_DLL
    Call_Method<void, BaseHeightMapRenderObjClass, DrawableID, Coord3D, float, float, float, W3DTreeDrawModuleData const *>(
        PICK_ADDRESS(0x00755240, 0x00601C53), this, drawable, location, scale, angle, random, module);
#endif
}
