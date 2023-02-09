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
#include "w3dshadow.h"
#ifndef GAME_DLL
W3DShadowManager *g_theW3DShadowManager = nullptr;
#endif

void Do_Shadows(RenderInfoClass &rinfo, bool stencil_pass)
{
#ifdef GAME_DLL
    Call_Function<void, RenderInfoClass &, bool>(PICK_ADDRESS(0x00781F40, 0x00605588), rinfo, stencil_pass);
#endif
}

Vector3 &W3DShadowManager::Get_Light_Pos_World(int light_index)
{
#ifdef GAME_DLL
    return Call_Method<Vector3 &, W3DShadowManager, int>(PICK_ADDRESS(0x007822A0, 0x00605AA4), this, light_index);
#else
    static Vector3 v(0.0f, 0.0f, 0.0f);
    return v;
#endif
}

Shadow *W3DShadowManager::Add_Shadow(RenderObjClass *robj, Shadow::ShadowTypeInfo *shadow_info, Drawable *drawable)
{
#ifdef GAME_DLL
    return Call_Method<Shadow *, W3DShadowManager, RenderObjClass *, Shadow::ShadowTypeInfo *, Drawable *>(
        PICK_ADDRESS(0x00782230, 0x0060599A), this, robj, shadow_info, drawable);
#else
    return nullptr;
#endif
}

void W3DShadowManager::Release_Resources()
{
#ifdef GAME_DLL
    Call_Method<void, W3DShadowManager>(PICK_ADDRESS(0x00782210, 0x00605967), this);
#endif
}

void W3DShadowManager::Re_Acquire_Resources()
{
#ifdef GAME_DLL
    Call_Method<void, W3DShadowManager>(PICK_ADDRESS(0x007821D0, 0x00605911), this);
#endif
}

void W3DShadowManager::Remove_Shadow(Shadow *shadow)
{
    shadow->Release();
}
