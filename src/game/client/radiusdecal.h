/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Radius Decal
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
#include "asciistring.h"
#include "coord.h"
#include "w3dshadow.h"

class Player;
class RadiusDecal;
class Xfer;
class INI;

class RadiusDecalTemplate
{
public:
    RadiusDecalTemplate() :
        m_style(SHADOW_ALPHA_DECAL),
        m_opacityMin(1.0f),
        m_opacityMax(1.0f),
        m_opacityThrobTime(30),
        m_color(0),
        m_onlyVisibleToOwningPlayer(true)
    {
    }
    ~RadiusDecalTemplate() {}
    void Create_Radius_Decal(const Coord3D *pos, float radius, const Player *owning_player, RadiusDecal *decal);
    void Xfer_Radius_Decal_Template(Xfer *xfer);

    static void Parse_Radius_Decal_Template(INI *ini, void *formal, void *store, const void *user_data);

private:
    Utf8String m_texture;
    ShadowType m_style;
    float m_opacityMin;
    float m_opacityMax;
    unsigned int m_opacityThrobTime;
    int m_color;
    bool m_onlyVisibleToOwningPlayer;
    friend class RadiusDecal;
};

class RadiusDecal
{
public:
    RadiusDecal() : m_template(nullptr), m_shadow(nullptr), m_needsInit(true) {}
    ~RadiusDecal() { Clear(); }
    bool Needs_Init() const { return m_needsInit; }
    RadiusDecal(const RadiusDecal &decal);
    RadiusDecal &operator=(const RadiusDecal &that);
    void Xfer_Radius_Decal(Xfer *xfer);
    void Clear();
    void Update();
    void Set_Opacity(float opacity);
    void Set_Position(const Coord3D &pos);

private:
    RadiusDecalTemplate *m_template;
    Shadow *m_shadow;
    bool m_needsInit;
    friend class RadiusDecalTemplate;
};
