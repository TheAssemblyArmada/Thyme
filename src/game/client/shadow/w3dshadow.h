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
class W3DShadowManager
{
    void Set_Shadow_Color(unsigned int color) { m_shadowColor = color; }

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
