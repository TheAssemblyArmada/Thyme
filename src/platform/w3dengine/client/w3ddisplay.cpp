/**
 * @file
 *
 * @author Duncans_pumpkin
 *
 * @brief W3D class for the display handling.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */

#include "w3ddisplay.h"
#include "assetmgr.h"
#include "baseheightmap.h"
#include "displaystring.h"
#include "displaystringmanager.h"
#include "dx8wrapper.h"
#include "filesystem.h"
#include "globaldata.h"
#include "image.h"
#include "predlod.h"
#include "renderdevicedesc.h"
#include "rtsutils.h"
#include "shadermanager.h"
#include "videobuffer.h"
#include "view.h"
#include "w3d.h"

#ifndef GAME_DLL
GameAssetManager *W3DDisplay::s_assetManager;
SceneClass *W3DDisplay::s_3DScene; // TODO: Actual type is RTS2DScene
SceneClass *W3DDisplay::s_2DScene; // TODO: Actual type is RTS2DScene
SceneClass *W3DDisplay::s_3DInterfaceScene; // TODO: Actual type is RTS3DInterfaceScene
#endif

// 0x0073C3C0
W3DDisplay::W3DDisplay() :
    m_initialized(false), m_2DRender(0), m_isClippedEnabled(false), m_clipRegion({ { 0, 0 }, { 0, 0 } })
{
    s_assetManager = nullptr;
    s_3DScene = nullptr;
    s_2DScene = nullptr;
    s_3DInterfaceScene = nullptr;
    m_averageFps = g_theWriteableGlobalData->m_framesPerSecondLimit;
#ifdef GAME_DEBUG_STRUCTS
    m_unknqword = 0;
#endif
    for (auto &str : m_displayStrings) {
        str = nullptr;
    }
    for (auto &light : m_myLight) {
        light = nullptr;
    }
}

// 0x0073C453
W3DDisplay::~W3DDisplay()
{
    // delete m_debugDisplay;
    // for (auto &str : m_displayStrings) {
    //    g_theDisplayStringManager->Free_Display_String(str);
    //}

    // delete m_2DRender;
    // m_2DRender = nullptr;
    // Delete_Views();
    // s_3DScene->Release_Ref();
    // s_3DScene = nullptr;
    // s_2DScene->Release_Ref();
    // s_2DScene = nullptr;
    // s_3DInterfaceScene->Release_Ref();
    // s_3DInterfaceScene = nullptr;

    // for (auto &light : m_myLight) {
    //    light->Release_Ref();
    //    light = nullptr;
    //}

    // PredictiveLODOptimizerClass::Free();
    //// unk string 0xA48068 StringClass::Release_Resources();
    // W3DShaderManager::Shutdown();
    // s_assetManager->Free_Assets();
    // delete s_assetManager;
    //// WW3D::Shutdown();
    //// WWMath::Shutdown();
    //// Browser Shutdown();
    //// delete g_theW3DFileSystem;

    // TODO: Requires WW3D, WWMath, g_theW3DFileSystem, 0xA48068, Browser
#ifdef GAME_DLL
    Call_Method<void, W3DDisplay>(0x0073C470, this);
#endif
}

// 0x0073CA80
void W3DDisplay::Init()
{
    // TODO: Requires RTS3DInterfaceScene, RTS2DScene, RTS3DScene, GameAssetManager, ...
#ifdef GAME_DLL
    Call_Method<void, W3DDisplay>(0x0073CA80, this);
#endif
}

// 0x0073D030
void W3DDisplay::Reset()
{
    Display::Reset();

    auto *iter = s_3DScene->Create_Iterator(false);
    for (iter->First(); !iter->Is_Done(); iter->Next()) {
        auto *renderObj = iter->Current_Item();
        renderObj->Add_Ref();
        s_3DScene->Remove_Render_Object(renderObj);
        renderObj->Release_Ref();
    }
    s_3DScene->Destroy_Iterator(iter);

    m_isClippedEnabled = false;

    s_assetManager->Release_Unused_Assets();

    if (g_theWriteableGlobalData != nullptr) {
        g_theWriteableGlobalData->m_drawSkyBox = false;
    }
}

