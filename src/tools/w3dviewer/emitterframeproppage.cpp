/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View emitter frame prop page
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "emitterframeproppage.h"
#include "colorbar.h"
#include "emitterinstancelist.h"
#include "particleframekeydialog.h"
#include "resource.h"
#include "utils.h"

IMPLEMENT_DYNCREATE(EmitterFramePropPageClass, CPropertyPage)

// clang-format off
BEGIN_MESSAGE_MAP(EmitterFramePropPageClass, CPropertyPage)
END_MESSAGE_MAP()
// clang-format on

EmitterFramePropPageClass::EmitterFramePropPageClass() :
    CPropertyPage(IDD_EMITTERFRAME),
    m_instanceList(nullptr),
    m_isValid(true),
    m_frameBar(nullptr),
    m_lifetime(0.0f),
    m_minFrame(0.0f),
    m_maxFrame(1.0f)
{
    memset(&m_frameKeyFrames, 0, sizeof(m_frameKeyFrames));
    Initialize();
}

EmitterFramePropPageClass::~EmitterFramePropPageClass()
{
    if (m_frameKeyFrames.KeyTimes != nullptr) {
        delete[] m_frameKeyFrames.KeyTimes;
    }

    if (m_frameKeyFrames.Values != nullptr) {
        delete[] m_frameKeyFrames.Values;
    }
}

BOOL EmitterFramePropPageClass::OnCommand(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam)) {
        case IDC_TEXTUREGRID:
            m_instanceList->Set_Frame_Mode(m_textureGrid.GetCurSel());
            break;
        case IDC_PFRAME:
            if (HIWORD(wParam) == EN_CHANGE) {
                SetModified();
            }

            if (HIWORD(wParam) == EN_KILLFOCUS && SendDlgItemMessage(LOWORD(wParam), EM_GETMODIFY, 0, 0) != 0) {
                SendDlgItemMessage(LOWORD(wParam), EM_SETMODIFY, 0, 0);
                m_frameKeyFrames.Rand = GetDlgItemInt(IDC_PFRAME) / 255.0f;
                m_instanceList->Set_Frame_Keyframes(m_frameKeyFrames);
                SetModified();
            }

            break;
    }

    return CWnd::OnCommand(wParam, lParam);
}

BOOL EmitterFramePropPageClass::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    NMHDR *hdr = reinterpret_cast<NMHDR *>(lParam);

    if (hdr != nullptr && hdr->code == UDN_DELTAPOS) {
        NMUPDOWN *nm = reinterpret_cast<NMUPDOWN *>(lParam);
        UpdateEditCtrl(hdr->hwndFrom, nm->iDelta);
    }

    switch (hdr->idFrom) {
        case IDC_FRAMEBAR: {
            ColorBarNotify *cbn = reinterpret_cast<ColorBarNotify *>(lParam);

            if (hdr->code == CLBN_DBLCLK) {
                float frame = (m_maxFrame - m_minFrame) * m_frameBar->GetGradientValue(cbn->keyposition) + m_minFrame;
                ParticleFrameKeyDialog dlg(frame, this);

                if (dlg.DoModal() == IDOK) {
                    float min = dlg.GetFrame();
                    float max;
                    m_frameBar->EnableRedraw(false);
                    m_frameBar->SetGradientValue(
                        cbn->keyposition, (dlg.GetFrame() - m_minFrame) / (m_maxFrame - m_minFrame));

                    if (min <= 1.0f) {
                        max = 1.0f;
                    } else {
                        max = min;
                    }

                    if (min >= 0.0f) {
                        min = 0.0f;
                    }

                    for (int i = 0; i < m_frameBar->GetKeyCount(); i++) {
                        int val = m_frameBar->GetGradientValue(i) * (m_maxFrame - m_minFrame);

                        if (val > max) {
                            max = val;
                        }

                        if (val < min) {
                            min = val;
                        }
                    }

                    if (max != m_maxFrame || min != m_minFrame) {
                        for (int i = 0; i < m_frameBar->GetKeyCount(); i++) {
                            m_frameBar->SetGradientValue(i,
                                (m_frameBar->GetGradientValue(i) * (m_maxFrame - m_minFrame) + m_minFrame - min)
                                    / (max - min));
                        }

                        m_minFrame = min;
                        m_maxFrame = max;
                    }

                    m_frameBar->EnableRedraw(true);
                    UpdateFrame();
                    m_instanceList->Set_Frame_Keyframes(m_frameKeyFrames);
                    SetModified();
                }

            } else if (hdr->code == CLBN_KEYCHANGING || hdr->code == CLBN_DELETE) {
                UpdateFrame();
                m_instanceList->Set_Frame_Keyframes(m_frameKeyFrames);
                SetModified();
            }

            break;
        }
        case IDC_PFRAMESPIN: {
            NMUPDOWN *nm = reinterpret_cast<NMUPDOWN *>(lParam);

            if (hdr->code == UDN_DELTAPOS) {
                m_frameKeyFrames.Rand = GetDlgItemFloat(m_hWnd, IDC_PFRAME);
                m_instanceList->Set_Frame_Keyframes(m_frameKeyFrames);
                SetModified();
            }

            break;
        }
    }

    return CPropertyPage::OnNotify(wParam, lParam, pResult);
}

