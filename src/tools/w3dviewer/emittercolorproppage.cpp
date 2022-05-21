/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View emitter color prop page
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "emittercolorproppage.h"
#include "colorbar.h"
#include "colorpickerdialogclass.h"
#include "emitterinstancelist.h"
#include "opacitysettingsdialog.h"
#include "resource.h"

IMPLEMENT_DYNCREATE(EmitterColorPropPageClass, CPropertyPage)

// clang-format off
BEGIN_MESSAGE_MAP(EmitterColorPropPageClass, CPropertyPage)
    ON_WM_DESTROY()
    ON_NOTIFY(UDN_DELTAPOS, IDC_RSPIN, OnDeltaPos)
END_MESSAGE_MAP()
// clang-format on

EmitterColorPropPageClass::EmitterColorPropPageClass() :
    CPropertyPage(IDD_EMITTERCOLOR),
    m_instanceList(nullptr),
    m_isValid(true),
    m_colorBar(nullptr),
    m_opacityBar(nullptr),
    m_lifetime(0.0f)
{
    memset(&m_oldColorKeyFrames, 0, sizeof(m_oldColorKeyFrames));
    memset(&m_oldOpacityKeyFrames, 0, sizeof(m_oldOpacityKeyFrames));
    memset(&m_colorKeyFrames, 0, sizeof(m_colorKeyFrames));
    memset(&m_opacityKeyFrames, 0, sizeof(m_opacityKeyFrames));
    Initialize();
}

EmitterColorPropPageClass::~EmitterColorPropPageClass()
{
    if (m_oldColorKeyFrames.KeyTimes != nullptr) {
        delete[] m_oldColorKeyFrames.KeyTimes;
        m_oldColorKeyFrames.KeyTimes = nullptr;
    }

    if (m_oldColorKeyFrames.Values != nullptr) {
        delete[] m_oldColorKeyFrames.Values;
        m_oldColorKeyFrames.Values = nullptr;
    }

    if (m_oldOpacityKeyFrames.KeyTimes != nullptr) {
        delete[] m_oldOpacityKeyFrames.KeyTimes;
        m_oldOpacityKeyFrames.KeyTimes = nullptr;
    }

    if (m_oldOpacityKeyFrames.Values != nullptr) {
        delete[] m_oldOpacityKeyFrames.Values;
        m_oldOpacityKeyFrames.Values = nullptr;
    }

    if (m_colorKeyFrames.KeyTimes != nullptr) {
        delete[] m_colorKeyFrames.KeyTimes;
        m_colorKeyFrames.KeyTimes = nullptr;
    }

    if (m_colorKeyFrames.Values != nullptr) {
        delete[] m_colorKeyFrames.Values;
        m_colorKeyFrames.Values = nullptr;
    }

    if (m_opacityKeyFrames.KeyTimes != nullptr) {
        delete[] m_opacityKeyFrames.KeyTimes;
        m_opacityKeyFrames.KeyTimes = nullptr;
    }

    if (m_opacityKeyFrames.Values != nullptr) {
        delete[] m_opacityKeyFrames.Values;
        m_opacityKeyFrames.Values = nullptr;
    }
}

