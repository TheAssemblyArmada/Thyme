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
class RenderObjClass;
class Drawable;

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
    // BUGFIX init all members
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
        bool m_allowUpdates;
        bool m_allowWorldAlign;
        float m_sizeX;
        float m_sizeY;
        float m_offsetX;
        float m_offsetY;
        const ThingTemplate *m_thing;
    };

    void Enable_Shadow_Render(bool enable) { m_isEnabled = enable; }
    void Enable_Shadow_Invisible(bool enable) { m_isInvisibleEnabled = enable; }

    void Set_Size(float x, float y)
    {
        m_decalOffsetU = x;
        m_decalOffsetV = y;

        if (x == 0.0f) {
            m_sizeX = 0.0f;
        } else {
            m_sizeX = 1.0f / x;
        }

        if (y == 0.0f) {
            m_sizeY = 0.0f;
        } else {
            m_sizeY = 1.0f / y;
        }
    }

    void Set_Opacity(int32_t opacity)
    {
        m_opacity = opacity;

        if ((m_type & SHADOW_ALPHA_DECAL) != 0) {
            m_color2 = (opacity << 24) + (m_color1 & 0xFFFFFF);
        } else if ((m_type & SHADOW_ADDITIVE_DECAL) != 0) {
            float o = m_opacity / 255.0f;
            m_color2 = GameMath::Fast_To_Int_Truncate(((m_color1 >> 16) & 0xFF) * o)
                | GameMath::Fast_To_Int_Truncate(((m_color1 >> 8) & 0xFF) * o)
                | GameMath::Fast_To_Int_Truncate((m_color1 & 0xFF) * o);
        }
    }

protected:
    bool m_isEnabled;
    bool m_isInvisibleEnabled;
    int32_t m_opacity;
    int32_t m_color1;
    ShadowType m_type;
    int32_t m_color2;
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
    Shadow *Add_Shadow(RenderObjClass *robj, Shadow::ShadowTypeInfo *shadow_info, Drawable *drawable);
    void Remove_Shadow(Shadow *shadow);
    void Set_Shadow_Color(uint32_t color) { m_shadowColor = color; }
    void Set_Is_Shadow_Scene(bool set) { m_isShadowScene = set; }
    uint32_t Get_Stencil_Mask() { return m_stencilMask; }
    void Set_Stencil_Mask(uint32_t mask) { m_stencilMask = mask; }
    Vector3 &Get_Light_Pos_World(int32_t light_index);

protected:
    bool m_isShadowScene;
    uint32_t m_shadowColor;
    uint32_t m_stencilMask;
};

#ifdef GAME_DLL
#include "hooker.h"

extern W3DShadowManager *&g_theW3DShadowManager;
#else
extern W3DShadowManager *g_theW3DShadowManager;
#endif

void Do_Shadows(RenderInfoClass &rinfo, bool stencil_pass);