// 0x0073E360
void W3DDisplay::Draw()
{
    // Requires GameLogic, GameClient, ...
#ifdef GAME_DLL
    Call_Method<void, W3DDisplay>(0x0073E360, this);
#endif
}

// 0x0073C980
void W3DDisplay::Set_Width(uint32_t width)
{
    Display::Set_Width(width);
    RectClass rect = { { 0.0f, 0.0f }, { static_cast<float>(Get_Width()), static_cast<float>(Get_Height()) } };
    m_2DRender->Set_Coordinate_Range(rect);
}

// 0x0073CA00
void W3DDisplay::Set_Height(uint32_t height)
{
    Display::Set_Height(height);
    RectClass rect = { { 0.0f, 0.0f }, { static_cast<float>(Get_Width()), static_cast<float>(Get_Height()) } };
    m_2DRender->Set_Coordinate_Range(rect);
}

// 0x0073C840
bool W3DDisplay::Set_Display_Mode(uint32_t width, uint32_t height, uint32_t bits, bool windowed)
{
    auto success = W3D::Set_Device_Resolution(width, height, bits, windowed, true) == W3D_ERROR_OK;

    if (success) {
        RectClass rect = { { 0.0f, 0.0f }, { static_cast<float>(width), static_cast<float>(height) } };
        m_2DRender->Set_Screen_Resolution(rect);

        Display::Set_Display_Mode(width, height, bits, windowed);

        return true;
    } else {
        // Set resolution failed revert to previous resolution
        W3D::Set_Device_Resolution(Get_Width(), Get_Height(), Get_Bit_Depth(), Get_Windowed(), true);

        RectClass rect = { { 0.0f, 0.0f }, { static_cast<float>(Get_Width()), static_cast<float>(Get_Height()) } };
        m_2DRender->Set_Screen_Resolution(rect);

        Display::Set_Display_Mode(Get_Width(), Get_Height(), Get_Bit_Depth(), Get_Windowed());

        return false;
    }
}

static bool Is_Resolution_Valid(const ResolutionDescClass &res)
{
    if (res.Get_Depth() < 24) {
        return false;
    }

    if (res.Get_Width() < 800) {
        return false;
    }

    if (res.Get_Height() == 0) {
        return false;
    }

    // Thyme Change: Aspect ratio check removed as users do not mind some aspect issues
    // float aspect_ratio = fabs(static_cast<float>(res.Get_Width()) / res.Get_Height());
    // if (aspect_ratio < 1.332f || aspect_ratio > 1.334f) {
    //    return false;
    //}
    return true;
}

// 0x0073C5D0
int W3DDisplay::Get_Display_Mode_Count()
{
    auto &desc = W3D::Get_Render_Device_Desc(0);
    auto &resolutions = desc.Get_Resolution_Array();

    int valid_resolutions = 0;
    for (auto i = 0; i < resolutions.Count(); ++i) {
        auto &resolution = resolutions[i];

        if (Is_Resolution_Valid(resolution) == false) {
            continue;
        }

        valid_resolutions++;
    }
    return valid_resolutions;
}

// 0x0073C650
void W3DDisplay::Get_Display_Mode_Description(int id, int *width, int *height, int *bit_depth)
{
    auto &desc = W3D::Get_Render_Device_Desc(0);
    auto &resolutions = desc.Get_Resolution_Array();

    // Id passed into function is in reference to valid resolutions
    int valid_resolutions = 0;
    for (auto i = 0; i < resolutions.Count(); ++i) {
        auto &resolution = resolutions[i];

        if (Is_Resolution_Valid(resolution) == false) {
            continue;
        }

        if (valid_resolutions != id) {
            valid_resolutions++;
            continue;
        }
        *width = resolution.Get_Width();
        *height = resolution.Get_Height();
        *bit_depth = resolution.Get_Depth();
        return;
    }
}

