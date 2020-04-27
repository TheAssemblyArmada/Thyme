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
#include "dx8wrapper.h"

#ifndef GAME_DLL
unsigned W3D::s_syncTime;
unsigned W3D::s_textureReduction;
unsigned W3D::s_textureMinDimension = 1;
bool W3D::s_largeTextureExtraReduction;
bool W3D::s_isScreenUVBiased;
bool W3D::s_texturingEnabled = true;
bool W3D::s_thumbnailEnabled = true;
bool W3D::s_textureFilter;
#endif

void W3D::Get_Device_Resolution(int &width, int &height, int &bit_depth, bool &windowed)
{
    DX8Wrapper::Get_Device_Resolution(width, height, bit_depth, windowed);
}

int W3D::Get_Texture_Bit_Depth()
{
    return DX8Wrapper::Get_Texture_Bit_Depth();
}

void W3D::_Invalidate_Mesh_Cache()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x00807840, 0x00503700));
#endif
}
