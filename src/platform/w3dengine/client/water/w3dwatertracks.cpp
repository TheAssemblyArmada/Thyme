/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Water Tracks system
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dwatertracks.h"
#include "assetmgr.h"
#include "baseheightmap.h"
#include "camera.h"
#include "display.h"
#include "dx8indexbuffer.h"
#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "filesystem.h"
#include "globaldata.h"
#include "ingameui.h"
#include "main.h"
#include "rinfo.h"
#include "rtsutils.h"
#include "shadermanager.h"
#include "terrainlogic.h"
#include "vertmaterial.h"
#include "view.h"
#include "w3dshroud.h"
#include "water.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif
#include <stdio.h>

#ifndef GAME_DLL
WaterTracksRenderSystem *g_theWaterTracksRenderSystem;
#endif

static bool s_pauseWaves;
static WaveInfo s_waveTypeInfo[6] = { { 28.0f, 18.0f, 25.0f, 0.018f, 900, 0.01f, 0.18f, 1500, 0, "wave256.tga", "Pond" },
    { 55.0f, 36.0f, 80.0f, 0.015f, 2000, 0.5f, 0.18f, 1000, 6267, "wave256.tga", "Ocean" },
    { 55.0f, 36.0f, 80.0f, 0.015f, 2000, 0.05f, 0.18f, 1000, 6267, "wave256.tga", "Close Ocean" },
    { 55.0f, 36.0f, 80.0f, 0.015f, 4000, 0.01f, 0.18f, 2000, 6267, "wave256.tga", "Close Ocean Double" },
    { 55.0f, 27.0f, 80.0f, 0.015f, 2000, 0.01f, 8.0f, 2000, 5367, "wave256.tga", "Radial" },
    { 0.0f, 0.0f, 0.0f, 0.0f, 0, 0.0f, 0.0f, 0, 0, nullptr, nullptr } };

WaterTracksObj::WaterTracksObj() : m_stageZeroTexture(nullptr), m_bound(false), m_timeOffsetSecondWave(0)
{
    // #BUGFIX Initialize important members
    m_nextSystem = nullptr;
    m_prevSystem = nullptr;
}

WaterTracksObj ::~WaterTracksObj()
{
    Free_Water_Tracks_Resources();
}

void WaterTracksObj::Render() {}

void WaterTracksObj::Get_Obj_Space_Bounding_Sphere(SphereClass &sphere) const
{
    sphere = m_boundingSphere;
}

void WaterTracksObj::Get_Obj_Space_Bounding_Box(AABoxClass &box) const
{
    box = m_boundingBox;
}

int WaterTracksObj::Free_Water_Tracks_Resources()
{
    Ref_Ptr_Release(m_stageZeroTexture);
    return 0;
}

void WaterTracksObj::Init(float width, Vector2 const &start, Vector2 const &end, const char *texture)
{
    Free_Water_Tracks_Resources();
    m_boundingSphere.Init(Vector3(0.0f, 0.0f, 0.0f), 400.0f);
    m_boundingBox.Init(Vector3(0.0f, 0.0f, 0.0f), Vector3(400.0f, 400.0f, 1.0f));
    m_perpDir = end - start;
    m_startPos = start + m_perpDir * 0.5f;
    m_perpDir *= 1.0f / m_perpDir.Length();
    m_waveDir = m_perpDir;
    m_waveDir.Rotate(DEG_TO_RADF(90.f));
    m_startPos -= m_waveDir * width;
    m_waveDir *= 13.0f;
    m_startPos -= m_waveDir;

    m_x = 2;
    m_y = 2;
    m_elapsedMs = 0;
    m_velocity = 0.01f;
    m_totalMs = m_waveDir.Length() / m_velocity;
    m_fadeMs = 3000;
    m_stageZeroTexture = W3DAssetManager::Get_Instance()->Get_Texture(texture);
}

