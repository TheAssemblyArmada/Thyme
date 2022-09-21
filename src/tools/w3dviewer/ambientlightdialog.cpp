/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View Ambient Light Dialog
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "ambientlightdialog.h"
#include "resource.h"
#include "utils.h"
#include "viewerscene.h"
#include "w3dviewdoc.h"

// clang-format off
BEGIN_MESSAGE_MAP(CAmbientLightDialog, CDialog)
    ON_WM_HSCROLL()
    ON_COMMAND(IDC_GREYSCALE, OnGreyscale)
END_MESSAGE_MAP()
// clang-format on

CAmbientLightDialog::CAmbientLightDialog(CWnd *pParentWnd) :
    CDialog(IDD_AMBIENT, pParentWnd), m_currentRed(0), m_currentGreen(0), m_currentBlue(0)
{
}

void CAmbientLightDialog::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_BLUESLIDER, m_blueSlider);
    DDX_Control(pDX, IDC_GREENSLIDER, m_greenSlider);
    DDX_Control(pDX, IDC_REDSLIDER, m_redSlider);
}

BOOL CAmbientLightDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    PositionWindow(this->m_hWnd);
    m_redSlider.SetRange(0, 100, 0);
    m_greenSlider.SetRange(0, 100, 0);
    m_blueSlider.SetRange(0, 100, 0);
    CW3DViewDoc *doc = GetCurrentDocument();

    if (doc != nullptr) {
        if (doc->m_scene != nullptr) {
            Vector3 ambient = doc->m_scene->Get_Ambient_Light();
            m_currentRed = ambient.X * 100.0f;
            m_currentGreen = ambient.Y * 100.0f;
            m_currentBlue = ambient.Z * 100.0f;
        }
    }

    if (m_currentRed == m_currentGreen && m_currentRed == m_currentBlue) {
        SendDlgItemMessage(IDC_GREYSCALE, BM_SETCHECK, TRUE, 0);
    }

    m_redSlider.SetPos(m_currentRed);
    m_greenSlider.SetPos(m_currentGreen);
    m_blueSlider.SetPos(m_currentBlue);
    return TRUE;
}

void CAmbientLightDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
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
        if (doc->m_scene != nullptr) {
            Vector3 ambient;
            ambient.X = m_redSlider.GetPos() / 100.0f;
            ambient.Y = m_greenSlider.GetPos() / 100.0f;
            ambient.Z = m_blueSlider.GetPos() / 100.0f;
            doc->m_scene->Set_Ambient_Light(ambient);
        }
    }

    CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CAmbientLightDialog::OnGreyscale()
{
    if (SendDlgItemMessage(IDC_GREYSCALE, BM_GETCHECK) != 0) {
        m_greenSlider.SetPos(m_redSlider.GetPos());
        m_blueSlider.SetPos(m_redSlider.GetPos());
    }

    CW3DViewDoc *doc = GetCurrentDocument();

    if (doc != nullptr) {
        if (doc->m_scene != nullptr) {
            Vector3 ambient;
            ambient.X = m_redSlider.GetPos() / 100.0f;
            ambient.Y = m_greenSlider.GetPos() / 100.0f;
            ambient.Z = m_blueSlider.GetPos() / 100.0f;
            doc->m_scene->Set_Ambient_Light(ambient);
        }
    }
}

void CAmbientLightDialog::OnCancel()
{
    CW3DViewDoc *doc = GetCurrentDocument();

    if (doc != nullptr) {
        if (doc->m_scene != nullptr) {
            Vector3 ambient;
            ambient.X = m_currentRed / 100.0f;
            ambient.Y = m_currentGreen / 100.0f;
            ambient.Z = m_currentBlue / 100.0f;
            doc->m_scene->Set_Ambient_Light(ambient);
        }
    }

    CDialog::OnCancel();
}

LRESULT CAmbientLightDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_PAINT) {
        PaintGradient(GetDlgItem(IDC_REDBOX)->m_hWnd, true, false, false);
        PaintGradient(GetDlgItem(IDC_GREENBOX)->m_hWnd, false, true, false);
        PaintGradient(GetDlgItem(IDC_BLUEBOX)->m_hWnd, false, false, true);
    }

    return CWnd::WindowProc(message, wParam, lParam);
}
