/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dview.h"
#include "baseheightmap.h"
#include "camera.h"
#include "camerashakesystem.h"
#include "coltest.h"
#include "drawable.h"
#include "drawmodule.h"
#include "gamewindowmanager.h"
#include "globaldata.h"
#include "ingameui.h"
#include "object.h"
#include "scriptengine.h"
#include "selectioninfo.h"
#include "shadermanager.h"
#include "terrainlogic.h"
#include "w3ddisplay.h"
#include "w3dscene.h"
static const int g_theW3DFrameLengthInMsec = 33;

void W3DView::Init()
{
    View::Init();
    Set_Name("W3DView");
    Coord3D pos;
    pos.z = 0.0f;
    pos.x = 87.0f * 10.0f;
    pos.y = 77.0f * 10.0f;
    Set_Position(&pos);

    m_3DCamera = new CameraClass();
    Set_Camera_Transform();

    m_2DCamera = new CameraClass();
    m_2DCamera->Set_Position(Vector3(0.0f, 0.0f, 1.0f));
    m_2DCamera->Set_View_Plane(Vector2(-1.0f, -0.75f), Vector2(1.0f, 0.75f));
    m_2DCamera->Set_Clip_Planes(0.995f, 2.0f);
    m_cameraConstraintValid = false;
    m_scrollAmountCutoff = g_theWriteableGlobalData->m_scrollAmountCutoff;
}

void W3DView::Reset()
{
    View::Reset();
    Set_Time_Multiplier(1);
    Coord3D o;
    memset(&o, 0, sizeof(o));
    Reset_Camera(&o, 1, 0.0f, 0.0f);
    Set_View_Filter(FT_VIEW_SCREEN_DEFAULT_FILTER);

    Coord2D bias;
    bias.x = 0.0f;
    bias.y = 0.0f;
    Set_Guard_Band_Bias(&bias);
}

void W3DView::Set_Camera_Transform()
{
    m_cameraHasMovedSinceRequest = true;
    Matrix3D tm(true);
    float znear = 10.0f;
    float zfar = 1200.0f;

    if (m_realZoom) {
        if (m_FXPitch < 0.95f) {
            zfar /= m_FXPitch;
        }
    } else if ((g_theWriteableGlobalData != nullptr && g_theWriteableGlobalData->m_drawEntireTerrain) || m_FXPitch < 0.95f
        || m_zoom > 1.05f) {
        zfar *= 10.0f;
    }

    m_3DCamera->Set_Clip_Planes(znear, zfar);

#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData->m_useCameraConstraints)
#endif
    {
        if (!m_cameraConstraintValid) {
            Build_Camera_Transform(&tm);
            m_3DCamera->Set_Transform(tm);
            Calc_Camera_Constraints();
        }

        if (!m_cameraConstraintValid) {
            captainslog_debug("*** cam constraints are not valid!!!");
        }

        if (m_cameraConstraintValid) {
            Coord3D pos = Get_Position();
            pos.x = GameMath::Max(m_cameraConstraint.lo.x, pos.x);
            pos.x = GameMath::Min(m_cameraConstraint.hi.x, pos.x);
            pos.y = GameMath::Max(m_cameraConstraint.lo.y, pos.y);
            pos.y = GameMath::Min(m_cameraConstraint.hi.y, pos.y);
            Set_Position(&pos);
        }
    }

    m_3DCamera->Set_View_Plane(m_FOV);
    Build_Camera_Transform(&tm);
    m_3DCamera->Set_Transform(tm);

    if (g_theTerrainRenderObject != nullptr) {
        RefMultiListIterator<RenderObjClass> *it = W3DDisplay::s_3DScene->Create_Lights_Iterator();
        g_theTerrainRenderObject->Update_Center(m_3DCamera, it);

        if (it != nullptr) {
            W3DDisplay::s_3DScene->Destroy_Lights_Iterator(it);
        }
    }
}