void WaterTracksObj::Init(
    float width, float length, Vector2 const &start, Vector2 const &end, const char *texture, int time_offset)
{
    Free_Water_Tracks_Resources();
    m_initialStart = start;
    m_initialEnd = end;
    m_timeOffsetSecondWave = time_offset;
    m_boundingSphere.Init(Vector3(0.0f, 0.0f, 0.0f), 400.0f);
    m_boundingBox.Init(Vector3(0.0f, 0.0f, 0.0f), Vector3(400.0f, 400.0f, 1.0f));
    m_x = 2;
    m_y = 2;

    m_elapsedMs = m_timeOffsetSecondWave;
    m_startPos = start;
    m_waveDir = end - start;
    m_perpDir = m_waveDir;
    m_perpDir.Rotate(DEG_TO_RADF(-90.f));
    m_perpDir.Normalize();
    m_waveDir = m_perpDir;
    m_waveDir.Rotate(DEG_TO_RADF(90.f));

    m_distanceFromShore = s_waveTypeInfo[m_type].distance_from_shore;
    m_waveDir *= m_distanceFromShore;
    m_startPos -= m_waveDir;
    m_velocity = s_waveTypeInfo[m_type].initial_velocity;
    m_totalMs = (int)(m_distanceFromShore / m_velocity);
    m_fadeMs = s_waveTypeInfo[m_type].time_to_fade;
    m_widthFraction = length * s_waveTypeInfo[m_type].initial_width_fraction;
    m_heightFraction = m_widthFraction * s_waveTypeInfo[m_type].initial_height_fraction;
    m_length = length;
    m_width = width;

    m_timeUntilBreak = (m_distanceFromShore - m_width) / m_velocity;
    m_velocityUnk = -(m_velocity * m_velocity) / (2.0f * m_width);
    m_timingUnk2 = -m_velocity / m_velocityUnk;
    m_timingUnk3 = GameMath::Sqrt(GameMath::Fabs(2.0f * m_width / m_velocityUnk));
    m_totalMs = (int)(m_timeUntilBreak + m_timingUnk2 + m_timingUnk3);
    m_scaleUnk = 2.0f * m_heightFraction / (m_timingUnk2 * m_timingUnk2);
    m_timeToCompress = s_waveTypeInfo[m_type].time_to_compress;

    if (m_type == WAVE_TYPE_UNK) {
        m_timingUnk3 = 1000.0f;
        m_totalMs = (int)(m_timeUntilBreak + m_timingUnk2 + (float)m_fadeMs + m_timingUnk3);
        m_startPos = start;
        m_fadeMs = 1000;
    }

    m_stageZeroTexture = W3DAssetManager::Get_Instance()->Get_Texture(texture);
}

