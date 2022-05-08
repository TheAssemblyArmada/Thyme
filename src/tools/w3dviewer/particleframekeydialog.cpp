/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View particle frame key dialog
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "particleframekeydialog.h"
#include "resource.h"
#include "utils.h"

// clang-format off
BEGIN_MESSAGE_MAP(ParticleFrameKeyDialog, CDialog)
END_MESSAGE_MAP()
// clang-format on

ParticleFrameKeyDialog::ParticleFrameKeyDialog(float frame, CWnd *pParentWnd) :
    CDialog(IDD_PARTICLEFRAME, pParentWnd), m_frame(frame)
{
}

void ParticleFrameKeyDialog::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_FRAMESPIN, m_frameSpin);
}

BOOL ParticleFrameKeyDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    NMUPDOWN *ud = reinterpret_cast<NMUPDOWN *>(lParam);

    if (ud != nullptr && ud->hdr.code == UDN_DELTAPOS) {
        UpdateEditCtrl(ud->hdr.hwndFrom, ud->iDelta);
    }

    return CWnd::OnNotify(wParam, lParam, pResult);
}

BOOL ParticleFrameKeyDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    InitializeSpinButton(&m_frameSpin, m_frame, -1024.0f, 1024.0f);
    return TRUE;
}

void ParticleFrameKeyDialog::OnOK()
{
    m_frame = GetDlgItemFloat(m_hWnd, IDC_FRAME);
    CDialog::OnOK();
}
