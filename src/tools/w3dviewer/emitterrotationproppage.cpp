/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View emitter rotation prop page
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "emitterrotationproppage.h"
#include "colorbar.h"
#include "emitterinstancelist.h"
#include "particlerotationkeydialog.h"
#include "resource.h"
#include "utils.h"

IMPLEMENT_DYNCREATE(EmitterRotationPropPageClass, CPropertyPage)

// clang-format off
BEGIN_MESSAGE_MAP(EmitterRotationPropPageClass, CPropertyPage)
END_MESSAGE_MAP()
// clang-format on

EmitterRotationPropPageClass::EmitterRotationPropPageClass() :
    CPropertyPage(IDD_EMITTERROT),
    m_instanceList(nullptr),
    m_isValid(true),
    m_rotationBar(nullptr),
    m_lifetime(0.0f),
    m_minRotation(0.0f),
    m_maxRotation(1.0f),
    m_initialOrientation(0.0f)
{
    memset(&m_rotationKeyFrames, 0, sizeof(m_rotationKeyFrames));
    Initialize();
}

EmitterRotationPropPageClass::~EmitterRotationPropPageClass()
{
    if (m_rotationKeyFrames.KeyTimes != nullptr) {
        delete[] m_rotationKeyFrames.KeyTimes;
    }

    if (m_rotationKeyFrames.Values != nullptr) {
        delete[] m_rotationKeyFrames.Values;
    }
}

BOOL EmitterRotationPropPageClass::OnCommand(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam)) {
        case IDC_VELOCITY:
            if (HIWORD(wParam) == EN_CHANGE) {
                SetModified();
            }

            if (HIWORD(wParam) == EN_KILLFOCUS && SendDlgItemMessage(LOWORD(wParam), EM_GETMODIFY, 0, 0) != 0) {
                SendDlgItemMessage(LOWORD(wParam), EM_SETMODIFY, 0, 0);
                m_rotationKeyFrames.Rand = GetDlgItemFloat(m_hWnd, IDC_VELOCITY);
                m_instanceList->Set_Rotation_Keyframes(m_rotationKeyFrames, m_initialOrientation);
                SetModified();
            }

            break;
        case IDC_ORIENTATION:
            if (HIWORD(wParam) == EN_CHANGE) {
                SetModified();
            }

            if (HIWORD(wParam) == EN_KILLFOCUS && SendDlgItemMessage(LOWORD(wParam), EM_GETMODIFY, 0, 0) != 0) {
                SendDlgItemMessage(LOWORD(wParam), EM_SETMODIFY, 0, 0);
                m_initialOrientation = GetDlgItemFloat(m_hWnd, IDC_ORIENTATION);
                m_instanceList->Set_Rotation_Keyframes(m_rotationKeyFrames, m_initialOrientation);
                SetModified();
            }

            break;
    }

    return CWnd::OnCommand(wParam, lParam);
}

BOOL EmitterRotationPropPageClass::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    NMHDR *hdr = reinterpret_cast<NMHDR *>(lParam);

    if (hdr != nullptr && hdr->code == UDN_DELTAPOS) {
        NMUPDOWN *nm = reinterpret_cast<NMUPDOWN *>(lParam);
        UpdateEditCtrl(hdr->hwndFrom, nm->iDelta);
    }

    switch (hdr->idFrom) {
        case IDC_VELOCITYSPIN: {
            NMUPDOWN *nm = reinterpret_cast<NMUPDOWN *>(lParam);

            if (hdr->code == UDN_DELTAPOS) {
                m_rotationKeyFrames.Rand = GetDlgItemFloat(m_hWnd, IDC_VELOCITY);
                m_instanceList->Set_Rotation_Keyframes(m_rotationKeyFrames, m_initialOrientation);
                SetModified();
            }

            break;
        }
        case IDC_ROTATIONBAR: {
            ColorBarNotify *cbn = reinterpret_cast<ColorBarNotify *>(lParam);

            if (hdr->code == CLBN_DBLCLK) {
                float rot =
                    (m_maxRotation - m_minRotation) * m_rotationBar->GetGradientValue(cbn->keyposition) + m_minRotation;
                ParticleRotationKeyDialog dlg(rot, this);

                if (dlg.DoModal() == IDOK) {
                    float min = dlg.GetRotation();
                    float max;
                    m_rotationBar->EnableRedraw(false);
                    m_rotationBar->SetGradientValue(
                        cbn->keyposition, (dlg.GetRotation() - m_minRotation) / (m_maxRotation - m_minRotation));

                    if (min <= 1.0f) {
                        max = 1.0f;
                    } else {
                        max = min;
                    }

                    if (min >= 0.0f) {
                        min = 0.0f;
                    }

                    for (int i = 0; i < m_rotationBar->GetKeyCount(); i++) {
                        int val = m_rotationBar->GetGradientValue(i) * (m_maxRotation - m_minRotation);

                        if (val > max) {
                            max = val;
                        }

                        if (val < min) {
                            min = val;
                        }
                    }

                    if (max != m_maxRotation || min != m_minRotation) {
                        for (int i = 0; i < m_rotationBar->GetKeyCount(); i++) {
                            m_rotationBar->SetGradientValue(i,
                                (m_rotationBar->GetGradientValue(i) * (m_maxRotation - m_minRotation) + m_minRotation - min)
                                    / (max - min));
                        }

                        m_minRotation = min;
                        m_maxRotation = max;
                    }

                    m_rotationBar->EnableRedraw(true);
                    UpdateRotation();
                    m_instanceList->Set_Rotation_Keyframes(m_rotationKeyFrames, m_initialOrientation);
                    SetModified();
                }

            } else if (hdr->code == CLBN_KEYCHANGING || hdr->code == CLBN_DELETE) {
                UpdateRotation();
                m_instanceList->Set_Rotation_Keyframes(m_rotationKeyFrames, m_initialOrientation);
                SetModified();
            }

            break;
        }
        case IDC_ORIENTATIONSPIN: {
            NMUPDOWN *nm = reinterpret_cast<NMUPDOWN *>(lParam);

            if (hdr->code == UDN_DELTAPOS) {
                m_initialOrientation = GetDlgItemFloat(m_hWnd, IDC_ORIENTATION);
                m_instanceList->Set_Rotation_Keyframes(m_rotationKeyFrames, m_initialOrientation);
                SetModified();
            }

            break;
        }
    }

    return CPropertyPage::OnNotify(wParam, lParam, pResult);
}

