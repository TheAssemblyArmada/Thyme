/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View graphic view
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "graphicview.h"
#include "assetmgr.h"
#include "camera.h"
#include "light.h"
#include "mainfrm.h"
#include "mpu.h"
#include "quaternion.h"
#include "rcfile.h"
#include "resource.h"
#include "utils.h"
#include "viewerscene.h"
#include "w3d.h"
#include "w3dview.h"
#include "w3dviewdoc.h"

extern int DeviceWidth;
extern int DeviceHeight;
extern int BPP;
extern int Device;

Vector3 Pos;
float Zoom;
Quaternion Rotation;
float Lightradius = 1.0f;

IMPLEMENT_DYNCREATE(CGraphicView, CView)

// clang-format off
BEGIN_MESSAGE_MAP(CGraphicView, CView)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_DESTROY()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_RBUTTONUP()
    ON_WM_RBUTTONDOWN()
    ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()
// clang-format on

CGraphicView::CGraphicView() :
    m_isInitialized(FALSE),
    m_isActive(TRUE),
    m_timer(0),
    m_camera(nullptr),
    m_light(nullptr),
    m_lightInScene(false),
    m_objCenter(0.0f, 0.0f, 0.0f),
    m_leftButtonDown(FALSE),
    m_rightButtonDown(FALSE),
    m_renderingDisabled(0),
    m_lastCountUpdate(0),
    m_plusXCamera(FALSE),
    m_time(0),
    m_objectRotationFlags(0),
    m_lightRotationFlags(0),
    m_cameraRotateConstraints(0),
    m_isWindowed(TRUE),
    m_animationSpeed(1.0f),
    m_animationPlaying(-1)
{
    m_isWindowed = atoi(theApp.GetProfileString("Config", "Windowed", "1"));
}

LRESULT CGraphicView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == GVM_RENDER) {
        Render(TRUE, 0);
        RemoveProp(m_hWnd, "WaitingToProcess");
        return CWnd::WindowProc(message, wParam, lParam);
    }

    if (message != WM_PAINT) {
        if (message == WM_KEYDOWN) {
            if (wParam == VK_CONTROL && !m_lightInScene) {
                m_light->Add(((CW3DViewDoc *)m_pDocument)->m_scene);
                m_lightInScene = true;
            }
        } else if (message == WM_KEYUP && wParam == VK_CONTROL && m_lightInScene) {
            m_light->Remove();
            m_lightInScene = false;
        }

        return CWnd::WindowProc(message, wParam, lParam);
    }

    if (!m_isWindowed) {
        RECT r;
        GetClientRect(&r);
        HDC dc = ::GetDC(m_hWnd);

        if (dc != nullptr) {
            FillRect(dc, &r, (HBRUSH)COLOR_WINDOWFRAME);
            ::ReleaseDC(m_hWnd, dc);
        }
    }

    Render(FALSE, 0);
    ValidateRect(nullptr);
    return 0;
}

void CGraphicView::OnInitialUpdate()
{
    CView::OnInitialUpdate();
    CW3DViewDoc *document = ((CW3DViewDoc *)m_pDocument);

    if (document != nullptr) {
        document->Initialize();
    }
}

int CGraphicView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    int result = CView::OnCreate(lpCreateStruct);

    if (result != -1) {
        m_time = timeGetTime();
        return 0;
    }

    return result;
}

void CGraphicView::OnSize(UINT nType, int cx, int cy)
{
    CWnd::Default();

    if (m_isInitialized) {
        int width;
        int height;

        if (m_isWindowed) {
            height = cy;
            width = cx;
        } else {
            width = DeviceWidth;
            height = DeviceHeight;
        }

        if (m_isWindowed) {
            W3D::Set_Device_Resolution(width, height, BPP, true, false);
        }

        UpdateCamera();
        Render(TRUE, 0);
    }
}

void CGraphicView::OnDestroy()
{
    CView::OnDestroy();
    Ref_Ptr_Release(m_camera);
    Ref_Ptr_Release(m_light);

    if (!m_timer) {
        timeKillEvent(0);
        m_timer = 0;
    }

    char str[12];
    _itoa(m_isWindowed, str, 10);
    theApp.WriteProfileString("Config", "Windowed", str);
    m_isInitialized = FALSE;
}

