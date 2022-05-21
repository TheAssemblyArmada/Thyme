/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View opacity settings dialog
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

class ColorBarClass;

class OpacitySettingsDialog : public CDialog
{
public:
    OpacitySettingsDialog(float opacity, CWnd *pParentWnd);
    virtual ~OpacitySettingsDialog() override {}
    virtual void DoDataExchange(CDataExchange *pDX) override {}
    virtual BOOL OnInitDialog() override;
    virtual void OnOK() override;

    float GetOpacity() const { return m_opacity; }

private:
    DECLARE_MESSAGE_MAP();

    ColorBarClass *m_opacityBar;
    float m_opacity;
};
