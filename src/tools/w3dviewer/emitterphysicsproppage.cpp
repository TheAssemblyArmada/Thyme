/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View emitter physics prop page
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "emitterphysicsproppage.h"
#include "emitterinstancelist.h"
#include "resource.h"
#include "utils.h"
#include "v3_rnd.h"
#include "volumerandomdialog.h"

IMPLEMENT_DYNCREATE(EmitterPhysicsPropPageClass, CPropertyPage)

// clang-format off
BEGIN_MESSAGE_MAP(EmitterPhysicsPropPageClass, CPropertyPage)
    ON_COMMAND(IDC_RANDOMIZER, OnRandomizer)
END_MESSAGE_MAP()
// clang-format on

EmitterPhysicsPropPageClass::EmitterPhysicsPropPageClass() :
    CPropertyPage(IDD_EMITTERPHYS),
    m_instanceList(nullptr),
    m_isValid(true),
    m_velocity(0.0f, 0.0f, 0.0f),
    m_acceleration(0.0f, 0.0f, 0.0f),
    m_outwardVel(0.0f),
    m_velInherit(0.0f),
    m_randomizer(nullptr)
{
    Initialize();
}

EmitterPhysicsPropPageClass::~EmitterPhysicsPropPageClass()
{
    if (m_randomizer != nullptr) {
        delete[] m_randomizer;
        m_randomizer = nullptr;
    }
}

BOOL EmitterPhysicsPropPageClass::OnCommand(WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(wParam)) {
        case IDC_XACCEL:
        case IDC_YACCEL:
        case IDC_ZACCEL:
        case IDC_XVEL:
        case IDC_YVEL:
        case IDC_ZVEL:
        case IDC_INHERIT:
        case IDC_OUTWARD:
            if (HIWORD(wParam) == EN_CHANGE) {
                SetModified();
            }

            if (HIWORD(wParam) == EN_KILLFOCUS && SendDlgItemMessage(LOWORD(wParam), EM_GETMODIFY, 0, 0) != 0) {
                SendDlgItemMessage(LOWORD(wParam), EM_SETMODIFY, 0, 0);
                OnUpdateEditCtrl(LOWORD(wParam));
            }

            break;
    }

    return CWnd::OnCommand(wParam, lParam);
}

BOOL EmitterPhysicsPropPageClass::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    NMUPDOWN *ud = reinterpret_cast<NMUPDOWN *>(lParam);

    if (ud != nullptr && ud->hdr.code == UDN_DELTAPOS) {
        OnUpdateEditCtrl(wParam);
    }

    return CWnd::OnNotify(wParam, lParam, pResult);
}

void EmitterPhysicsPropPageClass::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_OUTWARDSPIN, m_outwardSpin);
    DDX_Control(pDX, IDC_INHERITSPIN, m_inheritSpin);
    DDX_Control(pDX, IDC_ZVELSPIN, m_zVelSpin);
    DDX_Control(pDX, IDC_YVELSPIN, m_yVelSpin);
    DDX_Control(pDX, IDC_XVELSPIN, m_xVelSpin);
    DDX_Control(pDX, IDC_ZACCELSPIN, m_zAccelSpin);
    DDX_Control(pDX, IDC_YACCELSPIN, m_yAccelSpin);
    DDX_Control(pDX, IDC_XACCELSPIN, m_xAccelSpin);
}

BOOL EmitterPhysicsPropPageClass::OnInitDialog()
{
    CDialog::OnInitDialog();
    InitializeSpinButton(&m_outwardSpin, m_outwardVel, -10000.0f, 10000.0f);
    InitializeSpinButton(&m_inheritSpin, m_velInherit, -10000.0f, 10000.0f);
    InitializeSpinButton(&m_xVelSpin, m_velocity.X, -10000.0f, 10000.0f);
    InitializeSpinButton(&m_yVelSpin, m_velocity.Y, -10000.0f, 10000.0f);
    InitializeSpinButton(&m_zVelSpin, m_velocity.Z, -10000.0f, 10000.0f);
    InitializeSpinButton(&m_xAccelSpin, m_acceleration.X, -10000.0f, 10000.0f);
    InitializeSpinButton(&m_yAccelSpin, m_acceleration.Y, -10000.0f, 10000.0f);
    InitializeSpinButton(&m_zAccelSpin, m_acceleration.Z, -10000.0f, 10000.0f);
    return TRUE;
}

