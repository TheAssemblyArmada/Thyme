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
#include "part_buf.h"
#include "part_emt.h"
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
    CW3DViewDoc *document = (CW3DViewDoc *)m_pDocument;
    float ydiff = m_mousePos.y - point.y;

    if ((nFlags & MK_CONTROL) != 0 || !m_lightInScene) {
        if ((nFlags & MK_CONTROL) != 0 && !m_lightInScene) {
            m_light->Add(document->m_scene);
            m_lightInScene = true;
        }
    } else {
        m_light->Remove();
        m_lightInScene = false;
    }

    if (m_leftButtonDown && m_rightButtonDown) {
        Matrix3D tm = m_camera->Get_Transform();
        RECT r;
        GetClientRect(&r);

        float x = (((float)point.x - (float)(r.right >> 1)) / (float)(r.right >> 1)
                      - ((float)m_mousePos.x - (float)(r.right >> 1)) / (float)(r.right >> 1))
            * m_radius * -1.0f;

        float y = (((float)point.y - (float)(r.bottom >> 1)) / (float)(r.bottom >> 1)
                      - ((float)m_mousePos.y - (float)(r.bottom >> 1)) / (float)(r.bottom >> 1))
            * m_radius * -1.0f;

        tm.Translate(x, y, 0.0f);
        Matrix3 m = Build_Matrix3(Rotation);
        Pos += x * m.Get_X_Vector() + m.Get_Y_Vector() * y;
        m_camera->Set_Transform(tm);
        m_mousePos = point;
        CWnd::Default();
        return;
    }

    if ((nFlags & MK_CONTROL) != 0) {
        if (m_leftButtonDown) {
            LightClass *light = document->m_light;

            if (light != nullptr) {
                if (m_light != nullptr) {
                    RECT r;
                    GetClientRect(&r);
                    float x = r.right >> 1;
                    float y = r.bottom >> 1;
                    float x0 = (m_mousePos.x - x) / x;
                    float y0 = (y - m_mousePos.y) / y;
                    float y1 = (y - point.y) / y;
                    float x1 = (point.x - x) / x;

                    Quaternion q = Trackball(x0, y0, x1, y1, 0.8f);
                    Quaternion q2;

                    q2.X = -q.X;
                    q2.Y = -q.Y;
                    q2.Z = -q.Z;
                    q2.W = q.W;

                    Quaternion q3 = Build_Quaternion(m_camera->Get_Transform());
                    Quaternion q4 = Build_Quaternion(light->Get_Transform());

                    float f1 = q3.Y * q2.Z - q3.Z * q2.Y + q2.X * q3.W + q3.X * q2.W;
                    float f2 = q3.Y * q2.W + q3.W * q2.Y - (q3.X * q2.Z - q2.X * q3.Z);
                    float f3 = q3.X * q2.Y - q2.X * q3.Y + q3.Z * q2.W + q3.W * q2.Z;
                    float f4 = q3.W * q2.W - (q2.X * q3.X + q3.Z * q2.Z + q3.Y * q2.Y);
                    float f5 = -q3.X;
                    float f6 = -q3.Y;
                    float f7 = -q3.Z;

                    q2.X = f7 * f2 - f6 * f3 + f1 * q3.W + f5 * f4;
                    q2.Y = f6 * f4 + q3.W * f2 - (f1 * f7 - f5 * f3);
                    q2.Z = f1 * f6 - f5 * f2 + f7 * f4 + q3.W * f3;
                    q2.W = q3.W * f4 - (f7 * f3 + f6 * f2 + f5 * f1);

                    Quaternion q5;
                    q5.X = q4.Z * q2.Y - q4.Y * q2.Z + q2.X * q4.W + q4.X * q2.W;
                    q5.Y = q4.Y * q2.W + q4.W * q2.Y - (q2.X * q4.Z - q4.X * q2.Z);
                    q5.Z = q2.X * q4.Y - q4.X * q2.Y + q4.Z * q2.W + q4.W * q2.Z;
                    q5.W = q4.W * q2.W - (q4.Z * q2.Z + q4.Y * q2.Y + q2.X * q4.X);
                    q5.Normalize();

                    Matrix3D tm = light->Get_Transform();
                    float f8 = tm[0].X;
                    float f9 = tm[0].Y;
                    float f10 = Pos.X - tm[0].W;
                    float f11 = tm[1].X;
                    float f12 = tm[1].W;
                    float f13 = tm[1].Y;
                    float f14 = tm[0].Z;
                    float f15 = Pos.Y - f12;
                    float f16 = tm[2].X;
                    float f17 = tm[2].W;
                    float f18 = tm[2].Z;
                    float f19 = tm[1].Z;
                    float f20 = tm[2].Y;
                    float f21 = Pos.Z - f17;

                    float f22 = f16 * f21 + f11 * f15 + f10 * f8;
                    float f23 = f20 * f21 + f13 * f15 + f10 * f9;
                    float f24 = f18 * f21 + f19 * f15 + f10 * f14;

                    Matrix3D tm2;
                    tm2.Set(q5, Pos);
                    tm2.Translate(-f22, -f23, -f24);
                    m_light->Set_Transform(tm2);
                    light->Set_Transform(tm2);
                }
            }

            m_mousePos = point;
            CWnd::Default();
            return;
        }

        if (m_rightButtonDown) {
            LightClass *light = document->m_light;
            RenderObjClass *model = document->m_model;

            if (light != nullptr) {
                if (model != nullptr) {
                    RECT r;
                    GetClientRect(&r);
                    float f25 = ydiff / (r.bottom - r.top) * (m_objSphere.Radius * 3.0f);
                    Matrix3D tm = light->Get_Transform();
                    tm.Translate_Z(f25);
                    Vector3 v = tm.Get_Translation();
                    Vector3 v2 = model->Get_Position();

                    if ((v - v2).Length() > m_objSphere.Radius) {
                        m_light->Set_Transform(tm);
                        light->Set_Transform(tm);
                    }
                }
            }

            m_mousePos = point;
            CWnd::Default();
            return;
        }
    }

    if (m_leftButtonDown) {
        if (!m_isInitialized || document->m_scene == nullptr || document->m_model == nullptr) {
            m_mousePos = point;
            CWnd::Default();
            return;
        }

        RECT r;
        GetClientRect(&r);
        float f26 = r.right >> 1;
        float f27 = r.bottom >> 1;
        float x0 = (m_mousePos.x - f26) / f26;
        float y0 = (f27 - m_mousePos.y) / f27;
        float y1 = (f27 - point.y) / f27;
        float x1 = (point.x - f26) / f26;
        Rotation = Trackball(x0, y0, x1, y1, 0.8f);

        switch (m_cameraRotateConstraints) {
            case 1: {
                Matrix3D tm2 = Build_Matrix3D(Rotation);
                Matrix3D tm(true);
                tm.Rotate_X(tm2.Get_X_Rotation());
                tm.Set_Translation(tm2.Get_Translation());
                Rotation = Build_Quaternion(tm);
            } break;
            case 2: {
                Matrix3D tm2 = Build_Matrix3D(Rotation);
                Matrix3D tm(true);
                tm.Rotate_Y(tm2.Get_Y_Rotation());
                tm.Set_Translation(tm2.Get_Translation());
                Rotation = Build_Quaternion(tm);
            } break;
            case 4: {
                Matrix3D tm2 = Build_Matrix3D(Rotation);
                Matrix3D tm(true);
                tm.Rotate_Z(tm2.Get_Z_Rotation());
                tm.Set_Translation(tm2.Get_Translation());
                Rotation = Build_Quaternion(tm);
            } break;
        }

        Matrix3D tm3 = m_camera->Get_Transform();
        Matrix3D tm4;
        tm3.Get_Orthogonal_Inverse(tm4);
        Vector3 v;
        Matrix3D::Transform_Vector(tm4, Pos, &v);
        tm3.Translate(v);
        tm3 = tm3 * Build_Matrix3D(Rotation);
        tm3.Translate(-v);
        m_camera->Set_Transform(tm3);
        document->m_docCamera->Set_Transform(tm3);
        document->m_docCamera->Set_Position(Vector3(0.0f, 0.0f, 0.0f));
        m_mousePos = point;
        CWnd::Default();
        return;
    }

    if (m_rightButtonDown) {
        m_mousePos = point;
        Matrix3D tm = m_camera->Get_Transform();

        if (ydiff != 0.0f) {
            RECT r;
            GetClientRect(&r);
            float f28 = ydiff / (r.bottom - r.top) * m_radius * 3.0f;

            if (f28 < Zoom && f28 >= 0.0f) {
                f28 = Zoom;
            }

            if (f28 > -Zoom && f28 <= 0.0f) {
                f28 = -Zoom;
            }

            float f29 = f28 + m_radius;

            if (f29 > 0.0f) {
                m_radius = f29;
                tm.Translate_Z(f28);
                m_camera->Set_Transform(tm);
                CMainFrame *frame = (CMainFrame *)AfxGetMainWnd();

                if (frame != nullptr) {
                    document->m_docCamera->Set_Transform(tm);
                    document->m_docCamera->Set_Position(Vector3(0.0f, 0.0f, 0.0f));

                    if (document->m_model != nullptr) {
                        frame->UpdatePolyCount(document->m_model->Get_Num_Polys());
                    }

                    frame->UpdateCameraDistance(m_radius);
                }
            }
        }

        m_mousePos = point;
    }

    CWnd::Default();
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
    CW3DViewDoc *document = (CW3DViewDoc *)m_pDocument;

    if (document->m_model == nullptr) {
        return;
    }

    Matrix3D tm = document->m_model->Get_Transform();

    if ((m_objectRotationFlags & 1) == 1) {
        tm.Rotate_X(0.05f);
    }

    if ((m_objectRotationFlags & 8) == 8) {
        tm.Rotate_X(-0.05f);
    }

    if ((m_objectRotationFlags & 2) == 2) {
        tm.Rotate_Y(0.05f);
    }

    if ((m_objectRotationFlags & 16) == 16) {
        tm.Rotate_Y(-0.05f);
    }

    if ((m_objectRotationFlags & 4) == 4) {
        tm.Rotate_Z(0.05f);
    }

    if ((m_objectRotationFlags & 32) == 32) {
        tm.Rotate_Z(-0.05f);
    }

    if (!tm.Is_Orthogonal()) {
        tm.Re_Orthogonalize();
    }

    document->m_model->Set_Transform(tm);
}