BOOL EmitterColorPropPageClass::OnCommand(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam)) {
        case IDC_RED:
            if (HIWORD(wParam) == EN_CHANGE) {
                SetModified();
            }

            if (HIWORD(wParam) == EN_KILLFOCUS && SendDlgItemMessage(LOWORD(wParam), EM_GETMODIFY, 0, 0) != 0) {
                SendDlgItemMessage(LOWORD(wParam), EM_SETMODIFY, 0, 0);
                m_colorKeyFrames.Rand.X = GetDlgItemInt(IDC_RED) / 255.0f;
                m_instanceList->Set_Color_Keyframes(m_colorKeyFrames);
                SetModified();
            }
            break;
        case IDC_GREEN:
            if (HIWORD(wParam) == EN_CHANGE) {
                SetModified();
            }

            if (HIWORD(wParam) == EN_KILLFOCUS && SendDlgItemMessage(LOWORD(wParam), EM_GETMODIFY, 0, 0) != 0) {
                SendDlgItemMessage(LOWORD(wParam), EM_SETMODIFY, 0, 0);
                m_colorKeyFrames.Rand.Y = GetDlgItemInt(IDC_GREEN) / 255.0f;
                m_instanceList->Set_Color_Keyframes(m_colorKeyFrames);
                SetModified();
            }
            break;
        case IDC_BLUE:
            if (HIWORD(wParam) == EN_CHANGE) {
                SetModified();
            }

            if (HIWORD(wParam) == EN_KILLFOCUS && SendDlgItemMessage(LOWORD(wParam), EM_GETMODIFY, 0, 0) != 0) {
                SendDlgItemMessage(LOWORD(wParam), EM_SETMODIFY, 0, 0);
                m_colorKeyFrames.Rand.Z = GetDlgItemInt(IDC_BLUE) / 255.0f;
                m_instanceList->Set_Color_Keyframes(m_colorKeyFrames);
                SetModified();
            }
            break;
        case IDC_OPACITY:
            if (HIWORD(wParam) == EN_CHANGE) {
                SetModified();
            }

            if (HIWORD(wParam) == EN_KILLFOCUS && SendDlgItemMessage(LOWORD(wParam), EM_GETMODIFY, 0, 0) != 0) {
                SendDlgItemMessage(LOWORD(wParam), EM_SETMODIFY, 0, 0);
                m_opacityKeyFrames.Rand = GetDlgItemInt(IDC_OPACITY) / 255.0f;
                m_instanceList->Set_Opacity_Keyframes(m_opacityKeyFrames);
                SetModified();
            }
            break;
    }

    return CWnd::OnCommand(wParam, lParam);
}

BOOL EmitterColorPropPageClass::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    NMHDR *hdr = reinterpret_cast<NMHDR *>(lParam);

    switch (hdr->idFrom) {
        case IDC_COLORBAR: {
            ColorBarNotify *cbn = reinterpret_cast<ColorBarNotify *>(lParam);

            if (hdr->code == CLBN_DBLCLK) {
                int red = cbn->red;
                int green = cbn->green;
                int blue = cbn->blue;

                if (ColorPickerDialogClass::DoDialog(&red, &green, &blue)) {
                    m_colorBar->SetColor(cbn->keyposition, cbn->keytime, red, green, blue, 3);
                    UpdateColor();
                    m_instanceList->Set_Color_Keyframes(m_colorKeyFrames);
                    SetModified();
                }
            } else if (hdr->code == CLBN_KEYCHANGING || hdr->code == CLBN_DELETE) {
                UpdateColor();
                m_instanceList->Set_Color_Keyframes(m_colorKeyFrames);
                SetModified();
            }

            break;
        }
        case IDC_OPACITYBAR: {
            ColorBarNotify *cbn = reinterpret_cast<ColorBarNotify *>(lParam);

            if (hdr->code == CLBN_DBLCLK) {
                int opacity = cbn->red / 255.0f;

                OpacitySettingsDialog dialog(opacity, this);

                if (dialog.DoModal() == IDOK) {
                    m_opacityBar->SetColor(cbn->keyposition,
                        cbn->keytime,
                        dialog.GetOpacity() * 255.0f,
                        dialog.GetOpacity() * 255.0f,
                        dialog.GetOpacity() * 255.0f,
                        3);
                    UpdateOpacity();
                    m_instanceList->Set_Opacity_Keyframes(m_opacityKeyFrames);
                    SetModified();
                }
            } else if (hdr->code == CLBN_KEYCHANGING || hdr->code == CLBN_DELETE) {
                UpdateOpacity();
                m_instanceList->Set_Opacity_Keyframes(m_opacityKeyFrames);
                SetModified();
            }

            break;
        }
        case IDC_RSPIN: {
            NMUPDOWN *nm = reinterpret_cast<NMUPDOWN *>(lParam);

            if (hdr->code == UDN_DELTAPOS) {
                m_colorKeyFrames.Rand.X = (nm->iPos + nm->iDelta) / 255.0f;
                m_instanceList->Set_Color_Keyframes(m_colorKeyFrames);
                SetModified();
            }

            break;
        }
        case IDC_GSPIN: {
            NMUPDOWN *nm = reinterpret_cast<NMUPDOWN *>(lParam);

            if (hdr->code == UDN_DELTAPOS) {
                m_colorKeyFrames.Rand.Y = (nm->iPos + nm->iDelta) / 255.0f;
                m_instanceList->Set_Color_Keyframes(m_colorKeyFrames);
                SetModified();
            }

            break;
        }
        case IDC_BSPIN: {
            NMUPDOWN *nm = reinterpret_cast<NMUPDOWN *>(lParam);

            if (hdr->code == UDN_DELTAPOS) {
                m_colorKeyFrames.Rand.Z = (nm->iPos + nm->iDelta) / 255.0f;
                m_instanceList->Set_Color_Keyframes(m_colorKeyFrames);
                SetModified();
            }

            break;
        }
        case IDC_OPACITYSPIN: {
            NMUPDOWN *nm = reinterpret_cast<NMUPDOWN *>(lParam);

            if (hdr->code == UDN_DELTAPOS) {
                m_opacityKeyFrames.Rand = (nm->iPos + nm->iDelta) / 255.0f;
                m_instanceList->Set_Opacity_Keyframes(m_opacityKeyFrames);
                SetModified();
            }

            break;
        }
    }

    return CPropertyPage::OnNotify(wParam, lParam, pResult);
}

