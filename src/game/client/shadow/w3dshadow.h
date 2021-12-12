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
#pragma once
#include "always.h"
#include "coord.h"
#include "vector3.h"
class RenderInfoClass;
class ThingTemplate;
class DebugDrawStats;

enum ShadowType
{
    SHADOW_NONE = 0,
    SHADOW_DECAL = 1,
    SHADOW_VOLUME = 2,
    SHADOW_PROJECTION = 4,
    SHADOW_DYNAMIC_PROJECTION = 8,
    SHADOW_DIRECTIONAL_PROJECTION = 16,
    SHADOW_ALPHA_DECAL = 32,
    SHADOW_ADDITIVE_DECAL = 64,
};

class Shadow
{

public:
    // B?UGFIX init all members
    Shadow() :
        m_opacity(0xFF),
        m_color1(0xFFFFFFFF),
        m_color2(0xFFFFFFFF),
        m_angle(0.0f),
        m_decalOffsetU(0.0f),
        m_decalOffsetV(0.0f),
        m_isEnabled(false),
        m_isInvisibleEnabled(false),
        m_sizeX(0.0f),
        m_sizeY(0.0f),
        m_type(SHADOW_NONE)
    {
    }

    virtual void Release() = 0;
#ifdef GAME_DEBUG_STRUCTS
    virtual void Gather_Draw_Stats(DebugDrawStats *stats) = 0;
#endif

public:
    struct ShadowTypeInfo
    {
        char m_shadowName[64];
        ShadowType m_type;
        char m_allowUpdates;
        char m_allowWorldAlign;
        float m_sizeX;
        float m_sizeY;
        float m_offsetX;
        float m_offsetY;
        ThingTemplate *m_thing;
    };

protected:
    bool m_isEnabled;
    bool m_isInvisibleEnabled;
    int m_opacity;
    int m_color1;
    ShadowType m_type;
    int m_color2;
    Coord3D m_position;
    float m_sizeX;
    float m_sizeY;
    float m_decalOffsetU;
    float m_decalOffsetV;
    float m_angle;
};

class W3DShadowManager
{
public:
    void Set_Shadow_Color(unsigned int color) { m_shadowColor = color; }
    void Set_Is_Shadow_Scene(bool set) { m_isShadowScene = set; }
    unsigned int Get_Stencil_Mask() { return m_stencilMask; }
    void Set_Stencil_Mask(unsigned int mask) { m_stencilMask = mask; }
    Vector3 &Get_Light_Pos_World(int light_index);

protected:
    bool m_isShadowScene;
    unsigned int m_shadowColor;
    unsigned int m_stencilMask;
};

#ifdef GAME_DLL
#include "hooker.h"

extern W3DShadowManager *&g_theW3DShadowManager;
#else
extern W3DShadowManager *g_theW3DShadowManager;
#endif

void Do_Shadows(RenderInfoClass &rinfo, bool stencil_pass);
