/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View particle size dialog
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "particlesizedialog.h"
#include "resource.h"
#include "utils.h"

// clang-format off
BEGIN_MESSAGE_MAP(ParticleSizeDialog, CDialog)
END_MESSAGE_MAP()
// clang-format on

ParticleSizeDialog::ParticleSizeDialog(float size, CWnd *pParentWnd) : CDialog(IDD_PARTICLEROT, pParentWnd), m_size(size) {}

void ParticleSizeDialog::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_SIZESPIN, m_sizeSpin);
}

BOOL ParticleSizeDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    NMUPDOWN *ud = reinterpret_cast<NMUPDOWN *>(lParam);

    if (ud != nullptr && ud->hdr.code == UDN_DELTAPOS) {
        UpdateEditCtrl(ud->hdr.hwndFrom, ud->iDelta);
    }

    return CWnd::OnNotify(wParam, lParam, pResult);
}

BOOL ParticleSizeDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    InitializeSpinButton(&m_sizeSpin, m_size, 0.0f, 10000.0f);
    return TRUE;
}

void ParticleSizeDialog::OnOK()
{
    m_size = GetDlgItemFloat(m_hWnd, IDC_SIZEEDIT);
    CDialog::OnOK();
}
