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
#include "camera.h"
#include "critsection.h"
#include "dx8wrapper.h"
#include "hanim.h"
#include "image.h"
#include "ingameui.h"
#include "main.h"
#include "rtsutils.h"
#include "scene.h"
#include "texture.h"
#include "w3dassetmanager.h"
#include "w3ddisplay.h"
#include "w3dscene.h"
#include <cmath>
#include <cstdio>
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
TextureBaseClass *W3DMouse::s_D3DMouseAssets[CURSOR_COUNT][MAX_FRAMES]; // TODO unsure on type
Image *W3DMouse::s_PolyMouseAssets[CURSOR_COUNT];
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
    m_D3DCursorSurfaceCount(0),
    m_unk4(0),
    m_unk5(0),
    m_unk7(0.001f),
    m_setWinCursor(false),
    m_camera(nullptr),
    m_currentW3DCursor(MouseCursor::CURSOR_NONE),
    m_currentPolyCursor(MouseCursor::CURSOR_NONE)
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

void Pixel_Screen_To_W3D_Logical_Screen(
    int screen_x, int screen_y, float *log_x, float *log_y, int screen_width, int screen_height)
{
    *log_x = ((float)screen_x + (float)screen_x) / (float)screen_width - 1.0f;
    *log_y = -(((float)screen_y + (float)screen_y) / (float)screen_height - 1.0f);
}