int WaterTracksObj::Render(DX8VertexBufferClass *vertex_buffer, int batch_start)
{
#ifdef BUILD_WITH_D3D8
    Vector2 v1;
    Vector2 v2;
    float oo_wave_dir_len = 1.0f / m_waveDir.Length();
    VertexFormatXYZDUV1 *verts;

    if (batch_start < 4000 - m_y * m_x) {
        if (FAILED(vertex_buffer->Get_DX8_Vertex_Buffer()->Lock(vertex_buffer->FVF_Info().Get_FVF_Size() * batch_start,
                vertex_buffer->FVF_Info().Get_FVF_Size() * m_y * m_x,
                reinterpret_cast<BYTE **>(&verts),
                D3DLOCK_NOOVERWRITE))) {
            return batch_start;
        }
    } else {
        if (FAILED(vertex_buffer->Get_DX8_Vertex_Buffer()->Lock(0,
                vertex_buffer->FVF_Info().Get_FVF_Size() * m_y * m_x,
                reinterpret_cast<BYTE **>(&verts),
                D3DLOCK_DISCARD))) {
            return batch_start;
        }

        batch_start = 0;
    }

    float alpha;
    float f3 = 1.0f;

    if (m_type == WAVE_TYPE_UNK) {
        v2 = m_startPos;
        v2 -= (m_perpDir * m_length) * 0.5f;
        v1 = v2 - ((oo_wave_dir_len * m_width) * m_waveDir);
        alpha = 0.0f;

        if (m_elapsedMs >= m_totalMs) {
            m_elapsedMs = 0;
        }

        if (m_timeUntilBreak + m_timingUnk2 - 1000.0f + (float)m_fadeMs < (float)m_elapsedMs) {
            alpha = (float)m_elapsedMs - (m_timeUntilBreak + m_timingUnk2 - 1000.0f + (float)m_fadeMs);
            alpha = alpha / m_timingUnk3;
            alpha = 1.0f - alpha;

            if (alpha < 0.0f) {
                alpha = 0.0f;
            }
        } else if (m_timeUntilBreak + m_timingUnk2 - 1000.0f < (float)m_elapsedMs) {
            alpha = (float)m_elapsedMs - (m_timeUntilBreak + m_timingUnk2 - 1000.0f);
            alpha = alpha / (float)m_fadeMs;

            if (alpha > 1.0f) {
                alpha = 1.0f;
            }
        }
    } else if ((float)m_elapsedMs < m_timeUntilBreak) {
        alpha = (float)m_elapsedMs / m_timeUntilBreak;
        f3 = alpha;
        f3 = ((m_length - m_widthFraction) * f3 + m_widthFraction) / m_length;
        v2 = m_startPos + (((float)m_elapsedMs * m_velocity * oo_wave_dir_len) * m_waveDir);
        v2 -= ((m_perpDir * m_length) * 0.5f) * f3;
        v1 = v2 - ((oo_wave_dir_len * m_heightFraction) * m_waveDir);
    } else if (m_elapsedMs < m_totalMs) {
        alpha = 1.0f;
        f3 = 1.0f;
        v2 = m_startPos + ((m_velocity * m_timeUntilBreak * oo_wave_dir_len) * m_waveDir);
        v1 = v2;
        float f4 = (float)m_elapsedMs - m_timeUntilBreak;
        v2 += ((f4 * m_velocity + 0.5f * m_velocityUnk * f4 * f4) * oo_wave_dir_len) * m_waveDir;
        v2 -= ((m_perpDir * m_length) * 0.5f) * f3;
        float f5 = (float)m_elapsedMs - m_timeUntilBreak - m_timingUnk2;

        if (f5 < 0.0f) {
            f5 = 0.0f;
        }

        alpha = f5 / (float)m_fadeMs;

        if (alpha > 1.0f) {
            alpha = 1.0f;
        }

        alpha = 1.0f - alpha;
        v1 -= (oo_wave_dir_len * m_heightFraction) * m_waveDir;

        if (m_timeUntilBreak + m_timingUnk2 + m_timeToCompress < (float)m_elapsedMs) {
            v1 +=
                (0.5f * m_scaleUnk * (m_timingUnk2 + m_timeToCompress) * (m_timingUnk2 + m_timeToCompress) * oo_wave_dir_len)
                * m_waveDir;
            float f6 = (float)m_elapsedMs - (m_timeUntilBreak + m_timingUnk2 + m_timeToCompress);
            v1 += (0.5f * m_velocityUnk * f6 * f6 * oo_wave_dir_len) * m_waveDir;
        } else {
            v1 += (0.5f * m_scaleUnk * f4 * f4 * oo_wave_dir_len) * m_waveDir;
        }

        v1 -= ((m_perpDir * m_length) * 0.5f) * f3;
    } else {
        m_elapsedMs = 0;
        alpha = (float)m_elapsedMs / m_timeUntilBreak;
        f3 = alpha;
        f3 = ((m_length - m_widthFraction) * f3 + m_widthFraction) / m_length;
        v2 = m_startPos + (((float)m_elapsedMs * m_velocity * oo_wave_dir_len) * m_waveDir);
        v2 -= ((m_perpDir * m_length) * 0.5f) * f3;
        v1 = v2 - ((oo_wave_dir_len * m_heightFraction) * m_waveDir);
    }

    Vector2 vertex = v1;
    float waterz;
    g_theTerrainLogic->Is_Underwater(vertex.X, vertex.Y, &waterz, nullptr);
    verts->x = vertex.X;
    verts->y = vertex.Y;
    verts->z = waterz + 1.5f;
    verts->diffuse = (GameMath::Fast_To_Int_Truncate(alpha * 255.0f) << 24) | 0xFFFFFF;

    if (m_flipUV == 0.0f) {
        verts->u1 = 0.0f;
    } else {
        verts->u1 = 1.0f;
    }

    verts->v1 = 0.0f;
    verts++;

    vertex.Set(v1 + m_perpDir * m_length * f3);
    verts->x = vertex.X;
    verts->y = vertex.Y;
    verts->z = waterz + 1.5f;
    verts->diffuse = (GameMath::Fast_To_Int_Truncate(alpha * 255.0f) << 24) | 0xFFFFFF;

    if (m_flipUV == 0.0f) {
        verts->u1 = 1.0f;
    } else {
        verts->u1 = 0.0f;
    }

    verts->v1 = 0.0f;
    verts++;

    vertex.Set(v2);
    verts->x = vertex.X;
    verts->y = vertex.Y;
    verts->z = waterz + 1.5f;
    verts->diffuse = (GameMath::Fast_To_Int_Truncate(alpha * 255.0f) << 24) | 0xFFFFFF;

    if (m_flipUV == 0.0f) {
        verts->u1 = 0.0f;
    } else {
        verts->u1 = 1.0f;
    }

    verts->v1 = 1.0f;
    verts++;

    vertex.Set(v2 + m_perpDir * m_length * f3);
    verts->x = vertex.X;
    verts->y = vertex.Y;
    verts->z = waterz + 1.5f;
    verts->diffuse = (GameMath::Fast_To_Int_Truncate(alpha * 255.0f) << 24) | 0xFFFFFF;

    if (m_flipUV == 0.0f) {
        verts->u1 = 1.0f;
    } else {
        verts->u1 = 0.0f;
    }

    verts->v1 = 1.0f;
    verts++;

    vertex_buffer->Get_DX8_Vertex_Buffer()->Unlock();
    float f7 = (2 * m_x + 2) * (m_y - 1) - 2;
    DX8Wrapper::Set_Index_Buffer(g_theWaterTracksRenderSystem->m_indexBuffer, batch_start);
    DX8Wrapper::Draw_Strip(0, f7 - 2, 0, (unsigned short)m_y * (unsigned short)m_x);
#endif
    return m_y * m_x + batch_start;
}

