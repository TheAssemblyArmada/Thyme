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
#pragma once

#include "always.h"

// TODO This is a massive static class and is currently WIP to support implementing other classes.
class W3D
{
public:
    // Timing functions.
    static unsigned Get_Sync_Time() { return s_syncTime; }

    // Texture control functions.
    static unsigned Get_Texture_Reduction() { return s_textureReduction; }
    static unsigned Get_Texture_Min_Dimension() { return s_textureMinDimension; }
    static bool Large_Texture_Extra_Reduction_Enabled() { return s_largeTextureExtraReduction; }
    static bool Is_Screen_UVBiased() { return s_isScreenUVBiased; }
    static bool Is_Texturing_Enabled() { return s_texturingEnabled; }
    static bool Is_Thumbnail_Enabled() { return s_thumbnailEnabled; }
    static int Get_Texture_Filter() { return s_textureFilter; }
    static float Get_Default_Native_Screen_Size() { return s_defaultNativeScreenSize; }

    // Calls to the graphics wrapper.
    static void Get_Device_Resolution(int &width, int &height, int &bit_depth, bool &windowed);
    static int Get_Texture_Bit_Depth();
    static void Invalidate_Mesh_Cache();
    static void Invalidate_Textures();

private:
#ifdef GAME_DLL
    static unsigned &s_syncTime;
    static unsigned &s_textureReduction;
    static unsigned &s_textureMinDimension;
    static bool &s_largeTextureExtraReduction;
    static bool &s_isScreenUVBiased;
    static bool &s_texturingEnabled;
    static bool &s_thumbnailEnabled;
    static int &s_textureFilter;
    static float &s_defaultNativeScreenSize;
#else
    static unsigned s_syncTime;
    static unsigned s_textureReduction;
    static unsigned s_textureMinDimension;
    static bool s_largeTextureExtraReduction;
    static bool s_isScreenUVBiased;
    static bool s_texturingEnabled;
    static bool s_thumbnailEnabled;
    static int s_textureFilter;
    static float s_defaultNativeScreenSize;
#endif
};