void CGraphicView::UpdateLightTransform()
{
    CW3DViewDoc *document = (CW3DViewDoc *)m_pDocument;

    if (document->m_model == nullptr || document->m_light == nullptr) {
        return;
    }

    Matrix3D tm(true);

    if ((m_lightRotationFlags & 1) == 1) {
        tm.Rotate_X(0.05f);
    }

    if ((m_lightRotationFlags & 8) == 8) {
        tm.Rotate_X(-0.05f);
    }

    if ((m_lightRotationFlags & 2) == 2) {
        tm.Rotate_Y(0.05f);
    }

    if ((m_lightRotationFlags & 16) == 16) {
        tm.Rotate_Y(-0.05f);
    }

    if ((m_lightRotationFlags & 4) == 4) {
        tm.Rotate_Z(0.05f);
    }

    if ((m_lightRotationFlags & 32) == 32) {
        tm.Rotate_Z(-0.05f);
    }

    Matrix3D tm2;
    Matrix3D tm3;
    Matrix3D tm4;
    tm4 = document->m_model->Get_Transform();
    tm4.Get_Orthogonal_Inverse(tm2);
    Matrix3D tm5 = document->m_light->Get_Transform();
    Matrix3D::Multiply(tm2, tm5, &tm3);
    Matrix3D::Multiply(tm4, tm, &tm5);
    Matrix3D::Multiply(tm5, tm3, &tm5);

    if (!tm5.Is_Orthogonal()) {
        tm5.Re_Orthogonalize();
    }

    m_light->Set_Transform(tm5);
    document->m_light->Set_Transform(tm5);
}