void W3DView::Calc_Camera_Constraints()
{
    if (g_theTerrainLogic != nullptr) {
        Region3D extent;
        g_theTerrainLogic->Get_Extent(&extent);

        ICoord2D screen;
        screen.x = Get_Width() * 0.5f + m_originX;
        screen.y = Get_Height() * 0.5f + m_originY;

        Vector3 ray_start;
        Vector3 ray_end;
        Get_Pick_Ray(&screen, &ray_start, &ray_end);

        Coord3D coord;
        coord.x = Vector3::Find_X_At_Z(m_groundLevel, ray_start, ray_end);
        coord.y = Vector3::Find_Y_At_Z(m_groundLevel, ray_start, ray_end);
        coord.z = m_groundLevel;

        screen.y = Get_Height() * 0.95f + m_originY;
        Get_Pick_Ray(&screen, &ray_start, &ray_end);
        coord.x -= Vector3::Find_X_At_Z(m_groundLevel, ray_start, ray_end);
        coord.y -= Vector3::Find_Y_At_Z(m_groundLevel, ray_start, ray_end);
        float len = coord.Length();

        if (g_theWriteableGlobalData->m_debugAI) {
            len = -1000.0f;
        }

        m_cameraConstraint.lo.x = extent.lo.x + len;
        m_cameraConstraint.hi.x = extent.hi.x - len;
        m_cameraConstraint.lo.y = extent.lo.y + len;
        m_cameraConstraint.hi.y = extent.hi.y - len;
        m_cameraConstraintValid = true;
    }
}

static void Pixel_Screen_To_W3D_Logical_Screen(
    int screen_x, int screen_y, float *log_x, float *log_y, int screen_width, int screen_height)
{
    *log_x = ((float)screen_x + (float)screen_x) / (float)screen_width - 1.0f;
    *log_y = -(((float)screen_y + (float)screen_y) / (float)screen_height - 1.0f);
}

void W3DView::Get_Pick_Ray(const ICoord2D *screen, Vector3 *ray_start, Vector3 *ray_end)
{
    float x;
    float y;
    Pixel_Screen_To_W3D_Logical_Screen(screen->x - m_originX, screen->y - m_originY, &x, &y, Get_Width(), Get_Height());
    *ray_start = m_3DCamera->Get_Position();
    m_3DCamera->Un_Project(*ray_end, Vector2(x, y));
    *ray_end -= *ray_start;
    ray_end->Normalize();
    (*ray_end) *= m_3DCamera->Get_Depth();
    *ray_end += *ray_start;
}

