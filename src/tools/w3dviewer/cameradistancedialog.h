/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View camera distance dialog
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

class CameraDistanceDialogClass : public CDialog
{
public:
    CameraDistanceDialogClass(CWnd *pParentWnd);
    virtual ~CameraDistanceDialogClass() override {}
    virtual void DoDataExchange(CDataExchange *pDX) override;
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult) override;
    virtual BOOL OnInitDialog() override;
    virtual void OnOK() override;

private:
    DECLARE_MESSAGE_MAP();

    CSpinButtonCtrl m_spin;
};
