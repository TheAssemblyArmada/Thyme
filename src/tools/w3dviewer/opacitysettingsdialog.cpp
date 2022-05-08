/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View opacity settings dialog
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "opacitysettingsdialog.h"
#include "colorbar.h"
#include "resource.h"

// clang-format off
BEGIN_MESSAGE_MAP(OpacitySettingsDialog, CDialog)
END_MESSAGE_MAP()
// clang-format on

OpacitySettingsDialog::OpacitySettingsDialog(float opacity, CWnd *pParentWnd) :
    CDialog(IDD_OPACITY, pParentWnd), m_opacityBar(nullptr), m_opacity(opacity)
{
}

BOOL OpacitySettingsDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_opacityBar = reinterpret_cast<ColorBarClass *>(GetProp(::GetDlgItem(m_hWnd, IDC_OPACITYBAR), "CLASSPOINTER"));
    m_opacityBar->SetMinMax(0.0f, 1.0f);
    m_opacityBar->SetColor(0, 0.0f, 0.0f, 0.0f, 0.0f, 3);
    m_opacityBar->AddColor(1, 1.0f, 255.0f, 255.0f, 255.0f, 3);
    m_opacityBar->SetSliderPos(m_opacity);
    return TRUE;
}

void OpacitySettingsDialog::OnOK()
{
    m_opacity = m_opacityBar->GetSliderPos();
    CDialog::OnOK();
}