void W3DView::Build_Camera_Transform(Matrix3D *transform)
{
    Vector3 p;
    Vector3 t;
    float zoom = Get_Zoom();
    float angle = Get_Angle();
    float pitch = Get_Pitch();
    Coord3D pos = Get_Position();
    pos.x += m_shakeOffset.x;
    pos.y += m_shakeOffset.y;

    if (m_cameraConstraintValid) {
        pos.x = GameMath::Max(m_cameraConstraint.lo.x, pos.x);
        pos.x = GameMath::Min(m_cameraConstraint.hi.x, pos.x);
        pos.y = GameMath::Max(m_cameraConstraint.lo.y, pos.y);
        pos.y = GameMath::Min(m_cameraConstraint.hi.y, pos.y);
    }

    if (m_realZoom) {
        p.X = m_cameraOffset.x;
        p.Y = m_cameraOffset.y;
        p.Z = m_cameraOffset.z;
        float real_zoom = zoom;

        if (real_zoom > 1.0f) {
            real_zoom = 1.0f;
        }

        if (real_zoom < 0.5f) {
            real_zoom = 0.5f;
        }

        m_FOV = 0.87266463f * real_zoom * real_zoom;
    } else {
        p.X = zoom * m_cameraOffset.x;
        p.Y = zoom * m_cameraOffset.y;
        p.Z = zoom * m_cameraOffset.z;
    }

    memset(&t, 0, sizeof(t));
    float k = 1.0f - m_groundLevel / p.Z;
    Matrix3D m1(Vector3(0.0f, 0.0f, 1.0f), angle);
    Matrix3D m2(Vector3(1.0f, 0.0f, 0.0f), pitch);
    m2.Mul_Vector3(p);
    m1.Mul_Vector3(p);

    p *= k;
    p.X += pos.x;
    p.Y += pos.y;
    p.Z += m_groundLevel;
    t.X += pos.x;
    t.Y += pos.y;
    t.Z += m_groundLevel;

    if (m_realZoom) {
        float real_zoom = 1.0f;

        if (!g_theDisplay->Is_LetterBoxed()) {
            real_zoom = zoom;

            if (real_zoom > 1.0f) {
                real_zoom = 1.0f;
            }

            if (real_zoom < 0.5f) {
                real_zoom = 0.5f;
            }

            p.Z *= (real_zoom * 0.5f + 0.5f);
        }

        m_FXPitch = (real_zoom * 0.75f + 0.25f) * 1.0f;
    }

    if (m_realZoom) {
        p.X = (p.X - t.X) / m_FXPitch + t.X;
        p.Y = (p.Y - t.Y) / m_FXPitch + t.Y;
    } else if (m_FXPitch > 1.0f) {
        p.X = (p.X - t.X) / m_FXPitch + t.X;
        p.Y = (p.Y - t.Y) / m_FXPitch + t.Y;
    } else {
        float fxpitch = p.Z - t.Z;
        fxpitch *= m_FXPitch;
        t.Z = p.Z - fxpitch;
    }

    transform->Make_Identity();
    transform->Look_At(p, t, 0.0f);
    g_theCameraShakerSystem.Timestep(0.033333335f);
    g_theCameraShakerSystem.Update_Camera_Shaker(p, &m_cameraShakeAngles);
    transform->Rotate_X(m_cameraShakeAngles.X);
    transform->Rotate_Y(m_cameraShakeAngles.Y);
    transform->Rotate_Z(m_cameraShakeAngles.Z);

    if (m_cameraSlaveMode) {
        Object *obj = g_theScriptEngine->Get_Unit_Named(m_cameraSlaveThing);

        if (obj != nullptr) {
            Drawable *drawable = obj->Get_Drawable();

            if (drawable != nullptr) {
                for (DrawModule **modules = drawable->Get_Draw_Modules(); *modules != nullptr; modules++) {
                    ObjectDrawInterface *draw = (*modules)->Get_Object_Draw_Interface();

                    if (draw != nullptr) {
                        Matrix3D bone;
                        draw->Client_Only_Get_Render_Obj_Bone_Transform(m_cameraSlaveBone, &bone);
                        *transform = bone;
                        Vector3 translation = transform->Get_Translation();
                        m_pos.x = translation.X;
                        m_pos.y = translation.Y;
                        m_pos.z = translation.Z;
                        return;
                    }
                }
            } else {
                m_cameraSlaveMode = false;
            }
        } else {
            m_cameraSlaveMode = false;
        }
    }
}

Drawable *W3DView::Pick_Drawable(const ICoord2D *screen, bool force_attack, PickType type)
{
    RenderObjClass *robj = nullptr;
    Drawable *drawable = nullptr;
    DrawableInfo *info = nullptr;

    if (screen == nullptr) {
        return nullptr;
    }

    GameWindow *parent = nullptr;

    if (g_theWindowManager != nullptr) {
        parent = g_theWindowManager->Get_Window_Under_Cursor(screen->x, screen->y, false);
    }

    while (parent != nullptr) {
        if ((parent->Win_Get_Status() & WIN_STATUS_SEE_THRU) == 0) {
            return nullptr;
        }

        parent = parent->Win_Get_Parent();
    }

    Vector3 ray_start;
    Vector3 ray_end;
    Get_Pick_Ray(screen, &ray_start, &ray_end);

    LineSegClass ray;
    ray.Set(ray_start, ray_end);
    CastResultStruct res;

    if (force_attack) {
        res.compute_contact_point = true;
    }

    RayCollisionTestClass ray_test(ray, &res, COLLISION_TYPE_ALL);

    if (W3DDisplay::s_3DScene->Cast_Ray(ray_test, false, type)) {
        robj = ray_test.m_collidedRenderObj;
    }

    if (robj != nullptr) {
        info = static_cast<DrawableInfo *>(robj->Get_User_Data());
    }

    if (info != nullptr) {
        return info->drawable;
    }

    return drawable;
}