void CGraphicView::OnLButtonDown(UINT nFlags, CPoint point)
{
    SetCapture();
    m_mousePos = point;
    m_leftButtonDown = TRUE;

    if (m_rightButtonDown) {
        SetCursor(LoadCursor(AfxGetResourceHandle(), MAKEINTRESOURCE(IDC_MOVE)));
    }

    CWnd::Default();
}

void CGraphicView::OnLButtonUp(UINT nFlags, CPoint point)
{
    if (!m_rightButtonDown) {
        ReleaseCapture();
    }

    m_leftButtonDown = FALSE;

    if (m_rightButtonDown) {
        SetCursor(LoadCursor(AfxGetResourceHandle(), MAKEINTRESOURCE(IDC_ZOOM)));
    } else {
        SetCursor(LoadCursor(nullptr, IDC_ARROW));
    }

    CWnd::Default();
}

void CGraphicView::OnMouseMove(UINT nFlags, CPoint point)
{
    // TODO
}

void CGraphicView::OnRButtonUp(UINT nFlags, CPoint point)
{
    m_rightButtonDown = FALSE;

    if (!m_leftButtonDown) {
        SetCursor(LoadCursor(nullptr, IDC_ARROW));
    }

    CWnd::Default();
}

void CGraphicView::OnRButtonDown(UINT nFlags, CPoint point)
{
    SetCapture();
    m_mousePos = point;
    m_rightButtonDown = TRUE;

    if (m_leftButtonDown) {
        SetCursor(LoadCursor(AfxGetResourceHandle(), MAKEINTRESOURCE(IDC_MOVE)));
    } else {
        SetCursor(LoadCursor(AfxGetResourceHandle(), MAKEINTRESOURCE(IDC_ZOOM)));
    }

    CWnd::Default();
}

void CGraphicView::OnGetMinMaxInfo(MINMAXINFO FAR *lpMMI)
{
    CWnd::Default();
}

static void CALLBACK TimerProc(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    if (dwUser && !GetProp((HWND)dwUser, "WaitingToProcess") && !GetProp((HWND)dwUser, "Inactive")) {
        SetProp((HWND)dwUser, "WaitingToProcess", (HANDLE)1);
        PostMessage((HWND)dwUser, GVM_RENDER, 0, 0);
    }
}

BOOL CGraphicView::Create()
{
    if (Device < 0) {
        return FALSE;
    }

    m_isInitialized = FALSE;
    RECT r;
    GetClientRect(&r);
    int width = r.right - r.left;
    int height = r.bottom - r.top;

    if (!m_isWindowed) {
        width = DeviceWidth;
        height = DeviceHeight;
    }

    W3DErrorType error = W3D::Set_Render_Device(Device, width, height, BPP, m_isWindowed, true, false, true);
    bool success = error == W3D_ERROR_OK;

    if (error == W3D_ERROR_OK && m_camera == nullptr) {
        m_camera = new CameraClass();

        success = m_camera != nullptr;

        if (m_camera != nullptr) {
            Matrix3D m(true);
            m[2].W = 35.0f;
            m_camera->Set_Transform(m);
        }
    }

    UpdateCamera();

    if (m_light == nullptr) {
        RCFileClass file(nullptr, "Light.w3d");
        W3DAssetManager::Get_Instance()->Load_3D_Assets(file);
        m_light = W3DAssetManager::Get_Instance()->Create_Render_Obj("LIGHT");
        m_lightInScene = false;
    }

    m_isInitialized = success;

    if (success && m_timer == 0) {
        timecaps_tag ptc;
        ptc.wPeriodMin = 0;
        ptc.wPeriodMax = 0;
        timeGetDevCaps(&ptc, sizeof(ptc));
        UINT time = ptc.wPeriodMin;

        if (ptc.wPeriodMin <= 16) {
            time = 16;
        }

        m_timer = timeSetEvent(time, time, TimerProc, (DWORD_PTR)m_hWnd, TIME_PERIODIC);
    }

    return success;
}

void CGraphicView::EnableRendering(bool enable)
{
    if (enable) {
        m_renderingDisabled -= 1;
    } else {
        m_renderingDisabled += 1;
    }
}

