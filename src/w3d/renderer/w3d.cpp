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
#include "boxrobj.h"
#include "camera.h"
#include "dx8indexbuffer.h"
#include "dx8renderer.h"
#include "dx8texman.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "ffactory.h"
#include "hashtemplate.h"
#include "iniclass.h"
#include "scene.h"
#include "sortingrenderer.h"
#include "textureloader.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifdef PLATFORM_WINDOWS
#include <mmsystem.h>
#endif

#ifndef GAME_DLL
unsigned W3D::s_syncTime;
unsigned W3D::s_previousSyncTime;
unsigned W3D::s_textureReduction;
unsigned W3D::s_textureMinDimension = 1;
bool W3D::s_largeTextureExtraReduction;
bool W3D::s_isScreenUVBiased;
bool W3D::s_texturingEnabled = true;
bool W3D::s_thumbnailEnabled = true;
TextureFilterClass::TextureFilterMode W3D::s_textureFilter;
float W3D::s_defaultNativeScreenSize = 1;
bool W3D::s_isSortingEnabled = true;
bool W3D::s_isColoringEnabled;
bool W3D::s_mungeSortOnLoad;
bool W3D::s_overbrightModifyOnLoad;
W3D::PrelitModeEnum W3D::s_prelitMode = PRELIT_MODE_LIGHTMAP_MULTI_PASS;
bool W3D::s_areStaticSortListsEnabled;
bool W3D::s_isInited;
bool W3D::s_isRendering;
bool W3D::s_lite;
int W3D::s_frameCount;
StaticSortListClass *W3D::s_defaultStaticSortLists;
StaticSortListClass *W3D::s_currentStaticSortLists;
#ifdef PLATFORM_WINDOWS
HWND W3D::s_hwnd;
#endif
#endif

class StaticSortListClass
{
public:
    virtual ~StaticSortListClass() {}
    virtual void Add_To_List(RenderObjClass *robj, unsigned int sort_level) = 0;
    virtual void Render_And_Clear(RenderInfoClass &rinfo) = 0;
};

class DefaultStaticSortListClass : public StaticSortListClass
{
public:
    DefaultStaticSortListClass() : m_minLevel(1), m_maxLevel(ARRAY_SIZE(m_lists) - 1) {}
    virtual ~DefaultStaticSortListClass() override {}
    virtual void Add_To_List(RenderObjClass *robj, unsigned int sort_level) override;
    virtual void Render_And_Clear(RenderInfoClass &rinfo) override;

private:
    unsigned int m_minLevel;
    unsigned int m_maxLevel;
    RefMultiListClass<RenderObjClass> m_lists[33];
};

void DefaultStaticSortListClass::Add_To_List(RenderObjClass *robj, unsigned int sort_level)
{
    if (sort_level >= 1 && sort_level <= ARRAY_SIZE(m_lists) - 1) {
        m_lists[sort_level].Add_Tail(robj, false);
    } else {
        captainslog_assert(0);
    }
}

void DefaultStaticSortListClass::Render_And_Clear(RenderInfoClass &rinfo)
{
    for (unsigned int i = m_maxLevel; i >= m_minLevel; i--) {
        bool rendered = false;
        RenderObjClass *robj = m_lists[i].Remove_Head();

        while (robj) {
            auto *unk = robj->Get_Unknown();

            if (unk != nullptr) {
                if (unk->Unk1(robj, rinfo)) {
                    robj->Render(rinfo);
                    rendered = true;
                }

                unk->Unk2(robj, rinfo);
            } else {
                robj->Render(rinfo);
                rendered = true;
            }

            robj->Release_Ref();
            robj = m_lists[i].Remove_Head();
        }

        if (rendered) {
            g_theDX8MeshRenderer.Flush();
        }
    }
}

void W3D::Get_Device_Resolution(int &width, int &height, int &bit_depth, bool &windowed)
{
    DX8Wrapper::Get_Device_Resolution(width, height, bit_depth, windowed);
}

void W3D::Get_Render_Target_Resolution(int &set_w, int &set_h, int &set_bits, bool &set_windowed)
{
    DX8Wrapper::Get_Render_Target_Resolution(set_w, set_h, set_bits, set_windowed);
}

