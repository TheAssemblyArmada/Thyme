/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View Scene Light Dialog
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "scenelightdialog.h"
#include "light.h"
#include "resource.h"
#include "utils.h"
#include "w3dviewdoc.h"

// clang-format off
BEGIN_MESSAGE_MAP(CSceneLightDialog, CDialog)
    ON_WM_HSCROLL()
    ON_COMMAND(IDC_GREYSCALE, OnGreyscale)
    ON_COMMAND(IDC_BOTH, OnBoth)
    ON_COMMAND(IDC_DIFFUSE, OnDiffuse)
    ON_COMMAND(IDC_SPECULAR, OnSpecular)
    ON_COMMAND(IDC_ATTENUATION, OnAttenuation)
END_MESSAGE_MAP()
// clang-format on

CSceneLightDialog::CSceneLightDialog(CWnd *pParentWnd) :
    CDialog(IDD_SCENELIGHT, pParentWnd),
    m_diffuseRed(0),
    m_diffuseGreen(0),
    m_diffuseBlue(0),
    m_specularRed(0),
    m_specularGreen(0),
    m_specularBlue(0),
    m_flags(LIGHTING_DIFFUSE),
    m_farAttenStart(0.0f),
    m_farAttenEnd(0.0f),
    m_distance(0.0f),
    m_intensity(0.0f),
    m_attenuation(1)
{
}

void CSceneLightDialog::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_STARTSPIN, m_startSpin);
    DDX_Control(pDX, IDC_ENDSPIN, m_endSpin);
    DDX_Control(pDX, IDC_DISTANCESPIN, m_distanceSpin);
    DDX_Control(pDX, IDC_INTENSITYSLIDER, m_intensitySlider);
    DDX_Control(pDX, IDC_BLUESLIDER, m_blueSlider);
    DDX_Control(pDX, IDC_GREENSLIDER, m_greenSlider);
    DDX_Control(pDX, IDC_REDSLIDER, m_redSlider);
}

LRESULT CSceneLightDialog::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_PAINT) {
        PaintGradient(GetDlgItem(IDC_REDBOX)->m_hWnd, true, false, false);
        PaintGradient(GetDlgItem(IDC_GREENBOX)->m_hWnd, false, true, false);
        PaintGradient(GetDlgItem(IDC_BLUEBOX)->m_hWnd, false, false, true);
    } else if (message == WM_NOTIFY) {
        NMUPDOWN *nm = reinterpret_cast<NMUPDOWN *>(lParam);

        if (nm != nullptr && nm->hdr.code == UDN_DELTAPOS) {
            HWND edit = reinterpret_cast<HWND>(SendDlgItemMessage(wParam, UDM_GETBUDDY, 0, 0));

            if (IsWindow(edit)) {
                SetWindowFloat(edit, nm->iDelta * 100.0f + GetWindowFloat(edit));
                SetDistance(GetDlgItemFloat(m_hWnd, IDC_DISTANCE));
                CW3DViewDoc *doc = GetCurrentDocument();

                if (doc != nullptr && doc->m_scene != nullptr) {
                    doc->m_light->Set_Far_Attenuation_Range(
                        GetDlgItemFloat(m_hWnd, IDC_START), GetDlgItemFloat(m_hWnd, IDC_END));
                }
            }
        }
    } else if (message == WM_COMMAND && HIWORD(wParam) == EN_KILLFOCUS) {
        if (LOWORD(wParam) == IDC_DISTANCE) {
            SetDistance(GetDlgItemFloat(m_hWnd, IDC_DISTANCE));
            CW3DViewDoc *doc = GetCurrentDocument();
        } else if (LOWORD(wParam) >= IDC_START && LOWORD(wParam) <= IDC_END) {
            CW3DViewDoc *doc = GetCurrentDocument();

            if (doc != nullptr && doc->m_scene != nullptr) {
                doc->m_light->Set_Far_Attenuation_Range(
                    GetDlgItemFloat(m_hWnd, IDC_START), GetDlgItemFloat(m_hWnd, IDC_END));
            }
        }
    }

    return CWnd::WindowProc(message, wParam, lParam);
}