// 0x0073C710
void W3DDisplay::Set_Gamma(float gamma, float bright, float contrast, bool calibrate)
{
    if (m_windowed == true) {
        return;
    }

    DX8Wrapper::Set_Gamma(gamma, bright, contrast, calibrate, false);
}

// 0x00741360 Unsure if used
void W3DDisplay::Do_Smart_Asset_Purge_And_Preload(const char *asset)
{
    if (s_assetManager == nullptr) {
        return;
    }
    if (asset == nullptr) {
        return;
    }
    if (*asset == '\0') {
        return;
    }

    DynamicVectorClass<StringClass> assets_list(8000);
    auto *file = g_theFileSystem->Open(asset, File::TEXT | File::READ);
    if (file != nullptr) {
        Utf8String file_content;
        while (file->Scan_String(file_content)) {
            if (file_content.Starts_With(";") == false) {
                StringClass line{ file_content };
                assets_list.Add(line);
            }
        }
        file->Close();
    }
    s_assetManager->Free_Assets_With_Exclusion_List(assets_list);
}

// 0x00740B10
VideoBuffer *W3DDisplay::Create_VideoBuffer()
{
    // TODO: Requires W3DVideoBuffer
#ifdef GAME_DLL
    return Call_Method<VideoBuffer *, W3DDisplay>(0x00740B10, this);
#else
    return nullptr;
#endif
}

// 0x00740C80
void W3DDisplay::Set_Clip_Region(IRegion2D *region)
{
    m_clipRegion = *region;
    m_isClippedEnabled = true;
}

// 0x0073EE10
void W3DDisplay::Set_Time_Of_Day(TimeOfDayType time)
{
    if (s_3DScene != nullptr) {
        const auto &ambient = g_theWriteableGlobalData->m_terrainObjectLighting[time][0].ambient;
        s_3DScene->Set_Ambient_Light({ ambient.red, ambient.green, ambient.blue });
    }

    for (auto i = 0; i < 4; ++i) {
        if (m_myLight[i] == nullptr) {
            continue;
        }

        auto &light = *m_myLight[i];
        light.Set_Ambient({ 0.0f, 0.0f, 0.0f });
        const auto &lighting = g_theWriteableGlobalData->m_terrainObjectLighting[time][i];
        const auto &diffuse = lighting.diffuse;
        const auto &position = lighting.lightPos;
        light.Set_Diffuse({ diffuse.red, diffuse.green, diffuse.blue });
        light.Set_Specular({ 0.0f, 0.0f, 0.0f });
        Matrix3D mat{
            { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { position.x, position.y, position.z }, { 0.0f, 0.0f, 0.0f }
        };
        light.Set_Transform(mat);
    }

    if (g_theTerrainRenderObject != nullptr) {
        g_theTerrainRenderObject->Set_Time_Of_Day(time);
        g_theTacticalView->Force_Redraw();
    }
}

// 0x0073EC50
void W3DDisplay::Create_Light_Pulse(
    const Coord3D *pos, const RGBColor *color, float unk1, float unk2, unsigned unk3, unsigned unk4)
{
    // TODO: Requires 0x00772D00, 0x00769500
#ifdef GAME_DLL
    Call_Method<void, W3DDisplay, const Coord3D *, const RGBColor *, float, float, unsigned, unsigned>(
        0x0073EC50, this, pos, color, unk1, unk2, unk3, unk4);
#endif
}

// 0x0073EF90
void W3DDisplay::Draw_Line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, float width, uint32_t color)
{
    m_2DRender->Reset();
    m_2DRender->Enable_Texturing(false);
    Vector2 pos1 = { static_cast<float>(x1), static_cast<float>(y1) };
    Vector2 pos2 = { static_cast<float>(x2), static_cast<float>(y2) };
    m_2DRender->Add_Line(pos1, pos2, width, color);
    m_2DRender->Render();
}

