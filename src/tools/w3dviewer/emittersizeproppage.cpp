/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View emitter size prop page
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "emittersizeproppage.h"
#include "colorbar.h"
#include "emitterinstancelist.h"
#include "particlesizedialog.h"
#include "resource.h"
#include "utils.h"

IMPLEMENT_DYNCREATE(EmitterSizePropPageClass, CPropertyPage)

// clang-format off
BEGIN_MESSAGE_MAP(EmitterSizePropPageClass, CPropertyPage)
END_MESSAGE_MAP()
// clang-format on

EmitterSizePropPageClass::EmitterSizePropPageClass() :
    CPropertyPage(IDD_EMITTERSIZE),
    m_instanceList(nullptr),
    m_isValid(true),
    m_sizeBar(nullptr),
    m_lifetime(0.0f),
    m_scale(0.0f)
{
    memset(&m_oldSizeKeyFrames, 0, sizeof(m_oldSizeKeyFrames));
    memset(&m_sizeKeyFrames, 0, sizeof(m_sizeKeyFrames));
    Initialize();
}

EmitterSizePropPageClass::~EmitterSizePropPageClass()
{
    if (m_oldSizeKeyFrames.KeyTimes != nullptr) {
        delete[] m_oldSizeKeyFrames.KeyTimes;
    }

    if (m_oldSizeKeyFrames.Values != nullptr) {
        delete[] m_oldSizeKeyFrames.Values;
    }

    if (m_sizeKeyFrames.KeyTimes != nullptr) {
        delete[] m_sizeKeyFrames.KeyTimes;
    }

    if (m_sizeKeyFrames.Values != nullptr) {
        delete[] m_sizeKeyFrames.Values;
    }
}

BOOL EmitterSizePropPageClass::OnCommand(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam)) {
        case IDC_PSIZEEDIT:
            if (HIWORD(wParam) == EN_CHANGE) {
                SetModified();
            }

            if (HIWORD(wParam) == EN_KILLFOCUS && SendDlgItemMessage(LOWORD(wParam), EM_GETMODIFY, 0, 0) != 0) {
                SendDlgItemMessage(LOWORD(wParam), EM_SETMODIFY, 0, 0);
                m_sizeKeyFrames.Rand = GetDlgItemFloat(m_hWnd, IDC_PSIZEEDIT);
                m_instanceList->Set_Size_Keyframes(m_sizeKeyFrames);
                SetModified();
            }

            break;
    }

    return CWnd::OnCommand(wParam, lParam);
}

BOOL EmitterSizePropPageClass::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    NMHDR *hdr = reinterpret_cast<NMHDR *>(lParam);

    if (hdr != nullptr && hdr->code == UDN_DELTAPOS) {
        NMUPDOWN *nm = reinterpret_cast<NMUPDOWN *>(lParam);
        UpdateEditCtrl(hdr->hwndFrom, nm->iDelta);
    }

    switch (hdr->idFrom) {
        case IDC_SIZEBAR: {
            ColorBarNotify *cbn = reinterpret_cast<ColorBarNotify *>(lParam);

            if (hdr->code == CLBN_DBLCLK) {
                ParticleSizeDialog dlg(m_sizeBar->GetGradientValue(cbn->keyposition) * m_scale, this);

                if (dlg.DoModal() == IDOK) {
                    float size = dlg.GetSize();
                    m_sizeBar->EnableRedraw(false);
                    m_sizeBar->SetGradientValue(cbn->keyposition, size / m_scale);

                    for (int i = 0; i < m_sizeBar->GetKeyCount(); i++) {
                        int val = m_sizeBar->GetGradientValue(i) * m_scale;

                        if (val > size) {
                            size = val;
                        }
                    }

                    if (size != m_scale) {
                        for (int i = 0; i < m_sizeBar->GetKeyCount(); i++) {
                            m_sizeBar->SetGradientValue(i, size / m_scale);
                        }

                        m_scale = size;
                    }

                    m_sizeBar->EnableRedraw(true);
                    UpdateSize();
                    m_instanceList->Set_Size_Keyframes(m_sizeKeyFrames);
                    SetModified();
                }

            } else if (hdr->code == CLBN_KEYCHANGING || hdr->code == CLBN_DELETE) {
                UpdateSize();
                m_instanceList->Set_Size_Keyframes(m_sizeKeyFrames);
                SetModified();
            }

            break;
        }
        case IDC_PSIZESPIN: {
            NMUPDOWN *nm = reinterpret_cast<NMUPDOWN *>(lParam);

            if (hdr->code == UDN_DELTAPOS) {
                m_sizeKeyFrames.Rand = GetDlgItemFloat(m_hWnd, IDC_PSIZEEDIT);
                m_instanceList->Set_Size_Keyframes(m_sizeKeyFrames);
                SetModified();
            }

            break;
        }
    }

    return CPropertyPage::OnNotify(wParam, lParam, pResult);
}

