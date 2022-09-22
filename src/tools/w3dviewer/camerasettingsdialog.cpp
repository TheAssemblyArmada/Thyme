/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View camera distance dialog
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "camerasettingsdialog.h"
#include "camera.h"
#include "graphicview.h"
#include "resource.h"
#include "utils.h"
#include "viewerscene.h"
#include "w3dviewdoc.h"

bool CameraSettingsDialogClass::m_isUpdating = false;

// clang-format off
BEGIN_MESSAGE_MAP(CameraSettingsDialogClass, CDialog)
	ON_COMMAND(IDC_FOV, OnFieldOfView)
	ON_COMMAND(IDC_CLIPPLANES, OnClipPlanes)
	ON_COMMAND(IDABORT, OnReset)
END_MESSAGE_MAP()
// clang-format on

CameraSettingsDialogClass::CameraSettingsDialogClass(CWnd *pParentWnd) : CDialog(IDD_CAMERA, pParentWnd) {}

void CameraSettingsDialogClass::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_LENSFOVSPIN, m_lensSpin);
    DDX_Control(pDX, IDC_FARCLIPSPIN, m_farClipSpin);
    DDX_Control(pDX, IDC_VERTFOVSPIN, m_vertFovSpin);
    DDX_Control(pDX, IDC_NEARCLIPSPIN, m_nearClipSpin);
    DDX_Control(pDX, IDC_HORFOVSPIN, m_horizFovSpin);
}

BOOL CameraSettingsDialogClass::OnCommand(WPARAM wParam, LPARAM lParam)
{
    if (!m_isUpdating) {
        if (LOWORD(wParam) == IDC_HORFOV) {
            if (HIWORD(wParam) == EN_UPDATE) {
                m_isUpdating = true;
                double hfov = GetDlgItemFloat(m_hWnd, IDC_HORFOV);

                if (hfov > 0.0) {
                    float lensfov = 0.018 / tan(hfov * 0.018 * 0.5) * 1000.0;
                    SetDlgItemFloat(m_hWnd, IDC_LENSFOV, lensfov);
                }

                m_isUpdating = false;
            }
        } else if ((LOWORD(wParam) == IDC_LENSFOV) && (HIWORD(wParam) == EN_UPDATE)) {
            m_isUpdating = true;
            double lensfov = GetDlgItemFloat(m_hWnd, IDC_LENSFOV) * 0.001f;

            if (lensfov > 0.0) {
                float f1 = atan2(0.018 / lensfov, 1.0) + atan2(0.018 / lensfov, 1.0);
                float hfov = f1 * 57.29577791868204f;
                SetDlgItemFloat(m_hWnd, IDC_HORFOV, hfov);
                float vfov = f1 * 0.75f * 57.29577791868204f;
                SetDlgItemFloat(m_hWnd, IDC_VERTFOV, vfov);
            }

            m_isUpdating = false;
        }
    }

    return CWnd::OnCommand(wParam, lParam);
}

BOOL CameraSettingsDialogClass::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    NMUPDOWN *ud = reinterpret_cast<NMUPDOWN *>(lParam);

    if (ud != nullptr && ud->hdr.code == UDN_DELTAPOS) {
        UpdateEditCtrl(ud->hdr.hwndFrom, ud->iDelta);

        if (ud->hdr.idFrom == IDC_LENSFOVSPIN) {
            double lensfov = GetDlgItemFloat(m_hWnd, IDC_LENSFOV) * 0.001f;

            if (lensfov > 0.0) {
                float f1 = atan2(0.018 / lensfov, 1.0) + atan2(0.018 / lensfov, 1.0);
                float hfov = f1 * 57.29577791868204f;
                SetDlgItemFloat(m_hWnd, IDC_HORFOV, hfov);
                float vfov = f1 * 0.75f * 57.29577791868204f;
                SetDlgItemFloat(m_hWnd, IDC_VERTFOV, vfov);
            }
        } else if (ud->hdr.idFrom == IDC_HORFOVSPIN) {
            double hfov = GetDlgItemFloat(m_hWnd, IDC_HORFOV);

            if (hfov > 0.0) {
                float lensfov = 0.018 / tan(hfov * 0.01745329300562541 * 0.5) * 1000.0;
                SetDlgItemFloat(m_hWnd, IDC_LENSFOV, lensfov);
            }
        }
    }

    return CWnd::OnNotify(wParam, lParam, pResult);
}

