/**
 * @file
 *
 * @author DuncansPumpkin
 *
 * @brief W3D Mouse Input interface.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dmouse.h"
#include "assetmgr.h"
#include "camera.h"
#include "critsection.h"
#include "dx8wrapper.h"
#include "hanim.h"
#include "rtsutils.h"
#include "scene.h"
#include "texture.h"
#include "w3ddisplay.h"
#ifdef PLATFORM_WINDOWS
#include <winuser.h>
#endif

#ifdef GAME_DLL
extern CriticalSectionClass &g_mouseCriticalSection;
#else
MouseThreadClass W3DMouse::s_mouseThread("Thyme Mouse thread");
bool W3DMouse::s_mouseThreadIsDrawing;
HAnimClass *W3DMouse::s_W3DMouseAssets1[CURSOR_COUNT];
RenderObjClass *W3DMouse::s_W3DMouseAssets2[CURSOR_COUNT];
TextureBaseClass *W3DMouse::s_D3DMouseAssets[CURSOR_COUNT][21]; // TODO unsure on type
uint32_t W3DMouse::s_PolyMouseAssets[CURSOR_COUNT];
CriticalSectionClass g_mouseCriticalSection;
#endif

void MouseThreadClass::Thread_Function()
{
    while (Is_Running()) {
        W3DMouse::Set_Is_Drawing(true);
        auto *mouse = g_theMouse;
        if (mouse != nullptr) {
            mouse->Draw();
        }
        W3DMouse::Set_Is_Drawing(false);
        Switch_Thread();
    }
}

W3DMouse::W3DMouse() :
    m_currentD3DCursor(MouseCursor::CURSOR_NONE),
    m_currentW3DCursor(MouseCursor::CURSOR_NONE),
    m_currentPolyCursor(MouseCursor::CURSOR_NONE),
    unk4(0),
    unk5(0),
    m_D3DCursorSurfaceCount(0),
    m_camera(nullptr),
    m_setWinCursor(false),
    unk7(0.001f)
{
    memset(s_W3DMouseAssets1, 0, sizeof(s_W3DMouseAssets1));
    memset(s_W3DMouseAssets2, 0, sizeof(s_W3DMouseAssets2));
    memset(s_D3DMouseAssets, 0, sizeof(s_D3DMouseAssets));
}

// 0x007ACF00
W3DMouse::~W3DMouse()
{
#ifdef BUILD_WITH_D3D8
    auto device = DX8Wrapper::Get_D3D_Device8();
    if (device != nullptr) {
        device->ShowCursor(false);
        Win32Mouse::Set_Cursor(MouseCursor::CURSOR_ARROW);
    }
#endif
    Free_D3D_Assets();
    Free_W3D_Assets();

    s_mouseThread.Stop(3000);
}

// 0x007AD600
void W3DMouse::Init()
{
    Win32Mouse::Init();
    Set_Cursor(MouseCursor::CURSOR_ARROW);
    s_mouseThreadIsDrawing = false;
    if (m_currentRedrawMode == Mouse::RedrawMode::RM_DX8) {
        s_mouseThread.Execute();
    }
    s_mouseThread.Set_Priority(0);
}

// 0x007AD940
void W3DMouse::Draw()
{
    // TODO: Requires theDisplay, InGameUI
#ifdef GAME_DLL
    Call_Method<void, W3DMouse>(0x007AD940, this);
#endif
}

// 0x007AD650
void W3DMouse::Set_Cursor(MouseCursor cursor)
{

    CriticalSectionClass::LockClass lock(g_mouseCriticalSection);

    m_cursorDirection = 0;
    if (m_currentRedrawMode == RedrawMode::RM_WINDOWS) {
        m_currentD3DCursor = MouseCursor::CURSOR_NONE;
        m_currentW3DCursor = MouseCursor::CURSOR_NONE;
        m_currentPolyCursor = MouseCursor::CURSOR_NONE;
        Set_Cursor_Direction(cursor);
        if (m_setWinCursor == true) {
            Win32Mouse::Set_Cursor(cursor);
        }
        m_currentCursor = cursor;
        return;
    }

    Set_Mouse_Text(cursor);
    if (m_currentCursor == cursor && m_currentD3DCursor == cursor) {
        return;
    }

    switch (m_currentRedrawMode) {
        case RedrawMode::RM_DX8: {
#ifdef BUILD_WITH_D3D8
#ifdef PLATFORM_WINDOWS
            SetCursor(NULL); // Make Win32Cursor Invisible
#endif
            auto *device = DX8Wrapper::Get_D3D_Device8();
            if (device == nullptr) {
                break;
            }

            device->ShowCursor(false);

            if (m_currentD3DCursor != cursor && s_mouseThreadIsDrawing == false) {
                Release_D3D_Cursor_Texture(m_currentD3DCursor);
                Load_D3D_Cursor_Texture(cursor);
            }

            if (m_D3DCursorSurfaces[0] == nullptr) {
                break;
            }

            m_hotSpotX = m_cursorInfo[cursor].hot_spot.x;
            m_hotSpotY = m_cursorInfo[cursor].hot_spot.y;
            unk4 = 0;
            unk7 = m_cursorInfo[cursor].fps * 0.001f;
            device->SetCursorProperties(m_hotSpotX, m_hotSpotY, m_D3DCursorSurfaces[0]->Get_D3D_Surface());
            device->ShowCursor(true);
            unk5 = static_cast<int32_t>(unk4);
            m_currentD3DCursor = cursor;
            m_D3DCursorLastDrawn = rts::Get_Time();
#endif
            break;
        }

        case RedrawMode::RM_POLYGON: {
#ifdef PLATFORM_WINDOWS
            SetCursor(NULL); // Make Win32Cursor Invisible
#endif
            m_currentD3DCursor = MouseCursor::CURSOR_NONE;
            m_currentW3DCursor = MouseCursor::CURSOR_NONE;
            m_currentPolyCursor = cursor;
            m_hotSpotX = m_cursorInfo[cursor].hot_spot.x;
            m_hotSpotY = m_cursorInfo[cursor].hot_spot.y;
            break;
        }
        case RedrawMode::RM_W3D: {
#ifdef PLATFORM_WINDOWS
            SetCursor(NULL); // Make Win32Cursor Invisible
#endif
            m_currentD3DCursor = MouseCursor::CURSOR_NONE;
            m_currentPolyCursor = MouseCursor::CURSOR_NONE;
            if (m_currentW3DCursor == cursor) {
                break;
            }

            if (s_W3DMouseAssets2[MouseCursor::CURSOR_NORMAL] == nullptr) {
                Init_W3D_Assets();
                if (s_W3DMouseAssets2[MouseCursor::CURSOR_NORMAL] == nullptr) {
                    break;
                }
            }

            auto *current_w3d_asset = s_W3DMouseAssets2[m_currentW3DCursor];

            auto *scene = W3DDisplay::Get_3DInterfaceScene();
            if (current_w3d_asset != nullptr) {
                scene->Remove_Render_Object(current_w3d_asset);
            }

            m_currentW3DCursor = cursor;
            auto *new_w3d_asset = s_W3DMouseAssets2[cursor];
            if (new_w3d_asset == nullptr) {
                break;
            }

            scene->Add_Render_Object(new_w3d_asset);
            if (m_cursorInfo[cursor].loop == false) {
                break;
            }

            if (s_W3DMouseAssets1[cursor] == nullptr) {
                break;
            }

            s_W3DMouseAssets2[cursor]->Set_Animation(s_W3DMouseAssets1[cursor], 0.0f, RenderObjClass::ANIM_MODE_ONCE);
            break;
        }
        default:
            break;
    }
    m_currentCursor = cursor;
}

// 0x007ADF60
void W3DMouse::Set_Redraw_Mode(RedrawMode mode)
{
    const auto current_cursor = m_currentCursor;
    Set_Cursor(MouseCursor::CURSOR_NONE);

    switch (mode) {
        case RedrawMode::RM_WINDOWS:
            if (s_mouseThread.Is_Running()) {
                s_mouseThread.Stop(3000);
            }
            Free_D3D_Assets();
            Free_W3D_Assets();
            memset(s_PolyMouseAssets, 0, sizeof(s_PolyMouseAssets));
            m_currentD3DCursor = MouseCursor::CURSOR_NONE;
            m_currentW3DCursor = MouseCursor::CURSOR_NONE;
            m_currentPolyCursor = MouseCursor::CURSOR_NONE;
            break;
        case RedrawMode::RM_W3D:
            if (s_mouseThread.Is_Running()) {
                s_mouseThread.Stop(3000);
            }
            Free_D3D_Assets();
            memset(s_PolyMouseAssets, 0, sizeof(s_PolyMouseAssets));
            m_currentD3DCursor = MouseCursor::CURSOR_NONE;
            m_currentPolyCursor = MouseCursor::CURSOR_NONE;
            Init_W3D_Assets();
            break;
        case RedrawMode::RM_POLYGON:
            if (s_mouseThread.Is_Running()) {
                s_mouseThread.Stop(3000);
            }
            Free_D3D_Assets();
            Free_W3D_Assets();
            m_currentD3DCursor = MouseCursor::CURSOR_NONE;
            m_currentW3DCursor = MouseCursor::CURSOR_NONE;
            m_currentPolyCursor = MouseCursor::CURSOR_NONE;
            Init_Polygon_Assets();
            break;
        case RedrawMode::RM_DX8:
            Init_D3D_Assets();
            Free_W3D_Assets();
            memset(s_PolyMouseAssets, 0, sizeof(s_PolyMouseAssets));
            m_currentW3DCursor = MouseCursor::CURSOR_NONE;
            m_currentPolyCursor = MouseCursor::CURSOR_NONE;
            if (s_mouseThread.Is_Running()) {
                s_mouseThread.Execute();
            }
            break;
        default:
            break;
    }

    Set_Cursor(MouseCursor::CURSOR_NONE);
    Set_Cursor(current_cursor);
}

// 0x007AD2D0
void W3DMouse::Free_D3D_Assets()
{
    for (auto &surface : m_D3DCursorSurfaces) {
        if (surface == nullptr) {
            continue;
        }

        surface->Release_Ref();
        surface = nullptr;
    }

    for (auto i = 0; i < CURSOR_COUNT; ++i) {
        for (auto j = 0; j < 21; ++j) {
            auto *d3d_surface = s_D3DMouseAssets[i][j];
            if (d3d_surface == nullptr) {
                continue;
            }
            d3d_surface->Release_Ref();
            s_D3DMouseAssets[i][j] = nullptr;
        }
    }
}

// 0x007AD230
void W3DMouse::Init_D3D_Assets()
{
    CriticalSectionClass::LockClass lock(g_mouseCriticalSection);
    if (s_mouseThreadIsDrawing) {
        return;
    }

    if (m_currentRedrawMode != RedrawMode::RM_DX8) {
        return;
    }

    if (s_D3DMouseAssets[CURSOR_TARGET][0] != nullptr) {
        return;
    }

    auto *asset_manager = W3DAssetManager::Get_Instance();
    if (asset_manager == nullptr) {
        return;
    }

    for (auto i = 0; i < CURSOR_COUNT; ++i) {
        for (auto j = 0; j < 21; ++j) {
            s_D3DMouseAssets[i][j] = nullptr;
            m_D3DCursorSurfaces[j] = nullptr;
        }
    }
}

// 0x007AD580
void W3DMouse::Free_W3D_Assets()
{
    for (auto i = 0; i < CURSOR_COUNT; ++i) {
        auto *interface_scene = W3DDisplay::Get_3DInterfaceScene();
        auto &hanim = s_W3DMouseAssets1[i];
        auto &robj = s_W3DMouseAssets2[i];
        if (interface_scene != nullptr) {
            interface_scene->Remove_Render_Object(robj);
        }
        if (robj != nullptr) {

            robj->Release_Ref();
        }
        robj = nullptr;
        if (hanim != nullptr) {
            hanim->Release_Ref();
        }
        hanim = nullptr;
    }

    if (m_camera != nullptr) {
        m_camera->Release_Ref();
    }
    m_camera = nullptr;
}

// 0x007AD330
void W3DMouse::Init_W3D_Assets()
{
    CriticalSectionClass::LockClass lock(g_mouseCriticalSection);
    if (s_mouseThreadIsDrawing) {
        return;
    }

    auto *asset_manager = W3DDisplay::Get_AssetManager();
    if (s_W3DMouseAssets2[CURSOR_NORMAL] == nullptr && asset_manager != nullptr) {
        for (auto i = 1; i < CURSOR_COUNT; ++i) {
            const auto &cursor_info = m_cursorInfo[i];
            auto &asset = s_W3DMouseAssets2[i];
            if (cursor_info.w3d_model_name.Is_Empty()) {
                continue;
            }

            float scale = cursor_info.w3d_scale;
            if (m_orthoCamera) {
                scale = cursor_info.w3d_scale * m_orthoZoom;
            }

            asset = asset_manager->Create_Render_Obj(cursor_info.w3d_model_name, scale, 0, nullptr, nullptr);
            if (asset != nullptr) {
                asset->Set_Position({ 0.0f, 0.0f, -1.0f });
            }
        }
    }
    if (s_W3DMouseAssets1[CURSOR_NORMAL] == nullptr && asset_manager != nullptr) {
        for (auto i = 1; i < CURSOR_COUNT; ++i) {
            const auto &cursor_info = m_cursorInfo[i];
            auto &asset = s_W3DMouseAssets1[i];
            auto &asset_2 = s_W3DMouseAssets2[i];
            if (cursor_info.w3d_anim_name.Is_Empty()) {
                continue;
            }

            asset = asset_manager->Get_HAnim(cursor_info.w3d_anim_name);
            if (asset != nullptr && asset_2 != nullptr) {
                asset_2->Set_Animation(
                    asset, 0.0f, cursor_info.loop ? RenderObjClass::ANIM_MODE_LOOP : RenderObjClass::ANIM_MODE_ONCE);
            }
        }
    }

    m_camera = new CameraClass;
    m_camera->Set_Position({ 0.0f, 1.0f, 1.0f });
    m_camera->Set_View_Plane({ -1.0f, -1.0f }, { 1.0f, 1.0f });
    m_camera->Set_Clip_Planes(0.995f, 20.0f);
    if (m_orthoCamera) {
        m_camera->Set_Projection_Type(CameraClass::ProjectionType::ORTHO);
    }
}

// 0x007AD060
void W3DMouse::Release_D3D_Cursor_Texture(MouseCursor cursor)
{
    if (cursor == MouseCursor::CURSOR_NONE) {
        return;
    }

    for (auto i = 0; i < 21; ++i) {
        auto &surface = m_D3DCursorSurfaces[i];
        if (surface != nullptr) {
            surface->Release_Ref();
            surface = nullptr;
        }
        auto &d3d_surface = s_D3DMouseAssets[cursor][i];
        if (d3d_surface != nullptr) {
            d3d_surface->Release_Ref();
            d3d_surface = nullptr;
        }
    }
}

// 0x007AD0E0
bool W3DMouse::Load_D3D_Cursor_Texture(MouseCursor cursor)
{
    if (cursor == MouseCursor::CURSOR_NONE) {
        return true;
    }

    if (s_D3DMouseAssets[cursor][0] != nullptr) {
        return true;
    }

    if (m_cursorInfo[cursor].frames == 0) {
        return false;
    }
    auto *asset_manager = W3DAssetManager::Get_Instance();

    auto *texture_name = m_cursorInfo[cursor].texture_name.Str();
    auto frames = std::min(m_cursorInfo[cursor].frames, 21);
    m_D3DCursorSurfaceCount = frames;

    if (frames == 1) {
        char file_name[64]{};
        sprintf_s(file_name, "%s.tga", texture_name);
        auto *texture = asset_manager->Get_Texture(file_name);
        s_D3DMouseAssets[cursor][0] = texture;
        m_D3DCursorSurfaces[0] = texture->Get_Surface_Level(0);
        return true;
    }

    for (auto i = 0; i < frames; ++i) {
        auto &asset = s_D3DMouseAssets[cursor][i];
        char file_name[64]{};
        sprintf_s(file_name, "%s%04d.tga", texture_name, i);
        auto *texture = asset_manager->Get_Texture(file_name);
        asset = texture;
        m_D3DCursorSurfaces[i] = texture->Get_Surface_Level(0);
    }
    return true;
}

// 0x007ACFB0
void W3DMouse::Init_Polygon_Assets()
{
    // TODO: Requires ImageCollection
#ifdef GAME_DLL
    Call_Method<void, W3DMouse>(0x007ACFB0, this);
#endif
}

// 0x007AE0D0
void W3DMouse::Set_Cursor_Direction(MouseCursor cursor)
{
    // TODO: Requires InGameUI
#ifdef GAME_DLL
    Call_Method<void, W3DMouse, MouseCursor>(0x007AE0D0, this, cursor);
#endif
}

// TODO: Member of W3DGameClient move when implemented
Win32Mouse *Create_Mouse()
{
    Win32Mouse *mouse = new W3DMouse();
    g_theWin32Mouse = mouse;
    return mouse;
}