int WaterTracksObj::Update(int ms_elapsed)
{
    m_elapsedMs += ms_elapsed;
    return 1;
}

void Test_Water_Update()
{
#ifdef BUILD_WITH_D3D8
    static bool pause_waves = false;
    static POINT mouse_anchor;
    static Coord3D terrain_point_start;
    static Coord3D terrain_point_end;
    static WaterTracksObj *track = nullptr;
    static WaterTracksObj *track2 = nullptr;
    static bool track_edit_mode = false;
    static bool have_start = false;
    static bool have_end = false;
    static bool do_init = true;
    static WaveType current_wave_type = WAVE_TYPE_OCEAN;
    static bool track_edit_mode_reset = true;
    static bool add_point_reset = true;
    static bool delete_track_reset = true;
    static bool save_tracks_reset = true;
    static bool load_tracks_reset = true;
    static bool change_type_reset = true;

    pause_waves = false;

    if (do_init == true) {
        do_init = false;
    }

    if ((GetAsyncKeyState(VK_F5) & 0x8001) != 0) {
        if (track_edit_mode_reset == true) {
            if (track_edit_mode == true) {
                Utf16String str;
                str.Format(L"Leaving Water Track Edit Mode");
                g_theInGameUI->Message(str);
            } else {
                Utf16String str;
                str.Format(L"Entering Water Track Edit Mode");
                g_theInGameUI->Message(str);
                str.Format(L"Wave Type: %hs", s_waveTypeInfo[current_wave_type].wave_name);
                g_theInGameUI->Message(str);
            }

            track_edit_mode = !track_edit_mode;

            if (track_edit_mode == false) {
                have_start = false;
                have_end = false;
            }

            track_edit_mode_reset = false;
        }
    } else {
        track_edit_mode_reset = true;
    }

    if (track_edit_mode == true) {
        POINT point;

        if (GetCursorPos(&point)) {
            ScreenToClient(g_applicationHWnd, &point);

            if ((GetAsyncKeyState(VK_F6) & 0x8001) != 0) {
                if (add_point_reset == true) {
                    if (have_start == false) {
                        mouse_anchor = point;
                        ICoord2D coord;
                        coord.x = point.x;
                        coord.y = point.y;
                        g_theTacticalView->Screen_To_Terrain(&coord, &terrain_point_start);
                        have_start = true;
                        Utf16String str;
                        str.Format(L"Added Start");
                        g_theInGameUI->Message(str);
                    } else {
                        ICoord2D coord;
                        coord.x = point.x;
                        coord.y = point.y;
                        g_theTacticalView->Screen_To_Terrain(&coord, &terrain_point_end);
                        have_end = true;
                        track = g_theWaterTracksRenderSystem->Bind_Track(current_wave_type);

                        if (track != nullptr) {
                            Vector2 start(terrain_point_start.x, terrain_point_start.y);
                            Vector2 end(terrain_point_end.x, terrain_point_end.y);
                            Vector2 trackstart = end - start;
                            Vector2 rot = trackstart;
                            rot.Rotate(GAMEMATH_PI / 2);
                            rot.Normalize();
                            trackstart = start + (trackstart * 0.5f);
                            Vector2 trackend = trackstart + rot;
                            track->Init(s_waveTypeInfo[current_wave_type].final_height,
                                s_waveTypeInfo[current_wave_type].final_width,
                                Vector2(trackstart.X, trackstart.Y),
                                Vector2(trackend.X, trackend.Y),
                                s_waveTypeInfo[current_wave_type].texture_name,
                                0);

                            if (s_waveTypeInfo[current_wave_type].time_offset_second_wave != 0) {
                                track2 = g_theWaterTracksRenderSystem->Bind_Track(current_wave_type);

                                if (track2 != nullptr) {
                                    track2->Init(s_waveTypeInfo[current_wave_type].final_height,
                                        s_waveTypeInfo[current_wave_type].final_width,
                                        Vector2(trackstart.X, trackstart.Y),
                                        Vector2(trackend.X, trackend.Y),
                                        s_waveTypeInfo[current_wave_type].texture_name,
                                        s_waveTypeInfo[current_wave_type].time_offset_second_wave);
                                }
                            }

                            Utf16String str;
                            str.Format(L"Added End");
                            g_theInGameUI->Message(str);
                        }

                        have_start = false;
                        have_end = false;
                    }

                    add_point_reset = false;
                }
            } else {
                add_point_reset = true;
            }

            if ((GetAsyncKeyState(VK_DELETE) & 0x8001) != 0) {
                if (delete_track_reset == true && track != nullptr) {
                    delete_track_reset = false;
                    g_theWaterTracksRenderSystem->Un_Bind_Track(track);

                    if (track2 != nullptr) {
                        g_theWaterTracksRenderSystem->Un_Bind_Track(track2);
                    }

                    have_start = false;
                    have_end = false;
                    track = nullptr;
                    track2 = nullptr;
                }
            } else {
                delete_track_reset = true;
            }

            if ((GetAsyncKeyState(VK_INSERT) & 0x8001) != 0) {
                if (change_type_reset == true) {
                    change_type_reset = false;
                    current_wave_type++;

                    if (current_wave_type > WAVE_TYPE_RADIAL) {
                        current_wave_type = WAVE_TYPE_POND;
                    }

                    Utf16String str;
                    str.Format(L"Wave Type: %hs", s_waveTypeInfo[current_wave_type].wave_name);
                    g_theInGameUI->Message(str);
                }
            } else {
                change_type_reset = true;
            }

            if ((GetAsyncKeyState(VK_F7) & 0x8001) != 0) {
                if (save_tracks_reset == true) {
                    g_theWaterTracksRenderSystem->Save_Tracks();
                    have_start = false;
                    have_end = false;
                    track = nullptr;
                    track2 = nullptr;
                    Utf16String str;
                    str.Format(L"Saved Tracks");
                    g_theInGameUI->Message(str);
                }
            } else {
                save_tracks_reset = true;
            }

            if ((GetAsyncKeyState(VK_F8) & 0x8001) != 0) {
                if (load_tracks_reset == true) {
                    load_tracks_reset = false;
                    g_theWaterTracksRenderSystem->Reset();
                    g_theWaterTracksRenderSystem->Load_Tracks();
                    have_start = false;
                    have_end = false;
                    track = nullptr;
                    track2 = nullptr;
                    Utf16String str;
                    str.Format(L"Loaded Tracks");
                    g_theInGameUI->Message(str);
                }
            } else {
                save_tracks_reset = true;
            }
        }

        if (have_start == true && have_end == true) {
            ICoord2D coord;
            coord.x = point.x;
            coord.y = point.y;
            g_theTacticalView->Screen_To_Terrain(&coord, &terrain_point_end);
            int x = terrain_point_end.x - terrain_point_start.x;
            int y = terrain_point_end.y - terrain_point_start.y;

            if (s_waveTypeInfo[current_wave_type].final_width >= GameMath::Sqrt(x * x + y * y)) {
                g_theDisplay->Draw_Line(mouse_anchor.x, mouse_anchor.y, point.x, point.y, 1.0f, 0xFFCCCCFF);
                DX8Wrapper::Invalidate_Cached_Render_States();
                ShaderClass::Invalidate();
            }

            pause_waves = true;
        }
    }
#endif
}