void EmitterSizePropPageClass::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_PSIZESPIN, m_randomizerSpin);
}

BOOL EmitterSizePropPageClass::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_sizeBar = static_cast<ColorBarClass *>(GetProp(::GetDlgItem(m_hWnd, IDC_SIZEBAR), "CLASSPOINTER"));
    m_sizeBar->SetMinMax(0.0f, 1.0f);
    InitializeSpinButton(&m_randomizerSpin, m_oldSizeKeyFrames.Rand, 0.0f, 10000.0f);
    m_sizeBar->SetMinMax(0.0f, 1.0f);
    m_sizeBar->Clear();
    m_sizeBar->SetColor(0, 0.0f, 0.0f, 0.0f, 0.0f, 3);
    m_sizeBar->SetGradientValue(0, m_oldSizeKeyFrames.Start / m_scale);

    for (unsigned int i = 0; i < m_oldSizeKeyFrames.NumKeyFrames; i++) {
        m_sizeBar->SetColor(i + 1, m_oldSizeKeyFrames.KeyTimes[i] / m_lifetime, 0.0f, 0.0f, 0.0f, 0.0f);
        m_sizeBar->SetGradientValue(i + 1, m_oldSizeKeyFrames.Values[i] / m_scale);
    }

    return TRUE;
}

BOOL EmitterSizePropPageClass::OnApply()
{
    return CPropertyPage::OnApply();
}

void EmitterSizePropPageClass::UpdateLifetime(float lifetime)
{
    if (m_lifetime != lifetime) {
        float val = lifetime / m_lifetime;

        for (unsigned int i = 0; i < m_sizeKeyFrames.NumKeyFrames; i++) {
            m_sizeKeyFrames.KeyTimes[i] *= val;
        }

        m_instanceList->Set_Size_Keyframes(m_sizeKeyFrames);
        m_lifetime = lifetime;
    }
}

void EmitterSizePropPageClass::Initialize()
{
    if (m_oldSizeKeyFrames.KeyTimes != nullptr) {
        delete[] m_oldSizeKeyFrames.KeyTimes;
        m_oldSizeKeyFrames.KeyTimes = nullptr;
    }

    if (m_oldSizeKeyFrames.Values != nullptr) {
        delete[] m_oldSizeKeyFrames.Values;
        m_oldSizeKeyFrames.Values = nullptr;
    }

    if (m_sizeKeyFrames.KeyTimes != nullptr) {
        delete[] m_sizeKeyFrames.KeyTimes;
        m_sizeKeyFrames.KeyTimes = nullptr;
    }

    if (m_sizeKeyFrames.Values != nullptr) {
        delete[] m_sizeKeyFrames.Values;
        m_sizeKeyFrames.Values = nullptr;
    }

    if (m_instanceList != nullptr) {
        m_lifetime = m_instanceList->Get_Lifetime();
        m_instanceList->Get_Size_Keyframes(m_oldSizeKeyFrames);
        m_instanceList->Get_Size_Keyframes(m_sizeKeyFrames);

        m_scale = m_oldSizeKeyFrames.Start;

        for (unsigned int i = 0; i < m_oldSizeKeyFrames.NumKeyFrames; i++) {
            if (m_oldSizeKeyFrames.Values[i] > m_scale) {
                m_scale = m_oldSizeKeyFrames.Values[i];
            }
        }
    }
}

void EmitterSizePropPageClass::UpdateSize()
{
    float time = 0.0f;
    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;
    m_sizeKeyFrames.Start = m_sizeBar->GetGradientValue(0) * m_scale;

    if (m_sizeKeyFrames.KeyTimes != nullptr) {
        delete[] m_sizeKeyFrames.KeyTimes;
        m_sizeKeyFrames.KeyTimes = nullptr;
    }

    if (m_sizeKeyFrames.Values != nullptr) {
        delete[] m_sizeKeyFrames.Values;
        m_sizeKeyFrames.Values = nullptr;
    }

    int count = m_sizeBar->GetKeyCount();
    m_sizeKeyFrames.NumKeyFrames = count - 1;

    if (count > 1) {
        m_sizeKeyFrames.KeyTimes = new float[count - 1];
        m_sizeKeyFrames.Values = new float[count - 1];
        int i = 1;

        do {
            m_sizeBar->GetColor(i, &time, &red, &green, &blue);
            m_sizeKeyFrames.KeyTimes[i - 1] = time * m_lifetime;
            m_sizeKeyFrames.Values[i - 1] = m_sizeBar->GetGradientValue(i) * m_scale;
            i++;
        } while (i < count);
    }
}
