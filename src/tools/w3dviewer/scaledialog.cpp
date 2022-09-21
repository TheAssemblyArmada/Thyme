/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View Scale Dialog
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "scaledialog.h"
#include "resource.h"

// clang-format off
BEGIN_MESSAGE_MAP(CScaleDialog, CDialog)
END_MESSAGE_MAP()
// clang-format on

CScaleDialog::CScaleDialog(float scale, CWnd *pParentWnd, const char *label) :
    CDialog(IDD_SCALE, pParentWnd), m_scale(scale), m_label(label)
{
}

BOOL CScaleDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    return CDialog::OnNotify(wParam, lParam, pResult);
}

BOOL CScaleDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_scaleSpin.SetRange(0, (short)75536);
    m_scaleSpin.SetPos(m_scale * 100.0f);

    if (m_label.GetLength()) {
        SetDlgItemText(IDC_SCALELABEL, m_label);
    }

    return TRUE;
}

void CScaleDialog::OnOK()
{
    int scale = m_scaleSpin.GetPos();

    if ((scale & 0xFFFF0000) != 0) {
        MessageBoxA("Invalid scale value. Please enter a number between 1 and 10,000", "Invalid Scale", MB_ICONINFORMATION);
    } else {
        m_scale = scale / 100.0f;

        if (m_scale > 0.0f) {
            CDialog::OnOK();
        } else {
            MessageBoxA("Scale must be a value greater than zero!", "Invalid Scale", MB_ICONINFORMATION);
        }
    }
}