void EmitterColorPropPageClass::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_OPACITYSPIN, m_opacitySpin);
    DDX_Control(pDX, IDC_RSPIN, m_redSpin);
    DDX_Control(pDX, IDC_GSPIN, m_greenSpin);
    DDX_Control(pDX, IDC_BSPIN, m_blueSpin);
}

BOOL EmitterColorPropPageClass::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_colorBar = static_cast<ColorBarClass *>(GetProp(::GetDlgItem(m_hWnd, IDC_COLORBAR), "CLASSPOINTER"));
    m_opacityBar = static_cast<ColorBarClass *>(GetProp(::GetDlgItem(m_hWnd, IDC_OPACITYBAR), "CLASSPOINTER"));
    m_colorBar->SetMinMax(0.0f, 1.0f);
    m_opacityBar->SetMinMax(0.0f, 1.0f);
    m_opacityBar->SetColor(0, 0.0f, 255.0f, 255.0f, 255.0f, 3);
    m_opacitySpin.SetRange(0, 100);
    m_redSpin.SetRange(0, 255);
    m_greenSpin.SetRange(0, 255);
    m_blueSpin.SetRange(0, 255);
    m_opacitySpin.SetPos(m_oldOpacityKeyFrames.Rand * 100.0f);
    m_redSpin.SetPos(m_oldColorKeyFrames.Rand.X * 255.0f);
    m_greenSpin.SetPos(m_oldColorKeyFrames.Rand.Y * 255.0f);
    m_blueSpin.SetPos(m_oldColorKeyFrames.Rand.Z * 255.0f);
    m_colorBar->Clear();
    m_opacityBar->Clear();
    m_colorBar->SetColor(0,
        0.0f,
        m_oldColorKeyFrames.Start.X * 255.0f,
        m_oldColorKeyFrames.Start.Y * 255.0f,
        m_oldColorKeyFrames.Start.Z * 255.0f,
        3);
    m_opacityBar->SetColor(0,
        0.0f,
        m_oldOpacityKeyFrames.Start * 255.0f,
        m_oldOpacityKeyFrames.Start * 255.0f,
        m_oldOpacityKeyFrames.Start * 255.0f,
        3);
    m_colorBar->SetMinMax(0.0f, 1.0f);
    m_opacityBar->SetMinMax(0.0f, 1.0f);

    for (unsigned int i = 0; i < m_oldColorKeyFrames.NumKeyFrames; i++) {
        m_colorBar->SetColor(i + 1,
            m_oldColorKeyFrames.KeyTimes[i] / m_lifetime,
            m_oldColorKeyFrames.Values[i].X * 255.0f,
            m_oldColorKeyFrames.Values[i].Y * 255.0f,
            m_oldColorKeyFrames.Values[i].Z * 255.0f,
            3);
    }

    for (unsigned int i = 0; i < m_oldOpacityKeyFrames.NumKeyFrames; i++) {
        m_colorBar->SetColor(i + 1,
            m_oldOpacityKeyFrames.KeyTimes[i] / m_lifetime,
            m_oldOpacityKeyFrames.Values[i] * 255.0f,
            m_oldOpacityKeyFrames.Values[i] * 255.0f,
            m_oldOpacityKeyFrames.Values[i] * 255.0f,
            3);
    }

    m_colorKeyFrames.Rand = m_oldColorKeyFrames.Rand;
    m_opacityKeyFrames.Rand = m_oldOpacityKeyFrames.Rand;
    UpdateColor();
    UpdateOpacity();
    return TRUE;
}

