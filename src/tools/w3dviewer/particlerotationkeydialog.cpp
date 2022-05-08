/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View particle rotation key dialog
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "particlerotationkeydialog.h"
#include "resource.h"
#include "utils.h"

// clang-format off
BEGIN_MESSAGE_MAP(ParticleRotationKeyDialog, CDialog)
END_MESSAGE_MAP()
// clang-format on

ParticleRotationKeyDialog::ParticleRotationKeyDialog(float rotation, CWnd *pParentWnd) :
    CDialog(IDD_PARTICLEROT, pParentWnd), m_rotation(rotation)
{
}

void ParticleRotationKeyDialog::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_ROTATIONSPIN, m_rotationSpin);
}

BOOL ParticleRotationKeyDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    NMUPDOWN *ud = reinterpret_cast<NMUPDOWN *>(lParam);

    if (ud != nullptr && ud->hdr.code == UDN_DELTAPOS) {
        UpdateEditCtrl(ud->hdr.hwndFrom, ud->iDelta);
    }

    return CWnd::OnNotify(wParam, lParam, pResult);
}

BOOL ParticleRotationKeyDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    InitializeSpinButton(&m_rotationSpin, m_rotation, -10000.0f, 10000.0f);
    return TRUE;
}

void ParticleRotationKeyDialog::OnOK()
{
    m_rotation = GetDlgItemFloat(m_hWnd, IDC_ROTATION);
    CDialog::OnOK();
}