void EmitterRotationPropPageClass::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_ORIENTATIONSPIN, m_rotationSpin);
    DDX_Control(pDX, IDC_VELOCITYSPIN, m_velocitySpin);
}

BOOL EmitterRotationPropPageClass::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_rotationBar = static_cast<ColorBarClass *>(GetProp(::GetDlgItem(m_hWnd, IDC_ROTATIONBAR), "CLASSPOINTER"));
    InitializeSpinButton(&m_velocitySpin, m_rotationKeyFrames.Rand, 0.0f, 10000.0f);
    InitializeSpinButton(&m_rotationSpin, m_initialOrientation, 0.0f, 10000.0f);
    m_rotationBar->SetMinMax(0.0f, 1.0f);
    m_rotationBar->Clear();
    m_rotationBar->SetColor(0, 0.0f, 0.0f, 0.0f, 0.0f, 3);
    m_rotationBar->SetGradientValue(0, (m_rotationKeyFrames.Start - m_minRotation) / (m_maxRotation - m_minRotation));

    for (unsigned int i = 0; i < m_rotationKeyFrames.NumKeyFrames; i++) {
        m_rotationBar->SetColor(i + 1, m_rotationKeyFrames.KeyTimes[i] / m_lifetime, 0.0f, 0.0f, 0.0f, 3);
        m_rotationBar->SetGradientValue(
            i + 1, (m_rotationKeyFrames.Values[i] - m_minRotation) / (m_maxRotation - m_minRotation));
    }

    return TRUE;
}

void EmitterRotationPropPageClass::UpdateLifetime(float lifetime)
{
    if (m_lifetime != lifetime) {
        float val = lifetime / m_lifetime;

        for (unsigned int i = 0; i < m_rotationKeyFrames.NumKeyFrames; i++) {
            m_rotationKeyFrames.KeyTimes[i] *= val;
        }

        m_instanceList->Set_Rotation_Keyframes(m_rotationKeyFrames, m_initialOrientation);
        m_lifetime = lifetime;
    }
}

void EmitterRotationPropPageClass::Initialize()
{
    if (m_rotationKeyFrames.KeyTimes != nullptr) {
        delete[] m_rotationKeyFrames.KeyTimes;
    }

    if (m_rotationKeyFrames.Values != nullptr) {
        delete[] m_rotationKeyFrames.Values;
    }

    if (m_instanceList != nullptr) {
        m_lifetime = m_instanceList->Get_Lifetime();
        m_instanceList->Get_Rotation_Keyframes(m_rotationKeyFrames);
        int max = m_rotationKeyFrames.Start;

        if (max <= 1.0f) {
            max = 1.0f;
        }

        m_maxRotation = max;
        int min = m_rotationKeyFrames.Start;

        if (min >= 0.0f) {
            min = 0.0f;
        }

        m_minRotation = min;

        for (unsigned int i = 0; i < m_rotationKeyFrames.NumKeyFrames; i++) {
            if (m_rotationKeyFrames.Values[i] > m_maxRotation) {
                m_maxRotation = m_rotationKeyFrames.Values[i];
            }

            if (m_rotationKeyFrames.Values[i] < m_minRotation) {
                m_minRotation = m_rotationKeyFrames.Values[i];
            }
        }
    }
}

void EmitterRotationPropPageClass::UpdateRotation()
{
    float time = 0.0f;
    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;
    float value = m_rotationBar->GetGradientValue(0);
    m_rotationKeyFrames.Start = (m_maxRotation - m_minRotation) * value + m_minRotation;

    if (m_rotationKeyFrames.KeyTimes != nullptr) {
        delete[] m_rotationKeyFrames.KeyTimes;
    }

    if (m_rotationKeyFrames.Values != nullptr) {
        delete[] m_rotationKeyFrames.Values;
    }

    int count = m_rotationBar->GetKeyCount();
    m_rotationKeyFrames.NumKeyFrames = count - 1;

    if (count > 1) {
        m_rotationKeyFrames.KeyTimes = new float[count - 1];
        m_rotationKeyFrames.Values = new float[count - 1];
        int i = 1;

        do {
            m_rotationBar->GetColor(i, &time, &red, &green, &blue);
            m_rotationKeyFrames.KeyTimes[i - 1] = time * m_lifetime;
            m_rotationKeyFrames.Values[i - 1] =
                (m_maxRotation - m_minRotation) * m_rotationBar->GetGradientValue(i) + m_minRotation;
            i++;
        } while (i < count);
    }
}
