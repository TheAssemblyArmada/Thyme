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
#pragma once
#include "always.h"
#include "coord.h"
#include "subsysteminterface.h"

class Drawable;

struct RayEffectData
{
    Drawable *draw;
    Coord3D startLoc;
    Coord3D endLoc;
};

class RayEffectSystem : public SubsystemInterface
{
    enum
    {
        MAX_RAY_EFFECTS = 128
    };

public:
    RayEffectSystem();
    ~RayEffectSystem() {}

    virtual void Init();
    virtual void Reset();
    virtual void Update() {}

    void Add_Ray_Effect(Drawable const *drawable, Coord3D const *start_loc, Coord3D const *end_loc);
    void Delete_Ray_Effect(Drawable const *drawable);

    void Get_Ray_Effect_Data(Drawable const *drawable, RayEffectData *data);

protected:
    RayEffectData *Find_Entry(Drawable const *drawable);

private:
    RayEffectData m_effectData[MAX_RAY_EFFECTS];
};

#ifdef GAME_DLL
extern RayEffectSystem *&g_theRayEffects;
#else
extern RayEffectSystem *g_theRayEffects;
#endif
