/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View volume random dialog
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "volumerandomdialog.h"
#include "resource.h"
#include "utils.h"
#include "v3_rnd.h"

// clang-format off
BEGIN_MESSAGE_MAP(VolumeRandomDialog, CDialog)
    ON_COMMAND(IDC_BOX, OnBox)
    ON_COMMAND(IDC_CYLINDER, OnCylinder)
    ON_COMMAND(IDC_SPHERE, OnSphere)
END_MESSAGE_MAP()
// clang-format on

VolumeRandomDialog::VolumeRandomDialog(Vector3Randomizer *randomizer, CWnd *pParentWnd) :
    CDialog(IDD_VOLUME, pParentWnd), m_randomizer(randomizer)
{
}

void VolumeRandomDialog::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_RADIUSSPIN, m_radiusSpin);
    DDX_Control(pDX, IDC_CYLRADIUSSPIN, m_cylRadiusSpin);
    DDX_Control(pDX, IDC_CYLHEIGHTSPIN, m_cylHeightSpin);
    DDX_Control(pDX, IDC_BOXZSPIN, m_boxZSpin);
    DDX_Control(pDX, IDC_BOXYSPIN, m_boxYSpin);
    DDX_Control(pDX, IDC_BOXXSPIN, m_boxXSpin);
}

BOOL VolumeRandomDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
    NMUPDOWN *ud = reinterpret_cast<NMUPDOWN *>(lParam);

    if (ud != nullptr && ud->hdr.code == UDN_DELTAPOS) {
        UpdateEditCtrl(ud->hdr.hwndFrom, ud->iDelta);
    }

    return CWnd::OnNotify(wParam, lParam, pResult);
}

BOOL VolumeRandomDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    Vector3 v(1.0f, 1.0f, 1.0f);
    float radius = 1.0f;
    bool hollow = false;
    float cylradius = 1.0f;
    float cylheight = 1.0f;
    int type = IDC_BOX;

    if (m_randomizer != nullptr) {
        switch (m_randomizer->Class_ID()) {
            case Vector3Randomizer::CLASSID_SOLIDBOX: {
                type = IDC_BOX;
                v = static_cast<Vector3SolidBoxRandomizer *>(m_randomizer)->Get_Extents();
                break;
            }
            case Vector3Randomizer::CLASSID_SOLIDSPHERE: {
                type = IDC_SPHERE;
                radius = static_cast<Vector3SolidSphereRandomizer *>(m_randomizer)->Get_Radius();
                hollow = false;
                break;
            }
            case Vector3Randomizer::CLASSID_HOLLOWSPHERE: {
                type = IDC_SPHERE;
                radius = static_cast<Vector3HollowSphereRandomizer *>(m_randomizer)->Get_Radius();
                hollow = true;
                break;
            }
            case Vector3Randomizer::CLASSID_SOLIDCYLINDER: {
                type = IDC_CYLINDER;
                cylradius = static_cast<Vector3SolidCylinderRandomizer *>(m_randomizer)->Get_Radius();
                cylheight = static_cast<Vector3SolidCylinderRandomizer *>(m_randomizer)->Get_Height();
                break;
            }
            default:
                break;
        }
    }

    InitializeSpinButton(&m_boxXSpin, v.X, -10000.0f, 10000.0f);
    InitializeSpinButton(&m_boxYSpin, v.Y, -10000.0f, 10000.0f);
    InitializeSpinButton(&m_boxZSpin, v.Z, -10000.0f, 10000.0f);
    InitializeSpinButton(&m_radiusSpin, radius, 0.0f, 10000.0f);
    SendDlgItemMessage(IDC_HOLLOW, BM_SETCHECK, hollow, 0);
    InitializeSpinButton(&m_cylRadiusSpin, cylradius, 0.0f, 10000.0f);
    InitializeSpinButton(&m_cylHeightSpin, cylheight, 0.0f, 10000.0f);
    SendDlgItemMessage(type, BM_SETCHECK, TRUE, 0);
    UpdateDialog();
    return TRUE;
}

void VolumeRandomDialog::OnOK()
{
    if (SendDlgItemMessage(IDC_BOX, BM_GETCHECK, 0, 0) == 1) {
        Vector3 v(0.0f, 0.0f, 0.0f);
        v.X = GetDlgItemFloat(m_hWnd, IDC_BOXX);
        v.Y = GetDlgItemFloat(m_hWnd, IDC_BOXY);
        v.Z = GetDlgItemFloat(m_hWnd, IDC_BOXZ);
        m_randomizer = new Vector3SolidBoxRandomizer(v);
    }

    if (SendDlgItemMessage(IDC_SPHERE, BM_GETCHECK, 0, 0) == 1) {
        float radius = GetDlgItemFloat(m_hWnd, IDC_RADIUS);

        if (SendDlgItemMessage(IDC_HOLLOW, BM_GETCHECK, 0, 0) == 1) {
            m_randomizer = new Vector3HollowSphereRandomizer(radius);
        } else {
            m_randomizer = new Vector3SolidSphereRandomizer(radius);
        }
    }

    if (SendDlgItemMessage(IDC_CYLINDER, BM_GETCHECK, 0, 0) == 1) {
        float cylradius = GetDlgItemFloat(m_hWnd, IDC_CYLRADIUS);
        float cylheight = GetDlgItemFloat(m_hWnd, IDC_CYLHEIGHT);
        m_randomizer = new Vector3SolidCylinderRandomizer(cylheight, cylradius);
    }

    CDialog::OnOK();
}

void VolumeRandomDialog::OnBox()
{
    UpdateDialog();
}

void VolumeRandomDialog::OnCylinder()
{
    UpdateDialog();
}

void VolumeRandomDialog::OnSphere()
{
    UpdateDialog();
}

void VolumeRandomDialog::UpdateDialog()
{
    bool box = SendDlgItemMessage(IDC_BOX, BM_GETCHECK, 0, 0) == 1;
    bool sphere = SendDlgItemMessage(IDC_SPHERE, BM_GETCHECK, 0, 0) == 1;
    bool cylinder = SendDlgItemMessage(IDC_CYLINDER, BM_GETCHECK, 0, 0) == 1;
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_BOXX), box);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_BOXY), box);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_BOXZ), box);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_BOXXSPIN), box);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_BOXYSPIN), box);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_BOXZSPIN), box);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_RADIUS), sphere);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_RADIUSSPIN), sphere);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_HOLLOW), sphere);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_CYLRADIUS), cylinder);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_CYLRADIUSSPIN), cylinder);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_CYLHEIGHT), cylinder);
    ::EnableWindow(::GetDlgItem(m_hWnd, IDC_CYLHEIGHTSPIN), cylinder);
}
