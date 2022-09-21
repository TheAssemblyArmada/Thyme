/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View Scale Dialog
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

class CScaleDialog : public CDialog
{
public:
    CScaleDialog(float scale, CWnd *pParentWnd, const char *label);
    virtual ~CScaleDialog() override {}
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT *pResult) override;
    virtual BOOL OnInitDialog() override;
    virtual void OnOK() override;

    float Get_Scale() const { return m_scale; }

private:
    DECLARE_MESSAGE_MAP();

    CSpinButtonCtrl m_scaleSpin;
    float m_scale;
    CString m_label;
};