// 0x0073F000
void W3DDisplay::Draw_Line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, float width, uint32_t color1, uint32_t color2)
{
    m_2DRender->Reset();
    m_2DRender->Enable_Texturing(false);
    Vector2 pos1 = { static_cast<float>(x1), static_cast<float>(y1) };
    Vector2 pos2 = { static_cast<float>(x2), static_cast<float>(y2) };
    m_2DRender->Add_Line(pos1, pos2, width, color1, color2);
    m_2DRender->Render();
}

// 0x0073F070
void W3DDisplay::Draw_Open_Rect(int32_t x, int32_t y, int32_t width, int32_t height, float border_width, uint32_t color)
{
    if (m_isClippedEnabled) {
        // Requires Clip_Line_2D
#ifdef GAME_DLL
        Call_Method<void, W3DDisplay, int32_t, int32_t, int32_t, int32_t, float, uint32_t>(
            0x0073F070, this, x, y, width, height, border_width, color);
#endif
    } else {
        m_2DRender->Reset();
        m_2DRender->Enable_Texturing(false);
        RectClass rect = { { static_cast<float>(x), static_cast<float>(y) },
            { static_cast<float>(x + width), static_cast<float>(y + height) } };
        m_2DRender->Add_Outline(rect, border_width, color);
        m_2DRender->Render();
    }
}

// 0x0073F2A0
void W3DDisplay::Draw_Fill_Rect(int32_t x, int32_t y, int32_t width, int32_t height, uint32_t color)
{
    m_2DRender->Reset();
    m_2DRender->Enable_Texturing(false);
    RectClass rect = { { static_cast<float>(x), static_cast<float>(y) },
        { static_cast<float>(x + width), static_cast<float>(y + height) } };
    m_2DRender->Add_Rect(rect, 0.0f, 0, color);
    m_2DRender->Render();
}