BOOL CSceneLightDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    PositionWindow(m_hWnd);
    m_redSlider.SetRange(0, 100);
    m_greenSlider.SetRange(0, 100);
    m_blueSlider.SetRange(0, 100);
    CW3DViewDoc *doc = GetCurrentDocument();

    if (doc != nullptr && doc->m_scene != nullptr) {
        Vector3 diffuse;
        Vector3 specular;
        doc->m_light->Get_Diffuse(&diffuse);
        doc->m_light->Get_Specular(&specular);

        m_diffuseRed = diffuse.X * 100.0f;
        m_diffuseGreen = diffuse.Y * 100.0f;
        m_diffuseBlue = diffuse.Z * 100.0f;
        m_specularRed = specular.X * 100.0f;
        m_specularGreen = specular.Y * 100.0f;
        m_specularBlue = specular.Z * 100.0f;

        if (m_diffuseRed == m_diffuseGreen && m_diffuseRed == m_diffuseGreen) {
            SendDlgItemMessage(IDC_GREYSCALE, BM_SETCHECK, TRUE, 0);
        }

        m_redSlider.SetPos(m_diffuseRed);
        m_greenSlider.SetPos(m_diffuseGreen);
        m_blueSlider.SetPos(m_diffuseBlue);

        int attenuation = doc->m_light->Get_Flag(LightClass::FAR_ATTENUATION);
        SendDlgItemMessage(IDC_ATTENUATION, BM_SETCHECK, TRUE, attenuation);
        double start;
        double end;
        doc->m_light->Get_Far_Attenuation_Range(start, end);
        float distance = 0.0f;
        float intensity = doc->m_light->Get_Intensity();

        if (doc->m_model != nullptr) {
            distance = (doc->m_light->Get_Position() - doc->m_model->Get_Position()).Length();
        }

        SetDlgItemFloat(m_hWnd, IDC_START, start);
        SetDlgItemFloat(m_hWnd, IDC_END, end);
        SetDlgItemFloat(m_hWnd, IDC_DISTANCE, distance);

        m_distanceSpin.SetRange(0, 16960);
        m_distanceSpin.SetPos(distance * 100.0f);
        m_startSpin.SetRange(0, 16960);
        m_startSpin.SetPos(start * 100.0f);
        m_endSpin.SetRange(0, 16960);
        m_endSpin.SetPos(end * 100.0f);
        m_intensitySlider.SetRange(0, 100);
        m_intensitySlider.SetPos(intensity * 100.0f);

        m_farAttenStart = start;
        m_farAttenEnd = end;
        m_distance = distance;
        m_intensity = intensity;
        m_attenuation = attenuation;
    }

    SendDlgItemMessage(IDC_DIFFUSE, BM_SETCHECK, TRUE, 0);
    bool enable = SendDlgItemMessage(IDC_ATTENUATION, BM_GETCHECK) == 1;
    GetDlgItem(IDC_START)->EnableWindow(enable);
    GetDlgItem(IDC_STARTSPIN)->EnableWindow(enable);
    GetDlgItem(IDC_END)->EnableWindow(enable);
    GetDlgItem(IDC_ENDSPIN)->EnableWindow(enable);

    return TRUE;
}

void CSceneLightDialog::OnCancel()
{
    CW3DViewDoc *doc = GetCurrentDocument();

    if (doc != nullptr && doc->m_scene != nullptr) {
        doc->m_light->Set_Diffuse(Vector3(m_diffuseRed / 100.0f, m_diffuseGreen / 100.0f, m_diffuseBlue / 100.0f));
        doc->m_light->Set_Specular(Vector3(m_specularRed / 100.0f, m_specularGreen / 100.0f, m_specularBlue / 100.0f));
        doc->m_light->Set_Intensity(m_intensity);
        doc->m_light->Set_Far_Attenuation_Range(m_farAttenStart, m_farAttenEnd);
        doc->m_light->Set_Flag(LightClass::FAR_ATTENUATION, m_attenuation == 1);
        SetDistance(m_distance);
    }

    CDialog::OnCancel();
}

