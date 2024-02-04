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
#include "ai.h"
#include "aipathfind.h"
#include "baseheightmap.h"
#include "camera.h"
#include "camerashakesystem.h"
#include "coltest.h"
#include "drawable.h"
#include "drawmodule.h"
#include "dx8renderer.h"
#include "gamewindowmanager.h"
#include "globaldata.h"
#include "ingameui.h"
#include "object.h"
#include "playerlist.h"
#include "rinfo.h"
#include "scriptengine.h"
#include "selectioninfo.h"
#include "shadermanager.h"
#include "terrainlogic.h"
#include "w3ddisplay.h"
#include "w3dscene.h"
#include "water.h"

W3DView::W3DView() :
    m_3DCamera(nullptr),
    m_2DCamera(nullptr),
    m_viewFilterMode(FM_13),
    m_viewFilter(FT_VIEW_SCREEN_DEFAULT_FILTER),
    m_extraPass(false),
    m_wireframeMode(false),
    m_shakeAngleCos(0.0f),
    m_shakeAngleSin(0.0f),
    m_shakeIntensity(0.0f),
    m_cameraShakeAngles(0.0f, 0.0f, 0.0f),
    m_doingRotateCamera(false),
    m_doingPitchCamera(false),
    m_doingZoomCamera(false),
    m_doingScriptedCameraLock(false),
    m_FXPitch(1.0f),
    m_doingMoveCameraOnWaypointPath(false),
    m_doingMoveCameraAlongWaypointPath(false),
    m_freezeTimeForCameraMovement(false),
    m_timeMultiplier(0),
    m_cameraHasMovedSinceRequest(true),
    m_scrollAmountCutoff(0.0f),
    m_groundLevel(0.0f),
    m_cameraConstraintValid(false),
    m_cameraSlaveMode(false),
    m_realZoom(false)
{
    m_cameraOffset.z = g_theWriteableGlobalData->m_cameraHeight;
    m_cameraOffset.y = -(m_cameraOffset.z / GameMath::Tan(g_theWriteableGlobalData->m_cameraPitch * 0.01745329300562541f));
    m_cameraOffset.x = -(GameMath::Tan(g_theWriteableGlobalData->m_cameraYaw * 0.01745329300562541f) * m_cameraOffset.y);
    m_shakeOffset.x = 0.0f;
    m_shakeOffset.y = 0.0f;
    m_locationRequests.clear();
    m_locationRequests.reserve(50);
    m_previousLookAtPosition.x = 0.0f;
    m_previousLookAtPosition.y = 0.0f;
    m_previousLookAtPosition.z = 0.0f;
    m_scrollAmount.x = 0.0f;
    m_scrollAmount.y = 0.0f;
    m_cameraConstraint.hi.x = 0.0f;
    m_cameraConstraint.hi.y = 0.0f;
    m_cameraConstraint.lo.x = 0.0f;
    m_cameraConstraint.lo.y = 0.0f;
}