void EmitterFramePropPageClass::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_TEXTUREGRID, m_textureGrid);
    DDX_Control(pDX, IDC_PFRAMESPIN, m_frameSpin);
}

BOOL EmitterFramePropPageClass::OnInitDialog()
{
    CDialog::OnInitDialog();

    switch (m_instanceList->Get_Frame_Mode()) {
        case W3D_EMITTER_FRAME_MODE_1x1:
            m_textureGrid.SetCurSel(0);
            break;
        case W3D_EMITTER_FRAME_MODE_2x2:
            m_textureGrid.SetCurSel(1);
            break;
        case W3D_EMITTER_FRAME_MODE_4x4:
            m_textureGrid.SetCurSel(2);
            break;
        case W3D_EMITTER_FRAME_MODE_8x8:
            m_textureGrid.SetCurSel(3);
            break;
        case W3D_EMITTER_FRAME_MODE_16x16:
            m_textureGrid.SetCurSel(4);
            break;
        default:
            m_textureGrid.SetCurSel(4);
            break;
    }

    m_frameBar = static_cast<ColorBarClass *>(GetProp(::GetDlgItem(m_hWnd, IDC_FRAMEBAR), "CLASSPOINTER"));
    InitializeSpinButton(&m_frameSpin, m_frameKeyFrames.Rand, 0.0f, 10000.0f);
    m_frameBar->SetMinMax(0.0f, 1.0f);
    m_frameBar->Clear();
    m_frameBar->SetColor(0, 0.0f, 0.0f, 0.0f, 0.0f, 3);
    m_frameBar->SetGradientValue(0, (m_frameKeyFrames.Start - m_minFrame) / (m_maxFrame - m_minFrame));

    for (unsigned int i = 0; i < m_frameKeyFrames.NumKeyFrames; i++) {
        m_frameBar->SetColor(i + 1, m_frameKeyFrames.KeyTimes[i] / m_lifetime, 0.0f, 0.0f, 0.0f, 3);
        m_frameBar->SetGradientValue(i + 1, (m_frameKeyFrames.Values[i] - m_minFrame) / (m_maxFrame - m_minFrame));
    }

    return TRUE;
}

BOOL EmitterFramePropPageClass::OnApply()
{
    return CPropertyPage::OnApply();
}

void EmitterFramePropPageClass::UpdateLifetime(float lifetime)
{
    if (m_lifetime != lifetime) {
        float val = lifetime / m_lifetime;

        for (unsigned int i = 0; i < m_frameKeyFrames.NumKeyFrames; i++) {
            m_frameKeyFrames.KeyTimes[i] *= val;
        }

        m_instanceList->Set_Frame_Keyframes(m_frameKeyFrames);
        m_lifetime = lifetime;
    }
}

void EmitterFramePropPageClass::Initialize()
{
    if (m_frameKeyFrames.KeyTimes != nullptr) {
        delete[] m_frameKeyFrames.KeyTimes;
    }

    if (m_frameKeyFrames.Values != nullptr) {
        delete[] m_frameKeyFrames.Values;
    }

    if (m_instanceList != nullptr) {
        m_lifetime = m_instanceList->Get_Lifetime();
        m_instanceList->Get_Frame_Keyframes(m_frameKeyFrames);
        int max = m_frameKeyFrames.Start;

        if (max <= 1.0f) {
            max = 1.0f;
        }

        m_maxFrame = max;
        int min = m_frameKeyFrames.Start;

        if (min >= 0.0f) {
            min = 0.0f;
        }

        m_minFrame = min;

        for (unsigned int i = 0; i < m_frameKeyFrames.NumKeyFrames; i++) {
            if (m_frameKeyFrames.Values[i] > m_maxFrame) {
                m_maxFrame = m_frameKeyFrames.Values[i];
            }

            if (m_frameKeyFrames.Values[i] < m_minFrame) {
                m_minFrame = m_frameKeyFrames.Values[i];
            }
        }
    }
}

void EmitterFramePropPageClass::UpdateFrame()
{
    float time = 0.0f;
    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;
    float value = m_frameBar->GetGradientValue(0);
    m_frameKeyFrames.Start = (m_maxFrame - m_minFrame) * value + m_minFrame;

    if (m_frameKeyFrames.KeyTimes != nullptr) {
        delete[] m_frameKeyFrames.KeyTimes;
    }

    if (m_frameKeyFrames.Values != nullptr) {
        delete[] m_frameKeyFrames.Values;
    }

    int count = m_frameBar->GetKeyCount();
    m_frameKeyFrames.NumKeyFrames = count - 1;

    if (count > 1) {
        m_frameKeyFrames.KeyTimes = new float[count - 1];
        m_frameKeyFrames.Values = new float[count - 1];
        int i = 1;

        do {
            m_frameBar->GetColor(i, &time, &red, &green, &blue);
            m_frameKeyFrames.KeyTimes[i - 1] = time * m_lifetime;
            m_frameKeyFrames.Values[i - 1] = (m_maxFrame - m_minFrame) * m_frameBar->GetGradientValue(i) + m_minFrame;
            i++;
        } while (i < count);
    }
}
