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
#include "w3derr.h"

class RenderDeviceDescClass;
class RenderObjClass;

// TODO This is a massive static class and is currently WIP to support implementing other classes.
class W3D
{
public:
    enum PrelitModeEnum
    {
        PRELIT_MODE_VERTEX,
        PRELIT_MODE_LIGHTMAP_MULTI_PASS,
        PRELIT_MODE_LIGHTMAP_MULTI_TEXTURE
    };

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
    static bool Is_Sorting_Enabled() { return s_isSortingEnabled; }
    static bool Is_Coloring_Enabled() { return s_isColoringEnabled; }
    static bool Is_Munge_Sort_On_Load_Enabled() { return s_mungeSortOnLoad; }
    static bool Is_Overbright_Modify_On_Load_Enabled() { return s_overbrightModifyOnLoad; }
    static bool Are_Static_Sort_Lists_Enabled(void) { return s_areStaticSortListsEnabled; }

    static void Enable_Texturing(bool b) { s_texturingEnabled = b; }

    static void Add_To_Static_Sort_List(RenderObjClass *robj, unsigned int sort_level);

    // Calls to the graphics wrapper.
    static void Get_Device_Resolution(int &width, int &height, int &bit_depth, bool &windowed);
    static void Get_Render_Target_Resolution(int &set_w, int &set_h, int &set_bits, bool &set_windowed);
    static const RenderDeviceDescClass Get_Render_Device_Desc(int deviceidx);
    static int Get_Texture_Bit_Depth();
    static void Invalidate_Mesh_Cache();
    static void Invalidate_Textures();
    static W3DErrorType Set_Device_Resolution(int width, int height, int bits, int windowed, bool resize_window);
    static PrelitModeEnum Get_Prelit_Mode() { return (s_prelitMode); }

    // Platform functions.
#ifdef PLATFORM_WINDOWS
    static HWND Get_Window() { return s_hwnd; }
#endif

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
    static bool &s_isSortingEnabled;
    static bool &s_isColoringEnabled;
    static bool &s_mungeSortOnLoad;
    static bool &s_overbrightModifyOnLoad;
    static PrelitModeEnum &s_prelitMode;
    static bool &s_areStaticSortListsEnabled;
#ifdef PLATFORM_WINDOWS
    static HWND &s_hwnd;
#endif
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
    static bool s_isSortingEnabled;
    static bool s_isColoringEnabled;
    static bool s_mungeSortOnLoad;
    static bool s_overbrightModifyOnLoad;
    static PrelitModeEnum s_prelitMode;
    static bool s_areStaticSortListsEnabled;
#ifdef PLATFORM_WINDOWS
    static HWND s_hwnd;
#endif
#endif
};