WaterTracksRenderSystem::WaterTracksRenderSystem() :
    // BUGFIX Initialize all members
    m_vertexBuffer(nullptr),
    m_indexBuffer(nullptr),
    m_vertexMaterialClass(nullptr),
    m_usedModules(nullptr),
    m_freeModules(nullptr),
    m_stripSizeX(2),
    m_stripSizeY(2),
    m_batchStart(0),
    m_level(0.0f)
{
    g_theWaterTracksRenderSystem = this;
}

WaterTracksRenderSystem::~WaterTracksRenderSystem()
{
    Shutdown();
    m_vertexMaterialClass = nullptr;
}

void WaterTracksRenderSystem::Re_Acquire_Resources()
{
    Ref_Ptr_Release(m_indexBuffer);
    Ref_Ptr_Release(m_vertexBuffer);
    int size = (2 * m_stripSizeX + 2) * (m_stripSizeY - 1) - 2;
    m_indexBuffer = new DX8IndexBufferClass(size, DX8IndexBufferClass::USAGE_DEFAULT);

    {
        IndexBufferClass::WriteLockClass lock(m_indexBuffer, 0);
        unsigned short *indices = lock.Get_Index_Array();
        int i1 = 0;
        int i2 = 0;
        int i3 = 0;

        while (i1 < size) {
            while (i3 < (i2 + 1) * m_stripSizeX) {
                indices[i1] = m_stripSizeX + i3;
                indices[i1 + 1] = i3++;
                i1 += 2;
            }

            if (i1 < size) {
                indices[i1] = i3 - 1;
                indices[i1 + 1] = m_stripSizeX + i3;
                i1 += 2;
            }

            i2++;
        }
    }

    m_vertexBuffer = new DX8VertexBufferClass(
        DX8_FVF_XYZDUV1, 1000 * m_stripSizeY * m_stripSizeX, DX8VertexBufferClass::USAGE_DYNAMIC, 0);
    m_batchStart = 0;
}