BOOL CameraSettingsDialogClass::OnInitDialog()
{
    CDialog::OnInitDialog();
    CW3DViewDoc *doc = GetCurrentDocument();
    CameraClass *camera = doc->GetGraphicView()->m_camera;
    SendDlgItemMessage(IDC_FOV, BM_SETCHECK, doc->m_useManualFov);
    SendDlgItemMessage(IDC_CLIPPLANES, BM_SETCHECK, doc->m_useManualClipPlanes);

    float znear;
    float zfar;
    camera->Get_Clip_Planes(znear, zfar);
    InitializeSpinButton(&m_nearClipSpin, znear, 0.0f, 999999.0f);
    InitializeSpinButton(&m_farClipSpin, zfar, 1.0f, 999999.0f);

    int hfov = (camera->Get_Horizontal_FOV() * 57.29577791868204f);
    int vfov = (camera->Get_Vertical_FOV() * 57.29577791868204f);
    InitializeSpinButton(&m_horizFovSpin, hfov, 0.0f, 180.0f);
    InitializeSpinButton(&m_vertFovSpin, vfov, 0.0f, 180.0f);

    float lens = 0.018 / tan(camera->Get_Horizontal_FOV() * 0.5f) * 1000.0f;
    InitializeSpinButton(&m_lensSpin, lens, 1.0f, 200.0f);

    bool use_fov = SendDlgItemMessage(IDC_FOV, BM_GETCHECK) == 1;
    m_vertFovSpin.EnableWindow(use_fov);
    m_horizFovSpin.EnableWindow(use_fov);
    m_lensSpin.EnableWindow(use_fov);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_VERTFOV), use_fov);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_HORFOV), use_fov);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_LENSFOV), use_fov);

    bool use_clip = SendDlgItemMessage(IDC_CLIPPLANES, BM_GETCHECK) == 1;
    m_nearClipSpin.EnableWindow(use_clip);
    m_farClipSpin.EnableWindow(use_clip);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_NEARCLIP), use_clip);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_FARCLIP), use_clip);

    return TRUE;
}

void CameraSettingsDialogClass::OnOK()
{
    CDialog::OnInitDialog();
    CW3DViewDoc *doc = GetCurrentDocument();
    CameraClass *camera = doc->GetGraphicView()->m_camera;
    bool use_fov = SendDlgItemMessage(IDC_FOV, BM_GETCHECK) == 1;
    bool use_clip = SendDlgItemMessage(IDC_CLIPPLANES, BM_GETCHECK) == 1;
    doc->m_useManualFov = use_fov;
    doc->m_useManualClipPlanes = use_clip;

    if (use_fov) {
        camera->Set_View_Plane(GetDlgItemFloat(m_hWnd, IDC_HORFOV) * 0.01745329300562541f,
            GetDlgItemFloat(m_hWnd, IDC_VERTFOV) * 0.01745329300562541f);
    } else {
        doc->GetGraphicView()->UpdateCamera();
    }

    float znear = GetDlgItemFloat(m_hWnd, IDC_NEARCLIP);
    float zfar = GetDlgItemFloat(m_hWnd, IDC_FARCLIP);
    camera->Set_Clip_Planes(znear, zfar);
    doc->SaveCameraSettings();

    float start;
    float end;
    doc->m_scene->Get_Fog_Range(&start, &end);
    doc->m_scene->Set_Fog_Range(znear, end);
    doc->m_scene->Update_Fog_Range();

    if (doc->m_model != nullptr) {
        doc->GetGraphicView()->ResetCamera(doc->m_model);
    }

    CDialog::OnOK();
}

void CameraSettingsDialogClass::OnFieldOfView()
{
    bool use_fov = SendDlgItemMessage(IDC_FOV, BM_GETCHECK) == 1;
    m_vertFovSpin.EnableWindow(use_fov);
    m_horizFovSpin.EnableWindow(use_fov);
    m_lensSpin.EnableWindow(use_fov);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_VERTFOV), use_fov);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_HORFOV), use_fov);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_LENSFOV), use_fov);
}

void CameraSettingsDialogClass::OnClipPlanes()
{
    bool use_clip = SendDlgItemMessage(IDC_CLIPPLANES, BM_GETCHECK) == 1;
    m_nearClipSpin.EnableWindow(use_clip);
    m_farClipSpin.EnableWindow(use_clip);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_NEARCLIP), use_clip);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_FARCLIP), use_clip);
}

void CameraSettingsDialogClass::OnReset()
{
    CDialog::OnInitDialog();
    CW3DViewDoc *doc = GetCurrentDocument();
    CameraClass *camera = doc->GetGraphicView()->m_camera;
    doc->m_useManualFov = false;
    doc->m_useManualClipPlanes = false;
    doc->GetGraphicView()->UpdateCamera();

    if (doc->m_model != nullptr) {
        doc->GetGraphicView()->ResetCamera(doc->m_model);
    }

    float znear = 0.0f;
    float zfar = 0.0f;
    camera->Get_Clip_Planes(znear, zfar);
    SetDlgItemFloat(m_hWnd, IDC_NEARCLIP, znear);
    SetDlgItemFloat(m_hWnd, IDC_FARCLIP, zfar);

    float hfov = (camera->Get_Horizontal_FOV() * 57.29577791868204f);
    SetDlgItemFloat(m_hWnd, IDC_HORFOV, hfov);

    float vfov = (camera->Get_Vertical_FOV() * 57.29577791868204f);
    SetDlgItemFloat(m_hWnd, IDC_VERTFOV, vfov);

    float lens = (atan2(0.018, 1.0) / camera->Get_Vertical_FOV() + atan2(0.018, 1.0) / camera->Get_Vertical_FOV()) * 1000.0f;
    SetDlgItemFloat(m_hWnd, IDC_LENSFOV, lens);
}