// 0x007AD940
void W3DMouse::Draw()
{
#ifdef BUILD_WITH_D3D8
    CriticalSectionClass::LockClass lock(g_mouseCriticalSection);
    m_setWinCursor = true;
    Set_Cursor(m_currentCursor);

    if (m_currentRedrawMode == RM_DX8 && m_currentD3DCursor != MouseCursor::CURSOR_NONE) {
        if (DX8Wrapper::Get_D3D_Device8() != nullptr) {
            DX8Wrapper::Get_D3D_Device8()->ShowCursor(true);

            if (g_theDisplay != nullptr && g_theDisplay->Get_Windowed() != 0) {
                POINT point;
                GetCursorPos(&point);
                ScreenToClient(g_applicationHWnd, &point);
                DX8Wrapper::Get_D3D_Device8()->SetCursorPosition(point.x, point.y, D3DCURSOR_IMMEDIATE_UPDATE);
            }

            if (m_D3DCursorSurfaceCount > 1) {
                unsigned int time = rts::Get_Time();
                m_unk4 = fmod((time - m_D3DCursorLastDrawn) * m_unk7 + m_unk4, m_D3DCursorSurfaceCount);
                m_D3DCursorLastDrawn = time;

                if (m_unk4 != m_unk5) {
                    m_unk5 = m_unk4;
                    DX8Wrapper::Get_D3D_Device8()->SetCursorProperties(
                        m_hotSpotX, m_hotSpotY, m_D3DCursorSurfaces[m_unk5]->Peek_D3D_Surface());
                }
            }
        }
    } else if (m_currentRedrawMode == RM_POLYGON) {
        Image *cursor = s_PolyMouseAssets[m_currentPolyCursor];

        if (cursor != nullptr) {
            g_theDisplay->Draw_Image(cursor,
                m_currMouse.pos.x - m_hotSpotX,
                m_currMouse.pos.y - m_hotSpotY,
                m_currMouse.pos.x - m_hotSpotX + cursor->Get_Image_Width(),
                m_currMouse.pos.y - m_hotSpotY + cursor->Get_Image_Height(),
                0xFFFFFFFF,
                Display::DRAWIMAGE_ADDITIVE);
        }
    } else if (m_currentRedrawMode == RM_W3D && W3DDisplay::s_3DInterfaceScene != nullptr && m_camera != nullptr && m_visible
        && s_W3DMouseAssets2[m_currentW3DCursor] != nullptr) {
        int i1 = g_theDisplay->Get_Width() - m_currMouse.pos.x;
        float f1 = i1;
        i1 = g_theDisplay->Get_Width();
        float f2 = 1.0f - f1 / i1;
        i1 = g_theDisplay->Get_Height() - m_currMouse.pos.y;
        f1 = i1;
        i1 = g_theDisplay->Get_Height();
        f1 = f1 / i1;
        float f3;
        float f4;

        if (!m_orthoCamera) {
            Pixel_Screen_To_W3D_Logical_Screen(
                m_currMouse.pos.x, m_currMouse.pos.y, &f1, &f2, g_theDisplay->Get_Width(), g_theDisplay->Get_Height());
            Vector3 pos = m_camera->Get_Position();
            Vector2 v;
            v.X = f1;
            v.Y = f2;
            Vector3 v2;
            m_camera->Un_Project(v2, v);
            v2 -= pos;
            v2.Normalize();
            float depth = m_camera->Get_Depth();
            v2.X = v2.X * depth;
            v2.Y = v2.Y * depth;
            v2.X = v2.X + pos.X;
            v2.Y = v2.Y + pos.Y;
            f2 = depth * v2.Z + pos.Z - pos.Z;
            f1 = -1.0f - pos.Z;
            f3 = (v2.X - pos.X) / f2 * f1 + pos.X;
            f4 = (v2.Y - pos.Y) / f2 * f1 + pos.Y;
        } else {
            f3 = f2 + f2 - 1.0f;
            f4 = f1 + f1;
        }

        Matrix3D tm;
        tm[1].W = f1;
        tm[0].W = f3;
        tm[0].X = 1.0f;
        tm[0].Y = 0.0f;
        tm[0].Z = 0.0f;
        tm[1].X = 0.0f;
        tm[1].Y = 1.0f;
        tm[1].Z = 0.0f;
        tm[2].X = 0.0f;
        tm[2].Y = 0.0f;
        tm[2].Z = 1.0f;
        tm[2].W = -1.0f;

        if (g_theInGameUI != nullptr && g_theInGameUI->Is_Scrolling()) {
            Coord2D scroll = g_theInGameUI->Get_Scroll_Amount();
            scroll.Normalize();
            tm.Rotate_Z(GameMath::Atan2(-scroll.y, scroll.x) - GAMEMATH_PI / 2);
        }

        s_W3DMouseAssets2[m_currentW3DCursor]->Set_Transform(tm);
        W3D::Render(W3DDisplay::s_3DInterfaceScene, m_camera);
    }

    if (!s_mouseThreadIsDrawing) {
        Draw_Cursor_Text();
    }

    if (m_visible && !s_mouseThreadIsDrawing) {
        Draw_Tooltip();
    }

    m_setWinCursor = false;
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
                Load_D3D_Cursor_Textures(cursor);
            }

            if (m_D3DCursorSurfaces[0] == nullptr) {
                break;
            }

            m_hotSpotX = m_cursorInfo[cursor].hot_spot.x;
            m_hotSpotY = m_cursorInfo[cursor].hot_spot.y;
            m_unk4 = 0;
            m_unk7 = m_cursorInfo[cursor].fps * 0.001f;
            device->SetCursorProperties(m_hotSpotX, m_hotSpotY, m_D3DCursorSurfaces[0]->Peek_D3D_Surface());
            device->ShowCursor(true);
            m_unk5 = static_cast<int32_t>(m_unk4);
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

            auto *scene = W3DDisplay::s_3DInterfaceScene;
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
    m_currentRedrawMode = mode;

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

            if (s_mouseThread.Is_Running()) {
                s_mouseThread.Execute();
            }

            m_currentW3DCursor = MouseCursor::CURSOR_NONE;
            m_currentPolyCursor = MouseCursor::CURSOR_NONE;
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
        for (auto j = 0; j < MAX_FRAMES; ++j) {
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
        for (auto j = 0; j < MAX_FRAMES; ++j) {
            s_D3DMouseAssets[i][j] = nullptr;
            m_D3DCursorSurfaces[j] = nullptr;
        }
    }
}

