/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View device selection dialog
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "deviceselectiondialog.h"
#include "rddesc.h"
#include "resource.h"
#include "w3d.h"
#include "w3dview.h"

// clang-format off
BEGIN_MESSAGE_MAP(CDeviceSelectionDialog, CDialog)
    ON_COMMAND(IDC_DEVICECOMBO, OnComboChanged)
END_MESSAGE_MAP()
// clang-format on

CDeviceSelectionDialog::CDeviceSelectionDialog(BOOL skipDialog, CWnd *pParentWnd) :
    CDialog(IDD_DEVICESELECTION, pParentWnd), m_skipDialog(skipDialog), m_device(1), m_bpp(16)
{
}

CDeviceSelectionDialog::~CDeviceSelectionDialog() {}

INT_PTR CDeviceSelectionDialog::DoModal()
{
    int device = 0;
    BOOL found = FALSE;
    m_deviceName = theApp.GetProfileString("Config", "DeviceName");

    if (!m_skipDialog) {
        return CDialog::DoModal();
    }

    if (m_deviceName.GetLength() <= 0) {
        return CDialog::DoModal();
    }

    if ((GetKeyState(VK_SHIFT) & 0xF000) != 0) {
        return CDialog::DoModal();
    }

    int count = W3D::Get_Render_Device_Count();

    if (count <= 0) {
        return CDialog::DoModal();
    }

    while (!found) {
        const char *name = W3D::Get_Render_Device_Name(device);

        if (m_deviceName == name) {
            m_device = device;
            m_bpp = theApp.GetProfileInt("Config", "DeviceBitsPerPix", 16);
            found = TRUE;
        }

        if (++device >= count) {
            if (!found) {
                return CDialog::DoModal();
            }

            return TRUE;
        }
    }

    return TRUE;
}

void CDeviceSelectionDialog::DoDataExchange(CDataExchange *pDX)
{
    DDX_Control(pDX, IDC_DEVICECOMBO, m_deviceCombo);
}

BOOL CDeviceSelectionDialog::OnInitDialog()
{
    CDialog::OnInitDialog();
    int count = W3D::Get_Render_Device_Count();
    int selection = 0;

    for (int i = 0; i < count; i++) {
        const char *name = W3D::Get_Render_Device_Name(i);
        int index = m_deviceCombo.InsertString(i, name);

        if (m_deviceName == name) {
            selection = index;
        }

        m_deviceCombo.SetItemData(index, i);
        SendDlgItemMessage(IDC_16BIT, BM_SETCHECK, 1);
        m_deviceCombo.SetCurSel(selection);
        UpdateRenderDeviceDesc();
    }

    return TRUE;
}

void CDeviceSelectionDialog::OnOK()
{
    m_device = m_deviceCombo.GetItemData(m_deviceCombo.GetCurSel());
    m_bpp = SendDlgItemMessage(IDC_16BIT, BM_GETCHECK) != 1 ? 24 : 16;
    CString name;
    m_deviceCombo.GetLBText(m_deviceCombo.GetCurSel(), name);
    m_deviceName = name;
    theApp.WriteProfileString("Config", "DeviceName", name);
    theApp.WriteProfileInt("Config", "DeviceBitsPerPix", m_bpp);
    CDialog::OnOK();
}

void CDeviceSelectionDialog::UpdateRenderDeviceDesc()
{
    const RenderDeviceDescClass &desc = W3D::Get_Render_Device_Desc(-1);
    CWnd::SetDlgItemTextA(IDC_DRIVERNAME, m_deviceName);
    CWnd::SetDlgItemTextA(IDC_DEVICENAME, desc.Get_Device_Name());
    CWnd::SetDlgItemTextA(IDC_DEVICEVENDOR, desc.Get_Device_Vendor());
    CWnd::SetDlgItemTextA(IDC_DEVICEPLATFORM, desc.Get_Device_Platform());
    CWnd::SetDlgItemTextA(IDC_DRIVERFILE, desc.Get_Driver_Name());
    CWnd::SetDlgItemTextA(IDC_DRIVERVENDOR, desc.Get_Driver_Vendor());
    CWnd::SetDlgItemTextA(IDC_DRIVERVERSION, desc.Get_Driver_Version());
    CWnd::SetDlgItemTextA(IDC_HARDWARENAME, desc.Get_Hardware_Name());
    CWnd::SetDlgItemTextA(IDC_HARDWAREVENDOR, desc.Get_Hardware_Vendor());
    CWnd::SetDlgItemTextA(IDC_HARDWARECHIPSET, desc.Get_Hardware_Chipset());
}

void CDeviceSelectionDialog::OnComboChanged()
{
    if (m_deviceCombo.GetCurSel() != -1) {
        UpdateRenderDeviceDesc();
    }
}