BOOL EmitterPhysicsPropPageClass::OnApply()
{
    m_velocity.X = GetDlgItemFloat(m_hWnd, IDC_XVEL);
    m_velocity.Y = GetDlgItemFloat(m_hWnd, IDC_YVEL);
    m_velocity.Z = GetDlgItemFloat(m_hWnd, IDC_ZVEL);
    m_outwardVel = GetDlgItemFloat(m_hWnd, IDC_OUTWARDSPIN);
    m_velInherit = GetDlgItemFloat(m_hWnd, IDC_INHERITSPIN);
    m_acceleration.X = GetDlgItemFloat(m_hWnd, IDC_XACCEL);
    m_acceleration.Y = GetDlgItemFloat(m_hWnd, IDC_YACCEL);
    m_acceleration.Z = GetDlgItemFloat(m_hWnd, IDC_ZACCEL);
    return CPropertyPage::OnApply();
}

void EmitterPhysicsPropPageClass::OnRandomizer()
{
    VolumeRandomDialog dlg(m_randomizer, this);

    if (dlg.DoModal() == IDOK) {
        if (m_randomizer != nullptr) {
            delete[] m_randomizer;
            m_randomizer = nullptr;
        }

        m_randomizer = dlg.GetRandomizer();

        if (m_randomizer != nullptr) {
            m_instanceList->Set_Velocity_Random(m_randomizer->Clone());
            SetModified();
        }
    }
}

void EmitterPhysicsPropPageClass::Initialize()
{
    if (m_randomizer != nullptr) {
        delete[] m_randomizer;
        m_randomizer = nullptr;
    }

    if (m_instanceList != nullptr) {
        m_velocity = m_instanceList->Get_Velocity();
        m_acceleration = m_instanceList->Get_Acceleration();
        m_outwardVel = m_instanceList->Get_Outward_Vel();
        m_velInherit = m_instanceList->Get_Vel_Inherit();
        m_randomizer = m_instanceList->Get_Velocity_Random();
    }
}

void EmitterPhysicsPropPageClass::OnUpdateEditCtrl(int id)
{
    switch (id) {
        case IDC_XACCEL:
        case IDC_XACCELSPIN:
        case IDC_YACCEL:
        case IDC_YACCELSPIN:
        case IDC_ZACCEL:
        case IDC_ZACCELSPIN: {
            Vector3 v;
            v.X = GetDlgItemFloat(m_hWnd, IDC_XACCEL);
            v.Y = GetDlgItemFloat(m_hWnd, IDC_YACCEL);
            v.Z = GetDlgItemFloat(m_hWnd, IDC_ZACCEL);
            m_instanceList->Set_Acceleration(v);
            SetModified();
            break;
        }
        case IDC_XVEL:
        case IDC_XVELSPIN:
        case IDC_YVEL:
        case IDC_YVELSPIN:
        case IDC_ZVEL:
        case IDC_ZVELSPIN: {
            Vector3 v;
            v.X = GetDlgItemFloat(m_hWnd, IDC_XVEL);
            v.Y = GetDlgItemFloat(m_hWnd, IDC_YVEL);
            v.Z = GetDlgItemFloat(m_hWnd, IDC_ZVEL);
            m_instanceList->Set_Velocity(v);
            SetModified();
            break;
        }
        case IDC_OUTWARDSPIN:
        case IDC_OUTWARD:
            m_instanceList->Set_Outward_Vel(GetDlgItemFloat(m_hWnd, IDC_OUTWARD));
            SetModified();
            break;
        case IDC_INHERITSPIN:
        case IDC_INHERIT:
            m_instanceList->Set_Vel_Inherit(GetDlgItemFloat(m_hWnd, IDC_INHERIT));
            SetModified();
            break;
    }
}
