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
class Shadow;

class RadiusDecalTemplate
{
public:
    Utf8String m_texture;
    int32_t m_style;
    float m_opacityMin;
    float m_opacityMax;
    int32_t m_opacityThrobTime;
    int32_t m_oolor;
    bool m_onlyVisibleToOwningPlayer;
};

struct RadiusDecal
{
    RadiusDecalTemplate *m_template;
    Shadow *m_shadow;
    bool m_unk;
};