int W3DView::Iterate_Drawables_In_Region(IRegion2D *screen_region, bool (*callback)(Drawable *, void *), void *user_data)
{
    int count = 0;
    Vector3 screen;
    Vector3 world;
    bool is_point = false;
    Region2D normalized_region;

    if (screen_region != nullptr) {
        if (screen_region->Height() == 0 && screen_region->Width() == 0) {
            is_point = true;
        }

        float f1 = screen_region->lo.x - m_originX;
        float f2 = Get_Width();
        normalized_region.lo.x = f1 / f2 + f1 / f2 - 1.0f;

        float f3 = screen_region->hi.y - m_originY;
        float f4 = Get_Height();
        normalized_region.lo.y = -(f3 / f4 + f3 / f4 - 1.0f);

        float f5 = screen_region->hi.x - m_originX;
        float f6 = Get_Width();
        normalized_region.hi.x = f5 / f6 + f5 / f6 - 1.0f;

        float f7 = screen_region->lo.y - m_originY;
        float f8 = Get_Height();
        normalized_region.hi.y = -(f7 / f8 + f7 / f8 - 1.0f);
    }

    Drawable *drawable = nullptr;

    if (is_point) {
        drawable =
            Pick_Drawable(&screen_region->lo, true, Get_Pick_Types_For_Context(g_theInGameUI->Is_In_Force_To_Attack_Mode()));

        if (drawable == nullptr) {
            return 0;
        }
    }

    for (Drawable *draw = g_theGameClient->First_Drawable(); draw != nullptr; draw = draw->Get_Next()) {
        bool do_callback;

        if (drawable != nullptr) {
            draw = drawable;
            do_callback = true;
        } else {
            do_callback = false;

            if (screen_region != nullptr) {
                const Coord3D *pos = draw->Get_Position();
                world.X = pos->x;
                world.Y = pos->y;
                world.Z = pos->z;

                if (m_3DCamera->Project(screen, world) == CameraClass::INSIDE_FRUSTUM && screen.X >= normalized_region.lo.x
                    && screen.X <= normalized_region.hi.x && screen.Y >= normalized_region.lo.y
                    && screen.Y <= normalized_region.hi.y) {
                    do_callback = true;
                }
            } else {
                do_callback = true;
            }
        }

        if (do_callback) {
            if (callback(draw, user_data)) {
                count++;
            }
        }

        if (drawable != nullptr) {
            break;
        }
    }

    return count;
}

void W3DView::Set_Width(int32_t width)
{
    View::Set_Width(width);
    Vector2 min;
    Vector2 max;
    float f1 = width;
    float width_to_height = f1 / Get_Height();
    m_3DCamera->Set_Aspect_Ratio(width_to_height);
    m_3DCamera->Get_Viewport(min, max);
    float f2 = (float)(width + m_originX);
    max.X = f2 / g_theDisplay->Get_Width();
    m_3DCamera->Set_Viewport(min, max);
    float f3 = f1 / g_theDisplay->Get_Width() * 0.87266463f;
    m_3DCamera->Set_View_Plane(f3);
}

