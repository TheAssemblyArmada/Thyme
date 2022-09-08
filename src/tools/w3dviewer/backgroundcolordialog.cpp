/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View Background Color Dialog
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "backgroundcolordialog.h"
#include "resource.h"
#include "utils.h"
#include "w3dviewdoc.h"

// clang-format off
BEGIN_MESSAGE_MAP(CBackgroundColorDialog, CDialog)
    ON_WM_HSCROLL()
    ON_COMMAND(IDC_GREYSCALE, OnGreyscale)
END_MESSAGE_MAP()
// clang-format on

CBackgroundColorDialog::CBackgroundColorDialog(CWnd *pParentWnd) : CDialog(IDD_BGCOLOR, pParentWnd) {}

void CBackgroundColorDialog::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_BLUESLIDER, m_blueSlider);
    DDX_Control(pDX, IDC_GREENSLIDER, m_greenSlider);
    DDX_Control(pDX, IDC_REDSLIDER, m_redSlider);
}

BOOL CBackgroundColorDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    PositionWindow(this->m_hWnd);
    m_redSlider.SetRange(0, 100, 0);
    m_greenSlider.SetRange(0, 100, 0);
    m_blueSlider.SetRange(0, 100, 0);
    CW3DViewDoc *doc = GetCurrentDocument();

    if (doc != nullptr) {
        m_currentRed = doc->m_fogColor.X * 100.0f;
        m_currentGreen = doc->m_fogColor.Y * 100.0f;
        m_currentBlue = doc->m_fogColor.Z * 100.0f;
    }

    if (m_currentRed == m_currentGreen && m_currentRed == m_currentBlue) {
        SendDlgItemMessage(IDC_GREYSCALE, BM_SETCHECK, TRUE, 0);
    }

    m_redSlider.SetPos(m_currentRed);
    m_greenSlider.SetPos(m_currentGreen);
    m_blueSlider.SetPos(m_currentBlue);
    return TRUE;
}

void CBackgroundColorDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
    if (SendDlgItemMessage(IDC_GREYSCALE, BM_GETCHECK) != 0) {
        HWND handle;

        if (pScrollBar == GetDlgItem(IDC_REDSLIDER)) {
            handle = m_redSlider.m_hWnd;
        } else if (pScrollBar == GetDlgItem(IDC_GREENSLIDER)) {
            handle = m_greenSlider.m_hWnd;
        } else {
            handle = m_blueSlider.m_hWnd;
        }

        int value = ::SendMessage(handle, TBM_GETPOS, 0, 0);
        m_redSlider.SetPos(value);
        m_greenSlider.SetPos(value);
        m_blueSlider.SetPos(value);
    }

    CW3DViewDoc *doc = GetCurrentDocument();

    if (doc != nullptr) {
        Vector3 color;
        color.X = m_redSlider.GetPos() / 100.0f;
        color.Y = m_greenSlider.GetPos() / 100.0f;
        color.Z = m_blueSlider.GetPos() / 100.0f;
        doc->SetFogColor(color);
    }

    CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CBackgroundColorDialog::OnGreyscale()
{
    if (SendDlgItemMessage(IDC_GREYSCALE, BM_GETCHECK) != 0) {
        m_greenSlider.SetPos(m_redSlider.GetPos());
        m_blueSlider.SetPos(m_redSlider.GetPos());
    }

    CW3DViewDoc *doc = GetCurrentDocument();

    if (doc != nullptr) {
        Vector3 color;
        color.X = m_redSlider.GetPos() / 100.0f;
        color.Y = m_greenSlider.GetPos() / 100.0f;
        color.Z = m_blueSlider.GetPos() / 100.0f;
        doc->SetFogColor(color);
    }
}

void CBackgroundColorDialog::OnCancel()
{
    CW3DViewDoc *doc = GetCurrentDocument();

    if (doc != nullptr) {
        Vector3 color;
        color.X = m_currentRed / 100.0f;
        color.Y = m_currentGreen / 100.0f;
        color.Z = m_currentBlue / 100.0f;
        doc->SetFogColor(color);
    }

    CDialog::OnCancel();
}

LRESULT CBackgroundColorDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_PAINT) {
        PaintGradient(GetDlgItem(IDC_REDBOX)->m_hWnd, true, false, false);
        PaintGradient(GetDlgItem(IDC_GREENBOX)->m_hWnd, false, true, false);
        PaintGradient(GetDlgItem(IDC_BLUEBOX)->m_hWnd, false, false, true);
    }

    return CWnd::WindowProc(message, wParam, lParam);
}