W3DView::~W3DView()
{
    Ref_Ptr_Release(m_2DCamera);
    Ref_Ptr_Release(m_3DCamera);
}

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

                    angle -= DEG_TO_RADF(90.0f);
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
            m_angle = GameMath::Lerp(m_rcInfo.start_angle, m_rcInfo.end_angle, f2);
            Norm_Angle(&m_angle);
            float f3 = (m_rcInfo.end_time_multiplier - m_rcInfo.start_time_multiplier) * f2 + 0.5f;
            m_timeMultiplier = GameMath::Fast_To_Int_Floor(f3) + m_rcInfo.start_time_multiplier;
        }

        if (m_rcInfo.cur_frame >= m_rcInfo.num_hold_frames + m_rcInfo.num_frames) {
            m_doingRotateCamera = false;
            m_freezeTimeForCameraMovement = false;

            if (!m_rcInfo.track_object) {
                m_angle = m_rcInfo.end_angle;
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

const Coord3D &W3DView::Get_3D_Camera_Position()
{
    static Coord3D pos;
    Vector3 v = m_3DCamera->Get_Position();
    pos.Set(v.X, v.Y, v.Z);
    return pos;
}

void W3DView::Update_View()
{
#ifdef GAME_DEBUG_STRUCTS
    // TODO Subsystem debug stuff
#endif
    Update();
}

float Get_Height_Around_Pos(float x, float y)
{
    float z1 = g_theTerrainLogic->Get_Ground_Height(x, y, nullptr);
    float z2 = GameMath::Max(z1, g_theTerrainLogic->Get_Ground_Height(x + 40.0f, y - 40.0f, nullptr));
    float z3 = GameMath::Max(z2, g_theTerrainLogic->Get_Ground_Height(x - 40.0f, y - 40.0f, nullptr));
    float z4 = GameMath::Max(z3, g_theTerrainLogic->Get_Ground_Height(x + 40.0f, y + 40.0f, nullptr));
    return GameMath::Max(z4, g_theTerrainLogic->Get_Ground_Height(x - 40.0f, y + 40.0f, nullptr));
}

void Draw_Drawable(Drawable *draw, void *user_data)
{
    draw->Draw(static_cast<View *>(user_data));
}

void W3DView::Update()
{
    static float follow_factor = -1.0f;
    bool camera_changed = false;
    bool b2 = false;

    if (g_theTerrainRenderObject->Get_Need_Full_Update()) {
        RefMultiListIterator<RenderObjClass> *iter = W3DDisplay::s_3DScene->Create_Lights_Iterator();
        g_theTerrainRenderObject->Update_Center(m_3DCamera, iter);

        if (iter != nullptr) {
            W3DDisplay::s_3DScene->Destroy_Lights_Iterator(iter);
            iter = nullptr;
        }
    }

    ObjectID camera_lock = Get_Camera_Lock();

    if (camera_lock == INVALID_OBJECT_ID) {
        follow_factor = -1.0f;
    }

    if (camera_lock != INVALID_OBJECT_ID) {
        m_doingMoveCameraOnWaypointPath = false;
        m_doingMoveCameraAlongWaypointPath = false;
        Object *obj = g_theGameLogic->Find_Object_By_ID(camera_lock);

        if (obj != nullptr) {
            if (follow_factor >= 0.0f) {
                follow_factor += 0.05f;

                if (follow_factor > 1.0f) {
                    follow_factor = 1.0f;
                }
            } else {
                follow_factor = 0.05f;
            }

            if (Get_Camera_Lock_Drawable() != nullptr) {
                Drawable *drawable = Get_Camera_Lock_Drawable();

                if (drawable != nullptr) {
                    Matrix3D tm;
                    Coord3D position;
                    float radius;

                    if (drawable->Client_Only_Get_First_Render_Obj_Info(&position, &radius, &tm)) {
                        Vector3 v1(0.0f, 0.0f, 1.0f);
                        Vector3 v2;
                        v2.X = position.x;
                        v2.Y = position.y;
                        v2.Z = position.z;
                        float f1 = radius * 1.0f;
                        v2 += f1 * v1;
                        Vector3 v3 = tm.Get_X_Vector();
                        Vector3 v4 = v3 * radius;
                        Vector3 v5 = v4 * 4.5f;
                        Vector3 v6 = v2 - v5;
                        Vector3 v7 = m_3DCamera->Get_Position();
                        Vector3 v8 = v6 - v7;
                        Vector3 v9 = v8 * 0.1f;
                        v6 = v7 + v9;
                        Matrix3D new_tm;
                        new_tm.Look_At(v6, v2, 0.0f);
                        m_3DCamera->Set_Transform(new_tm);
                        camera_changed = false;
                    }
                } else {
                    Set_Camera_Lock_Drawable(nullptr);
                }
            } else {
                Coord3D obj_pos = *obj->Get_Position();
                Coord3D view_pos = Get_Position();
                float cell_size_sqr = GameMath::Square(g_theWriteableGlobalData->m_partitionCellSize);
                float view_to_obj_len_sqr =
                    GameMath::Square(view_pos.y - obj_pos.y) + GameMath::Square(view_pos.x - obj_pos.x);

                if (m_snapImmediate) {
                    view_pos.x = obj_pos.x;
                    view_pos.y = obj_pos.y;
                } else {
                    float f7 = obj_pos.x - view_pos.x;
                    float f8 = obj_pos.y - view_pos.y;

                    if (m_lockType == LOCK_TETHER) {
                        if (view_to_obj_len_sqr < cell_size_sqr) {
                            float f9 = 0.01f * m_lockDist;
                            view_pos.x += (obj_pos.x - view_pos.x) * f9;
                            view_pos.y += (obj_pos.y - view_pos.y) * f9;
                        } else {
                            float f12 = 1.0f - cell_size_sqr / view_to_obj_len_sqr;
                            view_pos.x += f7 * f12 * 0.5f;
                            view_pos.y += f8 * f12 * 0.5f;
                        }
                    } else {
                        view_pos.x += f7 * follow_factor;
                        view_pos.y += f8 * follow_factor;
                    }
                }

                if (!g_theScriptEngine->Is_Time_Frozen_Debug() && !g_theScriptEngine->Is_Time_Frozen_Script()
                    && !g_theGameLogic->Is_Game_Paused()) {
                    m_previousLookAtPosition = Get_Position();
                }

                Set_Position(&view_pos);

                if (m_lockType == LOCK_FOLLOW && obj->Is_Using_Airborne_Locomotor() && obj->Is_Above_Terrain_Or_Water()) {
                    float view_angle = m_angle;
                    float obj_back_angle = obj->Get_Orientation() - 1.570796326794897f;
                    Norm_Angle(&view_angle);
                    Norm_Angle(&obj_back_angle);
                    float angle_delta = obj_back_angle - view_angle;
                    Norm_Angle(&angle_delta);

                    if (m_snapImmediate) {
                        m_angle = obj_back_angle;
                    } else {
                        m_angle += angle_delta * 0.1f;
                    }

                    Norm_Angle(&m_angle);
                }

                if (m_snapImmediate) {
                    m_snapImmediate = false;
                }

                m_groundLevel = obj_pos.z;
                b2 = true;
                camera_changed = true;
            }
        } else {
            Set_Camera_Lock(INVALID_OBJECT_ID);
            Set_Camera_Lock_Drawable(nullptr);
            follow_factor = -1.0f;
        }
    }

    if (g_theScriptEngine->Is_Time_Frozen_Debug() || g_theGameLogic->Is_Game_Paused()) {
        if (m_doingRotateCamera || m_doingMoveCameraOnWaypointPath || m_doingPitchCamera || m_doingZoomCamera
            || m_doingScriptedCameraLock) {
            b2 = true;
        }
    } else if (Update_Camera_Movements()) {
        b2 = true;
        camera_changed = true;
    }

    if (m_shakeIntensity <= 0.01f) {
        m_shakeIntensity = 0.0f;
        m_shakeOffset.x = 0.0f;
        m_shakeOffset.y = 0.0f;
    } else {
        m_shakeOffset.x = m_shakeIntensity * m_shakeAngleCos;
        m_shakeOffset.y = m_shakeIntensity * m_shakeAngleSin;
        m_shakeIntensity *= 0.75f;
        m_shakeAngleCos = -m_shakeAngleCos;
        m_shakeAngleSin = -m_shakeAngleSin;
        camera_changed = true;
    }

    if (g_theCameraShakerSystem.Is_Camera_Shaking()) {
        camera_changed = true;
    }

    m_terrainHeightUnderCamera = Get_Height_Around_Pos(m_pos.x, m_pos.y);
    m_currentHeightAboveGround = m_cameraOffset.z * m_zoom - m_terrainHeightUnderCamera;

    if (g_theTerrainLogic != nullptr && g_theWriteableGlobalData != nullptr && g_theInGameUI != nullptr && m_okToAdjustHeight
        && !g_theGameLogic->Is_Game_Paused()) {
        float f16 = m_terrainHeightUnderCamera + m_heightAboveGround;
        float f17 = f16 / m_cameraOffset.z;

        if (b2 || (g_theGameLogic->Is_In_Replay_Game() && g_theWriteableGlobalData->m_useCameraInReplays)) {
            m_heightAboveGround = m_currentHeightAboveGround;
        }

        if (g_theInGameUI->Is_Scrolling()) {
            if (m_scrollAmount.Length() < m_scrollAmountCutoff || m_currentHeightAboveGround < m_minHeightAboveGround
                || (g_theWriteableGlobalData->m_enforceMaxCameraHeight
                    && m_currentHeightAboveGround > m_maxHeightAboveGround)) {
                float add_zoom = (f17 - m_zoom) * g_theWriteableGlobalData->m_cameraAdjustSpeed;

                if (GameMath::Fabs(add_zoom) >= 0.0001f) {
                    m_zoom += add_zoom;
                    camera_changed = true;
                }
            }
        } else {
            float sub_zoom = (m_zoom - f17) * g_theWriteableGlobalData->m_cameraAdjustSpeed;

            if (GameMath::Fabs(sub_zoom) >= 0.0001f && !b2) {
                m_zoom -= sub_zoom;
                camera_changed = true;
            }
        }
    }

    if (!g_theScriptEngine->Is_Time_Fast()) {
        if (camera_changed || m_cameraSlaveMode) {
            Set_Camera_Transform();
        }

        Region3D region;

        Get_Axis_Aligned_View_Region(region);

        if (W3D::Get_Frame_Time() != 0) {
            g_theGameClient->Iterate_Drawables_In_Region(&region, Draw_Drawable, this);
        }
    }
}

void W3DView::Get_Axis_Aligned_View_Region(Region3D &axis_aligned_region)
{
    Coord3D box[4];
    Get_Screen_Corner_World_Points_At_Z(&box[0], &box[1], &box[2], &box[3], 0.0f);
    axis_aligned_region.lo = box[0];
    axis_aligned_region.hi = box[0];

    for (int i = 0; i < 4; i++) {
        if (box[i].x < axis_aligned_region.lo.x) {
            axis_aligned_region.lo.x = box[i].x;
        }

        if (box[i].y < axis_aligned_region.lo.y) {
            axis_aligned_region.lo.y = box[i].y;
        }

        if (box[i].x > axis_aligned_region.hi.x) {
            axis_aligned_region.hi.x = box[i].x;
        }

        if (box[i].y > axis_aligned_region.hi.y) {
            axis_aligned_region.hi.y = box[i].y;
        }
    }

    float f1 = 999999.0f;
    Region3D extent;
    g_theTerrainLogic->Get_Extent(&extent);
    axis_aligned_region.lo.z = extent.lo.z - f1;
    axis_aligned_region.hi.z = extent.hi.z + f1;
    axis_aligned_region.lo.x -= m_guardBandBias.x + 75.0f;
    axis_aligned_region.lo.y -= m_guardBandBias.y + 75.0f + 60.0f;
    axis_aligned_region.hi.x += m_guardBandBias.x + 75.0f;
    axis_aligned_region.hi.y += m_guardBandBias.y + 75.0f;
}

void W3DView::Set_3D_Wireframe_Mode(bool on)
{
    m_wireframeMode = on;
}

bool W3DView::Set_View_Filter_Mode(FilterModes mode)
{
    FilterModes old_mode = m_viewFilterMode;
    m_viewFilterMode = mode;

    if (m_viewFilterMode == FM_NULL_MODE || m_viewFilter == FT_NULL_FILTER
        || W3DShaderManager::Filter_Setup(m_viewFilter, m_viewFilterMode)) {
        return true;
    }

    m_viewFilterMode = old_mode;
    return false;
}

bool W3DView::Set_View_Filter(FilterTypes filter)
{
    FilterTypes old_filter = m_viewFilter;
    m_viewFilter = filter;

    if (m_viewFilterMode == FM_NULL_MODE || m_viewFilter == FT_NULL_FILTER
        || W3DShaderManager::Filter_Setup(m_viewFilter, m_viewFilterMode)) {
        return true;
    }

    m_viewFilter = old_filter;
    return false;
}

void W3DView::Draw_View()
{
#ifdef GAME_DEBUG_STRUCTS
    // TODO Subsystem debug stuff
#endif
    Draw();
}

void W3DView::Calc_Delta_Scroll(Coord2D &screen_delta)
{
    screen_delta.x = 0.0f;
    screen_delta.y = 0.0f;
    Vector3 ws_point(m_previousLookAtPosition.x, m_previousLookAtPosition.y, m_groundLevel);
    Vector3 dest;

    if (m_3DCamera->Project(dest, ws_point) == CameraClass::INSIDE_FRUSTUM) {
        Vector3 ws_point2(m_pos.x, m_pos.y, m_groundLevel);
        Vector3 dest2;

        if (m_3DCamera->Project(dest2, ws_point2) == CameraClass::INSIDE_FRUSTUM) {
            screen_delta.x = dest2.X - dest.X;
            screen_delta.y = dest2.Y - dest.Y;
        }
    }
}

void Drawable_Post_Draw(Drawable *draw, void *user_data)
{
    if (!draw->Is_Hidden() && g_theTacticalView->Get_FX_Pitch() >= 0.0f) {
        Object *obj = draw->Get_Object();
        int index;

        if (g_thePlayerList != nullptr) {
            index = g_thePlayerList->Get_Local_Player()->Get_Player_Index();
        } else {
            index = 0;
        }

#ifdef GAME_DEBUG_STRUCTS
        ObjectShroudStatus status =
            obj != nullptr && g_theWriteableGlobalData->m_shroudOn ? obj->Get_Shrouded_Status(index) : SHROUDED_NONE;
#else
        ObjectShroudStatus status = obj != nullptr ? obj->Get_Shrouded_Status(index) : SHROUDED_NONE;
#endif

        if (status <= SHROUDED_TRANSITION) {
            draw->Draw_Icon_UI();

#ifdef GAME_DEBUG_STRUCTS
            if (g_theWriteableGlobalData->m_showCollisionExtents) {
                // TODO collision debug
            }

            if (g_theWriteableGlobalData->m_showAudioLocations) {
                // TODO audio debug
            }

            if (g_theWriteableGlobalData->m_showTerrainNormals) {
                // TODO terrain debug
            }
#endif

            g_theGameClient->Add_On_Screen_Object();
        }
    }
}

void W3DView::Draw()
{
    bool b = false;
    bool b2 = false;
    CustomScenePassModes mode = MODE_DEFAULT;
    bool b3 = false;

    if (m_viewFilterMode != FM_NULL_MODE) {
        if (m_viewFilter > FT_NULL_FILTER && m_viewFilter < FT_MAX) {
            b3 = W3DShaderManager::Filter_Pre_Render(m_viewFilter, b, mode);

            if (!b) {
                if (Get_Camera_Lock() != INVALID_OBJECT_ID) {
                    Object *obj = g_theGameLogic->Find_Object_By_ID(Get_Camera_Lock());

                    if (obj != nullptr) {
                        obj->Get_Drawable()->Set_Drawable_Hidden(true);
                    }
                }
            }
        }
    }

    if (!b) {
        W3DDisplay::s_3DScene->Set_Custom_Scene_Pass_Mode(mode);

        if (m_extraPass) {
            W3DDisplay::s_3DScene->Set_Extra_Pass_Polygon_Mode(SceneClass::EXTRA_PASS_CLEAR_LINE);
        }

        W3DDisplay::s_3DScene->Do_Render(m_3DCamera);
        W3DDisplay::s_3DScene->Set_Extra_Pass_Polygon_Mode(SceneClass::EXTRA_PASS_DISABLE);

        m_extraPass = m_wireframeMode;
    }

    if (m_viewFilterMode != FM_NULL_MODE && m_viewFilter > FT_NULL_FILTER && m_viewFilter < FT_MAX) {
        Coord2D screen_delta;
        Calc_Delta_Scroll(screen_delta);
        bool b4 = false;

        if (b3) {
            b4 = W3DShaderManager::Filter_Post_Render(m_viewFilter, m_viewFilterMode, screen_delta, b2);
        }

        if (!b) {
            if (Get_Camera_Lock() != INVALID_OBJECT_ID) {
                Object *obj = g_theGameLogic->Find_Object_By_ID(Get_Camera_Lock());

                if (obj != nullptr) {
                    Drawable *drawable = obj->Get_Drawable();
                    drawable->Set_Drawable_Hidden(false);
                    RenderInfoClass rinfo(*m_3DCamera);
                    m_3DCamera->Apply();
                    g_theDX8MeshRenderer.Set_Camera(&rinfo.m_camera);
                    W3DDisplay::s_3DScene->Render_Specific_Drawables(rinfo, 1, &drawable);
                    W3D::Flush(rinfo);
                }
            }
        }

        if (!b4) {
            m_viewFilter = FT_VIEW_SCREEN_DEFAULT_FILTER;
            m_viewFilterMode = FM_13;
        }
    }

    if (b2) {
        DX8Wrapper::Clear(false, true, Vector3(0.0f, 0.0f, 0.0f), g_theWaterTransparency->m_transparentWaterMinOpacity);
        W3DDisplay::s_3DScene->Set_Custom_Scene_Pass_Mode(MODE_DEFAULT);
        W3DDisplay::s_3DScene->Do_Render(m_3DCamera);

        Coord2D delta;
        W3DShaderManager::Filter_Post_Render(m_viewFilter, m_viewFilterMode, delta, b2);
    }

#ifdef GAME_DEBUG_STRUCTS
    if (g_theWriteableGlobalData->m_debugAI) {
        // TODO path debug
    }

    if (g_theWriteableGlobalData->m_cameraDebug) {
        // TODO camera debug
    }

    if (g_theWriteableGlobalData->m_showAudioLocations) {
        // TODO audio debug
    }
#endif

    Region3D axis_aligned_region;
    Get_Axis_Aligned_View_Region(axis_aligned_region);
    g_theGameClient->Reset_On_Screen_Object_Count();
    g_theGameClient->Iterate_Drawables_In_Region(&axis_aligned_region, Drawable_Post_Draw, this);
    g_theGameClient->Flush_Text_Bearing_Drawables();
    W3DDisplay::s_2DScene->Do_Render(m_2DCamera);
}

void W3DView::Set_Camera_Lock(ObjectID id)
{
    if (!g_theWriteableGlobalData->m_disableCameraMovements || id == INVALID_OBJECT_ID) {
        View::Set_Camera_Lock(id);
        m_doingScriptedCameraLock = false;
    }
}

void W3DView::Set_Snap_Mode(CameraLockType lock_type, float lock_dist)
{
    View::Set_Snap_Mode(lock_type, lock_dist);
    m_doingScriptedCameraLock = true;
}

void W3DView::Scroll_By(Coord2D *pos)
{
    if (pos && (pos->x != 0.0f || pos->y != 0.0f)) {
        float f1 = 250.0f;
        Vector3 world_view_transform;
        Vector3 world_view_origin;
        Vector3 world_view_dest;
        Vector2 local_view_origin;
        Vector2 local_view_dest;

        m_scrollAmount.x = pos->x;
        m_scrollAmount.y = pos->y;
        local_view_origin.X = Get_Width();
        local_view_origin.Y = Get_Height();

        float aspect_ratio = (Get_Width() / Get_Height());
        local_view_dest.X = f1 * pos->x + local_view_origin.X;
        local_view_dest.Y = f1 * pos->y * aspect_ratio + local_view_origin.Y;
        m_3DCamera->Device_To_World_Space(local_view_origin, &world_view_origin);
        m_3DCamera->Device_To_World_Space(local_view_dest, &world_view_dest);

        world_view_transform.X = world_view_dest.X - world_view_origin.X;
        world_view_transform.Y = world_view_dest.Y - world_view_origin.Y;
        world_view_transform.Z = world_view_dest.Z - world_view_origin.Z;
        Coord3D pos = Get_Position();
        pos.x += world_view_transform.X;
        pos.y += world_view_transform.Y;
        Set_Position(&pos);
        m_doingRotateCamera = false;
        Set_Camera_Transform();
    }
}

void W3DView::Force_Redraw()
{
    Set_Camera_Transform();
}

void W3DView::Set_Angle(float angle)
{
    Norm_Angle(&angle);
    View::Set_Angle(angle);
    m_doingMoveCameraOnWaypointPath = false;
    m_doingMoveCameraAlongWaypointPath = false;
    m_doingRotateCamera = false;
    m_doingPitchCamera = false;
    m_doingZoomCamera = false;
    m_doingScriptedCameraLock = false;
    Set_Camera_Transform();
}

void W3DView::Set_Pitch(float pitch)
{
    View::Set_Pitch(pitch);
    m_doingMoveCameraOnWaypointPath = false;
    m_doingRotateCamera = false;
    m_doingPitchCamera = false;
    m_doingZoomCamera = false;
    m_doingScriptedCameraLock = false;
    Set_Camera_Transform();
}

void W3DView::Set_Angle_And_Pitch_To_Default()
{
    View::Set_Angle_And_Pitch_To_Default();
    m_FXPitch = 1.0f;
    Set_Camera_Transform();
}

void W3DView::Set_Default_View(float pitch, float angle, float max_height)
{
    m_defaultPitchAngle = pitch;
    m_maxHeightAboveGround = max_height * g_theWriteableGlobalData->m_maxCameraHeight;

    if (m_minHeightAboveGround > m_maxHeightAboveGround) {
        m_maxHeightAboveGround = m_minHeightAboveGround;
    }
}

void W3DView::Set_Height_Above_Ground(float z)
{
    m_heightAboveGround = z;

    if (m_zoomLimited) {
        if (m_heightAboveGround < m_minHeightAboveGround) {
            m_heightAboveGround = m_minHeightAboveGround;
        }

        if (m_heightAboveGround > m_maxHeightAboveGround) {
            m_heightAboveGround = m_maxHeightAboveGround;
        }
    }

    m_doingMoveCameraOnWaypointPath = false;
    m_doingMoveCameraAlongWaypointPath = false;
    m_doingRotateCamera = false;
    m_doingPitchCamera = false;
    m_doingZoomCamera = false;
    m_doingScriptedCameraLock = false;
    m_cameraConstraintValid = false;
    Set_Camera_Transform();
}

void W3DView::Set_Zoom(float z)
{
    m_zoom = z;

    if (m_zoom < m_minZoom) {
        m_zoom = m_minZoom;
    }

    if (m_zoom > m_maxZoom) {
        m_zoom = m_maxZoom;
    }

    m_doingMoveCameraOnWaypointPath = false;
    m_doingMoveCameraAlongWaypointPath = false;
    m_doingRotateCamera = false;
    m_doingPitchCamera = false;
    m_doingZoomCamera = false;
    m_doingScriptedCameraLock = false;
    m_cameraConstraintValid = false;
    Set_Camera_Transform();
}

void W3DView::Set_Zoom_To_Default()
{
    float ground_height = Get_Height_Around_Pos(m_pos.x, m_pos.y);
    m_zoom = (ground_height + m_maxHeightAboveGround) / m_cameraOffset.z;
    m_heightAboveGround = m_maxHeightAboveGround;
    m_doingMoveCameraOnWaypointPath = false;
    m_doingMoveCameraAlongWaypointPath = false;
    m_doingRotateCamera = false;
    m_doingPitchCamera = false;
    m_doingZoomCamera = false;
    m_doingScriptedCameraLock = false;
    m_cameraConstraintValid = false;
    Set_Camera_Transform();
}

void W3DView::Set_Field_Of_View(float angle)
{
    View::Set_Field_Of_View(angle);
    Set_Camera_Transform();
}

void W3D_Logical_Screen_To_Pixel_Screen(
    float log_x, float log_y, int *screen_x, int *screen_y, int screen_width, int screen_height)
{
    *screen_x = GameMath::Fast_To_Int_Truncate(screen_width * (log_x + 1.0f) / 2.0f);
    *screen_y = GameMath::Fast_To_Int_Truncate(screen_height * (1.0f - log_y) / 2.0f);
}

int W3DView::World_To_Screen_Tri_Return(const Coord3D *w, ICoord2D *s)
{
    if (w == nullptr || s == nullptr) {
        return 2;
    }

    if (m_3DCamera == nullptr) {
        return 2;
    }

    Vector3 ws_point;
    Vector3 dest;
    ws_point.Set(w->x, w->y, w->z);
    CameraClass::ProjectionResType res = m_3DCamera->Project(dest, ws_point);

    if (res < CameraClass::OUTSIDE_NEAR_CLIP) {
        W3D_Logical_Screen_To_Pixel_Screen(dest.X, dest.Y, &s->x, &s->y, Get_Width(), Get_Height());
        s->x += m_originX;
        s->y += m_originY;
        return res != CameraClass::INSIDE_FRUSTUM ? 1 : 0;
    } else {
        s->x = 0;
        s->y = 0;
        return 2;
    }
}

void W3DView::Screen_To_World(const ICoord2D *s, Coord3D *w)
{
    captainslog_dbgassert(false, "implement me");
}

void W3DView::Screen_To_Terrain(const ICoord2D *screen, Coord3D *world)
{
    if (screen != nullptr && world != nullptr && g_theTerrainRenderObject != nullptr) {
        if (m_cameraHasMovedSinceRequest) {
            m_locationRequests.clear();
            m_cameraHasMovedSinceRequest = false;
        }

        if (m_locationRequests.size() > 40) {
            m_locationRequests.erase(m_locationRequests.begin(), m_locationRequests.begin() + 10);
        }

        for (int i = m_locationRequests.size() - 1; i >= 0; i--) {
            if (m_locationRequests[i].first.x == screen->x && m_locationRequests[i].first.y == screen->y) {
                *world = m_locationRequests[i].second;
                return;
            }
        }

        Vector3 ray_start;
        Vector3 ray_end;
        LineSegClass line;
        CastResultStruct result;
        Vector3 collision_pos(0.0f, 0.0f, 0.0f);
        Get_Pick_Ray(screen, &ray_start, &ray_end);
        line.Set(ray_start, ray_end);
        RayCollisionTestClass ray(line, &result);

        if (g_theTerrainRenderObject->Cast_Ray(ray)) {
            collision_pos = result.contact_point;
        }

        Vector3 bridge_pos;

        if (g_theTerrainLogic->Pick_Bridge(ray_start, ray_end, &bridge_pos) && bridge_pos.Z > collision_pos.Z) {
            collision_pos = bridge_pos;
        }

        world->x = collision_pos.X;
        world->y = collision_pos.Y;
        world->z = collision_pos.Z;
        m_locationRequests.push_back(std::make_pair(*screen, *world));
    }
}

void W3DView::Look_At(const Coord3D *pos)
{
    Coord3D c = *pos;

    if (g_theTerrainLogic->Get_Ground_Height(c.x, c.y, nullptr) + 10.0f < pos->z) {
        Vector3 v1;
        Vector3 v2;
        LineSegClass line;
        CastResultStruct result;
        Vector3 v3(0.0f, 0.0f, 0.0f);
        v1 = m_3DCamera->Get_Position();
        Vector2 v4(0.0f, 0.0f);
        m_3DCamera->Un_Project(v2, v4);
        v2 -= v1;
        v2.Normalize();
        v2 *= m_3DCamera->Get_Depth();
        v1.Set(c.x, c.y, c.z);
        v2 += v1;
        line.Set(v1, v2);
        RayCollisionTestClass ray(line, &result);

        if (g_theTerrainRenderObject->Cast_Ray(ray)) {
            c.x = result.contact_point.X;
            c.y = result.contact_point.Y;
        }
    }

    c.z = 0.0f;
    Set_Position(&c);
    m_doingRotateCamera = false;
    m_doingMoveCameraOnWaypointPath = false;
    m_doingMoveCameraAlongWaypointPath = false;
    m_doingScriptedCameraLock = false;
    Set_Camera_Transform();
}

void W3DView::Init_Height_For_Map()
{
    m_groundLevel = g_theTerrainLogic->Get_Ground_Height(m_pos.x, m_pos.y, nullptr);

    if (m_groundLevel > 120.0f) {
        m_groundLevel = 120.0f;
    }

    m_cameraOffset.z = m_groundLevel + g_theWriteableGlobalData->m_cameraHeight;
    m_cameraOffset.y = -(m_cameraOffset.z / GameMath::Tan(g_theWriteableGlobalData->m_cameraPitch * 0.01745329300562541f));
    m_cameraOffset.x = -(GameMath::Tan(g_theWriteableGlobalData->m_cameraYaw * 0.01745329300562541f) * m_cameraOffset.y);
    m_cameraConstraintValid = false;
    Set_Camera_Transform();
}

void W3DView::Move_Camera_To(const Coord3D *o, int frames, int shutter, bool orient, float in, float out)
{
    m_mcwpInfo.waypoints[0] = Get_Position();
    m_mcwpInfo.camera_angle[0] = Get_Angle();
    m_mcwpInfo.way_seg_length[0] = 0.0f;
    m_mcwpInfo.waypoints[1] = Get_Position();
    m_mcwpInfo.way_seg_length[1] = 0.0f;
    m_mcwpInfo.waypoints[2] = *o;
    m_mcwpInfo.way_seg_length[2] = 0.0f;
    m_mcwpInfo.num_waypoints = 2;

    if (frames < 1) {
        frames = 1;
    }

    m_mcwpInfo.total_time_milliseconds = frames;
    m_mcwpInfo.shutter = 1;

    m_mcwpInfo.ease.Set_Ease_Times(in / frames, out / frames);
    m_mcwpInfo.cur_segment = 1;
    m_mcwpInfo.cur_seg_distance = 0.0f;
    m_mcwpInfo.total_distance = 0.0f;
    Setup_Waypoint_Path(orient);

    if (m_mcwpInfo.total_time_milliseconds == 1) {
        Move_Along_Waypoint_Path(1);
        m_doingMoveCameraOnWaypointPath = true;
        m_doingMoveCameraAlongWaypointPath = false;
    }
}

void W3DView::Setup_Waypoint_Path(bool orient)
{
    m_mcwpInfo.cur_segment = 1;
    m_mcwpInfo.cur_seg_distance = 0.0f;
    m_mcwpInfo.total_distance = 0;
    m_mcwpInfo.rolling_average_frames = 1;
    float angle = Get_Angle();

    for (int i = 1; i < m_mcwpInfo.num_waypoints; i++) {
        Vector2 v(m_mcwpInfo.waypoints[i + 1].x - m_mcwpInfo.waypoints[i].x,
            m_mcwpInfo.waypoints[i + 1].y - m_mcwpInfo.waypoints[i].y);
        m_mcwpInfo.way_seg_length[i] = v.Length();
        m_mcwpInfo.total_distance += m_mcwpInfo.way_seg_length[i];

        if (orient) {
            angle = GameMath::Acos(v.X / m_mcwpInfo.way_seg_length[i]);

            if (v.Y < 0.0f) {
                angle = -angle;
            }

            angle -= DEG_TO_RADF(90.0f);
            Norm_Angle(&angle);
        }

        m_mcwpInfo.camera_angle[i] = angle;
    }

    m_mcwpInfo.camera_angle[1] = Get_Angle();
    m_mcwpInfo.camera_angle[m_mcwpInfo.num_waypoints] = m_mcwpInfo.camera_angle[m_mcwpInfo.num_waypoints - 1];

    for (int i = m_mcwpInfo.num_waypoints - 1; i > 1; i--) {
        m_mcwpInfo.camera_angle[i] = (m_mcwpInfo.camera_angle[i] + m_mcwpInfo.camera_angle[i - 1]) / 2.0f;
    }

    m_mcwpInfo.way_seg_length[m_mcwpInfo.num_waypoints + 1] = m_mcwpInfo.way_seg_length[m_mcwpInfo.num_waypoints];

    if (m_mcwpInfo.total_distance < 1.0f) {
        m_mcwpInfo.way_seg_length[m_mcwpInfo.num_waypoints - 1] += 1.0f - m_mcwpInfo.total_distance;
        m_mcwpInfo.total_distance = 1.0f;
    }

    float total_way_seg_length = 0.0f;
    Coord3D wp = m_mcwpInfo.waypoints[m_mcwpInfo.num_waypoints];
    float ground_height = g_theTerrainLogic->Get_Ground_Height(wp.x, wp.y, nullptr);

    for (int i = 0; i <= m_mcwpInfo.num_waypoints + 1; i++) {
        float f2 = total_way_seg_length / m_mcwpInfo.total_distance;
        float f3 = 1.0f - total_way_seg_length / m_mcwpInfo.total_distance;
        m_mcwpInfo.time_multiplier[i] = m_timeMultiplier;
        m_mcwpInfo.ground_height[i] = f3 * m_groundLevel + ground_height * f2;
        total_way_seg_length += m_mcwpInfo.way_seg_length[i];
    }

    m_mcwpInfo.waypoints[m_mcwpInfo.num_waypoints + 1] = m_mcwpInfo.waypoints[m_mcwpInfo.num_waypoints];
    Coord3D waypoints1 = m_mcwpInfo.waypoints[m_mcwpInfo.num_waypoints];
    Coord3D waypoints2 = m_mcwpInfo.waypoints[m_mcwpInfo.num_waypoints - 1];
    m_mcwpInfo.waypoints[m_mcwpInfo.num_waypoints + 1].x += waypoints1.x - waypoints2.x;
    m_mcwpInfo.waypoints[m_mcwpInfo.num_waypoints + 1].y += waypoints1.y - waypoints2.y;
    m_mcwpInfo.camera_angle[m_mcwpInfo.num_waypoints + 1] = m_mcwpInfo.camera_angle[m_mcwpInfo.num_waypoints];
    m_mcwpInfo.ground_height[m_mcwpInfo.num_waypoints + 1] = ground_height;
    waypoints1 = m_mcwpInfo.waypoints[2];
    waypoints2 = m_mcwpInfo.waypoints[1];
    m_mcwpInfo.waypoints[0].x -= waypoints1.x - waypoints2.x;
    m_mcwpInfo.waypoints[0].y -= waypoints1.y - waypoints2.y;
    m_doingMoveCameraOnWaypointPath = m_mcwpInfo.num_waypoints > 1;
    m_doingMoveCameraAlongWaypointPath = false;
    m_doingRotateCamera = false;
    m_mcwpInfo.elapsed_time_milliseconds = 0;
    m_mcwpInfo.cur_shutter = m_mcwpInfo.shutter;
}

void W3DView::Rotate_Camera(float rotations, int frames, float in, float out)
{
    m_rcInfo.num_hold_frames = 0;
    m_rcInfo.track_object = 0;

    if (frames < 1) {
        frames = 1;
    }

    m_rcInfo.num_frames = frames / g_theW3DFrameLengthInMsec;

    if (m_rcInfo.num_frames < 1) {
        m_rcInfo.num_frames = 1;
    }

    m_rcInfo.cur_frame = 0;
    m_doingRotateCamera = true;
    m_rcInfo.start_angle = m_angle;
    m_rcInfo.end_angle = 6.2831855f * rotations + m_angle;
    m_rcInfo.start_time_multiplier = m_timeMultiplier;
    m_rcInfo.end_time_multiplier = m_timeMultiplier;
    m_rcInfo.ease.Set_Ease_Times(in / frames, out / frames);
    m_doingMoveCameraOnWaypointPath = false;
    m_doingMoveCameraAlongWaypointPath = false;
}

bool W3DView::Is_Camera_Movement_At_Waypoint_Along_Path()
{
    bool b = m_doingMoveCameraAlongWaypointPath;
    m_doingMoveCameraAlongWaypointPath = false;
    return b;
}

void W3DView::Move_Camera_Along_Waypoint_Path(Waypoint *way, int frames, int shutter, bool orient, float in, float out)
{
    float way_len_threshold = 10.0f;
    m_mcwpInfo.waypoints[0] = Get_Position();
    m_mcwpInfo.camera_angle[0] = Get_Angle();
    m_mcwpInfo.way_seg_length[0] = 0.0f;
    m_mcwpInfo.waypoints[1] = Get_Position();
    m_mcwpInfo.num_waypoints = 1;

    if (frames < 1) {
        frames = 1;
    }

    m_mcwpInfo.total_time_milliseconds = frames;
    m_mcwpInfo.shutter = shutter / g_theW3DFrameLengthInMsec;

    if (m_mcwpInfo.shutter < 1) {
        m_mcwpInfo.shutter = 1;
    }

    m_rcInfo.ease.Set_Ease_Times(in / frames, out / frames);

    while (way != nullptr && m_mcwpInfo.num_waypoints < 25) {
        m_mcwpInfo.num_waypoints++;
        m_mcwpInfo.waypoints[m_mcwpInfo.num_waypoints] = *way->Get_Location();

        if (way->Get_Num_Links() <= 0) {
            way = nullptr;
        } else {
            way = way->Get_Link(0);
        }

        Vector2 v1(m_mcwpInfo.waypoints[m_mcwpInfo.num_waypoints].x - m_mcwpInfo.waypoints[m_mcwpInfo.num_waypoints - 1].x,
            m_mcwpInfo.waypoints[m_mcwpInfo.num_waypoints].y - m_mcwpInfo.waypoints[m_mcwpInfo.num_waypoints - 1].y);

        if (v1.Length() < way_len_threshold) {
            if (way != nullptr) {
                m_mcwpInfo.num_waypoints--;
            } else {
                m_mcwpInfo.waypoints[m_mcwpInfo.num_waypoints - 1] = m_mcwpInfo.waypoints[m_mcwpInfo.num_waypoints];
                m_mcwpInfo.num_waypoints--;
            }
        }
    }

    Setup_Waypoint_Path(orient);
}

void W3DView::Rotate_Camera_Toward_Object(ObjectID id, int milliseconds, int hold_milliseconds, float in, float out)
{
    m_rcInfo.track_object = true;

    if (hold_milliseconds < 1) {
        hold_milliseconds = 0;
    }

    m_rcInfo.num_hold_frames = hold_milliseconds / g_theW3DFrameLengthInMsec;

    if (m_rcInfo.num_hold_frames < 1) {
        m_rcInfo.num_hold_frames = 0;
    }

    if (milliseconds < 1) {
        milliseconds = 1;
    }

    m_rcInfo.num_frames = milliseconds / g_theW3DFrameLengthInMsec;

    if (m_rcInfo.num_frames < 1) {
        m_rcInfo.num_frames = 1;
    }

    m_rcInfo.cur_frame = 0;
    m_doingRotateCamera = true;
    m_rcInfo.target_object_id = id;
    m_rcInfo.start_time_multiplier = m_timeMultiplier;
    m_rcInfo.end_time_multiplier = m_timeMultiplier;
    m_rcInfo.ease.Set_Ease_Times(in / milliseconds, out / milliseconds);
    m_doingMoveCameraOnWaypointPath = false;
    m_doingMoveCameraAlongWaypointPath = false;
}

void W3DView::Rotate_Camera_Toward_Position(const Coord3D *p_loc, int milliseconds, float in, float out, bool b)
{
    m_rcInfo.num_hold_frames = 0;
    m_rcInfo.track_object = false;

    if (milliseconds < 1) {
        milliseconds = 1;
    }

    m_rcInfo.num_frames = milliseconds / g_theW3DFrameLengthInMsec;

    if (m_rcInfo.num_frames < 1) {
        m_rcInfo.num_frames = 1;
    }

    Coord3D pos = Get_Position();
    Vector2 transform(p_loc->x - pos.x, p_loc->y - pos.y);
    float length = transform.Length();

    if (length >= 0.1f) {
        float angle = GameMath::Acos(transform.X / length);

        if (transform.Y < 0.0f) {
            angle = -angle;
        }

        angle -= DEG_TO_RADF(90.0f);
        Norm_Angle(&angle);

        if (b) {
            if (m_angle >= angle) {
                angle += 6.2831855f;
            } else {
                angle -= 6.2831855f;
            }
        }

        m_rcInfo.cur_frame = 0;
        m_doingRotateCamera = true;
        m_rcInfo.start_angle = m_angle;
        m_rcInfo.end_angle = angle;
        m_rcInfo.start_time_multiplier = m_timeMultiplier;
        m_rcInfo.end_time_multiplier = m_timeMultiplier;
        m_rcInfo.ease.Set_Ease_Times(in / milliseconds, out / milliseconds);
        m_doingMoveCameraOnWaypointPath = false;
        m_doingMoveCameraAlongWaypointPath = false;
    }
}

void W3DView::Zoom_Camera(float final_zoom, int milliseconds, float in, float out)
{
    if (milliseconds < 1) {
        milliseconds = 1;
    }

    m_zcInfo.num_frames = milliseconds / g_theW3DFrameLengthInMsec;

    if (m_zcInfo.num_frames < 1) {
        m_zcInfo.num_frames = 1;
    }

    m_zcInfo.cur_frame = 0;
    m_doingZoomCamera = true;
    m_zcInfo.start_zoom = m_zoom;
    m_zcInfo.end_zoom = final_zoom;
    m_zcInfo.ease.Set_Ease_Times(in / milliseconds, out / milliseconds);
}

void W3DView::Pitch_Camera(float final_pitch, int milliseconds, float in, float out)
{
    if (milliseconds < 1) {
        milliseconds = 1;
    }

    m_pcInfo.num_frames = milliseconds / g_theW3DFrameLengthInMsec;
    if (m_pcInfo.num_frames < 1) {
        m_pcInfo.num_frames = 1;
    }

    m_pcInfo.cur_frame = 0;
    m_doingPitchCamera = true;
    m_pcInfo.start_pitch = m_FXPitch;
    m_pcInfo.end_pitch = final_pitch;
    m_pcInfo.ease.Set_Ease_Times(in / milliseconds, out / milliseconds);
}

void W3DView::Camera_Mod_Final_Zoom(float final_zoom, float in, float out)
{
    if (m_doingRotateCamera) {
        float ground_height = Get_Height_Around_Pos(m_pos.x, m_pos.y);
        float zoom = (ground_height + m_maxHeightAboveGround) / m_cameraOffset.z;
        float milliseconds =
            g_theW3DFrameLengthInMsec * (m_rcInfo.num_hold_frames + m_rcInfo.num_frames - m_rcInfo.cur_frame);
        Zoom_Camera(final_zoom * zoom, milliseconds, milliseconds * in, milliseconds * out);
    }

    if (m_doingMoveCameraOnWaypointPath) {
        float ground_height = Get_Height_Around_Pos(
            m_mcwpInfo.waypoints[m_mcwpInfo.num_waypoints].x, m_mcwpInfo.waypoints[m_mcwpInfo.num_waypoints].y);
        float zoom = (ground_height + m_maxHeightAboveGround) / m_cameraOffset.z;
        float milliseconds = m_mcwpInfo.total_time_milliseconds - m_mcwpInfo.elapsed_time_milliseconds;
        Zoom_Camera(final_zoom * zoom, milliseconds, milliseconds * in, milliseconds * out);
    }
}

void W3DView::Camera_Mod_Freeze_Angle()
{
    if (m_doingRotateCamera) {
        if (m_rcInfo.track_object != INVALID_OBJECT_ID) {
            m_rcInfo.track_object = INVALID_OBJECT_ID;
        } else {
            m_rcInfo.end_angle = m_angle;
            m_rcInfo.start_angle = m_rcInfo.end_angle;
        }
    }

    if (m_doingMoveCameraOnWaypointPath) {
        for (int i = 0; i < m_mcwpInfo.num_waypoints; i++) {
            m_mcwpInfo.camera_angle[i + 1] = m_mcwpInfo.camera_angle[0];
        }
    }
}

void W3DView::Camera_Mod_Final_Look_Toward(Coord3D *p_loc)
{
    if (!m_doingRotateCamera && m_doingMoveCameraOnWaypointPath) {
        for (int i = 2; i <= m_mcwpInfo.num_waypoints; i++) {
            float x1 = (m_mcwpInfo.waypoints[i - 1].x + m_mcwpInfo.waypoints[i].x) / 2.0f;
            float y1 = (m_mcwpInfo.waypoints[i - 1].y + m_mcwpInfo.waypoints[i].y) / 2.0f;
            Coord3D c1 = m_mcwpInfo.waypoints[i];
            Coord3D c2 = m_mcwpInfo.waypoints[i];
            c2.x += m_mcwpInfo.waypoints[i + 1].x;
            c2.y += m_mcwpInfo.waypoints[i + 1].y;
            c2.x /= 2.0f;
            c2.y /= 2.0f;
            float x2 = (c2.x - x1) * 0.5f + x1;
            float y2 = (c2.y - y1) * 0.5f + y1;
            x2 += (1.0f - 0.5f) * 0.5f * (c1.x - c2.x + c1.x - x1);
            y2 += (1.0f - 0.5f) * 0.5f * (c1.y - c2.y + c1.y - y1);
            Vector2 transform(p_loc->x - x2, p_loc->y - y2);
            float length = transform.Length();

            if (length >= 0.1f) {
                float angle = GameMath::Acos(transform.X / length);

                if (transform.Y < 0.0f) {
                    angle = -angle;
                }

                angle -= DEG_TO_RADF(90.0f);
                Norm_Angle(&angle);
                m_mcwpInfo.camera_angle[i] = angle;
            }
        }

        if (m_mcwpInfo.total_time_milliseconds == 1) {
            Move_Along_Waypoint_Path(1);
            m_doingMoveCameraOnWaypointPath = true;
            m_doingMoveCameraAlongWaypointPath = false;
        }
    }
}

void W3DView::Camera_Mod_Final_Move_To(Coord3D *p_loc)
{
    if (!m_doingRotateCamera && m_doingMoveCameraOnWaypointPath) {
        float x = p_loc->x - m_mcwpInfo.waypoints[m_mcwpInfo.num_waypoints].x;
        float y = p_loc->y - m_mcwpInfo.waypoints[m_mcwpInfo.num_waypoints].y;

        for (int i = 2; i <= m_mcwpInfo.num_waypoints; i++) {
            m_mcwpInfo.waypoints[i].x += x;
            m_mcwpInfo.waypoints[i].y += y;
        }
    }
}

void W3DView::Camera_Mod_Look_Toward(Coord3D *p_loc)
{
    if (!m_doingRotateCamera && m_doingMoveCameraOnWaypointPath) {
        int count = m_mcwpInfo.num_waypoints - 1;

        if (count < 2) {
            count = 2;
        }

        for (int i = count; i <= m_mcwpInfo.num_waypoints; i++) {
            Coord3D c1 = m_mcwpInfo.waypoints[i - 1];
            c1.x += m_mcwpInfo.waypoints[i].x;
            c1.y += m_mcwpInfo.waypoints[i].y;
            c1.x /= 2.0f;
            c1.y /= 2.0f;

            Coord3D c2 = m_mcwpInfo.waypoints[i];
            Coord3D c3 = m_mcwpInfo.waypoints[i];
            c3.x += m_mcwpInfo.waypoints[i + 1].x;
            c3.y += m_mcwpInfo.waypoints[i + 1].y;
            c3.x /= 2.0f;
            c3.y /= 2.0f;

            float x = (c3.x - c1.x) * 0.5f + c1.x;
            float y = (c3.y - c1.y) * 0.5f + c1.y;
            x += (1.0f - 0.5f) * 0.5f * (c2.x - c3.x + c2.x - c1.x);
            y += (1.0f - 0.5f) * 0.5f * (c2.y - c3.y + c2.y - c1.y);
            Vector2 transform(p_loc->x - x, p_loc->y - y);
            float length = transform.Length();

            if (length >= 0.1f) {
                float angle = GameMath::Acos(transform.X / length);

                if (transform.Y < 0.0f) {
                    angle = -angle;
                }

                angle -= DEG_TO_RADF(90.0f);
                Norm_Angle(&angle);

                if (i != m_mcwpInfo.num_waypoints) {
                    float angle2 = angle - m_mcwpInfo.camera_angle[i];
                    Norm_Angle(&angle2);
                    angle = angle2 / 2.0f + m_mcwpInfo.camera_angle[i];
                    Norm_Angle(&angle);
                }

                m_mcwpInfo.camera_angle[i] = angle;
            }
        }
    }
}

void W3DView::Camera_Mod_Final_Time_Multiplier(int final_multiplier)
{
    if (m_doingZoomCamera) {
        m_zcInfo.end_time_multiplier = final_multiplier;
    }

    if (m_doingPitchCamera) {
        m_pcInfo.end_time_multiplier = final_multiplier;
    }

    if (m_doingRotateCamera) {
        m_rcInfo.end_time_multiplier = final_multiplier;
    } else if (m_doingMoveCameraOnWaypointPath) {
        float total_length = 0.0f;

        for (int i = 0; i < m_mcwpInfo.num_waypoints; i++) {
            total_length += m_mcwpInfo.way_seg_length[i];
            float f1 = total_length / m_mcwpInfo.total_distance;
            float f2 = 1.0f - f1;
            float f3 = m_mcwpInfo.time_multiplier[i + 1] * f2 + 0.5f + final_multiplier * f1;
            m_mcwpInfo.time_multiplier[i + 1] = GameMath::Fast_To_Int_Floor(f3);
        }
    } else {
        m_timeMultiplier = final_multiplier;
    }
}

void W3DView::Camera_Mod_Rolling_Average(int frames_to_average)
{
    if (frames_to_average < 1) {
        frames_to_average = 1;
    }

    m_mcwpInfo.rolling_average_frames = frames_to_average;
}

void W3DView::Camera_Mod_Final_Pitch(float final_pitch, float in, float out)
{
    if (m_doingRotateCamera) {
        float milliseconds =
            g_theW3DFrameLengthInMsec * (m_rcInfo.num_hold_frames + m_rcInfo.num_frames - m_rcInfo.cur_frame);
        Pitch_Camera(final_pitch, milliseconds, milliseconds * in, milliseconds * out);
    }

    if (m_doingMoveCameraOnWaypointPath) {
        float milliseconds = m_mcwpInfo.total_time_milliseconds - m_mcwpInfo.elapsed_time_milliseconds;
        Pitch_Camera(final_pitch, milliseconds, milliseconds * in, milliseconds * out);
    }
}

void W3DView::Reset_Camera(const Coord3D *location, int frames, float in, float out)
{
    Move_Camera_To(location, frames, 0, false, in, out);
    m_mcwpInfo.camera_angle[2] = 0.0f;
    m_angle = m_mcwpInfo.camera_angle[0];
    float ground_height = Get_Height_Around_Pos(location->x, location->y);
    float zoom = (ground_height + m_maxHeightAboveGround) / m_cameraOffset.z;
    Zoom_Camera(zoom, frames, in, out);
    Pitch_Camera(1.0f, frames, in, out);
}

bool W3DView::Is_Camera_Movement_Finished()
{
    if (m_viewFilter == FT_VIEW_MOTION_BLUR_FILTER
        && (m_viewFilterMode == FM_VIEW_MB_IN_AND_OUT_ALPHA || m_viewFilterMode == FM_VIEW_MB_IN_AND_OUT_STATURATE
            || m_viewFilterMode == FM_8 || m_viewFilterMode == FM_VIEW_MB_OUT_ALPHA
            || m_viewFilterMode == FM_VIEW_MB_IN_STATURATE || m_viewFilterMode == FM_VIEW_MB_OUT_STATURATE)) {
        return true;
    }

    return !m_doingMoveCameraOnWaypointPath && !m_doingRotateCamera && !m_doingPitchCamera && !m_doingZoomCamera;
}

void W3DView::Shake(const Coord3D *epicenter, CameraShakeType shake_type)
{
    float angle = Get_Client_Random_Value_Real(0.0f, 6.2831855f);
    m_shakeAngleCos = GameMath::Cos(angle);
    m_shakeAngleSin = GameMath::Sin(angle);
    float intensity = 0.0f;

    switch (shake_type) {
        case SHAKE_SUBTLE:
            intensity = g_theWriteableGlobalData->m_shakeSubtleIntensity;
            break;
        case SHAKE_NORMAL:
            intensity = g_theWriteableGlobalData->m_shakeNormalIntensity;
            break;
        case SHAKE_STRONG:
            intensity = g_theWriteableGlobalData->m_shakeStrongIntensity;
            break;
        case SHAKE_SEVERE:
            intensity = g_theWriteableGlobalData->m_shakeSevereIntensity;
            break;
        case SHAKE_EXTREME:
            intensity = g_theWriteableGlobalData->m_shakeCineExtremeIntensity;
            break;
        case SHAKE_INSANE:
            intensity = g_theWriteableGlobalData->m_shakeCineInsaneIntensity;
            break;
        default:
            break;
    }

    Coord3D &pos = Get_Position();
    float x = epicenter->x - pos.x;
    float y = epicenter->y - pos.y;
    float range = GameMath::Sqrt(x * x + y * y);

    if (range <= g_theWriteableGlobalData->m_maxShakeRange) {
        m_shakeIntensity += (1.0f - range / g_theWriteableGlobalData->m_maxShakeRange) * intensity;

        if (m_shakeIntensity > g_theWriteableGlobalData->m_maxShakeIntensity) {
            m_shakeIntensity = 3.0f;
        }
    }
}

void W3DView::Screen_To_World_At_Z(const ICoord2D *s, Coord3D *w, float z)
{
    Vector3 ray_start;
    Vector3 ray_end;
    Get_Pick_Ray(s, &ray_start, &ray_end);
    w->x = Vector3::Find_X_At_Z(z, ray_start, ray_end);
    w->y = Vector3::Find_Y_At_Z(z, ray_start, ray_end);
    w->z = z;
}

void W3DView::Camera_Enable_Slave_Mode(const Utf8String &thing, const Utf8String &bone)
{
    m_cameraSlaveMode = true;
    m_cameraSlaveThing = thing;
    m_cameraSlaveBone = bone;
}

void W3DView::Camera_Disable_Slave_Mode()
{
    m_cameraSlaveMode = false;
}

void W3DView::Camera_Enable_Real_Zoom_Mode()
{
    m_realZoom = true;
    m_FXPitch = 1.0f;
    Update_View();
}

void W3DView::Camera_Disable_Real_Zoom_Mode()
{
    m_realZoom = false;
    m_FXPitch = 1.0f;
    m_FOV = 0.87266463f;
    Set_Camera_Transform();
    Update_View();
}

void W3DView::Add_Camera_Shake(const Coord3D &position, float radius, float duration, float amplitude)
{
    Vector3 pos;
    pos.X = position.x;
    pos.Y = position.y;
    pos.Z = position.z;
    g_theCameraShakerSystem.Add_Camera_Shake(pos, radius, duration, amplitude);
}

void W3DView::Camera_Mod_Freeze_Time()
{
    m_freezeTimeForCameraMovement = true;
}

bool W3DView::Is_Time_Frozen()
{
    return m_freezeTimeForCameraMovement;
}

int W3DView::Get_Time_Multiplier()
{
    return m_timeMultiplier;
}

void W3DView::Set_Time_Multiplier(int multiple)
{
    m_timeMultiplier = multiple;
}

float W3DView::Get_FX_Pitch()
{
    return m_FXPitch;
}

FilterModes W3DView::Get_View_Filter_Mode()
{
    return m_viewFilterMode;
}

FilterTypes W3DView::Get_View_Filter_Type()
{
    return m_viewFilter;
}

void W3DView::Force_Camera_Constraint_Recalc()
{
    Calc_Camera_Constraints();
}

void W3DView::Set_Guard_Band_Bias(Coord2D *bias)
{
    m_guardBandBias = *bias;
}
