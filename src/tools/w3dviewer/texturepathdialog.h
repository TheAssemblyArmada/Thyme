/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief W3D View Texture Path Dialog
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

class TexturePathDialogClass : public CDialog
{
public:
    TexturePathDialogClass(CWnd *pParentWnd);
    virtual ~TexturePathDialogClass() override {}
    virtual BOOL OnInitDialog() override;
    virtual void OnOK() override;

private:
    DECLARE_MESSAGE_MAP();

    afx_msg void OnBrowse1();
    afx_msg void OnBrowse2();
};