const RenderDeviceDescClass &W3D::Get_Render_Device_Desc(int deviceidx)
{
    return DX8Wrapper::Get_Render_Device_Desc(deviceidx);
}

int W3D::Get_Render_Device_Count()
{
    return DX8Wrapper::Get_Render_Device_Count();
}

const char *W3D::Get_Render_Device_Name(int device_index)
{
    return DX8Wrapper::Get_Render_Device_Name(device_index);
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
             !textureIter.Is_Done();
             textureIter.Next()) {
            TextureClass *texture = textureIter.Peek_Value();
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
    s_currentStaticSortLists->Add_To_List(robj, sort_level);
}

W3DErrorType W3D::Init(void *hwnd, char *defaultpal, bool lite)
{
#ifdef PLATFORM_WINDOWS
    s_hwnd = (HWND)hwnd;
    s_lite = lite;
    Init_D3D_To_WW3_Conversion();

    if (!DX8Wrapper::Init(s_hwnd, lite)) {
        return W3D_ERROR_INITIALIZATION_FAILED;
    }

    MMRESULT r = timeBeginPeriod(1);
    captainslog_assert(r == TIMERR_NOERROR);

    s_defaultStaticSortLists = new DefaultStaticSortListClass();
    Reset_Current_Static_Sort_Lists_To_Default();

    if (!lite) {
        s_isInited = true;
    }

    captainslog_debug("WW3D Init completed");

    return W3D_ERROR_OK;
#else
    return W3D_ERROR_INITIALIZATION_FAILED;
#endif
}

W3DErrorType W3D::Shutdown()
{
#ifdef PLATFORM_WINDOWS
    MMRESULT r = timeEndPeriod(1);
    captainslog_assert(r == TIMERR_NOERROR);

    if (W3DAssetManager::Get_Instance()) {
        W3DAssetManager::Get_Instance()->Free_Assets();
    }

    DX8TextureManagerClass::Shutdown();

    if (!s_lite) {
        DX8Wrapper::Shutdown();
    }

    if (s_defaultStaticSortLists) {
        delete s_defaultStaticSortLists;
    }

    s_isInited = false;
    return W3D_ERROR_OK;
#else
    return W3D_ERROR_OK;
#endif
}

W3DErrorType W3D::Set_Render_Device(
    int dev, int resx, int resy, int bits, int windowed, bool resize_window, bool reset_device, bool restore_assets)
{
    return DX8Wrapper::Set_Render_Device(dev, resx, resy, bits, windowed, resize_window, reset_device, restore_assets) != 0 ?
        W3D_ERROR_OK :
        W3D_ERROR_INITIALIZATION_FAILED;
}

int W3D::Get_Render_Device()
{
    return DX8Wrapper::Get_Render_Device();
}

W3DErrorType W3D::Begin_Render(bool clear, bool clearz, const Vector3 &color, float alpha, void (*network_callback)())
{
#ifdef BUILD_WITH_D3D8
    if (!s_isInited) {
        return W3D_ERROR_OK;
    }

    HRESULT res;

    if (DX8Wrapper::Get_D3D_Device8() != nullptr && FAILED(res = DX8Wrapper::Get_D3D_Device8()->TestCooperativeLevel())) {
        if (res == D3DERR_DEVICELOST) {
            return W3D_ERROR_GENERIC;
        } else if (res == D3DERR_DEVICENOTRESET) {
            DX8Wrapper::Reset_Device(true);
            return W3D_ERROR_GENERIC;
        }
    } else {
        TextureLoader::Update(network_callback);
        DynamicVBAccessClass::Reset(true);
        DynamicIBAccessClass::Reset(true);
        // Debug_Statistics::Begin_Statistics();
        captainslog_assert(!s_isRendering);
        s_isRendering = true;

        if (clear || clearz) {
            int width;
            int height;
            int depth;
            bool windowed;
            DX8Wrapper::Get_Render_Target_Resolution(width, height, depth, windowed);
            D3DVIEWPORT8 vp;
            vp.X = 0;
            vp.Y = 0;
            vp.Width = width;
            vp.Height = height;
            vp.MinZ = 0.0f;
            vp.MaxZ = 1.0f;
            DX8Wrapper::Set_Viewport(&vp);
            DX8Wrapper::Clear(clear, clearz, color, alpha);
        }

        DX8Wrapper::Begin_Scene();
    }
#endif
    return W3D_ERROR_OK;
}

