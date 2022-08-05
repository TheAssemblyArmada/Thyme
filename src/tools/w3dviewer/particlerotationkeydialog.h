/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View particle rotation key dialog
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

class ParticleRotationKeyDialog : public CDialog
{
public:
    ParticleRotationKeyDialog(float rotation, CWnd *pParentWnd);
    virtual ~ParticleRotationKeyDialog() override {}
    virtual void DoDataExchange(CDataExchange *pDX) override;
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult) override;
    virtual BOOL OnInitDialog() override;
    virtual void OnOK() override;

    float GetRotation() const { return m_rotation; }

private:
    DECLARE_MESSAGE_MAP();

    CSpinButtonCtrl m_rotationSpin;
    float m_rotation;
};