void ResetLODLevel(RenderObjClass *obj)
{
    if (obj != nullptr) {
        for (int i = 0; i < obj->Get_Num_Sub_Objects(); i++) {
            RenderObjClass *o = obj->Get_Sub_Object(i);

            if (o != nullptr) {
                ResetLODLevel(o);
                Ref_Ptr_Release(o);
            }
        }

        if (obj->Class_ID() == RenderObjClass::CLASSID_HLOD) {
            obj->Set_LOD_Level(0);
        }
    }
}

void CGraphicView::Render(BOOL update, unsigned int time)
{
    static bool IsRendering = false;

    if (!IsRendering) {
        IsRendering = true;
        CW3DViewDoc *document = (CW3DViewDoc *)m_pDocument;

        if (document->m_initialized && document->m_scene != nullptr && !m_renderingDisabled) {
            unsigned int newtime = timeGetTime();
            unsigned int currenttime = m_time;
            m_time = newtime;
            int tm = newtime - currenttime;
            unsigned int sync;

            if (time != 0) {
                sync = time + W3D::Get_Sync_Time();
            } else {
                sync = tm * m_animationSpeed + W3D::Get_Sync_Time();
            }

            W3D::Sync(sync);

            if (m_animationPlaying == 0 && update) {
                document->UpdateAnimation(tm * 0.001f * m_animationSpeed);
            }

            if (m_objectRotationFlags != 0 && update) {
                UpdateObjectRotation();
            }

            if (m_lightRotationFlags != 0 && update) {
                UpdateLightTransform();
            }

            if (document->m_model != nullptr && document->m_scene->Get_Auto_Switch_LOD()) {
                ResetLODLevel(document->m_model);
            }

            W3D::Begin_Render(true, true, document->m_fogColor);
            W3D::Render(document->m_textureScene, document->m_textureCamera);

            if (document->m_backgroundObjectName.GetLength() > 0) {
                W3D::Render(document->m_backgroundScene, document->m_docCamera);
            }

            unsigned long high = 0;
            unsigned long clock = Get_CPU_Clock(high);
            W3D::Render(document->m_scene, m_camera);
            unsigned long clock2 = Get_CPU_Clock(high);
            unsigned int frametime = (clock2 - clock) / 1000;
            W3D::End_Render(true);

            if (newtime - m_lastCountUpdate > 250) {
                m_lastCountUpdate = newtime;
                document->UpdateParticleCount();
                int polys = 0;

                if (document->m_model != nullptr) {
                    polys = document->m_model->Get_Num_Polys();
                }

                ((CMainFrame *)AfxGetMainWnd())->UpdatePolyCount(polys);
            }

            ((CMainFrame *)AfxGetMainWnd())->UpdateStatusBar(frametime);
        }

        IsRendering = false;
    }
}

void CGraphicView::UpdateCamera()
{
    int width;
    int height;

    if (m_isWindowed) {
        RECT r;
        GetClientRect(&r);
        width = r.right - r.left;
        height = r.bottom - r.top;
    } else {
        width = DeviceWidth;
        height = DeviceHeight;
    }

    float hfov;
    float vfov;

    if (height <= width) {
        hfov = 0.7853981852531433f;
        vfov = (float)height / (float)width * 0.7853981852531433f;
    } else {
        hfov = (float)height / (float)width * 0.7853981852531433f;
        vfov = 0.7853981852531433f;
    }

    if (!((CW3DViewDoc *)m_pDocument)->m_useManualFov) {
        m_camera->Set_View_Plane(hfov, vfov);
    }
}

void CGraphicView::UpdateObjectRotation()
{
    // TODO
}

void CGraphicView::UpdateLightTransform()
{
    // TODO
}

void CGraphicView::UpdateAnimation(int flag)
{
    if (m_animationPlaying == flag) {
        return;
    }

    if (flag == 0) {
        m_time = timeGetTime();
        m_animationPlaying = flag;
    } else if (flag != 1) {
        m_animationPlaying = flag;
    }

    CW3DViewDoc *document = (CW3DViewDoc *)m_pDocument;
    if (document->m_model != nullptr) {
        if (document->m_animation != nullptr) {
            document->m_model->Set_Animation(document->m_animation, 0.0f);
        }
    }
}