void EmitterColorPropPageClass::OnCancel()
{
    m_instanceList->Set_Color_Keyframes(m_oldColorKeyFrames);
    m_instanceList->Set_Opacity_Keyframes(m_oldOpacityKeyFrames);
    CPropertyPage::OnOK();
}

BOOL EmitterColorPropPageClass::OnApply()
{
    return CPropertyPage::OnApply();
}

void EmitterColorPropPageClass::UpdateLifetime(float lifetime)
{
    if (m_lifetime != lifetime) {
        float val = lifetime / m_lifetime;

        for (unsigned int i = 0; i < m_colorKeyFrames.NumKeyFrames; i++) {
            m_colorKeyFrames.KeyTimes[i] *= val;
        }

        for (unsigned int i = 0; i < m_opacityKeyFrames.NumKeyFrames; i++) {
            m_opacityKeyFrames.KeyTimes[i] *= val;
        }

        m_instanceList->Set_Color_Keyframes(m_colorKeyFrames);
        m_instanceList->Set_Opacity_Keyframes(m_opacityKeyFrames);
        m_lifetime = lifetime;
    }
}

void EmitterColorPropPageClass::OnDestroy()
{
    CWnd::OnDestroy();
}

void EmitterColorPropPageClass::OnDeltaPos(NMHDR *pNMHDR, LRESULT *pResult)
{
    *pResult = 0;
}