void WaterTracksRenderSystem::Release_Resources()
{
    Ref_Ptr_Release(m_indexBuffer);
    Ref_Ptr_Release(m_vertexBuffer);
}

void WaterTracksRenderSystem::Init()
{
    m_stripSizeX = 2;
    m_stripSizeY = 2;
    m_level = g_theWriteableGlobalData->m_waterPositionZ;

    Re_Acquire_Resources();

    m_vertexMaterialClass = VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);

    m_shaderClass = ShaderClass::s_presetAlphaShader;
    m_shaderClass.Set_Cull_Mode(ShaderClass::CULL_MODE_DISABLE);

    if (!m_freeModules && !m_usedModules) {
        for (int i = 0; i < 2000; i++) {
            WaterTracksObj *obj = new WaterTracksObj();

            if (!obj) {
                break;
            }

            obj->m_prevSystem = nullptr;
            obj->m_nextSystem = m_freeModules;

            if (m_freeModules) {
                m_freeModules->m_prevSystem = obj;
            }

            m_freeModules = obj;
        }
    }
}

void WaterTracksRenderSystem::Reset()
{
    WaterTracksObj *next;

    for (WaterTracksObj *i = m_usedModules; i; i = next) {
        next = i->m_nextSystem;
        i->m_bound = false;
        Release_Track(i);
    }
}

void WaterTracksRenderSystem::Shutdown()
{
    WaterTracksObj *next;

    for (WaterTracksObj *i = m_usedModules; i; i = next) {
        next = i->m_nextSystem;

        if (!i->m_bound) {
            Release_Track(i);
        }
    }

    while (m_freeModules) {
        next = m_freeModules->m_nextSystem;
        delete m_freeModules;
        m_freeModules = next;
    }

    Ref_Ptr_Release(m_indexBuffer);
    Ref_Ptr_Release(m_vertexMaterialClass);
    Ref_Ptr_Release(m_vertexBuffer);
}

void WaterTracksRenderSystem::Update()
{
    static unsigned int iLastTime = rts::Get_Time();
    iLastTime = rts::Get_Time();
    WaterTracksObj *obj = m_usedModules;

    while (obj) {
        WaterTracksObj *next = obj->m_nextSystem;

        if (!obj->m_bound || !obj->Update(33) && !obj->m_bound) {
            Release_Track(obj);
        }

        obj = next;
    }
}