// 0x0073F320 Draws the percentage ontop of a rectangle
void W3DDisplay::Draw_Rect_Clock(int32_t x, int32_t y, int32_t width, int32_t height, int percentage, uint32_t color)
{
    // TODO: Function maybe not used at all
    if (percentage < 1 || percentage > 100) {
        return;
    }

    m_2DRender->Reset();
    m_2DRender->Enable_Texturing(false);

    float left = x;
    float top = y;
    float right = x + width;
    float bottom = y + height;
    float mid_x = width / 2;
    float mid_left = x + width / 2;
    float mid_y = height / 2;
    float mid_top = y + height / 2;
    if (percentage == 100) {
        // Draw a full rect for the whole area
        RectClass rect = { { left, top }, { right, bottom } };
        m_2DRender->Add_Rect(rect, 0.0f, 0, color);
        m_2DRender->Render();
        return;
    }

    Vector2 uv0{ 0.0f, 0.0f };
    Vector2 uv1{ 0.0f, 0.0f };
    Vector2 uv2{ 0.0f, 0.0f };

    if (percentage <= 25) {
        // At 12, clock points at corner of Right Top of box
        if (percentage <= 12) {
            // Right Top Moving Wedge

            float pt = mid_left + static_cast<float>(percentage) * (1.0f / 12.0f) * mid_x;
            Vector2 pt0{ mid_left, top };
            Vector2 pt1{ mid_left, mid_top };
            Vector2 pt2{ pt, top };

            m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
        } else {
            // Right Top Static Wedge
            {
                Vector2 pt0{ mid_left, mid_top };
                Vector2 pt1{ right, top };
                Vector2 pt2{ mid_left, top };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
            // Right Top Moving Wedge
            {
                float pt = static_cast<float>(percentage - 12) * (1.0f / 13.0f) * mid_y + top;
                Vector2 pt0{ right, top };
                Vector2 pt1{ mid_left, mid_top };
                Vector2 pt2{ right, pt };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
        }
    } else if (percentage <= 50) {
        // Right Top Box
        RectClass right_top_box = { { mid_left, top }, { right, mid_top } };
        m_2DRender->Add_Rect(right_top_box, 0.0f, 0, color);

        // At 37, clock points at corner of Right Bottom of box
        if (percentage <= 37) {

            // Right Bottom Moving Wedge
            float pt = static_cast<float>(percentage - 25) * (1.0f / 12.0f) * mid_y + mid_top;
            Vector2 pt0{ right, mid_top };
            Vector2 pt1{ mid_left, mid_top };
            Vector2 pt2{ right, pt };

            m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
        } else {
            // Right Bottom Static Wedge
            {
                Vector2 pt0{ right, mid_top };
                Vector2 pt1{ mid_left, mid_top };
                Vector2 pt2{ right, bottom };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
            // Right Bottom Moving Wedge
            {
                float pt = right - static_cast<float>(percentage - 37) * (1.0f / 13.0f) * mid_x;
                Vector2 pt0{ right, bottom };
                Vector2 pt1{ mid_left, mid_top };
                Vector2 pt2{ pt, bottom };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
        }
    } else if (percentage <= 75) {
        // Right Two Boxes
        RectClass right_rect = { { mid_left, top }, { right, bottom } };
        m_2DRender->Add_Rect(right_rect, 0.0f, 0, color);

        // At 62, clock points at corner of Left Bottom of box
        Vector2 pt0{ mid_left, mid_top };
        if (percentage <= 62) {

            // Left Bottom Moving Wedge
            float pt = mid_left - static_cast<float>(percentage - 50) * (1.0f / 12.0f) * mid_x;
            Vector2 pt1{ pt, bottom };
            Vector2 pt2{ mid_left, bottom };

            m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
        } else {
            // Left Bottom Static Wedge
            {
                Vector2 pt1{ left, bottom };
                Vector2 pt2{ mid_left, bottom };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
            // Left Bottom Moving Wedge
            {
                float pt = bottom - static_cast<float>(percentage - 62) * (1.0f / 13.0f) * mid_y;
                Vector2 pt1{ left, pt };
                Vector2 pt2{ left, bottom };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
        }
    } else {
        // Right Two Boxes
        RectClass right_rect = { { mid_left, top }, { right, bottom } };
        m_2DRender->Add_Rect(right_rect, 0.0f, 0, color);

        // Left Bottom Box
        RectClass left_bottom_rect = { { left, mid_top }, { mid_left, bottom } };
        m_2DRender->Add_Rect(left_bottom_rect, 0.0f, 0, color);

        // At 87, clock points at corner of Left Top of box
        if (percentage <= 87) {

            // Left Top Moving Wedge
            float pt = mid_top - static_cast<float>(percentage - 75) * (1.0f / 12.0f) * mid_y;
            Vector2 pt0{ left, mid_top };
            Vector2 pt1{ mid_left, mid_top };
            Vector2 pt2{ left, pt };

            m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
        } else {
            // Left Top Static Wedge
            {
                Vector2 pt0{ left, mid_top };
                Vector2 pt1{ mid_left, mid_top };
                Vector2 pt2{ left, top };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
            // Left Top Moving Wedge
            {
                float pt = left + static_cast<float>(percentage - 87) * (1.0f / 13.0f) * mid_x;
                Vector2 pt0{ left, top };
                Vector2 pt1{ mid_left, mid_top };
                Vector2 pt2{ pt, top };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
        }
    }
    m_2DRender->Render();
}

// 0x0073FD40 Draws the percentage remaining ontop of a rectangle
void W3DDisplay::Draw_Remaining_Rect_Clock(
    int32_t x, int32_t y, int32_t width, int32_t height, int percentage, uint32_t color)
{
    if (percentage < 0 || percentage > 99) {
        return;
    }

    m_2DRender->Reset();
    m_2DRender->Enable_Texturing(false);

    float left = x;
    float top = y;
    float right = x + width;
    float bottom = y + height;
    float mid_x = width / 2;
    float mid_left = x + width / 2;
    float mid_y = height / 2;
    float mid_top = y + height / 2;
    if (percentage == 0) {
        // Draw a full rect for the whole area
        RectClass rect = { { left, top }, { right, bottom } };
        m_2DRender->Add_Rect(rect, 0.0f, 0, color);
        m_2DRender->Render();
        return;
    }

    Vector2 uv0{ 0.0f, 0.0f };
    Vector2 uv1{ 0.0f, 0.0f };
    Vector2 uv2{ 0.0f, 0.0f };

    Vector2 pt0{ mid_left, mid_top };

    if (percentage < 25) {
        // Left 2 Boxes
        RectClass left_rect = { { left, top }, { mid_left, bottom } };
        m_2DRender->Add_Rect(left_rect, 0.0f, 0, color);

        // Right Bottom Box
        RectClass right_bottom_rect = { { mid_left, mid_top }, { right, bottom } };
        m_2DRender->Add_Rect(right_bottom_rect, 0.0f, 0, color);

        // At 13, clock points at corner of Right Top of box
        if (percentage < 13) {
            // Right Top Static Wedge
            {
                Vector2 pt1{ right, mid_top };
                Vector2 pt2{ right, top };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
            // Right Top Moving Wedge
            {
                float pt = right - static_cast<float>(13 - percentage) * (1.0f / 13.0f) * mid_x;
                Vector2 pt1{ right, top };
                Vector2 pt2{ pt, top };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
        } else {
            // Right Top Moving Wedge
            float pt = static_cast<float>(percentage - 13) * (1.0f / 12.0f) * mid_y + top;
            Vector2 pt1{ right, mid_top };
            Vector2 pt2{ right, pt };

            m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
        }
    } else if (percentage < 50) {
        // Left 2 Boxes
        RectClass left_rect = { { left, top }, { mid_left, bottom } };
        m_2DRender->Add_Rect(left_rect, 0.0f, 0, color);

        // At 38, clock points at corner of Right Bottom of box
        if (percentage < 38) {
            // Right Bottom Static Wedge
            {
                Vector2 pt1{ mid_left, bottom };
                Vector2 pt2{ right, bottom };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
            // Right Bottom Moving Wedge
            {
                float pt = static_cast<float>(percentage - 25) * (1.0f / 13.0f) * mid_y + mid_top;
                Vector2 pt1{ right, bottom };
                Vector2 pt2{ right, pt };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
        } else {
            // Right Bottom Moving Wedge
            float pt = right - static_cast<float>(percentage - 38) * (1.0f / 12.0f) * mid_x;
            Vector2 pt1{ mid_left, bottom };
            Vector2 pt2{ pt, bottom };

            m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
        }
    } else if (percentage < 75) {
        // Left Top Box
        RectClass left_top_rect = { { left, top }, { mid_left, mid_top } };
        m_2DRender->Add_Rect(left_top_rect, 0.0f, 0, color);

        // At 63, clock points at corner of Left Bottom of box
        if (percentage < 63) {
            // Left Bottom Static Wedge
            {
                Vector2 pt1{ left, mid_top };
                Vector2 pt2{ left, bottom };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
            // Left Bottom Moving Wedge
            {
                float pt = mid_left - static_cast<float>(percentage - 50) * (1.0f / 13.0f) * mid_x;
                Vector2 pt1{ left, bottom };
                Vector2 pt2{ pt, bottom };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
        } else {
            // Left Bottom Moving Wedge
            float pt = bottom - static_cast<float>(percentage - 62) * (1.0f / 12.0f) * mid_y;
            Vector2 pt1{ left, mid_top };
            Vector2 pt2{ left, pt };

            m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
        }
    } else {
        // At 87, clock points at corner of Left Top of box
        if (percentage < 87) {
            // Left Top Static Wedge
            {
                Vector2 pt1{ mid_left, top };
                Vector2 pt2{ left, top };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
            // Left Top Moving Wedge
            {
                float pt = mid_top - static_cast<float>(percentage - 75) * (1.0f / 13.0f) * mid_y;
                Vector2 pt1{ left, top };
                Vector2 pt2{ left, pt };

                m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
            }
        } else {
            // Left Top Moving Wedge
            float pt = left + static_cast<float>(percentage - 88) * (1.0f / 12.0f) * mid_x;
            Vector2 pt1{ mid_left, top };
            Vector2 pt2{ pt, top };

            m_2DRender->Add_Tri(pt0, pt1, pt2, uv0, uv1, uv2, color);
        }
    }
    m_2DRender->Render();
}

// 0x00740640
void W3DDisplay::Draw_Image(
    Image *image, int32_t left, int32_t top, int32_t right, int32_t bottom, uint32_t color, DrawImageMode mode)
{
    if (image == nullptr) {
        return;
    }

    m_2DRender->Reset();
    m_2DRender->Enable_Texturing(true);

    bool renable_alpha = false;

    switch (mode) {
        case Display::DRAWIMAGE_ADDITIVE_ALPHA:
            m_2DRender->Enable_Additive(false);
            m_2DRender->Enable_Alpha(false);
            renable_alpha = true;
            break;
        case Display::DRAWIMAGE_GREYSCALE:
            m_2DRender->Enable_GreyScale(true);
            break;
        case Display::DRAWIMAGE_ADDITIVE:
            break;
        default:
            m_2DRender->Enable_Additive(true);
            renable_alpha = true;
            break;
    }

    if (image->Is_Set_Status(Image::IMAGE_STATUS_RAW_TEXTURE)) {
        m_2DRender->Set_Texture(image->Get_Raw_Texture_Data());
    } else {
        auto file_name = image->Get_File_Name();
        if (file_name.Is_Empty()) {
            file_name = Utf8String::s_emptyString;
        }
        m_2DRender->Set_Texture(file_name);
    }

    float f_left = static_cast<float>(left);
    float f_top = static_cast<float>(top);
    float f_right = static_cast<float>(right);
    float f_bottom = static_cast<float>(bottom);
    RectClass rect{ f_left, f_top, f_right, f_bottom };
    auto uv = image->Get_UV_Region();
    RectClass UVrect{ uv.lo.x, uv.lo.y, uv.hi.x, uv.hi.y };
    if (m_isClippedEnabled == true) {
        if (right <= m_clipRegion.lo.x) {
            return;
        }
        if (bottom <= m_clipRegion.lo.y) {
            return;
        }

        f_left = static_cast<float>(std::max(left, m_clipRegion.lo.x));
        f_right = static_cast<float>(std::min(right, m_clipRegion.hi.x));
        f_top = static_cast<float>(std::max(top, m_clipRegion.lo.y));
        f_bottom = static_cast<float>(std::min(bottom, m_clipRegion.hi.y));

        float width = rect.Width();
        float height = rect.Height();

        float uv_width = UVrect.Width();
        float uv_height = UVrect.Height();

        float uv_left = UVrect.left;
        float uv_top = UVrect.top;

        if (image->Is_Set_Status(Image::IMAGE_STATUS_ROTATED_90_CLOCKWISE)) {
            UVrect.top = ((f_left - rect.left) / width) * uv_height + uv_top;
            UVrect.bottom = ((f_right - rect.left) / width) * uv_height + uv_top;
            UVrect.left = ((f_top - rect.top) / height) * uv_width + uv_left;
            UVrect.right = ((f_bottom - rect.top) / height) * uv_width + uv_left;
            rect.Set(f_left, f_top, f_right, f_bottom);
        } else {
            UVrect.left = ((f_left - rect.left) / width) * uv_width + uv_left;
            UVrect.right = ((f_right - rect.left) / width) * uv_width + uv_left;
            UVrect.top = ((f_top - rect.top) / height) * uv_height + uv_top;
            UVrect.bottom = ((f_bottom - rect.top) / height) * uv_height + uv_top;
            rect.Set(f_left, f_top, f_right, f_bottom);
        }
    }

    if (image->Is_Set_Status(Image::IMAGE_STATUS_ROTATED_90_CLOCKWISE)) {
        m_2DRender->Add_Tri(rect.Upper_Left(),
            rect.Lower_Left(),
            rect.Upper_Right(),
            UVrect.Upper_Right(),
            UVrect.Upper_Left(),
            UVrect.Lower_Right(),
            color);
        m_2DRender->Add_Tri(rect.Upper_Right(),
            rect.Lower_Right(),
            rect.Lower_Left(),
            UVrect.Lower_Left(),
            UVrect.Lower_Right(),
            UVrect.Upper_Left(),
            color);
    } else {
        m_2DRender->Add_Quad(rect, UVrect, color);
    }

    m_2DRender->Render();
    m_2DRender->Enable_GreyScale(false);
    if (renable_alpha == true) {
        m_2DRender->Enable_Alpha(true);
    }
}

// 0x00740BF0
void W3DDisplay::Draw_VideoBuffer(VideoBuffer *vb, int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    // m_2DRender->Reset();
    // m_2DRender->Enable_Texturing(true);
    // TODO: Requries W3DVideoBuffer
    // videoBuffer is actually a W3DVideoBuffer*
#ifdef GAME_DLL
    Call_Method<void, W3DDisplay, VideoBuffer *, int32_t, int32_t, int32_t, int32_t>(0x00740BF0, this, vb, x1, y1, x2, y2);
#endif
}

// 0x00740CD0
void W3DDisplay::Set_Shroud_Level(int unk1, int unk2, CellShroudStatus status)
{
    // TODO: Requires W3DShroud::Set_Shroud_Level
#ifdef GAME_DLL
    Call_Method<void, W3DDisplay, int, int, CellShroudStatus>(0x00740CD0, this, unk1, unk2, status);
#endif
}

// 0x00740CB0
void W3DDisplay::Set_Border_Shroud_Level(uint8_t level)
{
    // TODO: Requires W3DShroud::Set_Border_Shroud_Level
#ifdef GAME_DLL
    Call_Method<void, W3DDisplay, uint8_t>(0x00740CB0, this, level);
#endif
}

// 0x007411F0
void W3DDisplay::Preload_Model_Assets(Utf8String model)
{
    if (s_assetManager != nullptr) {
        Utf8String filename;
        filename.Format("%s.w3d", model.Str());
        s_assetManager->Load_3D_Assets(filename);
    }
}

// 0x007412D0
void W3DDisplay::Preload_Texture_Assets(Utf8String texture)
{
    if (s_assetManager != nullptr) {
        auto *tex = s_assetManager->Get_Texture(texture);
        tex->Release_Ref();
    }
}

// 0x00740D40
void W3DDisplay::Take_ScreenShot()
{
#ifdef GAME_DLL
    Call_Method<void, W3DDisplay>(0x00740D40, this);
#endif
}

// 0x007411DA
void W3DDisplay::Toggle_Movie_Capture()
{
    // TODO: Requires Toggle_Movie_Capture
#ifdef GAME_DLL
    Call_Method<void, W3DDisplay>(0x007411DA, this);
#endif
}

// 0x0073ED70
void W3DDisplay::Toggle_LetterBox()
{
    m_letterBoxEnabled = !m_letterBoxEnabled;
    m_letterBoxFadeStartTime = rts::Get_Time();
    auto *view = g_theTacticalView;
    if (view != nullptr) {
        view->Set_Zoom_Limited(!m_letterBoxEnabled);
    }
}

// 0x0073EDB0
void W3DDisplay::Enable_LetterBox(bool enable)
{
    m_letterBoxEnabled = enable;
    m_letterBoxFadeStartTime = rts::Get_Time();
    auto *view = g_theTacticalView;
    if (view != nullptr) {
        view->Set_Zoom_Limited(!enable);
    }
}

// 0x0073EC10
bool W3DDisplay::Is_LetterBox_Fading()
{
    if (m_letterBoxEnabled == true) {
        return m_letterBoxFadeLevel == 1.0f;
    }

    return m_letterBoxFadeLevel > 0.0f;
}

// 0x0073E350
int W3DDisplay::Get_Last_Frame_Draw_Calls()
{
#ifdef GAME_DLL
    return Call_Method<int, W3DDisplay>(0x0073E350, this);
#else
    return 0;
#endif
}

// 0x007751A0
W3DDisplay *Create_Game_Display()
{
    return new W3DDisplay;
}