void W3DView::Set_Height(int32_t height)
{
    View::Set_Height(height);
    Vector2 min;
    Vector2 max;
    float width_to_height = (float)Get_Width() / (float)height;
    m_3DCamera->Set_Aspect_Ratio(width_to_height);
    m_3DCamera->Get_Viewport(min, max);
    float f2 = (float)(height + m_originY);
    max.Y = f2 / g_theDisplay->Get_Height();
    m_3DCamera->Set_Viewport(min, max);
}

void W3DView::Set_Origin(int32_t x, int32_t y)
{
    View::Set_Origin(x, y);
    Vector2 min;
    Vector2 max;
    m_3DCamera->Get_Viewport(min, max);
    min.X = (float)x / g_theDisplay->Get_Width();
    min.Y = (float)y / g_theDisplay->Get_Height();
    m_3DCamera->Set_Viewport(min, max);
    Set_Width(m_width);
    Set_Height(m_height);
}

void W3DView::Set_Fade_Parameters(int frames, int direction)
{
    ScreenBWFilter::Set_Fade_Parameters(frames, direction);
    ScreenCrossFadeFilter::Set_Fade_Parameters(frames, direction);
}

void W3DView::Set_View_Filter_Pos(const Coord3D *pos)
{
    ScreenMotionBlurFilter::Set_Zoom_To_Pos(pos);
}

bool W3DView::Update_Camera_Movements()
{
    bool ret = false;

    if (m_doingZoomCamera) {
        Zoom_Camera_One_Frame();
        ret = true;
    }

    if (m_doingPitchCamera) {
        Pitch_Camera_One_Frame();
        ret = true;
    }

    if (m_doingRotateCamera) {
        m_previousLookAtPosition = Get_Position();
        Rotate_Camera_One_Frame();
        ret = true;
    } else if (m_doingMoveCameraOnWaypointPath) {
        m_previousLookAtPosition = Get_Position();
        Move_Along_Waypoint_Path(g_theW3DFrameLengthInMsec);
        ret = true;
    }

    if (m_doingScriptedCameraLock) {
        return true;
    }

    return ret;
}

void W3DView::Zoom_Camera_One_Frame()
{
    m_zcInfo.cur_frame++;

    if (g_theWriteableGlobalData->m_disableCameraMovements) {
        if (m_zcInfo.cur_frame >= m_zcInfo.num_frames) {
            m_doingZoomCamera = false;
        }
    } else {
        if (m_zcInfo.cur_frame <= m_zcInfo.num_frames) {
            float param = (float)m_zcInfo.cur_frame / (float)m_zcInfo.num_frames;
            float lerp = m_zcInfo.ease(param);
            m_zoom = GameMath::Lerp(m_zcInfo.start_zoom, m_zcInfo.end_zoom, lerp);
        }

        if (m_zcInfo.cur_frame >= m_zcInfo.num_frames) {
            m_doingZoomCamera = false;
            m_zoom = m_zcInfo.end_zoom;
        }
    }
}

void Norm_Angle(float *angle)
{
    if (*angle < -31.415928f) {
        *angle = 0.0f;
    }

    if (*angle > 31.415928f) {
        *angle = 0.0f;
    }

    while (*angle < -3.1415927f) {
        *angle += 6.2831855f;
    }

    while (*angle > 3.1415927f) {
        *angle -= 6.2831855f;
    }
}