void CGraphicView::UpdateAnimation(int flag)
{
    if (m_animationPlaying == flag) {
        return;
    }

    if (flag == 0) {
        m_time = timeGetTime();
        m_animationPlaying = flag;
        return;
    } else if (flag != 1) {
        m_animationPlaying = flag;
        return;
    }

    CW3DViewDoc *document = (CW3DViewDoc *)m_pDocument;
    if (document->m_model != nullptr) {
        if (document->m_animation != nullptr) {
            document->m_model->Set_Animation(document->m_animation, 0.0f);
        }
    }

    document->UpdateFrameCount();
    m_animationPlaying = 1;
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
    ParticleBufferClass *buffer = emitter->Peek_Buffer();
    SphereClass s;
    s.Center = emitter->Get_Start_Velocity();
    Vector3 acceleration = buffer->Get_Acceleration();
    float lifetime = buffer->Get_Lifetime();
    float f1 = lifetime * lifetime;
    float f2 = acceleration.X * f1;
    float f3 = acceleration.Y * f1;
    float f4 = f2 * 0.5f;
    float f5 = f3 * 0.5f;
    float f6 = s.Center.Y * lifetime;
    float f7 = s.Center.Z * lifetime;
    float f8 = s.Center.X * lifetime + f4;
    float f9 = 0.0f;
    float f10 = 0.0f;
    float f11 = f6 + f5;
    float f12 = 0.0f;
    float f13 = f1 * acceleration.Z * 0.5f + f7;
    float f14;

    if (acceleration.X == 0.0f) {
        if (acceleration.Y == 0.0f && acceleration.Z == 0.0f) {
            goto l1;
        }

        f14 = 0.0f;
    } else {
        f14 = -s.Center.X / acceleration.X;
    }

    float f15;

    if (acceleration.Y == 0.0f) {
        f15 = 0.0f;
    } else {
        f15 = -s.Center.Y / acceleration.Y;
    }

    float f16;

    if (acceleration.Z == 0.0f) {
        f16 = 0.0f;
    } else {
        f16 = -s.Center.Z / acceleration.Z;
    }

    if (f14 >= 0.0f && f14 < lifetime) {
        f9 = GameMath::Fabs(f14 * f14 * acceleration.X * 0.5f + f14 * s.Center.X);
    }

    if (f15 >= 0.0f && f15 < lifetime) {
        f12 = GameMath::Fabs(f15 * f15 * acceleration.Y * 0.5f + f15 * s.Center.Y);
    }

    if (f16 >= 0.0f && f16 < lifetime) {
        f10 = GameMath::Fabs(f16 * f16 * acceleration.Z * 0.5f + f16 * s.Center.Z);
    }

l1:
    float f17 = GameMath::Fabs(f8);
    float f18 = GameMath::Fabs(f11);
    float f19 = GameMath::Fabs(f13);
    float f20;

    if (f17 <= f18) {
        f20 = f18;
    } else {
        f20 = f17;
    }

    if (f20 <= f19) {
        f20 = f19;
    }

    if (f20 <= f9) {
        f20 = f9;
    }

    if (f20 <= f12) {
        f20 = f12;
    }

    if (f20 <= f10) {
        f20 = f10;
    }

    float f21 = f17 * 0.5f;
    s.Center.Y = f18 * 0.5f;
    s.Center.Z = f19 * 0.5f;

    float f24 = f9 * 0.5f;

    if (f21 <= f24) {
        f21 = f24;
    }

    float f25 = f12 * 0.5f;

    if (s.Center.Y > f25) {
        f25 = s.Center.Y;
    }

    float f26 = f10 * 0.5f;

    if (s.Center.Z > f26) {
        f26 = s.Center.Z;
    }

    s.Center.X = f21;
    s.Center.Y = f25;
    s.Center.Z = f26;
    float f27 = f20 * 3.0f * 0.2f;
    float f28 = buffer->Get_Particle_Size() * 5.0f;

    if (f28 <= f27) {
        s.Radius = f27;
    } else {
        s.Radius = f28;
    }

    ResetCameraValues(s);
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

void CGraphicView::SetCameraDirection(int direction)
{
    CW3DViewDoc *document = ((CW3DViewDoc *)m_pDocument);

    if (document->m_model != nullptr) {
        Vector3 center = m_objSphere.Center;
        m_radius = m_objSphere.Radius * 3.0f;
        m_radius = std::clamp(m_radius, 1.0f, 400.0f);
        Matrix3D tm(true);
        Vector3 v;

        switch (direction) {
            case -1:
                v.X = m_radius + center.X;
                v.Y = center.Y;
                v.Z = center.Z;
                tm.Look_At(v, center, 0.0f);
                break;
            case 0:
                v.X = center.X - m_radius;
                v.Y = center.Y;
                v.Z = center.Z;
                tm.Look_At(v, center, 0.0f);
                break;
            case 1:
                v.X = center.X;
                v.Y = center.Y;
                v.Z = m_radius + center.Z;
                tm.Look_At(v, center, 3.1415927f);
                break;
            case 2:
                v.X = center.X;
                v.Y = center.Y;
                v.Z = center.Z - m_radius;
                tm.Look_At(v, center, 3.1415927f);
                break;
            case 3:
                v.X = center.X;
                v.Y = center.Y - m_radius;
                v.Z = center.Z;
                tm.Look_At(v, center, 0.0f);
                break;
            case 4:
                v.X = center.X;
                v.Y = m_radius + center.Y;
                v.Z = center.Z;
                tm.Look_At(v, center, 0.0f);
                break;
            default:
                break;
        }

        m_camera->Set_Transform(tm);
        CMainFrame *frame = (CMainFrame *)AfxGetMainWnd();

        if (frame != nullptr) {
            CW3DViewDoc *document = ((CW3DViewDoc *)m_pDocument);
            tm = document->m_docCamera->Get_Transform();
            document->m_docCamera->Set_Position(Vector3(0.0f, 0.0f, 0.0f));

            if (document->m_model != nullptr) {
                frame->UpdatePolyCount(document->m_model->Get_Num_Polys());
            }

            frame->UpdateCameraDistance(m_radius);
        }
    }
}
