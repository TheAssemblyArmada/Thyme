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
#include "cameradistancedialog.h"
#include "graphicview.h"
#include "resource.h"
#include "utils.h"

// clang-format off
BEGIN_MESSAGE_MAP(CameraDistanceDialogClass, CDialog)
END_MESSAGE_MAP()
// clang-format on

CameraDistanceDialogClass::CameraDistanceDialogClass(CWnd *pParentWnd) : CDialog(IDD_CAMERADIST, pParentWnd) {}

void CameraDistanceDialogClass::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_CAMDISTSPIN, m_spin);
}

BOOL CameraDistanceDialogClass::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    NMUPDOWN *ud = reinterpret_cast<NMUPDOWN *>(lParam);

    if (ud != nullptr && ud->hdr.code == UDN_DELTAPOS) {
        UpdateEditCtrl(ud->hdr.hwndFrom, ud->iDelta);
    }

    return CWnd::OnNotify(wParam, lParam, pResult);
}

BOOL CameraDistanceDialogClass::OnInitDialog()
{
    CDialog::OnInitDialog();
    InitializeSpinButton(&m_spin, GetCurrentGraphicView()->m_radius, 0.0f, 25000.0f);
    SetDlgItemFloat(m_hWnd, IDC_CAMDIST, GetCurrentGraphicView()->m_radius);
    return TRUE;
}

void CameraDistanceDialogClass::OnOK()
{
    CDialog::OnOK();
    GetCurrentGraphicView()->UpdateCameraDistance(GetDlgItemFloat(m_hWnd, IDC_CAMDIST));
}
