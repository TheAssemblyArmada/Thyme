/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View emitter line group prop page
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "emitterlinegroupproppage.h"
#include "colorbar.h"
#include "emitterinstancelist.h"
#include "particleblurtimekeydialog.h"
#include "resource.h"
#include "utils.h"

IMPLEMENT_DYNCREATE(EmitterLineGroupPropPageClass, CPropertyPage)

// clang-format off
BEGIN_MESSAGE_MAP(EmitterLineGroupPropPageClass, CPropertyPage)
END_MESSAGE_MAP()
// clang-format on

EmitterLineGroupPropPageClass::EmitterLineGroupPropPageClass() :
    CPropertyPage(IDD_LINEGROUP),
    m_instanceList(nullptr),
    m_isValid(true),
    m_blurTimeBar(nullptr),
    m_lifetime(0.0f),
    m_minBlurTime(0.0f),
    m_maxBlurTime(1.0f)
{
    memset(&m_blurTimeKeyFrames, 0, sizeof(m_blurTimeKeyFrames));
    Initialize();
}

EmitterLineGroupPropPageClass::~EmitterLineGroupPropPageClass()
{
    if (m_blurTimeKeyFrames.KeyTimes != nullptr) {
        delete[] m_blurTimeKeyFrames.KeyTimes;
        m_blurTimeKeyFrames.KeyTimes = nullptr;
    }

    if (m_blurTimeKeyFrames.Values != nullptr) {
        delete[] m_blurTimeKeyFrames.Values;
        m_blurTimeKeyFrames.Values = nullptr;
    }
}

BOOL EmitterLineGroupPropPageClass::OnCommand(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam)) {
        case IDC_LINE:
            if (HIWORD(wParam) == EN_CHANGE) {
                SetModified();
            }

            if (HIWORD(wParam) == EN_KILLFOCUS && SendDlgItemMessage(LOWORD(wParam), EM_GETMODIFY, 0, 0) != 0) {
                SendDlgItemMessage(LOWORD(wParam), EM_SETMODIFY, 0, 0);
                m_blurTimeKeyFrames.Rand = GetDlgItemInt(IDC_LINE) / 255.0f;
                m_instanceList->Set_Blur_Time_Keyframes(m_blurTimeKeyFrames);
                SetModified();
            }

            break;
    }

    return CWnd::OnCommand(wParam, lParam);
}

BOOL EmitterLineGroupPropPageClass::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    NMHDR *hdr = reinterpret_cast<NMHDR *>(lParam);

    if (hdr != nullptr && hdr->code == UDN_DELTAPOS) {
        NMUPDOWN *nm = reinterpret_cast<NMUPDOWN *>(lParam);
        UpdateEditCtrl(hdr->hwndFrom, nm->iDelta);
    }

    switch (hdr->idFrom) {
        case IDC_LINEBAR: {
            ColorBarNotify *cbn = reinterpret_cast<ColorBarNotify *>(lParam);

            if (hdr->code == CLBN_DBLCLK) {
                float blurtime =
                    (m_maxBlurTime - m_minBlurTime) * m_blurTimeBar->GetGradientValue(cbn->keyposition) + m_minBlurTime;
                ParticleBlurTimeKeyDialog dlg(blurtime, this);

                if (dlg.DoModal() == IDOK) {
                    float min = dlg.GetBlurTime();
                    float max;
                    m_blurTimeBar->EnableRedraw(false);
                    m_blurTimeBar->SetGradientValue(
                        cbn->keyposition, (dlg.GetBlurTime() - m_minBlurTime) / (m_maxBlurTime - m_minBlurTime));

                    if (min <= 1.0f) {
                        max = 1.0f;
                    } else {
                        max = min;
                    }

                    if (min >= 0.0f) {
                        min = 0.0f;
                    }

                    for (int i = 0; i < m_blurTimeBar->GetKeyCount(); i++) {
                        int val = m_blurTimeBar->GetGradientValue(i) * (m_maxBlurTime - m_minBlurTime);

                        if (val > max) {
                            max = val;
                        }

                        if (val < min) {
                            min = val;
                        }
                    }

                    if (max != m_maxBlurTime || min != m_minBlurTime) {
                        for (int i = 0; i < m_blurTimeBar->GetKeyCount(); i++) {
                            m_blurTimeBar->SetGradientValue(i,
                                (m_blurTimeBar->GetGradientValue(i) * (m_maxBlurTime - m_minBlurTime) + m_minBlurTime - min)
                                    / (max - min));
                        }

                        m_minBlurTime = min;
                        m_maxBlurTime = max;
                    }

                    m_blurTimeBar->EnableRedraw(true);
                    UpdateBlurTime();
                    m_instanceList->Set_Blur_Time_Keyframes(m_blurTimeKeyFrames);
                    SetModified();
                }

            } else if (hdr->code == CLBN_KEYCHANGING || hdr->code == CLBN_DELETE) {
                UpdateBlurTime();
                m_instanceList->Set_Blur_Time_Keyframes(m_blurTimeKeyFrames);
                SetModified();
            }

            break;
        }
        case IDC_BLURTIMESPIN: {
            NMUPDOWN *nm = reinterpret_cast<NMUPDOWN *>(lParam);

            if (hdr->code == UDN_DELTAPOS) {
                m_blurTimeKeyFrames.Rand = GetDlgItemFloat(m_hWnd, IDC_BLURTIME);
                m_instanceList->Set_Blur_Time_Keyframes(m_blurTimeKeyFrames);
                SetModified();
            }

            break;
        }
    }

    return CPropertyPage::OnNotify(wParam, lParam, pResult);
}