void W3DView::Rotate_Camera_One_Frame()
{
    m_rcInfo.cur_frame++;

    if (g_theWriteableGlobalData->m_disableCameraMovements) {
        if (m_rcInfo.cur_frame >= m_rcInfo.num_hold_frames + m_rcInfo.num_frames) {
            m_doingRotateCamera = false;
            m_freezeTimeForCameraMovement = false;
        }
    } else {
        if (m_rcInfo.track_object) {
            if (m_rcInfo.cur_frame <= m_rcInfo.num_hold_frames + m_rcInfo.num_frames) {
                Object *obj = g_theGameLogic->Find_Object_By_ID(m_rcInfo.target_object_id);

                if (obj != nullptr) {
                    m_rcInfo.target_object_pos = *obj->Get_Position();
                }

                Vector2 v(m_rcInfo.target_object_pos.x - m_pos.x, m_rcInfo.target_object_pos.y - m_pos.y);
                float len = v.Length();

                if (len >= 0.1f) {
                    float angle = GameMath::Acos(v.X / len);

                    if (v.Y < 0.0f) {
                        angle = -angle;
                    }

                    angle -= 1.5707964f;
                    Norm_Angle(&angle);

                    if (m_rcInfo.cur_frame > m_rcInfo.num_frames) {
                        m_angle = angle;
                    } else {
                        float f1 = (float)m_rcInfo.cur_frame / (float)m_rcInfo.num_frames;
                        float f2 = m_rcInfo.ease(f1);
                        float f3 = angle - m_angle;
                        Norm_Angle(&f3);
                        f3 *= f2;
                        m_angle = f3 + m_angle;
                        Norm_Angle(&m_angle);
                        float f4 = (m_rcInfo.end_time_multiplier - m_rcInfo.start_time_multiplier) * f2 + 0.5f;
                        m_timeMultiplier = GameMath::Fast_To_Int_Floor(f4) + m_rcInfo.start_time_multiplier;
                    }
                }
            }
        } else if (m_rcInfo.cur_frame <= m_rcInfo.num_frames) {
            float f1 = (float)m_rcInfo.cur_frame / (float)m_rcInfo.num_frames;
            float f2 = m_rcInfo.ease(f1);
            m_angle = GameMath::Lerp(m_rcInfo.angle, m_rcInfo.angle2, f2);
            Norm_Angle(&m_angle);
            float f3 = (m_rcInfo.end_time_multiplier - m_rcInfo.start_time_multiplier) * f2 + 0.5f;
            m_timeMultiplier = GameMath::Fast_To_Int_Floor(f3) + m_rcInfo.start_time_multiplier;
        }

        if (m_rcInfo.cur_frame >= m_rcInfo.num_hold_frames + m_rcInfo.num_frames) {
            m_doingRotateCamera = false;
            m_freezeTimeForCameraMovement = false;

            if (!m_rcInfo.track_object) {
                m_angle = m_rcInfo.angle2;
            }
        }
    }
}

void W3DView::Pitch_Camera_One_Frame()
{
    m_pcInfo.cur_frame++;

    if (g_theWriteableGlobalData->m_disableCameraMovements) {
        if (m_pcInfo.cur_frame >= m_pcInfo.num_frames) {
            m_doingPitchCamera = false;
        }
    } else {
        if (m_pcInfo.cur_frame <= m_pcInfo.num_frames) {
            float f1 = (float)m_pcInfo.cur_frame / (float)m_pcInfo.num_frames;
            float f2 = m_pcInfo.ease(f1);
            m_FXPitch = GameMath::Lerp(m_pcInfo.start_pitch, m_pcInfo.end_pitch, f2);
        }

        if (m_pcInfo.cur_frame >= m_pcInfo.num_frames) {
            m_doingPitchCamera = false;
            m_FXPitch = m_pcInfo.end_pitch;
        }
    }
}

