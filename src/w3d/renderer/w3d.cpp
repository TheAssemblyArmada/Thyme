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
#include "assetmgr.h"
#include "dx8renderer.h"
#include "dx8wrapper.h"
#include "hashtemplate.h"
#include "textureloader.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
unsigned W3D::s_syncTime;
unsigned W3D::s_textureReduction;
unsigned W3D::s_textureMinDimension = 1;
bool W3D::s_largeTextureExtraReduction;
bool W3D::s_isScreenUVBiased;
bool W3D::s_texturingEnabled = true;
bool W3D::s_thumbnailEnabled = true;
int W3D::s_textureFilter;
float W3D::s_defaultNativeScreenSize = 1;
bool W3D::s_isSortingEnabled = true;
bool W3D::s_isColoringEnabled;
bool W3D::s_mungeSortOnLoad;
bool W3D::s_overbrightModifyOnLoad;
W3D::PrelitModeEnum W3D::s_prelitMode;
bool W3D::s_areStaticSortListsEnabled;
#ifdef PLATFORM_WINDOWS
HWND W3D::s_hwnd;
#endif
#endif

void W3D::Get_Device_Resolution(int &width, int &height, int &bit_depth, bool &windowed)
{
    DX8Wrapper::Get_Device_Resolution(width, height, bit_depth, windowed);
}

void W3D::Get_Render_Target_Resolution(int &set_w, int &set_h, int &set_bits, bool &set_windowed)
{
    DX8Wrapper::Get_Render_Target_Resolution(set_w, set_h, set_bits, set_windowed);
}

const RenderDeviceDescClass W3D::Get_Render_Device_Desc(int deviceidx)
{
    return DX8Wrapper::Get_Render_Device_Desc(deviceidx);
}

int W3D::Get_Texture_Bit_Depth()
{
    return DX8Wrapper::s_textureBitDepth;
}

void W3D::Invalidate_Mesh_Cache()
{
    g_theDX8MeshRenderer.Invalidate(false);
}

void W3D::Invalidate_Textures()
{
    if (W3DAssetManager::Get_Instance()) {
        TextureLoader::Flush_Pending_Load_Tasks();
        for (HashTemplateIterator<StringClass, TextureClass *> textureIter(W3DAssetManager::Get_Instance()->Texture_Hash());
             textureIter;
             ++textureIter) {
            TextureClass *texture = textureIter.getValue();
            texture->Invalidate();
        }
    }
}

W3DErrorType W3D::Set_Device_Resolution(int width, int height, int bits, int windowed, bool resize_window)
{
    return DX8Wrapper::Set_Device_Resolution(width, height, bits, windowed, resize_window) != 0 ?
        W3D_ERROR_OK :
        W3D_ERROR_INITIALIZATION_FAILED;
}

void W3D::Add_To_Static_Sort_List(RenderObjClass *robj, unsigned int sort_level)
{
#ifdef GAME_DLL
    Call_Function<void, RenderObjClass *, unsigned int>(PICK_ADDRESS(0x008082C0, 0x00504E90), robj, sort_level);
#endif
}