void EmitterLineGroupPropPageClass::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_LINESPIN, m_blurTimeSpin);
}

BOOL EmitterLineGroupPropPageClass::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_blurTimeBar = static_cast<ColorBarClass *>(GetProp(::GetDlgItem(m_hWnd, IDC_LINEBAR), "CLASSPOINTER"));
    InitializeSpinButton(&m_blurTimeSpin, m_blurTimeKeyFrames.Rand, 0.0f, 10000.0f);
    m_blurTimeBar->SetMinMax(0.0f, 1.0f);
    m_blurTimeBar->Clear();
    m_blurTimeBar->SetColor(0, 0.0f, 0.0f, 0.0f, 0.0f, 3);
    m_blurTimeBar->SetGradientValue(0, (m_blurTimeKeyFrames.Start - m_minBlurTime) / (m_maxBlurTime - m_minBlurTime));

    for (unsigned int i = 0; i < m_blurTimeKeyFrames.NumKeyFrames; i++) {
        m_blurTimeBar->SetColor(i + 1, m_blurTimeKeyFrames.KeyTimes[i] / m_lifetime, 0.0f, 0.0f, 0.0f, 3);
        m_blurTimeBar->SetGradientValue(
            i + 1, (m_blurTimeKeyFrames.Values[i] - m_minBlurTime) / (m_maxBlurTime - m_minBlurTime));
    }

    return TRUE;
}

void EmitterLineGroupPropPageClass::UpdateLifetime(float lifetime)
{
    if (m_lifetime != lifetime) {
        float val = lifetime / m_lifetime;

        for (unsigned int i = 0; i < m_blurTimeKeyFrames.NumKeyFrames; i++) {
            m_blurTimeKeyFrames.KeyTimes[i] *= val;
        }

        m_instanceList->Set_Blur_Time_Keyframes(m_blurTimeKeyFrames);
        m_lifetime = lifetime;
    }
}

void EmitterLineGroupPropPageClass::Initialize()
{
    if (m_blurTimeKeyFrames.KeyTimes != nullptr) {
        delete[] m_blurTimeKeyFrames.KeyTimes;
        m_blurTimeKeyFrames.KeyTimes = nullptr;
    }

    if (m_blurTimeKeyFrames.Values != nullptr) {
        delete[] m_blurTimeKeyFrames.Values;
        m_blurTimeKeyFrames.Values = nullptr;
    }

    if (m_instanceList != nullptr) {
        m_lifetime = m_instanceList->Get_Lifetime();
        m_instanceList->Get_Blur_Time_Keyframes(m_blurTimeKeyFrames);
        int max = m_blurTimeKeyFrames.Start;

        if (max <= 1.0f) {
            max = 1.0f;
        }

        m_maxBlurTime = max;
        int min = m_blurTimeKeyFrames.Start;

        if (min >= 0.0f) {
            min = 0.0f;
        }

        m_minBlurTime = min;

        for (unsigned int i = 0; i < m_blurTimeKeyFrames.NumKeyFrames; i++) {
            if (m_blurTimeKeyFrames.Values[i] > m_maxBlurTime) {
                m_maxBlurTime = m_blurTimeKeyFrames.Values[i];
            }

            if (m_blurTimeKeyFrames.Values[i] < m_minBlurTime) {
                m_minBlurTime = m_blurTimeKeyFrames.Values[i];
            }
        }
    }
}

void EmitterLineGroupPropPageClass::UpdateBlurTime()
{
    float time = 0.0f;
    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;
    float value = m_blurTimeBar->GetGradientValue(0);
    m_blurTimeKeyFrames.Start = (m_maxBlurTime - m_minBlurTime) * value + m_minBlurTime;

    if (m_blurTimeKeyFrames.KeyTimes != nullptr) {
        delete[] m_blurTimeKeyFrames.KeyTimes;
        m_blurTimeKeyFrames.KeyTimes = nullptr;
    }

    if (m_blurTimeKeyFrames.Values != nullptr) {
        delete[] m_blurTimeKeyFrames.Values;
        m_blurTimeKeyFrames.Values = nullptr;
    }

    int count = m_blurTimeBar->GetKeyCount();
    m_blurTimeKeyFrames.NumKeyFrames = count - 1;

    if (count > 1) {
        m_blurTimeKeyFrames.KeyTimes = new float[count - 1];
        m_blurTimeKeyFrames.Values = new float[count - 1];
        int i = 1;

        do {
            m_blurTimeBar->GetColor(i, &time, &red, &green, &blue);
            m_blurTimeKeyFrames.KeyTimes[i - 1] = time * m_lifetime;
            m_blurTimeKeyFrames.Values[i - 1] =
                (m_maxBlurTime - m_minBlurTime) * m_blurTimeBar->GetGradientValue(i) + m_minBlurTime;
            i++;
        } while (i < count);
    }
}
