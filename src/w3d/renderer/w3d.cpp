/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Static class providing overall control and data for w3d rendering engine.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3d.h"

#ifdef GAME_DLL
unsigned &W3D::s_syncTime = Make_Global<unsigned>(0x00A47F9C);
unsigned &W3D::s_textureReduction = Make_Global<unsigned>(0x00A47FE4);
unsigned &W3D::s_textureMinDimension = Make_Global<unsigned>(0x00A1699C);
bool &W3D::s_largeTextureExtraReduction = Make_Global<bool>(0x00A47FE8);
#else
unsigned W3D::s_syncTime;
unsigned W3D::s_textureReduction;
unsigned W3D::s_textureMinDimension = 1;
bool W3D::s_largeTextureExtraReduction;
#endif
