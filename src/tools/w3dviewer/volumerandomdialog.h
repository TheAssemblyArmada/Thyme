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
#pragma once
#include "w3dafx.h"

class Vector3Randomizer;

class VolumeRandomDialog : public CDialog
{
public:
    VolumeRandomDialog(Vector3Randomizer *randomizer, CWnd *pParentWnd);
    virtual ~VolumeRandomDialog() override {}
    virtual void DoDataExchange(CDataExchange *pDX) override;
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult) override;
    virtual BOOL OnInitDialog() override;
    virtual void OnOK() override;

    Vector3Randomizer *GetRandomizer() { return m_randomizer; }

private:
    DECLARE_MESSAGE_MAP();

    afx_msg void OnBox();
    afx_msg void OnCylinder();
    afx_msg void OnSphere();

    void UpdateDialog();

    CSpinButtonCtrl m_radiusSpin;
    CSpinButtonCtrl m_cylRadiusSpin;
    CSpinButtonCtrl m_cylHeightSpin;
    CSpinButtonCtrl m_boxZSpin;
    CSpinButtonCtrl m_boxYSpin;
    CSpinButtonCtrl m_boxXSpin;
    Vector3Randomizer *m_randomizer;
};