void CGraphicView::ResetCamera(RenderObjClass *robj)
{
    SphereClass sphere = robj->Get_Bounding_Sphere();
    ResetCameraValues(sphere);
    int index = robj->Get_Bone_Index("CAMERA");

    if (index > 0) {
        Matrix3D tm = robj->Get_Bone_Transform(index);

        if (m_plusXCamera) {
            Matrix3D m;
            m[0].X = 0.0f;
            m[0].Y = 0.0f;
            m[0].Z = -1.0f;
            m[0].W = 0.0f;
            m[1].X = -1.0f;
            m[1].Y = 0.0f;
            m[1].Z = 0.0f;
            m[1].W = 0.0f;
            m[2].X = 0.0f;
            m[2].Y = 1.0f;
            m[2].Z = 0.0f;
            m[2].W = 0.0f;
            tm = tm * m;
        }

        m_camera->Set_Transform(tm);
    }

    CMainFrame *frame = (CMainFrame *)AfxGetMainWnd();

    if (frame != nullptr) {
        frame->UpdatePolyCount(robj->Get_Num_Polys());
    }
}

void CGraphicView::ResetCameraValues(SphereClass &sphere)
{
    m_radius = sphere.Radius * 3.0f;

    if (m_radius < 1.0f) {
        m_radius = 1.0f;
    }

    Matrix3D tm;
    tm.Look_At(Vector3(m_radius + sphere.Center.X, sphere.Center.Y, sphere.Center.Z), sphere.Center, 0.0f);
    Pos = sphere.Center;
    m_objCenter = Pos;
    Zoom = m_radius * 0.0052631581f;
    Rotation = Build_Quaternion(tm);
    m_camera->Set_Transform(tm);
    CW3DViewDoc *document = ((CW3DViewDoc *)m_pDocument);
    LightClass *light = document->m_light;

    if (m_light != nullptr && light != nullptr) {
        tm.Make_Identity();
        tm.Adjust_Translation(Pos);
        tm[2].W += m_radius * 0.69999999f;
        light->Set_Transform(tm);
        m_light->Set_Transform(tm);
        m_light->Scale(m_radius / Lightradius * 14.0f);
        Lightradius = m_radius * 0.071428575f;
    }

    float zfar = m_radius * 60.0f;
    float znear = Zoom * 0.5f;

    if (znear < 0.2f) {
        znear = 0.2f;
    }

    if (!document->m_useManualClipPlanes) {
        m_camera->Set_Clip_Planes(znear, zfar);
        float start;
        float end;
        document->m_scene->Get_Fog_Range(&start, &end);
        document->m_scene->Set_Fog_Range(end, end);
        document->m_scene->Update_Fog_Range();
    }

    document->m_docCamera->Set_Transform(tm);
    document->m_docCamera->Set_Position(Vector3(0.0f, 0.0f, 0.0f));

    CMainFrame *frame = (CMainFrame *)AfxGetMainWnd();

    if (frame != nullptr) {
        frame->UpdateCameraDistance(m_radius);
        frame->UpdateFrameCount(0, 0, 0.0f);
    }

    m_objSphere = sphere;
}

void CGraphicView::ResetParticleEmitterCamera(ParticleEmitterClass *emitter)
{
    // TODO
}

void CGraphicView::SetRotationFlags(int flags)
{
    if (m_objectRotationFlags != flags) {
        m_objectRotationFlags = flags;
    }
}

void CGraphicView::SetCameraRotateConstraints(int constraints)
{
    m_cameraRotateConstraints = constraints;
}

void CGraphicView::ResetRenderObj()
{
    CW3DViewDoc *document = GetCurrentDocument();

    if (document != nullptr) {
        if (document->m_model != nullptr) {
            Matrix3D tm(true);
            document->m_model->Set_Transform(tm);
        }
    }
}

void CGraphicView::UpdateCameraDistance(float distance)
{
    m_radius = distance;
    Matrix3D tm(true);
    tm.Look_At(
        Vector3(distance + m_objSphere.Center.X, m_objSphere.Center.Y, m_objSphere.Center.Z), m_objSphere.Center, 0.0f);
    m_camera->Set_Transform(tm);
    CMainFrame *frame = (CMainFrame *)AfxGetMainWnd();

    if (frame != nullptr) {
        frame->UpdateCameraDistance(m_radius);
    }
}

#if 0
void CGraphicView::SetCameraDirection(int direction)
{
    // do later
}
#endif
