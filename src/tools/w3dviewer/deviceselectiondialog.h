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
#pragma once
#include "w3dafx.h"

class CDeviceSelectionDialog : public CDialog
{
public:
    CDeviceSelectionDialog(BOOL skipDialog, CWnd *pParentWnd);
    virtual ~CDeviceSelectionDialog() override;
    virtual INT_PTR DoModal() override;

private:
    virtual void DoDataExchange(CDataExchange *pDX) override;
    virtual BOOL OnInitDialog() override;
    virtual void OnOK() override;

    void UpdateRenderDeviceDesc();

    DECLARE_MESSAGE_MAP()
    afx_msg void OnComboChanged();

    CComboBox m_deviceCombo;
    BOOL m_skipDialog;

public:
    int m_device;
    int m_bpp;
    CString m_deviceName;
};