void EmitterColorPropPageClass::Initialize()
{
    if (m_oldColorKeyFrames.KeyTimes != nullptr) {
        delete[] m_oldColorKeyFrames.KeyTimes;
        m_oldColorKeyFrames.KeyTimes = nullptr;
    }

    if (m_oldColorKeyFrames.Values != nullptr) {
        delete[] m_oldColorKeyFrames.Values;
        m_oldColorKeyFrames.Values = nullptr;
    }

    if (m_oldOpacityKeyFrames.KeyTimes != nullptr) {
        delete[] m_oldOpacityKeyFrames.KeyTimes;
        m_oldOpacityKeyFrames.KeyTimes = nullptr;
    }

    if (m_oldOpacityKeyFrames.Values != nullptr) {
        delete[] m_oldOpacityKeyFrames.Values;
        m_oldOpacityKeyFrames.Values = nullptr;
    }

    if (m_colorKeyFrames.KeyTimes != nullptr) {
        delete[] m_colorKeyFrames.KeyTimes;
        m_colorKeyFrames.KeyTimes = nullptr;
    }

    if (m_colorKeyFrames.Values != nullptr) {
        delete[] m_colorKeyFrames.Values;
        m_colorKeyFrames.Values = nullptr;
    }

    if (m_opacityKeyFrames.KeyTimes != nullptr) {
        delete[] m_opacityKeyFrames.KeyTimes;
        m_opacityKeyFrames.KeyTimes = nullptr;
    }

    if (m_opacityKeyFrames.Values != nullptr) {
        delete[] m_opacityKeyFrames.Values;
        m_opacityKeyFrames.Values = nullptr;
    }

    if (m_instanceList != nullptr) {
        m_lifetime = m_instanceList->Get_Lifetime();
        m_instanceList->Get_Color_Keyframes(m_oldColorKeyFrames);
        m_instanceList->Get_Opacity_Keyframes(m_oldOpacityKeyFrames);
        m_instanceList->Get_Color_Keyframes(m_colorKeyFrames);
        m_instanceList->Get_Opacity_Keyframes(m_opacityKeyFrames);
    }
}

void EmitterColorPropPageClass::UpdateOpacity()
{
    float time = 0.0f;
    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;
    m_opacityBar->GetColor(0, &time, &red, &green, &blue);
    m_opacityKeyFrames.Start = red / 255.0f;

    if (m_opacityKeyFrames.KeyTimes != nullptr) {
        delete[] m_opacityKeyFrames.KeyTimes;
        m_opacityKeyFrames.KeyTimes = nullptr;
    }

    if (m_opacityKeyFrames.Values != nullptr) {
        delete[] m_opacityKeyFrames.Values;
        m_opacityKeyFrames.Values = nullptr;
    }

    int count = m_opacityBar->GetKeyCount();
    m_opacityKeyFrames.NumKeyFrames = count - 1;

    if (count > 1) {
        m_opacityKeyFrames.KeyTimes = new float[count - 1];
        m_opacityKeyFrames.Values = new float[count - 1];
        int i = 1;

        do {
            m_opacityBar->GetColor(i, &time, &red, &green, &blue);
            m_opacityKeyFrames.KeyTimes[i - 1] = time * m_lifetime;
            m_opacityKeyFrames.Values[i - 1] = red / 255.0f;
            i++;
        } while (i < count);
    }
}

void EmitterColorPropPageClass::UpdateColor()
{
    float time = 0.0f;
    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;
    m_colorBar->GetColor(0, &time, &red, &green, &blue);
    m_colorKeyFrames.Start.X = red / 255.0f;
    m_colorKeyFrames.Start.Y = green / 255.0f;
    m_colorKeyFrames.Start.Z = blue / 255.0f;

    if (m_colorKeyFrames.KeyTimes != nullptr) {
        delete[] m_colorKeyFrames.KeyTimes;
        m_colorKeyFrames.KeyTimes = nullptr;
    }

    if (m_colorKeyFrames.Values != nullptr) {
        delete[] m_colorKeyFrames.Values;
        m_colorKeyFrames.Values = nullptr;
    }

    int count = m_colorBar->GetKeyCount();
    m_colorKeyFrames.NumKeyFrames = count - 1;

    if (count > 1) {
        m_colorKeyFrames.KeyTimes = new float[count - 1];
        m_colorKeyFrames.Values = new Vector3[count - 1];
        int i = 1;

        do {
            m_colorBar->GetColor(i, &time, &red, &green, &blue);
            m_colorKeyFrames.KeyTimes[i - 1] = time * m_lifetime;
            m_colorKeyFrames.Values[i - 1].X = red / 255.0f;
            m_colorKeyFrames.Values[i - 1].Y = green / 255.0f;
            m_colorKeyFrames.Values[i - 1].Z = blue / 255.0f;
            i++;
        } while (i < count);
    }
}
