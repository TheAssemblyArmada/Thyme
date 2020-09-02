/**
 * @file
 *
 * @author tomsons26
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
#include "rayeffect.h"

#ifndef GAME_DLL
RayEffectSystem *g_theRayEffects;
#endif

RayEffectSystem::RayEffectSystem()
{
    Init();
}

void RayEffectSystem::Init()
{
    for (int i = 0; i < MAX_RAY_EFFECTS; ++i) {
        m_effectData[i].draw = nullptr;
        m_effectData[i].startLoc.Zero();
        m_effectData[i].endLoc.Zero();
    }
}

void RayEffectSystem::Reset()
{
    Init();
}

void RayEffectSystem::Add_Ray_Effect(Drawable const *drawable, Coord3D const *start_loc, Coord3D const *end_loc)
{
    RayEffectData *fx = nullptr;

    if (drawable != nullptr && start_loc != nullptr && end_loc != nullptr) {
        for (int i = 0; i < MAX_RAY_EFFECTS; ++i) {
            if (m_effectData[i].draw == nullptr) {
                fx = &m_effectData[i];
                break;
            }
        }

        if (fx != nullptr) {
            fx->draw = const_cast<Drawable *>(drawable);
            fx->startLoc = *start_loc;
            fx->endLoc = *end_loc;
        }
    }
}

void RayEffectSystem::Delete_Ray_Effect(Drawable const *drawable)
{
    if (drawable != nullptr) {
        RayEffectData *fx = Find_Entry(drawable);
        if (fx != nullptr) {
            fx->draw = nullptr;
        }
    }
}

void RayEffectSystem::Get_Ray_Effect_Data(Drawable const *drawable, RayEffectData *data)
{
    if (drawable != nullptr && data != nullptr) {
        RayEffectData *fx = Find_Entry(drawable);
        if (fx != nullptr) {
            memcpy(data, fx, sizeof(RayEffectData));
        }
    }
}

RayEffectData *RayEffectSystem::Find_Entry(Drawable const *drawable)
{
    RayEffectData *fx = nullptr;

    for (int i = 0; i < MAX_RAY_EFFECTS; ++i) {
        if (m_effectData[i].draw == drawable) {
            fx = &m_effectData[i];
            break;
        }
    }

    return fx;
}
