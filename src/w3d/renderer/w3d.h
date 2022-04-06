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
#include "texture.h"
#include "vector3.h"
#include "w3derr.h"

class RenderDeviceDescClass;
class RenderObjClass;
class RenderInfoClass;
class SceneClass;
class CameraClass;
class StaticSortListClass;

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
    static bool Is_Large_Texture_Extra_Reduction_Enabled() { return s_largeTextureExtraReduction; }
    static bool Is_Screen_UVBiased() { return s_isScreenUVBiased; }
    static void Set_Screen_UV_Bias(bool bias) { s_isScreenUVBiased = bias; }
    static bool Is_Texturing_Enabled() { return s_texturingEnabled; }
    static bool Is_Thumbnail_Enabled() { return s_thumbnailEnabled; }
    static void Set_Thumbnail_Enabled(bool enable) { s_thumbnailEnabled = enable; }
    static TextureFilterClass::TextureFilterMode Get_Texture_Filter() { return s_textureFilter; }
    static float Get_Default_Native_Screen_Size() { return s_defaultNativeScreenSize; }
    static bool Is_Sorting_Enabled() { return s_isSortingEnabled; }
    static bool Is_Coloring_Enabled() { return s_isColoringEnabled; }
    static bool Is_Munge_Sort_On_Load_Enabled() { return s_mungeSortOnLoad; }
    static bool Is_Overbright_Modify_On_Load_Enabled() { return s_overbrightModifyOnLoad; }
    static bool Are_Static_Sort_Lists_Enabled() { return s_areStaticSortListsEnabled; }
    static void Enable_Static_Sort_Lists(bool enable) { s_areStaticSortListsEnabled = enable; }
    static void Enable_Munge_Sort_On_Load(bool onoff) { s_mungeSortOnLoad = onoff; }
    static void Enable_Sorting(bool onoff);

    static void Enable_Texturing(bool b) { s_texturingEnabled = b; }

    static void Add_To_Static_Sort_List(RenderObjClass *robj, unsigned int sort_level);

    // Calls to the graphics wrapper.
    static void Get_Device_Resolution(int &width, int &height, int &bit_depth, bool &windowed);
    static void Get_Render_Target_Resolution(int &set_w, int &set_h, int &set_bits, bool &set_windowed);
    static const RenderDeviceDescClass &Get_Render_Device_Desc(int deviceidx);
    static int Get_Render_Device_Count();
    static const char *Get_Render_Device_Name(int device_index);
    static int Get_Texture_Bit_Depth();
    static void Invalidate_Mesh_Cache();
    static void Invalidate_Textures();
    static W3DErrorType Set_Device_Resolution(int width, int height, int bits, int windowed, bool resize_window);
    static PrelitModeEnum Get_Prelit_Mode() { return (s_prelitMode); }
    static void Set_Prelit_Mode(PrelitModeEnum mode) { s_prelitMode = mode; }

    static W3DErrorType Init(void *hwnd, char *defaultpal = nullptr, bool lite = false);
    static W3DErrorType Shutdown();
    static W3DErrorType Set_Render_Device(
        int dev, int resx, int resy, int bits, int windowed, bool resize_window, bool reset_device, bool restore_assets);
    static int Get_Render_Device();
    static W3DErrorType Begin_Render(bool clear = false,
        bool clearz = true,
        const Vector3 &color = Vector3(0, 0, 0),
        float alpha = 0.0f,
        void (*network_callback)() = nullptr);
    static W3DErrorType Render(SceneClass *scene,
        CameraClass *cam,
        bool clear = false,
        bool clearz = false,
        const Vector3 &color = Vector3(0, 0, 0));
    static W3DErrorType Render(RenderObjClass &obj, RenderInfoClass &rinfo);
    static void Flush(RenderInfoClass &rinfo);
    static W3DErrorType End_Render(bool flip_frame = true);
    static void Sync(unsigned int sync_time);
    static void Set_Collision_Box_Display_Mask(int mask);
    static void Set_Texture_Reduction(int value, int min_dim);
    static void Enable_Coloring(unsigned int enable);
    static void Allocate_Debug_Resources();
    static void Release_Debug_Resources();
    static void Set_Texture_Bit_Depth(int bitdepth);
    static void Render_And_Clear_Static_Sort_Lists(RenderInfoClass &rinfo);
    static void Reset_Current_Static_Sort_Lists_To_Default();
    static unsigned int Get_Frame_Time() { return s_syncTime - s_previousSyncTime; }
    static unsigned int Get_Frame_Count() { return s_frameCount; }
    static bool Is_Initted() { return s_isInited; }

    // Platform functions.
#ifdef PLATFORM_WINDOWS
    static HWND Get_Window() { return s_hwnd; }
#endif

private:
#ifdef GAME_DLL
    static unsigned &s_syncTime;
    static unsigned &s_previousSyncTime;
    static unsigned &s_textureReduction;
    static unsigned &s_textureMinDimension;
    static bool &s_largeTextureExtraReduction;
    static bool &s_isScreenUVBiased;
    static bool &s_texturingEnabled;
    static bool &s_thumbnailEnabled;
    static TextureFilterClass::TextureFilterMode &s_textureFilter;
    static float &s_defaultNativeScreenSize;
    static bool &s_isSortingEnabled;
    static bool &s_isColoringEnabled;
    static bool &s_mungeSortOnLoad;
    static bool &s_overbrightModifyOnLoad;
    static PrelitModeEnum &s_prelitMode;
    static bool &s_areStaticSortListsEnabled;
    static bool &s_isInited;
    static bool &s_isRendering;
    static bool &s_lite;
    static int &s_frameCount;
    static StaticSortListClass *&s_defaultStaticSortLists;
    static StaticSortListClass *&s_currentStaticSortLists;
#ifdef PLATFORM_WINDOWS
    static HWND &s_hwnd;
#endif
#else
    static unsigned s_syncTime;
    static unsigned s_previousSyncTime;
    static unsigned s_textureReduction;
    static unsigned s_textureMinDimension;
    static bool s_largeTextureExtraReduction;
    static bool s_isScreenUVBiased;
    static bool s_texturingEnabled;
    static bool s_thumbnailEnabled;
    static TextureFilterClass::TextureFilterMode s_textureFilter;
    static float s_defaultNativeScreenSize;
    static bool s_isSortingEnabled;
    static bool s_isColoringEnabled;
    static bool s_mungeSortOnLoad;
    static bool s_overbrightModifyOnLoad;
    static PrelitModeEnum s_prelitMode;
    static bool s_areStaticSortListsEnabled;
    static bool s_isInited;
    static bool s_isRendering;
    static bool s_lite;
    static int s_frameCount;
    static StaticSortListClass *s_defaultStaticSortLists;
    static StaticSortListClass *s_currentStaticSortLists;
#ifdef PLATFORM_WINDOWS
    static HWND s_hwnd;
#endif
#endif
};
