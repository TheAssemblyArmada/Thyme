/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View particle blur time key dialog
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "particleblurtimekeydialog.h"
#include "resource.h"
#include "utils.h"

// clang-format off
BEGIN_MESSAGE_MAP(ParticleBlurTimeKeyDialog, CDialog)
END_MESSAGE_MAP()
// clang-format on

ParticleBlurTimeKeyDialog::ParticleBlurTimeKeyDialog(float blurtime, CWnd *pParentWnd) :
    CDialog(IDD_BLURTIME, pParentWnd), m_blurTime(blurtime)
{
}

void ParticleBlurTimeKeyDialog::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_BLURTIMESPIN, m_blurTimeSpin);
}

BOOL ParticleBlurTimeKeyDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    NMUPDOWN *ud = reinterpret_cast<NMUPDOWN *>(lParam);

    if (ud != nullptr && ud->hdr.code == UDN_DELTAPOS) {
        UpdateEditCtrl(ud->hdr.hwndFrom, ud->iDelta);
    }

    return CWnd::OnNotify(wParam, lParam, pResult);
}

BOOL ParticleBlurTimeKeyDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    InitializeSpinButton(&m_blurTimeSpin, m_blurTime, -1024.0f, 1024.0f);
    return TRUE;
}

void ParticleBlurTimeKeyDialog::OnOK()
{
    m_blurTime = GetDlgItemFloat(m_hWnd, IDC_BLURTIME);
    CDialog::OnOK();
}