// 0x007AD580
void W3DMouse::Free_W3D_Assets()
{
    for (auto i = 0; i < CURSOR_COUNT; ++i) {
        auto *interface_scene = W3DDisplay::s_3DInterfaceScene;
        auto &hanim = s_W3DMouseAssets1[i];
        auto &robj = s_W3DMouseAssets2[i];

        if (interface_scene != nullptr && robj != nullptr) {
            interface_scene->Remove_Render_Object(robj);
        }

        if (robj != nullptr) {
            robj->Release_Ref();
            robj = nullptr;
        }

        if (hanim != nullptr) {
            hanim->Release_Ref();
            hanim = nullptr;
        }
    }

    if (m_camera != nullptr) {
        m_camera->Release_Ref();
        m_camera = nullptr;
    }
}

// 0x007AD330
void W3DMouse::Init_W3D_Assets()
{
    CriticalSectionClass::LockClass lock(g_mouseCriticalSection);
    if (s_mouseThreadIsDrawing) {
        return;
    }

    auto *asset_manager = W3DDisplay::s_assetManager;
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

            asset = asset_manager->Create_Render_Obj(cursor_info.w3d_model_name.Str(), scale, 0, nullptr, nullptr);
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

            asset = asset_manager->Get_HAnim(cursor_info.w3d_anim_name.Str());
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

    for (auto i = 0; i < MAX_FRAMES; ++i) {
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
bool W3DMouse::Load_D3D_Cursor_Textures(MouseCursor cursor)
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
    auto frames = std::min<int32_t>(m_cursorInfo[cursor].frames, MAX_FRAMES);

    if (frames == 1) {
        char file_name[64]{};
        snprintf(file_name, ARRAY_SIZE(file_name), "%s.tga", texture_name);
        auto *texture = asset_manager->Get_Texture(file_name);

        s_D3DMouseAssets[cursor][0] = texture;
        m_D3DCursorSurfaces[0] = texture->Get_Surface_Level(0);
        m_D3DCursorSurfaceCount = 1; // Note: This just assumes it loaded fine
        return true;
    }

    for (auto i = 0; i < frames; ++i) {
        char file_name[64]{};
        snprintf(file_name, ARRAY_SIZE(file_name), "%s%04d.tga", texture_name, i);
        auto *texture = asset_manager->Get_Texture(file_name);

        s_D3DMouseAssets[cursor][i] = texture;
        if (texture != nullptr) {
            m_D3DCursorSurfaces[m_D3DCursorSurfaceCount++] = texture->Get_Surface_Level(0);
        }
    }
    return true;
}

// 0x007ACFB0
void W3DMouse::Init_Polygon_Assets()
{
    CriticalSectionClass::LockClass lock(g_mouseCriticalSection);

    if (!s_mouseThreadIsDrawing && m_currentRedrawMode == RM_POLYGON && s_W3DMouseAssets2[CURSOR_NORMAL] == nullptr) {
        for (int i = 1; i < CURSOR_COUNT; ++i) {
            m_currentPolyCursor = m_currentCursor;

            if (m_cursorInfo[i].image_name.Is_Not_Empty()) {
                s_PolyMouseAssets[i] = g_theMappedImageCollection->Find_Image_By_Name(m_cursorInfo[i].image_name);
            }
        }
    }
}

// 0x007AE0D0
void W3DMouse::Set_Cursor_Direction(MouseCursor cursor)
{
    if (m_cursorInfo[cursor].directions > 1 && g_theInGameUI != nullptr && g_theInGameUI->Is_Scrolling()) {
        Coord2D scroll = g_theInGameUI->Get_Scroll_Amount();

        if (scroll.x == 0 && scroll.y == 0) {
            m_cursorDirection = 0;
        } else {
            scroll.Normalize();
            uint32_t directions = m_cursorInfo[m_currentCursor].directions;
            m_cursorDirection =
                (fmod(GameMath::Atan2(scroll.y, scroll.x) + GAMEMATH_PI2, GAMEMATH_PI2) / (GAMEMATH_PI2 / directions)
                    + 0.5f);

            if (m_cursorDirection >= directions) {
                m_cursorDirection = 0;
            }
        }
    }
}