void W3DView::Move_Along_Waypoint_Path(int milliseconds)
{
    m_mcwpInfo.elapsed_time_milliseconds += milliseconds;

    if (g_theWriteableGlobalData->m_disableCameraMovements) {
        if (m_mcwpInfo.elapsed_time_milliseconds > m_mcwpInfo.total_time_milliseconds) {
            m_doingMoveCameraOnWaypointPath = false;
            m_freezeTimeForCameraMovement = false;
        }
    } else if (m_mcwpInfo.elapsed_time_milliseconds > m_mcwpInfo.total_time_milliseconds) {
        m_doingMoveCameraOnWaypointPath = false;
        m_doingMoveCameraAlongWaypointPath = false;
        m_freezeTimeForCameraMovement = false;
        m_angle = m_mcwpInfo.camera_angle[m_mcwpInfo.num_waypoints];
        m_groundLevel = m_mcwpInfo.ground_height[m_mcwpInfo.num_waypoints];
        m_cameraOffset.y =
            -(m_cameraOffset.z / GameMath::Tan(g_theWriteableGlobalData->m_cameraPitch * 0.01745329300562541f));
        m_cameraOffset.x = -(GameMath::Tan(g_theWriteableGlobalData->m_cameraYaw * 0.01745329300562541f) * m_cameraOffset.y);

        Coord3D pos;
        pos.x = m_mcwpInfo.waypoints[m_mcwpInfo.num_waypoints].x;
        pos.y = m_mcwpInfo.waypoints[m_mcwpInfo.num_waypoints].y;
        pos.z = 0.0f;
        Set_Position(&pos);
        m_cameraConstraint.lo.x = GameMath::Min(m_cameraConstraint.lo.x, pos.x);
        m_cameraConstraint.hi.x = GameMath::Max(m_cameraConstraint.hi.x, pos.x);
        m_cameraConstraint.lo.y = GameMath::Min(m_cameraConstraint.lo.y, pos.y);
        m_cameraConstraint.hi.y = GameMath::Max(m_cameraConstraint.hi.y, pos.y);
    } else {
        float f1 = (float)m_mcwpInfo.total_time_milliseconds;
        float f2 = m_mcwpInfo.elapsed_time_milliseconds / f1;
        float f3 = m_mcwpInfo.ease(f2);
        float f4 = (m_mcwpInfo.elapsed_time_milliseconds - milliseconds) / f1;
        float f5 = f3 - m_mcwpInfo.ease(f4);
        m_mcwpInfo.cur_seg_distance += f5 * m_mcwpInfo.total_distance;

        while (m_mcwpInfo.cur_seg_distance >= m_mcwpInfo.way_seg_length[m_mcwpInfo.cur_segment]) {
            if (m_doingMoveCameraOnWaypointPath) {
                m_doingMoveCameraAlongWaypointPath = true;
            }

            m_mcwpInfo.cur_seg_distance -= m_mcwpInfo.way_seg_length[m_mcwpInfo.cur_segment];
            m_mcwpInfo.cur_segment++;

            if (m_mcwpInfo.cur_segment >= m_mcwpInfo.num_waypoints) {
                m_mcwpInfo.total_time_milliseconds = 0;
                return;
            }
        }

        float f6 = 1.0f / m_mcwpInfo.rolling_average_frames;
        m_mcwpInfo.cur_shutter--;

        if (m_mcwpInfo.cur_shutter <= 0) {
            m_mcwpInfo.cur_shutter = m_mcwpInfo.shutter;
            float f7 = m_mcwpInfo.cur_seg_distance / m_mcwpInfo.way_seg_length[m_mcwpInfo.cur_segment];

            if (m_mcwpInfo.cur_segment == m_mcwpInfo.num_waypoints - 1) {
                f6 = f6 + (1.0f - f6) * f7;
            }

            float f8 = 1.0f - f7;
            float f9 = 1.0f - f8;
            float f10 = m_mcwpInfo.camera_angle[m_mcwpInfo.cur_segment];
            float f11 = m_mcwpInfo.camera_angle[m_mcwpInfo.cur_segment + 1];

            if (f11 - f10 > GAMEMATH_PI) {
                f10 += GAMEMATH_PI2;
            }

            if (f11 - f10 < -GAMEMATH_PI) {
                f10 -= GAMEMATH_PI2;
            }

            float f12 = f10 * f8 + f11 * f9;
            Norm_Angle(&f12);
            float f13 = f12 - m_angle;
            Norm_Angle(&f13);

            if (GameMath::Fabs(f13) > 0.3141592741012573f) {
                captainslog_debug("Huh.");
            }

            m_angle += f6 * f13;
            Norm_Angle(&m_angle);
            float f14 = m_mcwpInfo.time_multiplier[m_mcwpInfo.cur_segment] * f8
                + m_mcwpInfo.time_multiplier[m_mcwpInfo.cur_segment + 1] * f9;
            m_timeMultiplier = GameMath::Fast_To_Int_Floor(f14 + 0.5f);
            m_groundLevel = f8 * m_mcwpInfo.ground_height[m_mcwpInfo.cur_segment]
                + f9 * m_mcwpInfo.ground_height[m_mcwpInfo.cur_segment + 1];
            m_cameraOffset.y =
                -(m_cameraOffset.z / GameMath::Tan(g_theWriteableGlobalData->m_cameraPitch * 0.01745329300562541f));
            m_cameraOffset.x =
                -(GameMath::Tan(g_theWriteableGlobalData->m_cameraYaw * 0.01745329300562541f) * m_cameraOffset.y);

            Coord3D wp;
            Coord3D wp2;
            Coord3D wp3;
            if (f7 < 0.5f) {
                wp = m_mcwpInfo.waypoints[m_mcwpInfo.cur_segment - 1];
                wp.x += m_mcwpInfo.waypoints[m_mcwpInfo.cur_segment].x;
                wp.y += m_mcwpInfo.waypoints[m_mcwpInfo.cur_segment].y;
                wp.x /= 2.0f;
                wp.y /= 2.0f;
                wp2 = m_mcwpInfo.waypoints[m_mcwpInfo.cur_segment];
                wp3 = m_mcwpInfo.waypoints[m_mcwpInfo.cur_segment];
                wp3.x += m_mcwpInfo.waypoints[m_mcwpInfo.cur_segment + 1].x;
                wp3.y += m_mcwpInfo.waypoints[m_mcwpInfo.cur_segment + 1].y;
                wp3.x /= 2.0f;
                wp3.y /= 2.0f;
                f7 += 0.5f;
            } else {
                wp = m_mcwpInfo.waypoints[m_mcwpInfo.cur_segment];
                wp.x += m_mcwpInfo.waypoints[m_mcwpInfo.cur_segment + 1].x;
                wp.y += m_mcwpInfo.waypoints[m_mcwpInfo.cur_segment + 1].y;
                wp.x /= 2.0f;
                wp.y /= 2.0f;
                wp2 = m_mcwpInfo.waypoints[m_mcwpInfo.cur_segment + 1];
                wp3 = m_mcwpInfo.waypoints[m_mcwpInfo.cur_segment + 1];
                wp3.x += m_mcwpInfo.waypoints[m_mcwpInfo.cur_segment + 2].x;
                wp3.y += m_mcwpInfo.waypoints[m_mcwpInfo.cur_segment + 2].y;
                wp3.x /= 2.0f;
                wp3.y /= 2.0f;
                f7 -= 0.5f;
            }

            Coord3D pos;
            pos.x = (wp3.x - wp.x) * f7 + wp.x;
            pos.y = (wp3.y - wp.y) * f7 + wp.y;
            pos.x = (1.0f - f7) * f7 * (wp2.x - wp3.x + wp2.x - wp.x) + pos.x;
            pos.y = (1.0f - f7) * f7 * (wp2.y - wp3.y + wp2.y - wp.y) + pos.y;
            pos.z = 0.0f;
            Set_Position(&pos);
            m_cameraConstraint.lo.x = GameMath::Min(m_cameraConstraint.lo.x, pos.x);
            m_cameraConstraint.hi.x = GameMath::Max(m_cameraConstraint.hi.x, pos.x);
            m_cameraConstraint.lo.y = GameMath::Min(m_cameraConstraint.lo.y, pos.y);
            m_cameraConstraint.hi.y = GameMath::Max(m_cameraConstraint.hi.y, pos.y);
        }
    }
}