void WaterTracksRenderSystem::Flush(RenderInfoClass &rinfo)
{
#ifdef BUILD_WITH_D3D8
    if (g_theWriteableGlobalData->m_showSoftWaterEdge && g_theWaterTransparency->m_transparentWaterDepth != 0.0f) {
        if (g_theWriteableGlobalData->m_useWaveEditor) {
            Test_Water_Update();
        }

        Update();
        rinfo.m_camera.Apply();

        if (m_usedModules) {
            if (!ShaderClass::Is_Backface_Culling_Inverted()) {
                m_batchStart = 0xFFFF;
                float r = g_theWriteableGlobalData->m_terrainAmbient[0].red;
                float g = g_theWriteableGlobalData->m_terrainAmbient[0].green;
                float b = g_theWriteableGlobalData->m_terrainAmbient[0].blue;
                r = g_theWriteableGlobalData->m_terrainDiffuse[0].red / 2.0f + r;
                g = g_theWriteableGlobalData->m_terrainDiffuse[0].green / 2.0f + g;
                b = g_theWriteableGlobalData->m_terrainDiffuse[0].blue / 2.0f + b;
                r = r * 255.0f;
                g = g * 255.0f;
                b = b * 255.0f;
                // TODO investigate this, seems that color is leftover code
                int color = (GameMath::Fast_To_Int_Truncate(r) << 16) | (GameMath::Fast_To_Int_Truncate(g) << 8)
                    | GameMath::Fast_To_Int_Truncate(b);
                Matrix3D m(true);
                DX8Wrapper::Set_Transform(D3DTS_WORLD, m);
                DX8Wrapper::Set_Material(m_vertexMaterialClass);
                DX8Wrapper::Set_Shader(m_shaderClass);
                DX8Wrapper::Set_Vertex_Buffer(m_vertexBuffer, 0);
                DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS, 8);
                DX8Wrapper::Apply_Render_State_Changes();

                if (g_theTerrainRenderObject->Get_Shroud()) {
                    W3DShaderManager::Set_Texture(0, g_theTerrainRenderObject->Get_Shroud()->Get_Shroud_Texture());
                    W3DShaderManager::Set_Shader(W3DShaderManager::ST_SHROUD_TEXTURE, 1);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
                    DX8Wrapper::Set_DX8_Texture_Stage_State(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                    DX8Wrapper::Set_DX8_Render_State(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
                }

                for (WaterTracksObj *i = m_usedModules; i; i = i->m_nextSystem) {
                    if (-1 != i->m_type) {
                        DX8Wrapper::Set_Texture(0, i->m_stageZeroTexture);
                    }

                    m_batchStart = i->Render(m_vertexBuffer, m_batchStart);
                }

                DX8Wrapper::Set_DX8_Render_State(D3DRS_ZBIAS, 0);
                if (g_theTerrainRenderObject->Get_Shroud()) {
                    DX8Wrapper::Set_DX8_Render_State(D3DRS_ZFUNC, D3DCMP_EQUAL);
                    W3DShaderManager::Reset_Shader(W3DShaderManager::ST_SHROUD_TEXTURE);
                }
            }
        }
    }
#endif
}

WaterTracksObj *WaterTracksRenderSystem::Find_Track(Vector2 &start, Vector2 &end, WaveType type)
{
    for (WaterTracksObj *i = m_usedModules; i; i = i->m_nextSystem) {
        if (i->m_initialEnd == end && i->m_initialStart == start && i->m_type == type) {
            return i;
        }
    }

    return nullptr;
}

void WaterTracksRenderSystem::Save_Tracks()
{
    if (g_theTerrainLogic) {
        Utf8String filename = g_theTerrainLogic->Get_Source_Filename();
        char fname[256];
        strcpy(fname, filename.Str());
        strcpy(&fname[strlen(fname) - 4], ".wak");
        FILE *f = fopen(fname, "wb");
        int count = 0;
        if (f) {
            for (WaterTracksObj *i = m_usedModules; i; i = i->m_nextSystem) {
                if (!i->m_timeOffsetSecondWave) {
                    fwrite(&i->m_initialStart, sizeof(i->m_initialStart), 1, f);
                    fwrite(&i->m_initialEnd, sizeof(i->m_initialEnd), 1, f);
                    fwrite(&i->m_type, sizeof(i->m_type), 1, f);
                    count++;
                }
            }

            fwrite(&count, sizeof(count), 1, f);
            fclose(f);
        }
    }
}

void WaterTracksRenderSystem::Load_Tracks()
{
    if (g_theTerrainLogic) {
        Utf8String filename = g_theTerrainLogic->Get_Source_Filename();
        char fname[256];
        strcpy(fname, filename.Str());
        strcpy(&fname[strlen(fname) - 4], ".wak");
        File *f = g_theFileSystem->Open_File(fname, File::BINARY | File::READ);
        int count = 0;
        int flip = 0;
        Vector2 start;
        Vector2 end;
        WaveType type;

        if (f) {
            f->Seek(-4, File::END);
            f->Read(&count, sizeof(count));
            f->Seek(0, File::START);

            for (int i = 0; i < count; i++) {
                for (;;) {
                    f->Read(&start, sizeof(start));
                    f->Read(&end, sizeof(end));
                    f->Read(&type, sizeof(type));

                    if (!Find_Track(start, end, type)) {
                        break;
                    }
                    i++;
                }

                WaterTracksObj *track = g_theWaterTracksRenderSystem->Bind_Track(type);

                if (track) {
                    flip ^= 1;
                    track->Init(s_waveTypeInfo[type].final_height,
                        s_waveTypeInfo[type].final_width,
                        start,
                        end,
                        s_waveTypeInfo[type].texture_name,
                        0);
                    track->m_flipUV = flip;

                    if (s_waveTypeInfo[type].time_offset_second_wave) {
                        WaterTracksObj *track2 = g_theWaterTracksRenderSystem->Bind_Track(type);

                        if (track2) {
                            track2->Init(s_waveTypeInfo[type].final_height,
                                s_waveTypeInfo[type].final_width,
                                start,
                                end,
                                s_waveTypeInfo[type].texture_name,
                                s_waveTypeInfo[type].time_offset_second_wave);
                            track2->m_flipUV = (flip == 0);
                        }
                    }
                }
            }

            f->Close();
        }
    }
}

WaterTracksObj *WaterTracksRenderSystem::Bind_Track(WaveType type)
{
    WaterTracksObj *obj = m_freeModules;

    if (obj) {
        if (obj->m_nextSystem) {
            obj->m_nextSystem->m_prevSystem = obj->m_prevSystem;
        }

        if (obj->m_prevSystem) {
            obj->m_prevSystem->m_nextSystem = obj->m_nextSystem;
        } else {
            m_freeModules = obj->m_nextSystem;
        }

        obj->m_type = type;
        WaterTracksObj *next = nullptr;
        WaterTracksObj *i;

        for (i = m_usedModules; i; i = i->m_nextSystem) {
            if (i->m_type == type) {
                obj->m_nextSystem = i;
                obj->m_prevSystem = next;
                i->m_prevSystem = obj;

                if (next) {
                    next->m_nextSystem = obj;
                } else {
                    m_usedModules = obj;
                }

                break;
            }

            next = i;
        }

        if (!i) {
            obj->m_nextSystem = m_usedModules;

            if (m_usedModules) {
                m_usedModules->m_prevSystem = obj;
            }

            m_usedModules = obj;
        }
        obj->m_bound = true;
    }

    for (WaterTracksObj *j = m_usedModules; j; j = j->m_nextSystem) {
        j->m_elapsedMs = j->m_timeOffsetSecondWave;
    }

    return obj;
}

void WaterTracksRenderSystem::Un_Bind_Track(WaterTracksObj *mod)
{
    mod->m_bound = false;
    Release_Track(mod);
}

void WaterTracksRenderSystem::Release_Track(WaterTracksObj *mod)
{
    if (mod) {
        if (mod->m_nextSystem) {
            mod->m_nextSystem->m_prevSystem = mod->m_prevSystem;
        }

        if (mod->m_prevSystem) {
            mod->m_prevSystem->m_nextSystem = mod->m_nextSystem;
        } else {
            m_usedModules = mod->m_nextSystem;
        }

        mod->m_prevSystem = nullptr;
        mod->m_nextSystem = m_freeModules;

        if (m_freeModules) {
            m_freeModules->m_prevSystem = mod;
        }

        m_freeModules = mod;
        mod->Free_Water_Tracks_Resources();
    }
}