void CSceneLightDialog::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar *pScrollBar)
{
    if (pScrollBar == GetDlgItem(IDC_INTENSITYSLIDER)) {
        GetCurrentDocument()->m_light->Set_Intensity(m_intensitySlider.GetPos() / 100.0f);
    } else {
        if (SendDlgItemMessage(IDC_GREYSCALE, BM_GETCHECK)) {
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

        if (doc != nullptr && doc->m_scene != nullptr) {
            Vector3 color;
            color.X = m_redSlider.GetPos() / 100.0f;
            color.Y = m_greenSlider.GetPos() / 100.0f;
            color.Z = m_blueSlider.GetPos() / 100.0f;

            if ((m_flags & LIGHTING_DIFFUSE) != 0) {
                doc->m_light->Set_Diffuse(color);
            }

            if ((m_flags & LIGHTING_SPECULAR) != 0) {
                doc->m_light->Set_Specular(color);
            }
        }
    }

    CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CSceneLightDialog::OnGreyscale()
{
    if (SendDlgItemMessage(IDC_GREYSCALE, BM_GETCHECK) != 0) {
        m_greenSlider.SetPos(m_redSlider.GetPos());
        m_blueSlider.SetPos(m_redSlider.GetPos());
    }

    CW3DViewDoc *doc = GetCurrentDocument();

    if (doc != nullptr && doc->m_scene != nullptr) {
        Vector3 color;
        color.X = m_redSlider.GetPos() / 100.0f;
        color.Y = m_greenSlider.GetPos() / 100.0f;
        color.Z = m_blueSlider.GetPos() / 100.0f;

        if ((m_flags & LIGHTING_DIFFUSE) != 0) {
            doc->m_light->Set_Diffuse(color);
        }

        if ((m_flags & LIGHTING_SPECULAR) != 0) {
            doc->m_light->Set_Specular(color);
        }
    }
}

void CSceneLightDialog::OnBoth()
{
    m_flags = LIGHTING_DIFFUSE | LIGHTING_SPECULAR;
}

void CSceneLightDialog::OnDiffuse()
{
    Vector3 color;
    GetCurrentDocument()->m_light->Get_Diffuse(&color);

    if (color.X == color.Y && color.X == color.Z) {
        SendDlgItemMessage(IDC_GREYSCALE, BM_SETCHECK, TRUE, 0);
    }

    m_redSlider.SetPos(color.X * 100.0f);
    m_greenSlider.SetPos(color.Y * 100.0f);
    m_blueSlider.SetPos(color.Z * 100.0f);
    m_flags = LIGHTING_DIFFUSE;
}

void CSceneLightDialog::OnSpecular()
{
    Vector3 color;
    GetCurrentDocument()->m_light->Get_Specular(&color);

    if (color.X == color.Y && color.X == color.Z) {
        SendDlgItemMessage(IDC_GREYSCALE, BM_SETCHECK, TRUE, 0);
    }

    m_redSlider.SetPos(color.X * 100.0f);
    m_greenSlider.SetPos(color.Y * 100.0f);
    m_blueSlider.SetPos(color.Z * 100.0f);
    m_flags = LIGHTING_SPECULAR;
}

void CSceneLightDialog::OnAttenuation()
{
    bool enable = SendDlgItemMessage(IDC_ATTENUATION, BM_GETCHECK) == 1;
    GetCurrentDocument()->m_light->Set_Flag(LightClass::FAR_ATTENUATION, enable);
    GetDlgItem(IDC_START)->EnableWindow(enable);
    GetDlgItem(IDC_STARTSPIN)->EnableWindow(enable);
    GetDlgItem(IDC_END)->EnableWindow(enable);
    GetDlgItem(IDC_ENDSPIN)->EnableWindow(enable);
}

void CSceneLightDialog::SetDistance(float distance)
{
    CW3DViewDoc *doc = GetCurrentDocument();

    if (doc != nullptr && doc->m_scene != nullptr) {
        Vector3 pos(0.0f, 0.0f, 0.0f);

        if (doc->m_model != nullptr) {
            pos = doc->m_model->Get_Position();
        }

        Vector3 lightpos = doc->m_light->Get_Position();
        Vector3 diff = lightpos = pos;
        diff.Normalize();
        diff *= distance;
        doc->m_light->Set_Position(diff);
    }
}
