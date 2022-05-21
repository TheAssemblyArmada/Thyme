/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View emitter particle prop page
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "emitterparticleproppage.h"
#include "emitterinstancelist.h"
#include "resource.h"
#include "utils.h"
#include "v3_rnd.h"
#include "volumerandomdialog.h"

IMPLEMENT_DYNCREATE(EmitterParticlePropPageClass, CPropertyPage)

// clang-format off
BEGIN_MESSAGE_MAP(EmitterParticlePropPageClass, CPropertyPage)
    ON_COMMAND(IDC_VOLUME, OnSpecify)
    ON_COMMAND(IDC_MAXCHECK, OnMaxParticles)
END_MESSAGE_MAP()
// clang-format on

EmitterParticlePropPageClass::EmitterParticlePropPageClass() :
    CPropertyPage(IDD_EMITTERPARTICLE),
    m_instanceList(nullptr),
    m_isValid(true),
    m_emissionRate(0.0f),
    m_burstSize(0),
    m_maxEmissions(0),
    m_creationVolume(nullptr)
{
    Initialize();
}

EmitterParticlePropPageClass::~EmitterParticlePropPageClass()
{
    if (m_creationVolume != nullptr) {
        delete m_creationVolume;
        m_creationVolume = nullptr;
    }
}

BOOL EmitterParticlePropPageClass::OnCommand(WPARAM wParam, LPARAM lParam)
{
    if (LOWORD(wParam) >= IDC_MAXPART && LOWORD(wParam) <= IDC_PARTICLES) {
        SetModified();
    }

    return CWnd::OnCommand(wParam, lParam);
}

BOOL EmitterParticlePropPageClass::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    NMUPDOWN *ud = reinterpret_cast<NMUPDOWN *>(lParam);

    if (ud != nullptr && ud->hdr.code == UDN_DELTAPOS) {
        UpdateEditCtrl(ud->hdr.hwndFrom, ud->iDelta);
    }

    return CWnd::OnNotify(wParam, lParam, pResult);
}

void EmitterParticlePropPageClass::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_PARTICLESSPIN, m_particlesSpin);
    DDX_Control(pDX, IDC_RATESPIN, m_rateSpin);
    DDX_Control(pDX, IDC_MAXPARTSPIN, m_maxPartSpin);
}

BOOL EmitterParticlePropPageClass::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_particlesSpin.SetRange(0, 10000);
    m_particlesSpin.SetPos(m_burstSize);
    InitializeSpinButton(&m_rateSpin, m_emissionRate, -10000.0f, 10000.0f);
    m_maxPartSpin.SetRange(0, 10000);
    m_maxPartSpin.SetPos(m_maxEmissions);
    SendDlgItemMessage(IDC_MAXCHECK, BM_SETCHECK, m_maxEmissions != 0, 0);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_MAXPART), m_maxEmissions != 0);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_MAXPARTSPIN), m_maxEmissions != 0);
    return TRUE;
}

BOOL EmitterParticlePropPageClass::OnApply()
{
    m_emissionRate = GetDlgItemFloat(m_hWnd, IDC_RATE);
    m_burstSize = GetDlgItemInt(IDC_PARTICLES);
    m_maxEmissions = 0;

    if (SendDlgItemMessage(IDC_MAXCHECK, BM_GETCHECK, 0, 0) != 0) {
        m_maxEmissions = GetDlgItemInt(IDC_MAXPART);
    }

    m_instanceList->Set_Emission_Rate(m_emissionRate);
    m_instanceList->Set_Burst_Size(m_burstSize);
    m_instanceList->Set_Max_Emissions(m_maxEmissions);
    m_instanceList->Set_Creation_Volume(m_creationVolume->Clone());
    return CPropertyPage::OnApply();
}

LRESULT EmitterParticlePropPageClass::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    return CWnd::WindowProc(message, wParam, lParam);
}

void EmitterParticlePropPageClass::OnSpecify()
{
    VolumeRandomDialog dlg(m_creationVolume, this);

    if (dlg.DoModal() == IDOK) {
        if (m_creationVolume != nullptr) {
            delete m_creationVolume;
            m_creationVolume = nullptr;
        }

        m_creationVolume = dlg.GetRandomizer();
        SetModified();
    }
}

void EmitterParticlePropPageClass::OnMaxParticles()
{
    BOOL max = SendDlgItemMessage(IDC_MAXCHECK, BM_GETCHECK, 0, 0);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_MAXPART), max);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_MAXPARTSPIN), max);
    SetModified();
}

void EmitterParticlePropPageClass::Initialize()
{
    if (m_creationVolume != nullptr) {
        delete m_creationVolume;
        m_creationVolume = nullptr;
    }

    if (m_instanceList != nullptr) {
        m_emissionRate = m_instanceList->Get_Emission_Rate();
        m_burstSize = m_instanceList->Get_Burst_Size();
        m_maxEmissions = m_instanceList->Get_Max_Emissions();
        m_creationVolume = m_instanceList->Get_Creation_Volume();
    }
}