W3DErrorType W3D::Render(SceneClass *scene, CameraClass *cam, bool clear, bool clearz, const Vector3 &color)
{
#ifdef BUILD_WITH_D3D8
    if (!s_isInited) {
        return W3D_ERROR_OK;
    }

    captainslog_assert(s_isRendering);
    captainslog_assert(scene);
    captainslog_assert(cam);

    cam->On_Frame_Update();
    RenderInfoClass rinfo(*cam);
    cam->Apply();

    if (clear || clearz) {
        DX8Wrapper::Clear(clear, clearz, color, 0.0f);
    }

    switch (scene->Get_Polygon_Mode()) {
        case SceneClass::POINT:
            DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE, D3DFILL_POINT);
            break;
        case SceneClass::LINE:
            DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
            break;
        case SceneClass::FILL:
            DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE, D3DFILL_SOLID);
            break;
    }

    DX8Wrapper::Set_Ambient_Color(scene->Get_Ambient_Light());

    g_theDX8MeshRenderer.Set_Camera(&rinfo.m_camera);
    scene->Render(rinfo);
    Flush(rinfo);
#endif
    return W3D_ERROR_OK;
}

W3DErrorType W3D::Render(RenderObjClass &obj, RenderInfoClass &rinfo)
{
#ifdef BUILD_WITH_D3D8
    if (s_isInited) {
        return W3D_ERROR_OK;
    }

    rinfo.m_camera.On_Frame_Update();
    rinfo.m_camera.Apply();
    DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE, D3DFILL_SOLID);

    if (rinfo.m_lightEnvironment) {
        DX8Wrapper::Set_Light_Environment(rinfo.m_lightEnvironment);
    }

    g_theDX8MeshRenderer.Set_Camera(&rinfo.m_camera);
    obj.Render(rinfo);
    Flush(rinfo);
#endif
    return W3D_ERROR_OK;
}

void W3D::Flush(RenderInfoClass &rinfo)
{
    g_theDX8MeshRenderer.Flush();
    Render_And_Clear_Static_Sort_Lists(rinfo);
    SortingRendererClass::Flush();
    g_theDX8MeshRenderer.Clear_Pending_Delete_Lists();
}

W3DErrorType W3D::End_Render(bool flip_frame)
{
    if (s_isInited) {
        SortingRendererClass::Flush();
        s_isRendering = false;
        DX8Wrapper::End_Scene();
        s_frameCount++;
        // Debug_Statistics::End_Statistics();
        DX8Wrapper::Invalidate_Cached_Render_States();
    }

    return W3D_ERROR_OK;
}

void W3D::Sync(unsigned int sync_time)
{
    s_previousSyncTime = s_syncTime;
    s_syncTime = sync_time;
}

void W3D::Set_Collision_Box_Display_Mask(int mask)
{
    BoxRenderObjClass::Set_Box_Display_Mask(mask);
}

void W3D::Set_Texture_Reduction(int value, int min_dim)
{
    if (s_textureReduction != value || s_textureMinDimension != min_dim) {
        s_textureReduction = value;
        s_textureMinDimension = min_dim;
        Invalidate_Textures();
    }
}

void W3D::Enable_Coloring(unsigned int enable)
{
    s_isColoringEnabled = enable != 0;
}

void W3D::Enable_Sorting(bool onoff)
{
    s_isSortingEnabled = onoff;
    g_theDX8MeshRenderer.Invalidate(false);
}

void W3D::Allocate_Debug_Resources() {}

void W3D::Release_Debug_Resources() {}

void W3D::Set_Texture_Bit_Depth(int bitdepth)
{
    DX8Wrapper::Set_Texture_Bitdepth(bitdepth);
}

void W3D::Render_And_Clear_Static_Sort_Lists(RenderInfoClass &rinfo)
{
    bool enabled = s_areStaticSortListsEnabled;
    s_areStaticSortListsEnabled = false;
    s_currentStaticSortLists->Render_And_Clear(rinfo);
    s_areStaticSortListsEnabled = enabled;
}

void W3D::Reset_Current_Static_Sort_Lists_To_Default()
{
    s_currentStaticSortLists = s_defaultStaticSortLists;
}
